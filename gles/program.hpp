#ifndef GLES_SHADER_HPP
#define GLES_SHADER_HPP

#include <GLES2/gl2.h>

namespace gles
{

    // STUB
    class program
    {
    public:
        program()
        {}

        ~program()
        {}

        void add_frag_shader(std::string const& code)
        {}

        void add_vert_shader(std::string const& code)
        {}

        template <typename T>
        void bind(std::string const&, T)
        {}

        void link()
        {}

        void use()
        {}

    private:
        GLuint program_;
    };

}

#endif
