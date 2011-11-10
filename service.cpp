#include <fstream>

#include <syslog.h>
// mkdir, move to filesystem.hpp
#include <sys/stat.h>

#include <PDL.h>
#include <SDL.h>

// For md5
extern "C"
{
#include <fitz.h>
}

#include <boost/format.hpp>
#include <boost/thread.hpp>

#include "util/md5_to_string.hpp"
#include "util/filesystem.hpp"
#include "pdf_document.hpp"
#include "pixmap_renderer.hpp"

using namespace lector;

typedef boost::mutex mutex_type;
typedef mutex_type::scoped_try_lock scoped_try_lock;
typedef mutex_type::scoped_lock scoped_lock;
typedef boost::shared_lock<mutex_type> shared_lock;

// Globals :/
boost::thread_group render_threads;
boost::mutex mutex;
pdf_document* document = 0;

pixmap_renderer renderer;

// <path><prefix><page>-<zoom><suffix>
const boost::format filename_format ("%1$s%2$s%5$04d-%3$03d%4$s");
const boost::format error ("{\"error\":\"%s\"");

namespace service
{

PDL_bool do_shell(PDL_JSParameters* params)
{
    // STUB!
    return PDL_TRUE;
}

const boost::format open_response
    ("{\"pages\":%d,\"width\":%d,\"height\":%d,\"units\":\"pt\",\"dst\":\"/media/internal/appdata/com.quickoffice.ar/.cache/%s\"}");

PDL_bool do_open(PDL_JSParameters* params)
{
    std::string r = "";

    try
    {
        std::string filename = PDL_GetJSParamString(params, 1);

        scoped_lock lock(mutex);
        document = new pdf_document(filename);

        // Generate Unique ID as an md5 of the first kilobyte of the file
        std::ifstream file (filename.c_str());
        
        fz_md5 md5;
        fz_md5_init(&md5);
        unsigned char buffer[1024];
        file.read((char*)buffer, 1024);
        fz_md5_update(&md5, buffer, file.gcount());
        file.close();

        // Reuse buffer
        fz_md5_final(&md5, buffer);

        pdf_page_ptr page = document->get_page(0);

        // Generate hex output
        std::string digest = md5::md5_to_string(buffer + 0, buffer + 16);

        r = (boost::format(open_response) % document->pages()
                                          % page->width()
                                          % page->height()
                                          % digest).str();
    }
    catch (std::exception const& exc)
    {
        r = (boost::format(error) % exc.what()).str();
    }

    const char* ptr = r.c_str();

    PDL_CallJS("OpenCallback", &ptr, 1);
    return PDL_TRUE;
}

PDL_bool do_cover(PDL_JSParameters* params)
{
    // STUB!
    return PDL_TRUE;
}

const boost::format toc_response ("{\"toc\":[%s]}");
const boost::format toc_element ("{\"p\":%d,\"l\":%d,\"c\":\"%s\"},");
// p: Page
// l: Level
// c: Content (aka Name)
namespace
{
    void print_outline(std::string& str, pdf_outline* out, unsigned level)
    {
        while (out)
        {
            if (out->link->kind == PDF_LINK_GOTO)
            {
                const std::size_t page = document->get_page_number(out->link);

                str += (boost::format(toc_element) % page
                                                   % level
                                                   % out->title).str();
            }

            if (out->child)
                print_outline(str, out->child, level + 1);            
            out = out->next;
        }
    }
}

PDL_bool do_toc(PDL_JSParameters* params)
{
    std::string out;

    scoped_try_lock lock (mutex);

    if (!lock.owns_lock())
    {
        PDL_JSException(params, "worker thread busy");
        return PDL_FALSE;
    }

    // TODO: C++ize
    pdf_outline* outline = document->get_outline();
    print_outline(out, outline, 0);
    // pdf_free_outline(outline);

    std::string const& result = (boost::format(toc_response) % out).str();
    const char* ptr = result.c_str();
    PDL_CallJS("TocCallback", &ptr, 1);

    return PDL_TRUE;
}

const boost::format render_response
    ("{\"from\":%d,\"image\":\"%s\"}");

namespace
{
    void render_png(float zoom, pdf_page_ptr page, std::string const& filename,
                    int i)
    {
        {
            scoped_lock lock(mutex);
            renderer.render_full(zoom, page).write_png(filename);
        }

        std::string response_json =
            (boost::format(render_response) % i % filename).str();

        const char* response = response_json.c_str();

        PDL_CallJS("RenderCallback", &response, 1);
        syslog(LOG_INFO, "Done rendering page %d", i);
    }

