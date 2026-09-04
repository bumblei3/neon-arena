cmake_minimum_required(VERSION 3.16)
project(neon-arena-vulkan LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

# SDL2_mixer may not have pkg-config, find it manually
find_path(SDL2_MIXER_INCLUDE_DIR SDL_mixer.h PATHS /usr/include/SDL2)
find_library(SDL2_MIXER_LIBRARY SDL2_mixer)

if(NOT SDL2_MIXER_INCLUDE_DIR OR NOT SDL2_MIXER_LIBRARY)
    message(WARNING "SDL2_mixer not found, audio will be disabled")
    set(SDL2_MIXER_INCLUDE_DIR "")
    set(SDL2_MIXER_LIBRARY "")
endif()

add_executable(neon-arena main.cpp game.cpp renderer.cpp hud.cpp particle_system.cpp audio.cpp map_loader.cpp)
add_executable(compute_demo compute_demo.cpp)

target_link_libraries(compute_demo PRIVATE
    SDL2::SDL2
    Vulkan::Vulkan
)

target_include_directories(neon-arena PRIVATE ${SDL2_MIXER_INCLUDE_DIR})
target_link_libraries(neon-arena PRIVATE
    SDL2::SDL2
    Vulkan::Vulkan
    ${SDL2_MIXER_LIBRARY}
)

# Compile GLSL to SPIR-V at build time (requires glslangValidator)
set(SHADER_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(SHADER_OUT_DIR ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_OUT_DIR})

set(SHADERS
    ${SHADER_DIR}/triangle.vert
    ${SHADER_DIR}/triangle.frag
    ${SHADER_DIR}/fullscreen.vert
    ${SHADER_DIR}/brightpass.frag
    ${SHADER_DIR}/blur.frag
    ${SHADER_DIR}/composite.frag
    ${SHADER_DIR}/particle.comp
)

foreach(SHADER ${SHADERS})
    get_filename_component(FILENAME ${SHADER} NAME)
    set(OUTPUT_FILE ${SHADER_OUT_DIR}/${FILENAME}.spv)
    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND glslangValidator -V ${SHADER} -o ${OUTPUT_FILE}
        DEPENDS ${SHADER}
        COMMENT "Compiling ${FILENAME}"
    )
    list(APPEND SPIRV_BINARIES ${OUTPUT_FILE})
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPIRV_BINARIES})
add_dependencies(neon-arena shaders)

target_compile_definitions(neon-arena PRIVATE
    SHADER_DIR="${SHADER_OUT_DIR}"
)
