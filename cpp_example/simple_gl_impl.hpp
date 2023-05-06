#ifndef SIMPLE_GL_IMPL_HPP
#define SIMPLE_GL_IMPL_HPP

#include "libs.hpp"
#include "simple_gl.hpp"

namespace SimpleGL {
    class ComputeShaderImpl : public ComputeShader {
    public:
        ComputeShaderImpl();

        virtual bool initialize(const char * const filename);
        virtual operator GLuint() const;

        virtual ~ComputeShaderImpl();
    private:
        GLuint shader_id_;
    };

    class ProgramImpl : public Program {
    public:
        ProgramImpl();
        virtual bool initialize(GLuint shader_id);
        virtual void use();
        virtual operator GLuint() const;

        virtual ~ProgramImpl();

    private:
        GLuint program_id_;
    };

    class BufferImpl : public Buffer {
    public:
        BufferImpl();
        virtual void initialize(GLsizeiptr size, const void * data, GLbitfield flags);
        virtual void bind_buffer(GLint index);
        virtual void copy_buffer(void * data, GLsizeiptr size);
        virtual operator GLuint() const;

        virtual ~BufferImpl();

    private:
        GLuint buffer_id_;
        void * buffer_ptr_;
    };
}

#endif
