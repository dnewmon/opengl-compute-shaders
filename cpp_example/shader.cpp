#include "simple_gl_impl.hpp"

namespace SimpleGL {
    std::unique_ptr<ComputeShader> createShader() {
        return std::make_unique<ComputeShaderImpl>();
    }

    std::string readShaderCode(const char * const filename);

    ComputeShaderImpl::ComputeShaderImpl() {
        shader_id_ = 0;
    }

    bool ComputeShaderImpl::initialize(const char * const filename) {
        std::string source_code = readShaderCode(filename);
        int source_code_length = static_cast<int>(source_code.size());
        const char * const source_code_c = source_code.c_str();

        shader_id_ = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(shader_id_, 1, &source_code_c, &source_code_length);
        glCompileShader(shader_id_);

        GLint compile_status;
        glGetShaderiv(shader_id_, GL_COMPILE_STATUS, &compile_status);

        if (compile_status == GL_FALSE)
        {
            GLint compile_log_length = 0;
            glGetShaderiv(shader_id_, GL_INFO_LOG_LENGTH, &compile_log_length);

            if (compile_log_length > 1)
            {
                std::string error_message;
                error_message.resize(compile_log_length);
                glGetShaderInfoLog(shader_id_, compile_log_length, NULL, &error_message[0]);
                std::cout << "Compilation Errors:" << std::endl << error_message << std::endl;
            }
            glDeleteShader(shader_id_);
            shader_id_ = 0;

            return false;
        }

        return true;
    }

    ComputeShaderImpl::operator GLuint() const {
        return shader_id_;
    }

    ComputeShaderImpl::~ComputeShaderImpl() {
        if (shader_id_ != 0) {
            glDeleteShader(shader_id_);
        }
    }

    std::string readShaderCode(const char * const filename) {
        std::string code_buffer;

        std::ifstream file_reader{filename, std::ios::in};

        file_reader.seekg(0, std::ios::end);
        auto file_size = file_reader.tellg();
        code_buffer.resize(file_size);

        file_reader.seekg(0, std::ios::beg);
        file_reader.read(&code_buffer[0], code_buffer.capacity());

        return code_buffer;
    }
}
