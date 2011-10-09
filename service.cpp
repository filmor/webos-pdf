#include <pthread.h>
#include <syslog.h>

#include <PDL.h>
#include <SDL.h>

#include <boost/format.hpp>

#include "pdf_document.hpp"
#include "png_renderer.hpp"

pthread_mutex_t mutex;
viewer::pdf_document* document = 0;

// <path><prefix><page>-<zoom><suffix>
const boost::format format ("%1$s%2$s%5$04d-%3$03d%4$s");

namespace service
{
PDL_bool do_shell(PDL_JSParameters* params)
{
    // STUB!
    return PDL_TRUE;
}

const boost::format response ("{\"pages\":%d,\"width\":%d,\"height\":%d}");
const boost::format error ("{\"error\":\"%s\"");

PDL_bool do_open(PDL_JSParameters* params)
{
    std::string r = "";
    pthread_mutex_lock(&mutex);
    try
    {
        document = new viewer::pdf_document(PDL_GetJSParamString(params, 1));
        r = (boost::format(response) % document->pages()
                                    % (*document)[0].width()
                                    % (*document)[0].height()).str();
    }
    catch (viewer::pdf_exception const& exc)
    {
        r = (boost::format(error) % exc.what()).str();
    }

    const char* ptr = r.c_str();

    PDL_CallJS("OpenCallback", &ptr, 1);

    // {
    //  error : "String" if failed,
    //  pages : document.pages(),
    //  width : document[0].width(),
    //  height : document[0].height,
    // }

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

PDL_bool do_render(PDL_JSParameters* params)
{
    PDL_bool return_value = PDL_TRUE;

    viewer::png_renderer renderer;

    boost::format my_formatter(format);

    int from = PDL_GetJSParamInt(params, 1);
    int count = PDL_GetJSParamInt(params, 2);
    int zoom = PDL_GetJSParamInt(params, 3);
    std::string directory = PDL_GetJSParamString(params, 4);
    std::string prefix = PDL_GetJSParamString(params, 5);
    std::string suffix = PDL_GetJSParamString(params, 6);

    my_formatter % directory % prefix % zoom % suffix;

    pthread_mutex_lock(&mutex);
    try
    {
        if (!document)
            throw "";

        for (int i = from; i < from + count; ++i)
        {
            viewer::pdf_page& page = (*document)[i];
            renderer.render_full(zoom / 100., page,
                                 (boost::format(my_formatter) % i).str()
                                );
        }
    }
    catch (...)
    {
        return_value = PDL_FALSE;
    }
    pthread_mutex_unlock(&mutex);

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

PDL_bool do_saveas(PDL_JSParameters* params)
{
    // STUB!
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

exit:
    return return_value;
}

const char* version_information = "mupdf 0.9";

int main()
{
    syslog(LOG_WARNING, "Starting up");
    SDL_Init(SDL_INIT_VIDEO);
    PDL_Init(0);

    pthread_mutex_init(&mutex, 0);

    PDL_RegisterJSHandler("Handler", &handler);
    PDL_JSRegistrationComplete();

    PDL_CallJS("ready", 0, 0);
    PDL_CallJS("VersionCallback", &version_information, 1);

    syslog(LOG_WARNING, "Hello world! :)");

    SDL_Event event;
    do
    {
        SDL_WaitEvent(&event);
    }
    while (event.type != SDL_QUIT);

    pthread_mutex_destroy(&mutex);

    if (document)
        delete document;

    PDL_Quit();
    SDL_Quit();
}
