// NEON ARENA - Vulkan + SDL2 Prototype
// HUD implementation
#include "hud.h"
#include <cstring>
#include <cstdio>

void HudRenderer::reset() {
    vertices.clear();
}

void HudRenderer::drawText(const char* text, float x, float y, float scale, float r, float g, float b) {
    float penX = x;
    float penY = y;
    while (*text) {
        char c = *text;
        int idx = -1;
        if (c >= '0' && c <= '9') idx = c - '0';
        else if (c >= 'A' && c <= 'Z') idx = c - 'A' + 10;
        else if (c >= 'a' && c <= 'z') idx = c - 'a' + 10;
        else if (c == ' ') { penX += scale * 4; text++; continue; }
        else if (c == '-') idx = 10 + ('N' - 'A');
        
        if (idx >= 0) {
            for (int row = 0; row < 5; row++) {
                for (int col = 0; col < 3; col++) {
                    if (FONT[idx][row * 3 + col] == '1') {
                        float px = penX + col * scale;
                        float py = penY - row * scale;
                        // Draw pixel as small quad
                        HudVertex v0 = {{px, py}, {0, 0}, {r, g, b}};
                        HudVertex v1 = {{px + scale, py}, {1, 0}, {r, g, b}};
                        HudVertex v2 = {{px + scale, py - scale}, {1, 1}, {r, g, b}};
                        HudVertex v3 = {{px, py}, {0, 0}, {r, g, b}};
                        HudVertex v4 = {{px + scale, py - scale}, {1, 1}, {r, g, b}};
                        HudVertex v5 = {{px, py - scale}, {0, 1}, {r, g, b}};
                        vertices.push_back(v0);
                        vertices.push_back(v1);
                        vertices.push_back(v2);
                        vertices.push_back(v3);
                        vertices.push_back(v4);
                        vertices.push_back(v5);
                    }
                }
            }
        }
        penX += scale * 4;
        text++;
    }
}

void HudRenderer::drawRect(float x, float y, float w, float h, float r, float g, float b, float a) {
    HudVertex v0 = {{x, y}, {0, 0}, {r, g, b}};
    HudVertex v1 = {{x + w, y}, {1, 0}, {r, g, b}};
    HudVertex v2 = {{x + w, y + h}, {1, 1}, {r, g, b}};
    HudVertex v3 = {{x, y}, {0, 0}, {r, g, b}};
    HudVertex v4 = {{x + w, y + h}, {1, 1}, {r, g, b}};
    HudVertex v5 = {{x, y + h}, {0, 1}, {r, g, b}};
    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);
    vertices.push_back(v4);
    vertices.push_back(v5);
}

void HudRenderer::buildHud(int hp, int score, int wave, int screenW, int screenH) {
    reset();
    
    // Background bar at top
    drawRect(0, screenH - 60, screenW, 60, 0.02f, 0.02f, 0.05f, 0.8f);
    
    // HP
    char buf[64];
    snprintf(buf, sizeof(buf), "HP %d", hp);
    drawText(buf, 20, screenH - 40, 3, 0.2f, 1.0f, 0.8f);
    
    // Score
    snprintf(buf, sizeof(buf), "SCORE %d", score);
    drawText(buf, 200, screenH - 40, 3, 1.0f, 0.8f, 0.2f);
    
    // Wave
    snprintf(buf, sizeof(buf), "WAVE %d", wave);
    drawText(buf, 500, screenH - 40, 3, 0.8f, 0.2f, 1.0f);
    
    // Crosshair center (always on top)
    float cx = screenW * 0.5f;
    float cy = screenH * 0.5f;
    drawRect(cx - 15, cy - 2, 10, 4, 0.2f, 1.0f, 1.0f, 1.0f);
    drawRect(cx + 5, cy - 2, 10, 4, 0.2f, 1.0f, 1.0f, 1.0f);
    drawRect(cx - 2, cy - 15, 4, 10, 0.2f, 1.0f, 1.0f, 1.0f);
    drawRect(cx - 2, cy + 5, 4, 10, 0.2f, 1.0f, 1.0f, 1.0f);
}
