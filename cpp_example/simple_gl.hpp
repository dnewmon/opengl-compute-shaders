#ifndef SIMPLE_GL_HPP
#define SIMPLE_GL_HPP

#include "libs.hpp"

// These interfaces are here to force you to
// create instances through the provided factory methods.
//
// This ensures each destructor is called exactly once per unique object
// and no copies of these objects with duplicate OpenGL identifiers are created.

namespace SimpleGL {
    class ComputeShader;
    class Program;
    class Buffer;

    std::unique_ptr<ComputeShader> createShader();
    std::unique_ptr<Program> createProgram();
    std::unique_ptr<Buffer> createBuffer();

    class ComputeShader {
    public:
        virtual bool initialize(const char * const filename) = 0;
        virtual operator GLuint() const = 0;
    };

    class Program {
    public:
        virtual bool initialize(GLuint shader_id) = 0;
        virtual void use() = 0;
        virtual operator GLuint() const = 0;
    };

    class Buffer {
    public:
        virtual void initialize(GLsizeiptr size, const void * data, GLbitfield flags) = 0;
        virtual void bind_buffer(GLint index) = 0;
        virtual operator GLuint() const = 0;
        virtual void copy_buffer(void * data, GLsizeiptr size) = 0;
    };
}

#endif
