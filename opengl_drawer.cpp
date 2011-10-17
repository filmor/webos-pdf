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

        // TODO: Mipmaps?
        glTexImage2D(GL_TEXTURE_2D, 0 /* mipmap level */, 3 /* color components*/,
                     page.width(), page.height(), 0 /* border */,
                     GL_RGB, GL_UNSIGNED_INT,
                     pix.get_data());
    }

    void opengl_drawer::operator() ()
    {
        // glDrawArrays();

        SDL_GL_SwapBuffers();
    }

}
