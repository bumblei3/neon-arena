cmake_minimum_required(VERSION 3.16)
project(neon-arena-vulkan LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(neon-arena main.cpp)

target_link_libraries(neon-arena PRIVATE
    SDL2::SDL2
    Vulkan::Vulkan
)

# Compile GLSL to SPIR-V at build time (requires glslangValidator)
set(SHADER_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(SHADER_OUT_DIR ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_OUT_DIR})

set(SHADERS
    ${SHADER_DIR}/triangle.vert
    ${SHADER_DIR}/triangle.frag
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
