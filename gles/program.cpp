#include "program.hpp"

#include "exception.hpp"

namespace gles
{

    program::program() : handle_(glCreateProgram()),
                         linked_(false)
    {}

    program::~program()
    {
        glDeleteProgram(handle_);
        for (std::vector<GLuint>::iterator i = shaders_.begin();
             i != shaders_.end();
             ++i)
            glDeleteShader(*i);
    }

    void program::add_shader(std::string const& code, bool vertex_shader)
    {
        const GLuint shader = glCreateShader(vertex_shader ?
                                             GL_VERTEX_SHADER :
                                             GL_FRAGMENT_SHADER);

        glShaderSource(shader, 1, (const char**) (code.c_str()), 0);
        glCompileShader(shader);

        GLint shader_compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_compiled);

        if (!shader_compiled)
        {
            int info_log_length, chars_written;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);

            std::string message ("Failed to compile shader: ");
            message.reserve(message.size() + info_log_length);

            glGetShaderInfoLog(shader, info_log_length, &chars_written,
                               &message[0] + message.size());

            glDeleteShader(shader);

            throw gles_exception(message);
        }

        shaders_.push_back(shader);
        glAttachShader(handle_, shader);

        linked_ = false;
    }

    void program::link()
    {
        glLinkProgram(handle_);

        GLint linked;
        glGetProgramiv(handle_, GL_LINK_STATUS, &linked);

        if (!linked)
        {
            int info_log_length, chars_written;
            glGetProgramiv(handle_, GL_INFO_LOG_LENGTH, &info_log_length);

            std::string message ("Failed to link program: ");
            message.resize(message.size() + info_log_length);

            glGetProgramInfoLog(handle_, info_log_length, &chars_written,
                                &message[0] + message.size());

            throw gles_exception(message);
        }

        linked_ = true;
    }

    void program::use()
    {
        if (!linked_)
            link();

        glUseProgram(handle_);
    }
    

}
