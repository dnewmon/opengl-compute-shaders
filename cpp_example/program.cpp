#include "simple_gl_impl.hpp"

namespace SimpleGL {
    std::unique_ptr<Program> createProgram() {
        return std::make_unique<ProgramImpl>();
    }

    ProgramImpl::ProgramImpl() {
        program_id_ = 0;
    }

    bool ProgramImpl::initialize(GLuint shader_id) {
        program_id_ = glCreateProgram();
        glAttachShader(program_id_, shader_id);

        glLinkProgram(program_id_);
        GLint link_status;
        glGetProgramiv(program_id_, GL_LINK_STATUS, &link_status);

        if (link_status == GL_FALSE) {
            GLint message_length;
            glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &message_length);

            std::string error_message;
            error_message.resize(message_length);

            glGetProgramInfoLog(program_id_, message_length, &message_length, &error_message[0]);

            std::cout << "Program Link Errors: " << std::endl << error_message << std::endl;

            glDeleteProgram(program_id_);
            program_id_ = 0;
            return false;
        }

        return true;
    }

    void ProgramImpl::use() {
        glUseProgram(program_id_);
    }

    ProgramImpl::operator GLuint() const {
        return program_id_;
    }

    ProgramImpl::~ProgramImpl() {
        glDeleteProgram(program_id_);
        program_id_ = 0;
    }
}
