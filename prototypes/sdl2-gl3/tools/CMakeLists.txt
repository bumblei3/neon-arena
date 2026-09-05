# NeonArena Tools — CLI tools for modding and community
# Standalone executable, no SDL/game dependencies

cmake_minimum_required(VERSION 3.16)
project(neon-arena-tools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(neon-tools
    src/main.cpp
    src/wave_editor.cpp
    src/map_validator.cpp
    src/json_helpers.cpp
)

target_include_directories(neon-tools PRIVATE src)
