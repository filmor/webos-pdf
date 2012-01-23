#ifndef LECTOR_RENDERER_HPP
#define LECTOR_RENDERER_HPP

#include <vector>

#include "context.hpp"
#include "tile_manager.hpp"
#include "gles/program.hpp"

namespace lector
{

    class renderer
    {
    public:
        renderer(context& ctx, std::size_t width, std::size_t height);
        ~renderer();

        void draw_frame();

        void cache(std::size_t n, std::size_t radius);

        void resize (std::size_t, std::size_t);

        void render_page(std::size_t page,
                         std::size_t x, std::size_t y,
                         float zoom_level);

        void switch_to_page(std::size_t n)
        {
            render_texture(n);
        }

        std::size_t get_page_count() const
        {
            return ctx_.get_page_count();
        }

    private:
        void render_texture(std::size_t n);

        bool tile_callback(std::size_t page, std::size_t x, std::size_t y,
                             int level, bool page_complete, pixmap const& pix);

        context& ctx_;

        // Shader and Vertex Buffer objects
        gles::program program_;
        GLuint vbos_[2];
        GLuint** handles_;

        // Dimensions of the screen to determine which tiles can actually be
        // seen
        std::pair<std::size_t, std::size_t> dimensions_;

        // Array of vertices, since we only draw quads atm this is trivial
        std::vector<float> vertex_array_;
        // Array of texture coordinates
        std::vector<float> texcoord_array_;

        // Manager
        tile_manager manager_;
    };

}

#endif
