#include "renderer.hpp"
#include "gles/exception.hpp"

#include <iostream>
#include <functional>
#include <GLES2/gl2.h>

namespace lector
{
    namespace
    {
        static const char vert_shader[] =
"attribute highp vec4 vertex;                                       \
 uniform highp vec4 aspect_ratios;                                  \
 uniform highp vec4 translate;                                      \
 uniform mediump mat4 mvp_matrix;                                   \
 varying mediump vec2 tex_coord;                                    \
                                                                    \
 void main()                                                        \
 {                                                                  \
     vec4 position = translate + vertex;                            \
     position.y *= aspect_ratios.x;                                 \
     gl_Position = mvp_matrix * position;                           \
     tex_coord = vertex.st;                                         \
 }                                                                  \
";

        static const char frag_shader[] =
"uniform sampler2D texture;                                         \
 varying mediump vec2 tex_coord;                                    \
 void main()                                                        \
 {                                                                  \
    gl_FragColor = texture2D(texture, tex_coord);                   \
 }                                                                  \
";
    }

    renderer::renderer(context& ctx, std::size_t width, std::size_t height)
        : ctx_(ctx)
        , manager_(ctx)
    {
        glClearColor(0.0, 0.0, 0.0, 1.0);
        resize(width, height);

        program_.add_frag_shader(frag_shader);
        program_.add_vert_shader(vert_shader);

        program_.use();
        
        // Generate textures and buffer objects
        glGenBuffers(2, vbos_);

        glUniform1i(program_.get_uniform_location("texture"), 0);

        static const GLfloat modelview[] =
            {
                0, 2, 0, 0,
                2, 0, 0, 0,
                0, 0, 1, 0,
                -1, -1, 0, 1
            };

        glUniformMatrix4fv(program_.get_uniform_location("mvp_matrix"), 1,
                           GL_FALSE, // don't transpose
                           modelview);

        static const GLfloat vertex_array[] =
            {
                0.f, 0.f,
                1.f, 0.f,
                0.f, 1.f,
                1.f, 1.f
            };

        static const GLushort element_array[] =
            {
                0, 1, 2, 3
            };

        glBindBuffer(GL_ARRAY_BUFFER, vbos_[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_array), vertex_array,
                     GL_STATIC_DRAW);

        glVertexAttribPointer(
            program_.get_attrib_location("vertex"),
            2, // size of the attribute (2)
            GL_FLOAT, // type
            GL_FALSE, // not normalized
            2 * sizeof(GLfloat), // stride
            0 // no offset
        );

        glEnableVertexAttribArray(program_.get_attrib_location("vertex"));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(element_array),
                     element_array, GL_STATIC_DRAW);

        // Callback that gets called whenever a new tile is ready. If the tile
        // has been rejected, return false
        manager_.register_callback(std::bind(&renderer::tile_callback, this));
    }

    bool renderer::tile_callback(std::size_t page, std::size_t x, std::size_t y,
                                 int level, bool page_complete, pixmap const& pix)
    {
        // BUG!
        //
        // The tile callback should only add (most of its parameters but
        // especially the pixmap reference) to the queue, so that it's blitted in
        // the rendering loop. The gl functions /have/ to be called inside of
        // the main thread, otherwise hell breaks loose.
        //
        // Heap structure:
        // The first item in the texture is the whole page on zoom level 0, next
        // are (0,0), (0,1), (1,0), (1,1) on level 1 and (0,0), (0,1), (0,2),
        // (0,3), (1,0), (1,1), … on level 2 so the formula is
        //
        //   \sum_{i=0}^{level} 4^i + 2^{level} * x + y
        //       = (1 - 4^{level - 1}) / (1 - 4) + 2^{level} * x + y
        //
        const int index = (1 << 2 * (level - 1)) / 3
                           + (1 << level) * x
                           + y;

        const unsigned TILE_SIZE = 256;

        if (handles_[page][level] == -1)
        {
            const std::size_t size = TILE_SIZE * (1 << level);
            glGenTextures(1, &handles_[page][level]);

            glBindTexture(GL_TEXTURE_2D, handles_[page][level]);

            // This has to be done before rendering, it doesn't concern the
            // loading of the texture!
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            // Generate empty texture
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         GL_RGBA,
                         size,
                         size,
                         0,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         0
                    );
        }


        // TODO: Skip second bind
        // Blit pixmap into texture
        glBindTexture(GL_TEXTURE_2D, handles_[page][level]);
        glTexSubImage2D(GL_TEXTURE_2D,
                        0,
                        x * 256,
                        y * 256,
                        pix.width(),
                        pix.height(),
                        GL_RGBA,
                        GL_UNSIGNED_BYTE, // One byte per component 
                        pix.get_data()
                        );

    }

    void renderer::resize(std::size_t width, std::size_t height)
    {
        glViewport(0, 0, width, height);
    }

    renderer::~renderer()
    {
        glDeleteBuffers(2, vbos_);
    }

    void renderer::render_page(std::size_t page, std::size_t x, std::size_t y,
                               float zoom_level)
    {
        // Which part of the page is seen at this zoom-level

        // Ask the tile manager to prepare those tiles
        manager_.generate();

        glClear(GL_COLOR_BUFFER_BIT);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[1]);
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);

        gles::get_error();
    }


}
