# OpenGL Compute Shader Example

This is an example of how to use OpenGL Compute Shaders for general purpose computing. This was built for a Linux host machine so won't compile on Windows. This is written in C++ in two basic versions. One is a very simplified single-source-file example, and the other is a more C++-ish implementation.

## Compute Shader

This example runs on 4 generic data buffers and it runs a calculation similar to how most nerural networks are computed (A * B + C).

```
#version 460 core

layout (local_size_x = 1024) in;

layout(binding = 1)
buffer A {
    float A_values[];
};

layout(binding = 2)
buffer B {
    float B_values[];
};

layout(binding = 3)
buffer C {
    float C_values[];
};

layout(binding = 4)
buffer O {
    float answers[];
};

void main() {
    const uint i = gl_GlobalInvocationID.x;
    answers[i] = A_values[i] * B_values[i] + C_values[i];
}
```

There are 3 buffers with 60 million randomized floats for the computations, and then a 4th buffer to store the outputs.

## Build / Execution

On Linux, ensure you have these packages installed:

    freeglut3 freeglut3-dev

The provided build.sh script should work if you have a recent cmake installed along with the other essential build tools.
