#include <fstream>

#include <pthread.h>
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

#include "md5_to_string.hpp"
#include "filesystem.hpp"
#include "pdf_document.hpp"
#include "png_renderer.hpp"

pthread_mutex_t mutex;
viewer::pdf_document* document = 0;

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
    syslog(LOG_INFO, "Open Called");
    pthread_mutex_lock(&mutex);
    try
    {
        std::string filename = PDL_GetJSParamString(params, 1);
        document = new viewer::pdf_document(filename);

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

        // Generate hex output
        std::string digest = md5::md5_to_string(buffer + 0, buffer + 16);

        r = (boost::format(open_response) % document->pages()
                                          % (*document)[0].width()
                                          % (*document)[0].height()
                                          % digest).str();
    }
    catch (std::exception const& exc)
    {
        r = (boost::format(error) % exc.what()).str();
    }
    catch (...)
    {
        r = (boost::format(error) % "Unknown error").str();
    }
    pthread_mutex_unlock(&mutex);

    const char* ptr = r.c_str();

    syslog(LOG_INFO, "Open Finished");

    PDL_CallJS("OpenCallback", &ptr, 1);
    return PDL_TRUE;
}

PDL_bool do_cover(PDL_JSParameters* params)
{
    // STUB!
    return PDL_TRUE;
}

PDL_bool do_toc(PDL_JSParameters* params)
{
    // STUB!
    return PDL_TRUE;
}

const boost::format render_response
    ("{\"from\":%d,\"image\":\"%s\"}");

PDL_bool do_render(PDL_JSParameters* params)
{
    PDL_bool return_value = PDL_TRUE;

    syslog(LOG_INFO, "Render called");

    viewer::png_renderer renderer;

    boost::format my_formatter(filename_format);

    int from = PDL_GetJSParamInt(params, 1);
    int count = PDL_GetJSParamInt(params, 2);
    int zoom = PDL_GetJSParamInt(params, 3);
    std::string directory =  PDL_GetJSParamString(params, 4);
    syslog(LOG_INFO, directory.c_str());
    std::string prefix = PDL_GetJSParamString(params, 5);
    std::string suffix = PDL_GetJSParamString(params, 6);

    my_formatter % directory % prefix % zoom % suffix;

    if (pthread_mutex_trylock(&mutex))
    {
        PDL_JSException(params, "worker thread busy");
        return PDL_FALSE;
    }
    try
    {
        if (!document)
            throw "";

        int err = ::mkdir(directory.c_str(), 0755);
        if (errno != 0 && errno != EEXIST)
        {
            PDL_JSException(params, "could not create directory");
            throw "";
        }

        for (int i = from; i < from + count; ++i)
        {
            syslog(LOG_INFO, "Starting rendering");
            std::string filename = (boost::format(my_formatter) % i).str();
            viewer::pdf_page& page = (*document)[i];
            syslog(LOG_INFO, filename.c_str());
            renderer.render_full(zoom / 100., page, filename);

            std::string response_json =
                (boost::format(render_response) % i % filename).str();

            const char* response = response_json.c_str();

            syslog(LOG_INFO, ("Finished rendering " + response_json).c_str());

            PDL_CallJS("RenderCallback", &response, 1);
        }
    }
    catch (...)
    {
        return_value = PDL_FALSE;
    }
    pthread_mutex_unlock(&mutex);

    syslog(LOG_INFO, "Done rendering");

    return return_value;
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

#define IF_TYPE(name, n)                                        \
    if (type == #name)                                          \
        if (argc == n+1)                                        \
            return_value = service::do_##name(params);\
        else                                                    \
            return_value = PDL_FALSE;

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

    return return_value;
}

const char* version_information = "{\"version\":\"mupdf 0.9\"}";

int main()
{
    syslog(LOG_INFO, "Starting up");
    SDL_Init(SDL_INIT_VIDEO);
    PDL_Init(0);

    pthread_mutex_init(&mutex, 0);

    PDL_RegisterJSHandler("Handler", &handler);
    PDL_JSRegistrationComplete();

    PDL_CallJS("ready", 0, 0);
    PDL_CallJS("VersionCallback", &version_information, 1);

    syslog(LOG_INFO, "Hello world! :)");

    SDL_Event event;
    do
    {
        SDL_WaitEvent(&event);
    }
    while (event.type != SDL_QUIT);

    pthread_mutex_destroy(&mutex);

    syslog(LOG_INFO, "Goodbye");

    if (document)
        delete document;

    PDL_Quit();
    SDL_Quit();
}
