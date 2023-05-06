
#define GL_GLEXT_PROTOTYPES

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <ctime>
#include <random>
#include <chrono>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>

const int DATASET_SIZE = 1000 * 1000 * 60;

std::string readShaderSourceFile(const char * const filename);
GLuint createComputeBuffer(GLsizeiptr size, const void * data, GLbitfield flags);
bool checkShaderCompilerErrors(GLuint shader_id);
bool checkProgramLink(GLuint program_id);
void checkGLErrors(const char * func);

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutCreateWindow("Not Important");

    std::cout << "OpenGL Version: " << (char*)glGetString(GL_VERSION) << std::endl;
    std::cout << "Shader Language Version: " << (char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl << std::endl;

    GLint max_wg_counts[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &max_wg_counts[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &max_wg_counts[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &max_wg_counts[2]);

    GLint max_wg_sizes[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &max_wg_sizes[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &max_wg_sizes[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &max_wg_sizes[2]);

    GLint max_invocations;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_invocations);

    std::cout << "Max Compute WG Counts: [" <<
        " x: " << max_wg_counts[0] <<
        " y: " << max_wg_counts[1] <<
        " z: " << max_wg_counts[2] << " ] " <<  std::endl;

    std::cout << "Max Compute WG Sizes: [" <<
        " x: " << max_wg_sizes[0] <<
        " y: " << max_wg_sizes[1] <<
        " z: " << max_wg_sizes[2] << " ] " << std::endl;

    std::cout << "Max Compute WG Invocations: " << max_invocations << std::endl;

    std::string source_code = readShaderSourceFile("compute_shader.comp");
    const char * const source_code_c = source_code.c_str();
    int source_code_length = static_cast<int>(source_code.size());

    GLuint shader_id = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader_id, 1, &source_code_c, &source_code_length);
    glCompileShader(shader_id);
    if (!checkShaderCompilerErrors(shader_id)) {
        return -1;
    }

    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, shader_id);
    glLinkProgram(program_id);
    if (!checkProgramLink(program_id)) {
        return -1;
    }

    int work_units = std::ceil((double)DATASET_SIZE / 1024); // layout (local_size_x = 1024) in;
    work_units = std::min(work_units, (int)max_wg_counts[0]);

    std::cout << "Work Units: " << work_units << '\n';

    std::cout << "Preparing Buffers..." << std::endl;

    float * random_a = new float[DATASET_SIZE];
    float * random_b = new float[DATASET_SIZE];
    float * random_c = new float[DATASET_SIZE];
    float * answers = new float[DATASET_SIZE];

    std::random_device device;
    std::mt19937 engine{};
    std::uniform_real_distribution<float> generator(0.01, 1000.0);

    for (int i = 0; i < DATASET_SIZE; i++) {
        random_a[i] = generator(engine);
        random_b[i] = generator(engine);
        random_c[i] = generator(engine);
    }

    auto start_time_full = std::chrono::steady_clock::now();

    std::cout << "Creating Buffers..." << std::endl;

    GLuint input_a_buffer_id = createComputeBuffer(sizeof(float) * DATASET_SIZE, random_a, 0);
    GLuint input_b_buffer_id = createComputeBuffer(sizeof(float) * DATASET_SIZE, random_b, 0);
    GLuint input_c_buffer_id = createComputeBuffer(sizeof(float) * DATASET_SIZE, random_c, 0);
    GLuint outputs_buffer_id = createComputeBuffer(sizeof(float) * DATASET_SIZE, nullptr, GL_MAP_READ_BIT|GL_CLIENT_STORAGE_BIT);

    std::cout << "Binding Buffers..." << std::endl;

    glUseProgram(program_id);
    checkGLErrors("glUseProgram");

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, input_a_buffer_id); // layout(binding = 1)
    checkGLErrors("glBindBufferBase");

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, input_b_buffer_id); // layout(binding = 2)
    checkGLErrors("glBindBufferBase");

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, input_c_buffer_id); // layout(binding = 3)
    checkGLErrors("glBindBufferBase");

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, outputs_buffer_id); // layout(binding = 4)
    checkGLErrors("glBindBufferBase");

    std::cout << "Calculating..." << std::endl;

    auto start_time = std::chrono::steady_clock::now();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputs_buffer_id);
    checkGLErrors("glBindBuffer");

    glDispatchCompute(work_units, 1, 1);
    checkGLErrors("glDispatchCompute");

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    checkGLErrors("glMemoryBarrier");

    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * DATASET_SIZE, answers);
    checkGLErrors("glMapBufferRange");

    auto end_time = std::chrono::steady_clock::now();

    // Provide proof of work
    std::cout << std::endl;
    for (int i = 0; i < std::min(DATASET_SIZE, 15); i++) {
        std::cout << random_a[i] << " * " << random_b[i] << " + " << random_c[i] << " = " << answers[i] << std::endl;
    }
    std::cout << std::endl;

    std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Time Spent: " << duration.count() << " milliseconds" << std::endl;

    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_full);
    std::cout << "Time Spent [full]: " << duration.count() << " milliseconds" << std::endl;

    start_time = std::chrono::steady_clock::now();
    for (int i = 0; i < DATASET_SIZE; i++) {
        answers[i] = random_a[i] * random_b[i] + random_c[i];
    }
    end_time = std::chrono::steady_clock::now();

    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Time Spent [cpu ]: " << duration.count() << " milliseconds" << std::endl;

    glDeleteProgram(program_id);
    glDeleteShader(shader_id);
    glDeleteBuffers(1, &input_a_buffer_id);
    glDeleteBuffers(1, &input_b_buffer_id);
    glDeleteBuffers(1, &outputs_buffer_id);

    delete [] random_a;
    delete [] random_b;
    delete [] random_c;
    delete [] answers;

    return 0;
}

