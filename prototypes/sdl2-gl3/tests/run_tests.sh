#!/bin/bash
# Test runner for NeonArena SDL2-GL3 Prototype
set -e

echo "=== Building Tests ==="
cd "$(dirname "$0")"

# Unit test for wave config
echo ""
echo "=== Building Wave Config Tests ==="
g++ -std=c++17 -O2 -o test_wave_config test_wave_config.cpp -I../src

echo ""
echo "=== Running Wave Config Tests ==="
./test_wave_config

# Original unit tests
echo ""
echo "=== Building Unit Tests ==="
g++ -std=c++17 -O2 -o test_game test_game.cpp

echo ""
echo "=== Running Unit Tests ==="
./test_game

# Cleanup
rm -f test_wave_config test_game
