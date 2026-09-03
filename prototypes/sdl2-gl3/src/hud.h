// hud.h - HUD overlay for the game
#pragma once
#include <SDL.h>
#include "renderer.h"
#include <string>

class HUD {
public:
    HUD();
    ~HUD();

    void init(SDL_Window* window);
    void shutdown();

    void render(SDL_Surface* surface, int width, int height);

private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* sdlRenderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;
    Uint32 textureFormat = 0;
    int textureAccess = 0;
    int textureWidth = 0;
    int textureHeight = 0;

    void drawText(const std::string& text, int x, int y, int size, int r, int g, int b);
    void drawRect(int x, int y, int w, int h, int r, int g, int b, int a = 255);
    void drawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int a = 255);
    void fillRect(int x, int y, int w, int h, int r, int g, int b, int a = 255);
};