std::string readShaderSourceFile(const char * const filename) {
    std::string code_buffer;

    std::ifstream file_reader{filename, std::ios::in};

    file_reader.seekg(0, std::ios::end);
    auto file_size = file_reader.tellg();
    code_buffer.resize(file_size);

    file_reader.seekg(0, std::ios::beg);
    file_reader.read(&code_buffer[0], code_buffer.capacity());

    return code_buffer;
}

GLuint createComputeBuffer(GLsizeiptr size, const void * data, GLbitfield flags) {
    GLuint buffer_id;
    glGenBuffers(1, &buffer_id);
    checkGLErrors("glGenBuffers");

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id);
    checkGLErrors("glBindBuffer");

    glBufferStorage(GL_SHADER_STORAGE_BUFFER, size, data, flags);
    checkGLErrors("glBufferStorage");

    return buffer_id;
}

bool checkShaderCompilerErrors(GLuint shader_id) {
    GLint compile_status;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_status);

    if (compile_status == GL_FALSE)
    {
        GLint compile_log_length = 0;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &compile_log_length);

        if (compile_log_length > 1)
        {
            std::string error_message;
            error_message.resize(compile_log_length);
            glGetShaderInfoLog(shader_id, compile_log_length, NULL, &error_message[0]);
            std::cout << "Compilation Errors:" << std::endl << error_message << std::endl;
        }
        return false;
    }

    return true;
}

bool checkProgramLink(GLuint program_id) {
    GLint link_status;
    glGetProgramiv(program_id, GL_LINK_STATUS, &link_status);

    if (link_status == GL_FALSE) {
        GLint message_length;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &message_length);

        std::string error_message;
        error_message.resize(message_length);

        glGetProgramInfoLog(program_id, message_length, &message_length, &error_message[0]);
        std::cout << "Program Link Errors: " << std::endl << error_message << std::endl;
        return false;
    }

    return true;
}

void checkGLErrors(const char * func) {
    GLenum error = glGetError();

    switch (error) {
        case GL_INVALID_ENUM:
            std::cout << "Error [" << func << "]: GL_INVALID_ENUM" << std::endl;
            break;
        case GL_INVALID_VALUE:
            std::cout << "Error [" << func << "]: GL_INVALID_VALUE" << std::endl;
            break;
        case GL_INVALID_OPERATION:
            std::cout << "Error [" << func << "]: GL_INVALID_OPERATION" << std::endl;
            break;
        case GL_STACK_OVERFLOW:
            std::cout << "Error [" << func << "]: GL_STACK_OVERFLOW" << std::endl;
            break;
        case GL_STACK_UNDERFLOW:
            std::cout << "Error [" << func << "]: GL_STACK_UNDERFLOW" << std::endl;
            break;
        case GL_OUT_OF_MEMORY:
            std::cout << "Error [" << func << "]: GL_OUT_OF_MEMORY" << std::endl;
            break;
    }
}