    void render_loop(float zoom, int from, int count, std::string directory,
                     boost::format format)
    {
        int err = ::mkdir(directory.c_str(), 0755);
        if (err != 0 && errno != EEXIST)
            throw std::runtime_error("could not create directory");

        for (int i = from; i < from + count; ++i)
        {
            std::string filename = (boost::format(format) % i).str();

            if (::access(filename.c_str(), R_OK) == -1 && errno == ENOENT)
            {
                syslog(LOG_INFO, "Starting rendering of page %d", i);
                pdf_page_ptr page;

                {
                    scoped_lock lock(mutex);
                    page = document->get_page(i);
                }

                // Don't need a seperate thread for small zoomlevels
                render_png(zoom / 100., page, filename, i);
                /*if (zoom < 200.)
                    render_png(zoom / 100., page, filename, i);
                else
                    render_threads.add_thread(
                        new boost::thread(render_png, zoom / 100., page, filename, i)
                        );*/

                {
                    scoped_lock lock(mutex);
                    page.reset();
                }
            }
            else
            {
                syslog(LOG_INFO, "Reusing cached image of page %d", i);
                std::string response_json =
                    (boost::format(render_response) % i % filename).str();

                const char* response = response_json.c_str();
                PDL_CallJS("RenderCallback", &response, 1);
            }
        }
        // Age store afterwards. This should be done in a nicer or more rigorous
        // way …
        scoped_lock lock(mutex);

        int memory_before = fz_get_memory_used();
        document->age_store(3);
        int memory_after = fz_get_memory_used();
        syslog(LOG_INFO, "Aged storage: %d vs. %d", memory_before >> 20
                                                  , memory_after >> 20);
    }
}

PDL_bool do_render(PDL_JSParameters* params)
{
    boost::format my_formatter(filename_format);

    int from = PDL_GetJSParamInt(params, 1);
    int count = PDL_GetJSParamInt(params, 2);
    int zoom = PDL_GetJSParamInt(params, 3);
    std::string directory = PDL_GetJSParamString(params, 4);
    std::string prefix = PDL_GetJSParamString(params, 5);
    std::string suffix = PDL_GetJSParamString(params, 6);

    my_formatter % directory % prefix % zoom % suffix;

    if (!document)
        throw std::runtime_error("Document has not been opened yet");

    render_threads.add_thread(
            new boost::thread(render_loop, zoom, from, count, directory,
                              my_formatter)
    );

    return PDL_TRUE;
}

PDL_bool do_delete(PDL_JSParameters* params)
{
    // STUB!
    return PDL_TRUE;
}

PDL_bool do_find(PDL_JSParameters* params)
{
    // STUB!
    return PDL_TRUE;
}

const boost::format saveas_response("{\"result\":%d}");
PDL_bool do_saveas(PDL_JSParameters* params)
{
    std::string src = PDL_GetJSParamString(params, 2);
    std::string dst = PDL_GetJSParamString(params, 3);
    bool overwrite = PDL_GetJSParamInt(params, 4);

    int result = filesystem::copy_file(src, "/media/internal/" + dst, overwrite);
    std::string response = (boost::format(saveas_response) % result).str();
    
    const char* ptr = response.c_str();
    PDL_CallJS("SaveAsCallback", &ptr, 1);
    return PDL_TRUE;
}
}

PDL_bool handler(PDL_JSParameters* params)
{
    PDL_bool return_value = PDL_TRUE;
    int argc = PDL_GetNumJSParams(params);

    std::string type(PDL_GetJSParamString(params, 0));
    syslog(LOG_INFO, "Handler called with mode %s", type.c_str());

#define IF_TYPE(name, n)                                        \
    if (type == #name)                                          \
        if (argc == n+1)                                        \
            return_value = service::do_##name(params);          \
        else                                                    \
        {                                                       \
            PDL_JSException(params,                             \
                    ("Invalid parameter count for function "    \
                     + type).c_str());                          \
            return_value = PDL_FALSE;                           \
        }

#define ELSE_TYPE(name, n)                                      \
    else IF_TYPE(name, n)

    try
    {
        IF_TYPE(shell, 1)
        ELSE_TYPE(open, 1)
        ELSE_TYPE(cover, 5)
        ELSE_TYPE(toc, 0)
        ELSE_TYPE(render, 6)
        ELSE_TYPE(delete, 1)
        ELSE_TYPE(find, 8)
        ELSE_TYPE(saveas, 3)
        else
        {
            PDL_JSException(params, ("Handler has no method " + type).c_str());
            return_value = PDL_FALSE;
        }
    }
    catch (...)
    {
        return_value = PDL_FALSE;
    }

    if (int count = fz_get_error_count())
        for (int i = 0; i < count; ++i)
            syslog(LOG_INFO, "Fitz error: %s", fz_get_error_line(i));

    return return_value;
}

const char* version_information = "{\"version\":\"mupdf 0.9\"}";

int main()
{
    syslog(LOG_INFO, "Starting up");
    SDL_Init(SDL_INIT_VIDEO);
    PDL_Init(0);

    fz_accelerate();
    fz_set_aa_level(8);

    PDL_RegisterJSHandler("Handler", &handler);
    PDL_JSRegistrationComplete();

    PDL_CallJS("ready", 0, 0);
    PDL_CallJS("VersionCallback", &version_information, 1);

    SDL_Event event;
    do
    {
        SDL_WaitEvent(&event);
    }
    while (event.type != SDL_QUIT);

    render_threads.join_all();

    scoped_lock lock (mutex);
    if (document)
        delete document;

    PDL_Quit();
    SDL_Quit();
}
