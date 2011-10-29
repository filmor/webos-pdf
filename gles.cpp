#include <string>
#include <iostream>

#include <SDL.h>
#include <PDL.h>
#include <GLES2/gl2.h>

#include "pdf_document.hpp"
#include "renderer.hpp"

using namespace lector;

int main (int argc, char** argv)
{
    bool paused = false, running = true;
    if (argc != 2)
        return 1;

    std::string filename = argv[1];

    pdf_document* doc;

    try
    {
        doc = new pdf_document(argv[1]);
    }
    catch (pdf_exception const& exc)
    {
        std::cerr << "Exception: " << exc.what();
        return 1;
    }

    // Initialize PDL, SDL and OpenGLES
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
    PDL_Init(0);

    unsigned current_page = 0;
    
    try 
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);

        SDL_Surface* screen = SDL_SetVideoMode(0, 0, 0, SDL_OPENGL);

        lector::renderer renderer(*doc, screen->w, screen->h);

        SDL_Event event;
        do
        {
            renderer.draw_frame();

            bool got_event = false;
            if (paused)
            {
                SDL_WaitEvent(&event);
                got_event = true;
            }
            else
            {
                got_event = SDL_PollEvent(&event);
            }

            while (got_event)
            {
                switch (event.type)
                {
                case SDL_MOUSEBUTTONDOWN:
                    current_page = (current_page + 1) % doc->pages();
                    renderer.switch_to_page(current_page);
                    break;

                case SDL_ACTIVEEVENT:
                    if (event.active.state == SDL_APPACTIVE)
                    {
                        paused = !event.active.gain;
                    }
                    break;

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
