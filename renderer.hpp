#ifndef LECTOR_RENDERER_HPP
#define LECTOR_RENDERER_HPP

#include <vector>
#include <boost/circular_buffer.hpp>

#include "pdf_document.hpp"
#include "pixmap_renderer.hpp"
#include "texture_manager.hpp"
#include "gles/program.hpp"

namespace lector
{

    class renderer
    {
    public:
        renderer(fz_context*, pdf_document&, std::size_t width, std::size_t height);
        ~renderer();

        void draw_frame();

        // Cache (read: render texture) of the pages [n-radius, n+radius]
        void cache(std::size_t n, std::size_t radius) {}

        void render_texture(std::size_t n);

        void resize (std::size_t, std::size_t);

        void switch_to_page(std::size_t n)
        {
            render_texture(n);
        }

        std::size_t page_count() const
        {
            return doc_.pages();
        }

    private:
        pdf_document& doc_;
        pixmap_renderer renderer_;
        gles::program program_;
        std::vector<float> vertex_array_;
        std::vector<float> texcoord_array_;
        texture_manager manager_;
        GLuint vbos_[2];
    };

}

#endif
