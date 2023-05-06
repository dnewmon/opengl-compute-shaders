cmake_minimum_required(VERSION 3.16.0)

project(compute)

add_executable(compute_cpp
    cpp_example/shader.cpp
    cpp_example/buffer.cpp
    cpp_example/program.cpp
    cpp_example/main.cpp)

find_package(OpenGL REQUIRED)
find_package(GLU REQUIRED)
find_package(GLUT REQUIRED)

target_link_libraries(compute_cpp ${OPENGL_LIBRARIES} ${GLU_LIBRARIES} ${GLUT_LIBRARIES})

add_executable(compute_simple
    basic_example/simple_compute_example.cpp)

target_link_libraries(compute_simple ${OPENGL_LIBRARIES} ${GLU_LIBRARIES} ${GLUT_LIBRARIES})
