#include "texture_manager.hpp"

#include <GL/gl.h>

namespace lector
{

    texture_manager::texture_manager()
    {
        glGenTextures(1, &handle_);
    }

    texture_manager::~texture_manager()
    {
        glDeleteTextures(1, &handle_);
    }

    void texture_manager::bind() const
    {
        glBindTexture(GL_TEXTURE_2D, handle_);
    }

    void texture_manager::upload(std::size_t width, std::size_t height,
                                 void* data) const
    {
        bind();
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D,
                     0, // mipmap level
                     GL_RGBA, // color components
                     width,
                     height,
                     0, // border, must be 0
                     GL_RGBA,
                     GL_UNSIGNED_BYTE, // One byte per component 
                     data
                     );
    }

}
