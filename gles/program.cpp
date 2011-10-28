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

        const GLint code_size = code.size();
        const char* code_ptr = code.c_str();
        glShaderSource(shader, 1, &code_ptr, &code_size);
        glCompileShader(shader);

        GLint shader_compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_compiled);

        if (!shader_compiled)
        {
            int info_log_length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);

            std::string message ("Failed to compile ");

            if (vertex_shader)
                message += "vertex shader";
            else
                message += "fragment shader";

            if (info_log_length)
            {
                message += ":\n";
                message.resize(message.size() + info_log_length);

                glGetShaderInfoLog(shader, info_log_length, 0,
                                   &message[0] + message.size());
            }

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

        GLint status;
        glGetProgramiv(handle_, GL_LINK_STATUS, &status);

        if (!status)
        {
            int info_log_length, chars_written;
            glGetProgramiv(handle_, GL_INFO_LOG_LENGTH, &info_log_length);

            std::string message ("Failed to link program: ");
            message.resize(message.size() + info_log_length);

            glGetProgramInfoLog(handle_, info_log_length, &chars_written,
                                &message[0] + message.size());

            throw gles_exception(message);
        }

        glValidateProgram(handle_);
        glGetProgramiv(handle_, GL_VALIDATE_STATUS, &status);

        if (!status)
            throw gles_exception("Failed to validate GLSL program");

        linked_ = true;
    }

    void program::use()
    {
        if (!linked_)
            link();

        glUseProgram(handle_);
    }

    void program::bind_attrib(std::string const& name, GLuint index) const
    {
        glBindAttribLocation(handle_, index, name.c_str());
    }

    GLint program::get_uniform_location(std::string const& name) const
    {
        return glGetUniformLocation(handle_, name.c_str());
    }

    GLint program::get_attrib_location(std::string const& name) const
    {
        return glGetAttribLocation(handle_, name.c_str());
    }

}
