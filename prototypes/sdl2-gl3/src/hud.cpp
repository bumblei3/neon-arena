// hud.cpp - HUD implementation
#include "hud.h"
#include <SDL.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

HUD::HUD() {}

HUD::~HUD() {
    shutdown();
}

void HUD::init(SDL_Window* window) {
    window_ = window;

    // Create SDL2 renderer for HUD
    sdlRenderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!sdlRenderer_) {
        fprintf(stderr, "Failed to create SDL2 renderer: %s\n", SDL_GetError());
    }
}

void HUD::shutdown() {
    if (texture_) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
    if (sdlRenderer_) {
        SDL_DestroyRenderer(sdlRenderer_);
        sdlRenderer_ = nullptr;
    }
}

void HUD::render(SDL_Surface* surface, int width, int height) {
    // In a real HUD we'd use OpenGL directly, but for now we use SDL2
    // This is a placeholder - in practice we'd render HUD directly in OpenGL
}

void HUD::drawText(const std::string& text, int x, int y, int size, int r, int g, int b) {
    // Placeholder - in practice we'd use a bitmap font or SDL_ttf
    // For now, just draw a colored rectangle as placeholder
    drawRect(x, y, text.length() * size / 2, size, r, g, b, 128);
}

void HUD::drawRect(int x, int y, int w, int h, int r, int g, int b, int a) {
    // Placeholder
}

void HUD::drawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int a) {
    // Placeholder
}

void HUD::fillRect(int x, int y, int w, int h, int r, int g, int b, int a) {
    // Placeholder
}
