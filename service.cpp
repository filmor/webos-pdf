#include <pthread.h>
#include "pdf_document.hpp"
#include "png_renderer.hpp"

pthread_mutex_t mutex;
viewer::pdf_document* document = 0;

namespace service
{
PDL_bool do_shell(PDL_JSParameters* params)
{
    // STUB!
    return PDL_TRUE;
}

PDL_bool do_open(PDL_JSParameters* params)
{
    pthread_mutex_lock(&mutex);
    try
    {
        document = new viewer::pdf_document(JS_GetStringParam(params, 1));
        JS_Call("OpenCallback", 0, 0);
    }
    catch (viewer::pdf_exception const&)
    {
        return PDL_FALSE;
    }
    pthread_mutex_unlock(&mutex);
    return PDL_TRUE;
}

PDL_bool do_cover(PDL_JSParameters* params)
{
    // STUB!
    return ;
}

PDL_bool do_toc(PDL_JSParameters* params)
{
    // STUB!
    return PDL_TRUE;
}

PDL_bool do_render(PDL_JSParameters* params)
{
    PDL_bool return_value = PDL_TRUE;

    pdf_renderer renderer;

    int from = JS_GetParamInt(params, 1);
    int count = JS_GetParamInt(params, 2);
    float zoom = JS_GetParamFloat(params, 3);
    std::string directory = JS_GetParamString(params, 4);
    std::string prefix = JS_GetParamString(params, 5);
    std::string suffix = JS_GetParamString(params, 6);

    pthread_mutex_lock(&mutex);
    try
    {
        if (!document)
            throw "";

        for (int i = from; i < from + count; ++i)
        {
            viewer::pdf_page& page = *document[i];
            std::string filename =
            renderer.render_full(
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
            return_value = service::do_##name(document, params);\
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
            PDL_JSException(parms, ("Handler has no method " + type).c_str());
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

const char version_information[] = "mupdf 0.9"

int main()
{
    PDL_Init();
    SDL_Init(SDL_INIT_VIDEO);

    pthread_mutex_init(&mutex);

    PDL_RegisterJSHandler("Handler", &handler);
    PDL_JSRegistrationComplete();

    PDL_CallJS("VersionCallback", &version_information, 1);

    pthread_mutex_destroy(&m_mutex);

    if (document)
        delete document;

    SDL_Quit();
    PDL_Quit();
}
