#ifndef GLES_SHADER_HPP
#define GLES_SHADER_HPP

#include <string>
#include <vector>
#include <GLES2/gl2.h>

namespace gles
{

    // STUB
    class program
    {
    public:
        program();
        ~program();

        void add_frag_shader(std::string const& code)
        {
            add_shader(code, false);
        }

        void add_vert_shader(std::string const& code)
        {
            add_shader(code, true);
        }

        void add_shader(std::string const& code, bool vertex_shader);

        void bind_attrib(std::string const& name, GLuint index) const;
        GLint get_uniform_location(std::string const& name) const;
        GLint get_attrib_location(std::string const& name) const;

        void link();
        void use();

    private:
        GLuint handle_;
        std::vector<GLuint> shaders_;
        bool linked_;
    };

}

#endif
