#ifndef GLES_EXCEPTION_HPP
#define GLES_EXCEPTION_HPP

#include <stdexcept>

namespace gles
{

    class gles_exception : std::runtime_error
    {
    public:
        gles_exception(std::string const& msg) : std::runtime_error(msg)
        {}
    };
}

#endif
