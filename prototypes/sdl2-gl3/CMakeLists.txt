# NEON ARENA - SDL2 + OpenGL 3.3+ Core Profile Prototype
# Modern OpenGL with shaders, VBOs, VAOs and bloom post-processing

cmake_minimum_required(VERSION 3.16)
project(neon-arena-gl3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)

# Find SDL2_Mixer
find_path(SDL2_MIXER_INCLUDE_DIR SDL_mixer.h
    PATHS /usr/include/SDL2
)
find_library(SDL2_MIXER_LIBRARY SDL2_mixer)

add_executable(neon-arena
    src/main.cpp
    src/shader.cpp
    src/renderer.cpp
    src/game.cpp
    src/texture.cpp
    src/audio_manager.cpp
    src/spatial_hash.cpp
    src/savegame.cpp
    src/music_generator.cpp
    src/particle_ecs.cpp
    src/menu.cpp
    src/text.cpp
    src/weapons.cpp
    src/wave_config.h
    src/bots.cpp
    src/score.cpp
    src/powerups.cpp
    src/specials.cpp
    src/hud.cpp
)

target_include_directories(neon-arena PRIVATE src)
target_link_options(neon-arena PRIVATE "LINKER:-no-pie")
target_link_libraries(neon-arena PRIVATE
    OpenGL::GL
    SDL2::SDL2
    GLEW::GLEW
    ${SDL2_MIXER_LIBRARY}
)
