#include "tile_manager.hpp"

#include <GL/gl.h>

namespace lector
{

    tile_manager::tile_manager(context& ctx)
        : ctx_(ctx)
    {
        glGenTextures(1, &handle_);
    }

    tile_manager::~tile_manager()
    {
        glDeleteTextures(1, &handle_);
    }

    void tile_manager::bind() const
    {
        glBindTexture(GL_TEXTURE_2D, handle_);
    }

    void tile_manager::generate()
    {
        int width, height;
        void* data = 0;
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
