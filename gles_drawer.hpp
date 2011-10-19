#ifndef GLES_DRAWER_HPP
#define GLES_DRAWER_HPP

#include <vector>

#include <SDL.h>

#include "pdf_document.hpp"
#include "pixmap_renderer.hpp"
#include "gles/program.hpp"

namespace viewer
{

    class gles_drawer
    {
    public:
        gles_drawer (pdf_document&, SDL_Surface*);
        ~gles_drawer ();

        void operator()();

        void switch_to_page(std::size_t n);

    private:
        pdf_document& doc_;
        pixmap_renderer renderer_;
        gles::program program_;
        std::vector<float> vertex_array_;
        std::vector<float> texcoord_array_;
        unsigned int texture_;
    };

}

#endif
