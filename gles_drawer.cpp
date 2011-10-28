#include "gles_drawer.hpp"
#include "gles/exception.hpp"

#include <GLES2/gl2.h>
#include <iostream>

namespace viewer
{

    namespace
    {
        inline unsigned next_power_of_2 (unsigned v)
        {
            v--;
            v |= v >> 1;
            v |= v >> 2;
            v |= v >> 4;
            v |= v >> 8;
            v |= v >> 16;
            return v + 1;
        }
    }

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

    gles_drawer::gles_drawer(pdf_document& doc, SDL_Surface* screen)
        : doc_(doc)
    {
        glClearColor(0.0, 0.0, 0.5, 1.0);
        // glEnable(GL_BLEND);

        glViewport(0, 0, screen->w, screen->h);

        std::cout << "Width: " << screen->w << "\nHeight: " << screen->h << std::endl;

        program_.add_frag_shader(frag_shader);
        program_.add_vert_shader(vert_shader);

        program_.use();
        
        // Generate textures and buffer objects
        glGenTextures(3, textures_);
        glGenBuffers(2, vbos_);

        // TODO: We need three textures: Previous, next and current
        // Set to GL_TEXTURE_0
        glUniform1i(program_.get_uniform_location("texture"), 0);
        // glUniform1f(program_.get_uniform_location("aspect"), screen->w / float(screen->h));

        static const GLfloat modelview[] =
            {
                1, 0, 0, 0,
                0, 1, 0, 0,
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

    void gles_drawer::switch_to_page(std::size_t n)
    {
        std::cout << "Switching to page " << n << std::endl;
        pdf_page& page = doc_[n];
        pixmap pix = renderer_.render_full(1.0, page);

        glBindTexture(GL_TEXTURE_2D, textures_[0]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        gles::get_error();

        // TODO: Compress? TouchPad doesn't support PVRTC ...
        glTexImage2D(GL_TEXTURE_2D,
                     0, // mipmap level
                     GL_RGBA, // color components
                     page.width(),
                     page.height(),
                     0, // border, must be 0
                     GL_RGBA,
                     GL_UNSIGNED_BYTE, // One byte per component 
                     pix.get_data()
                     );

        gles::get_error();

        const static GLfloat aspect = 1024. / 768.;
        const static GLfloat page_aspect = page.height() / GLfloat(page.width());

        const float scale = page_aspect * aspect;

        gles::get_error();
    }

    gles_drawer::~gles_drawer()
    {
        glDeleteBuffers(2, vbos_);
        glDeleteTextures(3, textures_);
    }

    void gles_drawer::operator() ()
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[1]);
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);

        gles::get_error();

        SDL_GL_SwapBuffers();
    }


}
