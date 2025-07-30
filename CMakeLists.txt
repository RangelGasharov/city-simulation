cmake_minimum_required(VERSION 3.12)
project(city_simulator)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include)

link_directories(${CMAKE_SOURCE_DIR}/lib)

file(GLOB SOURCES "src/*.cpp" "src/*.c")

add_executable(OpenGLApp ${SOURCES})

target_link_libraries(OpenGLApp glfw3dll opengl32 gdi32)

add_custom_command(TARGET OpenGLApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/glfw3.dll
        $<TARGET_FILE_DIR:OpenGLApp>
)

add_custom_target(run
    COMMAND OpenGLApp.exe
    DEPENDS OpenGLApp
    WORKING_DIRECTORY $<TARGET_FILE_DIR:OpenGLApp>
)