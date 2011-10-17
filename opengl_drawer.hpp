#ifndef OPENGL_DRAWER_HPP
#define OPENGL_DRAWER_HPP

#include <SDL.h>

#include "pdf_document.hpp"
#include "pixmap_renderer.hpp"

namespace viewer
{

    class opengl_drawer
    {
    public:
        opengl_drawer (pdf_document&, SDL_Surface*);
        ~opengl_drawer ();

        void operator()();

        void switch_to_page(std::size_t n);

    private:
        pdf_document& doc_;
        pixmap_renderer renderer_;
        unsigned int texture_;
    };

}

#endif
