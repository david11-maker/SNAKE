cmake_minimum_required(VERSION 3.15)
project(SnakeGame)

set(CMAKE_CXX_STANDARD 17)

# Tell CMake to use vcpkg
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(snake snake.cpp)

target_link_libraries(snake PRIVATE
    OpenGL::GL
    GLEW::GLEW
    glfw
)
