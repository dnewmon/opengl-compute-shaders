#include "simple_gl_impl.hpp"

namespace SimpleGL {
    std::unique_ptr<Buffer> createBuffer() {
        return std::make_unique<BufferImpl>();
    }

    BufferImpl::BufferImpl() {
        buffer_id_ = 0;
        buffer_ptr_ = nullptr;
    }

    void BufferImpl::initialize(GLsizeiptr size, const void * data, GLbitfield flags) {
        glGenBuffers(1, &buffer_id_);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id_);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, size, data, flags);
    }

    void BufferImpl::bind_buffer(GLint index) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buffer_id_);
    }

    void BufferImpl::copy_buffer(void * data, GLsizeiptr size) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id_);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
    }

    BufferImpl::~BufferImpl() {
        if (buffer_ptr_ != nullptr) {
            glUnmapNamedBuffer(buffer_id_);
        }

        if (buffer_id_ != 0) {
            glDeleteBuffers(1, &buffer_id_);
            buffer_id_ = 0;
        }
    }

    BufferImpl::operator GLuint() const {
        return buffer_id_;
    }
}
