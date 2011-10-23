#include "gles_drawer.hpp"
#include "gles/exception.hpp"

#include <GLES2/gl2.h>

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
         attribute mediump vec4 uv; \
         /* uniform mediump mat4 modelview_matrix; */\
         varying mediump vec2 tex_coord; \
         void main() \
         { \
             mediump mat4 modelview_matrix = mat4(1.); \
             gl_Position = modelview_matrix * vertex; \
             tex_coord = uv.st; \
         }";

    static const char frag_shader[] =
        "uniform sampler2D texture; \
         varying mediump vec2 tex_coord; \
         void main() \
         { \
             gl_FragColor = texture2D(texture, tex_coord); \
         }";

    static const GLint VERTEX_ARRAY = 0;
    static const GLint TEXTURE_ARRAY = 1;

    gles_drawer::gles_drawer(pdf_document& doc, SDL_Surface* screen)
        : doc_(doc)
    {
        glClearColor(0.0, 0.0, 0.5, 1.0);
        glEnable(GL_BLEND);

        // glEnable(GL_TEXTURE_2D);

        // glViewPort(0, 0, screen->h, screen->w);
        // glLoadIdentity();
        // TODO: Get Orientation
        // TODO: Write Ortho-shader
        // glOrthof(0, screen->h, screen->w, 0, -1, 1);

        // glMatrixMode(GL_MODELVIEW);
        // glLoadIdentity();

        program_.add_frag_shader(frag_shader);
        program_.add_vert_shader(vert_shader);

        program_.bind_attrib("vertex", VERTEX_ARRAY);
        program_.bind_attrib("uv", TEXTURE_ARRAY);

        // use() links if this hasn't been done yet
        program_.use();
        
        glGenTextures(1, &texture_);

        glUniform1i(program_.get_uniform_location("texture"), 0);

        switch_to_page(0);
    }

    void gles_drawer::switch_to_page(std::size_t n)
    {
        pdf_page& page = doc_[n];
        pixmap pix = renderer_.render_full(1.0, page);

        glBindTexture(GL_TEXTURE_2D, texture_);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        // TODO: Compress? TouchPad doesn't support PVRTC ...
        // TODO: Mipmaps?
        glTexImage2D(GL_TEXTURE_2D,
                     0, // mipmap level
                     GL_RGBA, // color components
                     next_power_of_2(page.width()),
                     next_power_of_2(page.height()),
                     0, // border, must be 0
                     GL_RGBA,
                     GL_UNSIGNED_BYTE, // One byte per component 
                     0
                     );

        gles::get_error();

        glTexSubImage2D(GL_TEXTURE_2D,
                        0, // mipmap level
                        0, 0, // offset
                        page.width(),
                        page.height(),
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        pix.get_data()
                );


        static const float vertex_array[] = 
            {
                -1.f, -1.f, 0.f,
                -1.f, 1.f, 0.f,
                1.f, -1.f, 0.f,
                1.f, 1.f, 0.f
            };

        static const float texture_array[] =
            {
                0.f, 1.f,
                0.f, 0.f,
                1.f, 1.f,
                1.f, 0.f
            };
        
        glEnableVertexAttribArray(VERTEX_ARRAY);
        glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, 0, &vertex_array);

        glEnableVertexAttribArray(TEXTURE_ARRAY);
        glVertexAttribPointer(TEXTURE_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, &texture_array);

        glGenBuffers(1, &vbo_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(texture_array), vertex_array,
                     GL_STATIC_DRAW);
    }

    gles_drawer::~gles_drawer()
    {
        glDisableVertexAttribArray(VERTEX_ARRAY);
        glDisableVertexAttribArray(TEXTURE_ARRAY);
        glDeleteTextures(1, &texture_);
    }

    void gles_drawer::operator() ()
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        SDL_GL_SwapBuffers();
    }


}
