#ifndef GLES_EXCEPTION_HPP
#define GLES_EXCEPTION_HPP

#include <stdexcept>
#include <GLES2/gl2.h>

namespace gles
{

    struct gles_exception : std::runtime_error
    {
        gles_exception(std::string const& msg) : std::runtime_error(msg)
        {}
    };

    inline void get_error()
    {
#define GLES_CASE(name) \
    case name: \
        throw gles_exception(#name); \
        break;

        switch (glGetError())
        {
        case GL_NO_ERROR:
            return;
            GLES_CASE(GL_INVALID_ENUM)
            GLES_CASE(GL_INVALID_VALUE)
            GLES_CASE(GL_INVALID_OPERATION)
            GLES_CASE(GL_OUT_OF_MEMORY);
        default:
            throw gles_exception("Unknown error");
        };
#undef GLES_CASE
    }
}

#endif
