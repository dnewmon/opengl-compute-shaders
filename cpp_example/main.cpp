#include "simple_gl.hpp"

struct ComputeInputs {
    float a;
    float b;
};

struct ComputeOutputs {
    float c;
};

const int DATASET_SIZE = 1000 * 1000 * 60;

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutCreateWindow("Hidden");

    std::cout << "OpenGL Version: " << (char*)glGetString(GL_VERSION) << std::endl;
    std::cout << "Shader Language Version: " << (char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

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

    std::cout << "Max Compute WG Counts: [ " <<
        " x: " << max_wg_counts[0] <<
        " y: " << max_wg_counts[1] <<
        " z: " << max_wg_counts[2] << " ] " <<  std::endl;

    std::cout << "Max Compute WG Sizes: [ " <<
        " x: " << max_wg_sizes[0] <<
        " y: " << max_wg_sizes[1] <<
        " z: " << max_wg_sizes[2] << " ] " << std::endl;

    std::cout << "Max Compute WG Invocations: " << max_invocations << std::endl;

    std::cout << std::endl;

    int work_units = std::ceil((double)DATASET_SIZE / 1024); // layout (local_size_x = 1024) in;
    work_units = std::min(work_units, (int)max_wg_counts[0]);

    std::cout << "Work Units: " << work_units << '\n';

    auto compute_shader = SimpleGL::createShader();
    if (!compute_shader->initialize("compute_shader.comp")) {
        return 1;
    }

    auto program = SimpleGL::createProgram();
    if (!program->initialize(*compute_shader)) {
        return 1;
    }

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

    auto inputs_a_buffer = SimpleGL::createBuffer();
    auto inputs_b_buffer = SimpleGL::createBuffer();
    auto inputs_c_buffer = SimpleGL::createBuffer();
    auto outputs_buffer = SimpleGL::createBuffer();

    std::cout << "Buffers Prepared..." << std::endl;

    auto start_time = std::chrono::steady_clock::now();

    inputs_a_buffer->initialize(sizeof(float) * DATASET_SIZE, random_a, 0);
    inputs_b_buffer->initialize(sizeof(float) * DATASET_SIZE, random_b, 0);
    inputs_c_buffer->initialize(sizeof(float) * DATASET_SIZE, random_c, 0);

    outputs_buffer->initialize(sizeof(float) * DATASET_SIZE, nullptr, GL_MAP_READ_BIT|GL_CLIENT_STORAGE_BIT);

    std::cout << "Binding Buffers..." << std::endl;

    program->use();
    inputs_a_buffer->bind_buffer(1);
    inputs_b_buffer->bind_buffer(2);
    inputs_c_buffer->bind_buffer(3);
    outputs_buffer->bind_buffer(4);

    std::cout << "Calculating..." << std::endl;

    glDispatchCompute(work_units, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    outputs_buffer->copy_buffer(answers, sizeof(float) * DATASET_SIZE);

    auto end_time = std::chrono::steady_clock::now();

    // Provide proof of work
    std::cout << std::endl;
    for (int i = 0; i < std::min(DATASET_SIZE, 15); i++) {
        std::cout << random_a[i] << " * " << random_b[i] << " + " << random_c[i] << " = " << answers[i] << std::endl;
    }
    std::cout << std::endl;

    std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Time Spent [gpu]: " << duration.count() << " milliseconds" << std::endl;

    start_time = std::chrono::steady_clock::now();
    for (int i = 0; i < DATASET_SIZE; i++) {
        answers[i] = random_a[i] * random_b[i] + random_c[i];
    }
    end_time = std::chrono::steady_clock::now();

    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Time Spent [cpu]: " << duration.count() << " milliseconds" << std::endl;

    delete [] random_a;
    delete [] random_b;
    delete [] random_c;
    delete [] answers;

    return 0;
}
