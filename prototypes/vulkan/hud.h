// NEON ARENA - Vulkan + SDL2 Prototype
// HUD: bitmap font rendering for HP, Score, Wave
#pragma once

#include "renderer.h"
#include <vector>
#include <cstring>

// 3x5 pixel font (0-9, A-Z)
static const char* FONT[36] = {
    "111101101101111", //0
    "010110010010111", //1
    "111001111100111", //2
    "111001111001111", //3
    "101101111001001", //4
    "111100111001111", //5
    "111100111101111", //6
    "111001001010010", //7
    "111101111101111", //8
    "111101111001111", //9
    "111101111101101", //A
    "110101110101110", //B
    "111100100100111", //C
    "110101101101110", //D
    "111100111100111", //E
    "111100111100100", //F
    "111100101101111", //G
    "101101111101101", //H
    "111010010010111", //I
    "001001001101111", //J
    "101101110101101", //K
    "100100100100111", //L
    "101111111101101", //M
    "110101101101101", //N
    "111101101101111", //O
    "111101111100100", //P
    "111101101111001", //Q
    "111101110101101", //R
    "111100111001111", //S
    "111010010010010", //T
    "101101101101111", //U
    "101101101101010", //V
    "101101111111101", //W
    "101101010101101", //X
    "101101010010010", //Y
    "111001010100111"  //Z
};

struct HudVertex {
    float pos[2];
    float uv[2];
    float color[3];
};

class HudRenderer {
public:
    std::vector<HudVertex> vertices;

    void reset();
    void drawText(const char* text, float x, float y, float scale, float r, float g, float b);
    void drawRect(float x, float y, float w, float h, float r, float g, float b, float a);
    void buildHud(int hp, int score, int wave, int screenW, int screenH);
    const std::vector<HudVertex>& getVertices() const { return vertices; }
};
