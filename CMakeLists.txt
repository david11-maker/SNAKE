cmake_minimum_required(VERSION 3.15)
project(SnakeGame)

set(CMAKE_CXX_STANDARD 17)

# Vcpkg toolchain beállítás (ha használod)
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# Külső könyvtárak keresése
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

# Futtatható fájl definiálása
add_executable(snake snake.cpp)

# Linkelés
target_link_libraries(snake PRIVATE
    OpenGL::GL
    GLEW::GLEW
    glfw
)

# Kimeneti mappák konfigurálása build típus szerint
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)

# Assetek elérési útja
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)
set(ASSETS_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/assets)

# Asset fájlok listája
file(GLOB ASSET_FILES
    "${ASSETS_DIR}/*.bmp"
    "${ASSETS_DIR}/*.frag"
    "${ASSETS_DIR}/*.vert"
)

# Assetek másolása build után a Release mappába
add_custom_command(TARGET snake POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ASSETS_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ASSET_FILES} ${ASSETS_OUTPUT_DIR}
)

set(RELEASE_ZIP_NAME "snake_release.zip")
set(ZIP_SOURCE_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/${RELEASE_ZIP_NAME}")
set(ZIP_TARGET_PATH "${CMAKE_SOURCE_DIR}/${RELEASE_ZIP_NAME}")

# ZIP létrehozása
add_custom_command(TARGET snake POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo ">> Csomagolás ZIP-be..."
    COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${ZIP_SOURCE_PATH}" --format=zip
        "$<TARGET_FILE:snake>"
        "${CMAKE_SOURCE_DIR}/assets"
)

# ZIP átmásolása a /snake/ mappába (projekt gyökérbe)
add_custom_command(TARGET snake POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo ">> ZIP átmásolása /snake mappába..."
    COMMAND ${CMAKE_COMMAND} -E copy "${ZIP_SOURCE_PATH}" "${ZIP_TARGET_PATH}"
)
