#ifndef LECTOR_RENDERER_HPP
#define LECTOR_RENDERER_HPP

#include <vector>

#include "pdf_document.hpp"
#include "pixmap_renderer.hpp"
#include "gles/program.hpp"

namespace lector
{

    class renderer
    {
    public:
        renderer(pdf_document&, std::size_t width, std::size_t height);
        ~renderer();

        void draw_frame();

        void switch_to_page(std::size_t n);

    private:
        pdf_document& doc_;
        pixmap_renderer renderer_;
        gles::program program_;
        std::vector<float> vertex_array_;
        std::vector<float> texcoord_array_;
        GLuint textures_[3];
        GLuint vbos_[2];
    };

}

#endif
