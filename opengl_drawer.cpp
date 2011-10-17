#include "opengl_drawer.hpp"

#include <GLES2/gl2.h>

namespace viewer
{

    opengl_drawer::opengl_drawer(pdf_document& doc, SDL_Surface* screen)
        : doc_(doc)
    {
        glClearColor(0, 0, 0, 0);
//        glLoadIdentity();
        // TODO: Get Orientation
        // TODO: Write Ortho-shader
//        glOrthof(0, screen->h, screen->w, 0, -1, 1);
//        glViewPort(0, 0, screen->h, screen->w);

//        glMatrixMode(GL_MODELVIEW);
//        glLoadIdentity();

        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &texture_);

        switch_to_page(0);
    }

    void opengl_drawer::switch_to_page(std::size_t n)
    {
        pdf_page& page = doc_[n];
        pixmap pix = renderer_.render_full(1.0, page);

        glBindTexture(GL_TEXTURE_2D, texture_);

        // TODO: convert binary data
        // TODO: Compress
        // TODO: Mipmaps?
        glTexImage2D(GL_TEXTURE_2D, 0 /* mipmap level */, 3 /* color components*/,
                     page.width(), page.height(), 0 /* border */,
                     GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
                     pix.get_data());

        static const float array[] = 
            {
                0f, 0f, -1f,
                1f, 0f, -1f,
                1f, 1f, -1f,
                0f, 1f, -1f
            };
        
        glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, &array);
        glEnableVertexAttribArray(0);
        // TODO: Load texture
    }

    opengl_drawer::opengl_drawer()
    {
        glDisableVertexAttribArray(0);
        glDeleteTextures(1, &texture_);
    }

    void opengl_drawer::operator() ()
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        SDL_GL_SwapBuffers();
    }


}
