#include "renderer.hpp"
#include "gles/exception.hpp"

#include <iostream>
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

    renderer::renderer(fz_context* ctx, 
                       pdf_document& doc, std::size_t width, std::size_t height)
        : doc_(doc)
        , renderer_(ctx)
    {
        glClearColor(0.0, 0.0, 0.0, 1.0);
        resize(width, height);

        program_.add_frag_shader(frag_shader);
        program_.add_vert_shader(vert_shader);

        program_.use();
        
        // Generate textures and buffer objects
        // prepare_cache(3);
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

        switch_to_page(0);
    }

    void renderer::resize(std::size_t width, std::size_t height)
    {
        glViewport(0, 0, width, height);
    }

#if 0
    void renderer::cache(std::size_t n, std::size_t radius)
    {
        std::size_t const start = n > radius ? n - radius : 0;
        std::size_t const end = n + radius < doc_.pages() ? n + radius : doc_.pages();

        prepare_cache(end - start);

        // TODO: Catch existing textures
        for (unsigned i = start; i < n; ++i)
            cache(i);
    }

    void renderer::prepare_cache(std::size_t n)
    {
        // TODO: Cache format: circular_buffer<pair<page, texturehandle>>
        if (cache_.size() >= n)
            return;

        if (cache_.size() != 0)
            glDeleteTextures(cache_.size(), &cache_[0]);
        
        cache_.resize(n);
        
        glGenTextures(cache_.size(), &cache_[0]);
        map_.clear();
    }
#endif

    void renderer::render_texture(std::size_t n)
    {
        std::cout << "Caching page " << n << "\n";
        pdf_page_ptr page = doc_.get_page(n);
        const float zoom_factor = 2.;
        // TODO: handle zoom_factor in texture_manager
        pixmap pix = renderer_.render_full(zoom_factor, page);

        manager_.upload(pix.width(), pix.height(), pix.get_data());

        gles::get_error();
    }

    renderer::~renderer()
    {
        glDeleteBuffers(2, vbos_);
    }

    void renderer::draw_frame()
    {
        manager_.bind();
        glClear(GL_COLOR_BUFFER_BIT);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[1]);
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);

        gles::get_error();
    }


}
