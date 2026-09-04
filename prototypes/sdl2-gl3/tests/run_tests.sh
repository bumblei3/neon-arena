#!/bin/bash
# Test runner for NeonArena SDL2-GL3 Prototype
set -e

cd "$(dirname "$0")"

echo "=== Building Tests ==="

# --- 1. Wave Config Tests (standalone, no game.h) ---
echo ""
echo "=== Building Wave Config Tests ==="
g++ -std=c++17 -O2 -o test_wave_config test_wave_config.cpp -I../src

echo ""
echo "=== Running Wave Config Tests ==="
./test_wave_config

# --- 2. Unit Tests (mock-based) ---
echo ""
echo "=== Building Unit Tests ==="
g++ -std=c++17 -O2 -o test_game test_game.cpp

echo ""
echo "=== Running Unit Tests ==="
./test_game

# --- 3. Game State Feature Tests (splitter, shake, HUD) ---
echo ""
echo "=== Building Game State Tests ==="
g++ -std=c++17 -O2 -o test_game_state test_game_state.cpp

echo ""
echo "=== Running Game State Tests ==="
./test_game_state

# --- 4. Spatial Hash Tests ---
echo ""
echo "=== Building Spatial Hash Tests ==="
g++ -std=c++17 -O2 -o test_spatial_hash test_spatial_hash.cpp -I../src

echo ""
echo "=== Running Spatial Hash Tests ==="
./test_spatial_hash

# --- 5. Audio Manager Logic Tests ---
echo ""
echo "=== Building Audio Tests ==="
g++ -std=c++17 -O2 -o test_audio test_audio.cpp

echo ""
echo "=== Running Audio Tests ==="
./test_audio

# --- 6. Savegame Tests ---
echo ""
echo "=== Building Savegame Tests ==="
SDL_CFLAGS=$(sdl2-config --cflags)
SDL_LIBS=$(sdl2-config --libs)
g++ -std=c++17 -O2 $SDL_CFLAGS -I../src -o test_savegame test_savegame.cpp ../src/savegame.cpp $SDL_LIBS

echo ""
echo "=== Running Savegame Tests ==="
./test_savegame

# --- 7. Music Generator Tests ---
echo ""
echo "=== Building Music Tests ==="
SDL_CFLAGS=$(sdl2-config --cflags)
SDL_LIBS=$(sdl2-config --libs)
g++ -std=c++17 -O2 $SDL_CFLAGS -I../src -o test_music test_music.cpp ../src/music_generator.cpp $SDL_LIBS -lSDL2_mixer

echo ""
echo "=== Running Music Tests ==="
./test_music

# --- Summary ---
echo ""
echo "=== ALL TESTS PASSED ==="

# Cleanup
rm -f test_wave_config test_game test_game_state test_spatial_hash test_audio test_savegame test_music
