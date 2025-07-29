cmake_minimum_required(VERSION 3.12)
project(OpenGLApp)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

link_directories(${CMAKE_SOURCE_DIR}/lib)

file(GLOB SOURCES "src/*.cpp" "src/*.c")

add_executable(OpenGLApp ${SOURCES})

target_link_libraries(OpenGLApp glfw3dll opengl32 gdi32)
