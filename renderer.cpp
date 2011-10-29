#include "renderer.hpp"
#include "gles/exception.hpp"

#include <iostream>
#include <GLES2/gl2.h>

namespace lector
{
    namespace
    {
        static const char vert_shader[] =
            "attribute highp vec4 vertex; \
             uniform mediump mat4 mvp_matrix; \
             varying mediump vec2 tex_coord; \
             void main() \
             { \
                 gl_Position = mvp_matrix * vertex; \
                 tex_coord = vertex.st; \
             }";

        static const char frag_shader[] =
            "uniform sampler2D texture; \
             varying mediump vec2 tex_coord; \
             void main() \
             { \
                 gl_FragColor = texture2D(texture, tex_coord); \
             }";
    }

    renderer::renderer(pdf_document& doc, std::size_t width, std::size_t height)
        : doc_(doc)
    {
        glClearColor(0.0, 0.0, 0.5, 1.0);

        // TODO: Add resize member function
        glViewport(0, 0, width, height);

        program_.add_frag_shader(frag_shader);
        program_.add_vert_shader(vert_shader);

        program_.use();
        
        // Generate textures and buffer objects
        glGenTextures(3, textures_);
        glGenBuffers(2, vbos_);

        // TODO: We need three textures: Previous, next and current
        // Set to GL_TEXTURE_0
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

    void renderer::switch_to_page(std::size_t n)
    {
        std::cout << "Switching to page " << n << "\n";
        pdf_page_ptr page = doc_.get_page(n);
        const unsigned zoom_factor = 2;
        pixmap pix = renderer_.render_full(zoom_factor, page);

        glBindTexture(GL_TEXTURE_2D, textures_[0]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        // TODO: Compress? TouchPad doesn't support PVRTC ...
        glTexImage2D(GL_TEXTURE_2D,
                     0, // mipmap level
                     GL_RGBA, // color components
                     page->width() * zoom_factor,
                     page->height() * zoom_factor,
                     0, // border, must be 0
                     GL_RGBA,
                     GL_UNSIGNED_BYTE, // One byte per component 
                     pix.get_data()
                     );

        // const static GLfloat page_aspect = page.height() / GLfloat(page.width());

        gles::get_error();

        doc_.age_store(1);
    }

    renderer::~renderer()
    {
        glDeleteBuffers(2, vbos_);
        glDeleteTextures(3, textures_);
    }

    void renderer::draw_frame()
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[1]);
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);

        gles::get_error();
    }


}
