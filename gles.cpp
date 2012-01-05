#include <string>
#include <iostream>
#include <cstdint>

#include <SDL.h>
#include <PDL.h>
#include <GLES2/gl2.h>

#include "pdf_document.hpp"
#include "renderer.hpp"
#include "event_transform.hpp"
#include "scene.hpp"

using namespace lector;

int main (int argc, char** argv)
{
    bool paused = false, running = true;
    if (argc != 2)
        return 1;

    std::string filename = argv[1];

    fz_context* ctx = fz_new_context(&fz_alloc_default, 512 << 20);
    pdf_document* doc;

    try
    {
        doc = new pdf_document(ctx, argv[1]);
    }
    catch (pdf_exception const& exc)
    {
        std::cerr << "Exception: " << exc.what();
        return 1;
    }

    // Initialize PDL, SDL and OpenGLES
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
    PDL_Init(0);

    try 
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_Surface* screen = SDL_SetVideoMode(0, 0, 0, SDL_OPENGL);

        lector::renderer renderer(ctx, *doc, screen->w, screen->h);

        lector::scene scene(renderer);
        lector::event_transform handler(scene);

        SDL_Event event;
        do
        {
            const std::uint32_t current_time = SDL_GetTicks();

            scene.draw(current_time);

            bool got_event = false;
            if (paused)
            {
                SDL_WaitEvent(&event);
                got_event = true;
            }
            else
                got_event = SDL_PollEvent(&event);

            handler.set_current_time(current_time);

            while (got_event)
            {
                switch (event.type)
                {
                case SDL_MOUSEBUTTONDOWN:
                    handler.handle_down(event.button.which, event.button.x,
                                        event.button.y);
                    break;

                case SDL_MOUSEBUTTONUP:
                    handler.handle_up(event.button.which, event.button.x,
                                      event.button.y);
                    break;

                case SDL_MOUSEMOTION:
                    handler.handle_motion(event.motion.which, event.motion.x,
                                          event.motion.y, event.motion.xrel,
                                          event.motion.yrel);
                    break;

                case SDL_ACTIVEEVENT:
                    if (event.active.state == SDL_APPACTIVE)
                        paused = !event.active.gain;
                    break;

                case SDL_VIDEORESIZE:
                    renderer.resize(event.resize.w, event.resize.h);
                    break;

                // TODO SDL_USEREVENT (from JS calls)

                case SDL_QUIT:
                    running = false;
                    break;
                }

                got_event = SDL_PollEvent(&event);
            }

            SDL_GL_SwapBuffers();
        }
        while (running);
    }
    catch (std::exception const& exc)
    {
        std::cerr << "Exception during rendering:\n" << exc.what() << std::endl;
        throw;
    }

    delete doc;
    PDL_Quit();
    SDL_Quit();
}
