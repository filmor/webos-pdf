#include "gles_drawer.hpp"

#include <GLES2/gl2.h>

namespace viewer
{

    static const char frag_shader[] =
        "uniform sampler2D texture; \
         varying mediump vec2 tex_coord; \
         void main() \
         { \
             gl_FragColor = texture2D(texture, tex_coord); \
         }";

    static const char vert_shader[] =
        "attribute highp vec4 vertex;\
         attribute mediump vec4 uv;\
         uniform mediump mat4 modelview_matrix;\
         varying mediump vec2 tex_coord;\
         void main()\
         {\
             gl_Position = modelview_matrix * vertex;\
             tex_coord = uv.st;\
         }";

    gles_drawer::gles_drawer(pdf_document& doc, SDL_Surface* screen)
        : doc_(doc)
    {
        glClearColor(0, 0, 0, 0);
        // glViewPort(0, 0, screen->h, screen->w);
        // glLoadIdentity();
        // TODO: Get Orientation
        // TODO: Write Ortho-shader
        // glOrthof(0, screen->h, screen->w, 0, -1, 1);

        // glMatrixMode(GL_MODELVIEW);
        // glLoadIdentity();

        program_.add_frag_shader(frag_shader);
        program_.add_vert_shader(vert_shader);

        program_.bind("vertex", vertex_array_);
        program_.bind("uv", texcoord_array_);

        // use() links if this hasn't been done yet
        program_.use();
        
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &texture_);

        switch_to_page(0);
    }

    void gles_drawer::switch_to_page(std::size_t n)
    {
        pdf_page& page = doc_[n];
        pixmap pix = renderer_.render_full(1.0, page);

        glBindTexture(GL_TEXTURE_2D, texture_);

        // TODO: Compress? TouchPad doesn't support PVRTC ...
        // TODO: Mipmaps?
        glTexImage2D(GL_TEXTURE_2D, 0 /* mipmap level */, 3 /* color components*/,
                     page.width(), page.height(), 0 /* border */,
                     GL_RGBA, GL_UNSIGNED_BYTE /* One byte per component */, 
                     pix.get_data());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        static const float array[] = 
            {
                0.f, 0.f, -1.f,
                1.f, 0.f, -1.f,
                1.f, 1.f, -1.f,
                0.f, 1.f, -1.f
            };
        
        glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, &array);
        glEnableVertexAttribArray(0);
        // TODO: Load texture
    }

    gles_drawer::~gles_drawer()
    {
        glDisableVertexAttribArray(0);
        glDeleteTextures(1, &texture_);
    }

    void gles_drawer::operator() ()
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        SDL_GL_SwapBuffers();
    }


}
