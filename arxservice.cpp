#include <PDL.h>
#include <SDL.h>

#include "log.hpp"
#include "service/service.hpp"

lector::service* service;

PDL_bool handler(PDL_JSParameters* params)
{
    PDL_bool return_value = PDL_TRUE;
    int argc = PDL_GetNumJSParams(params);

    std::string type(PDL_GetJSParamString(params, 0));
    LECTOR_LOG("Handler called with mode %s", type.c_str());

#define IF_TYPE2(name, n, m)                                        \
    if (type == #name)                                          \
        if (argc >= n + 1 && argc <= m + 1)                     \
            return_value = service->do_##name(params);          \
        else                                                    \
        {                                                       \
            PDL_JSException(params,                             \
                    ("Invalid parameter count for function "    \
                     + type).c_str());                          \
            return_value = PDL_FALSE;                           \
        }

#define IF_TYPE(name, n) \
    IF_TYPE2(name, n, n)

#define ELSE_TYPE2(name, n, m)                                   \
    else IF_TYPE2(name, n, m)

#define ELSE_TYPE(name, n) \
    ELSE_TYPE2(name, n, n)

    try
    {
        IF_TYPE(shell, 1)
        ELSE_TYPE2(open, 1, 2)
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

const char* version_information = "{\"version\":\"mupdf 0.9-e045\"}";

int main()
{
    LECTOR_LOG("Starting up");
    SDL_Init(SDL_INIT_VIDEO);
    PDL_Init(0);

    service = new lector::service(512, 3);

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

    service->stop();

    delete service;

    PDL_Quit();
    SDL_Quit();
}
