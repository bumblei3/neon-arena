// menu.cpp - Menu system for neon arena prototype
#include "game.h"

void Game::updateMenu(float dt) {
    stateTimer += dt;
}

void Game::renderMenu() {
    // Render plasma background
    renderArena();

    // Overlay box
    float cx = 0.0f, cy = 0.0f;
    Vertex box[] = {
        Vertex(Vec3(cx - 0.35f, cy - 0.3f, 0), Vec3(0, 0, 0.15f)),
        Vertex(Vec3(cx + 0.35f, cy - 0.3f, 0), Vec3(0, 0, 0.15f)),
        Vertex(Vec3(cx + 0.35f, cy + 0.3f, 0), Vec3(0, 0, 0.15f)),
        Vertex(Vec3(cx - 0.35f, cy + 0.3f, 0), Vec3(0, 0, 0.15f)),
    };
    renderer_->drawQuad(box);

    // Menu items with selection highlight
    for (int i = 0; i < (int)menuItems.size(); i++) {
        Vec3 itemColor = (i == menuSelection) ?
            Vec3(0.0f, 1.0f, 0.8f) : Vec3(0.5f, 0.5f, 0.5f);

        float yPos = 0.1f - i * 0.12f;

        // Selection arrow
        if (i == menuSelection) {
            Vertex arrow[] = {
                Vertex(Vec3(-0.18f, yPos - 0.015f, 0), itemColor),
                Vertex(Vec3(-0.15f, yPos, 0), itemColor),
                Vertex(Vec3(-0.18f, yPos + 0.015f, 0), itemColor),
            };
            renderer_->drawLineLoop(arrow, 3, itemColor);
        }
    }

    renderer_->endFrame();
    SDL_GL_SwapWindow(window_);
}

void Game::renderPauseMenu() {
    // Render game world in background
    renderArena();
    renderBots();
    renderProjectiles();
    renderParticles();

    // Dark overlay
    float cx = 0.0f, cy = 0.0f;
    Vertex box[] = {
        Vertex(Vec3(cx - 0.2f, cy - 0.12f, 0), Vec3(0, 0, 0.25f)),
        Vertex(Vec3(cx + 0.2f, cy - 0.12f, 0), Vec3(0, 0, 0.25f)),
        Vertex(Vec3(cx + 0.2f, cy + 0.12f, 0), Vec3(0, 0, 0.25f)),
        Vertex(Vec3(cx - 0.2f, cy + 0.12f, 0), Vec3(0, 0, 0.25f)),
    };
    renderer_->drawQuad(box);

    renderer_->endFrame();
    SDL_GL_SwapWindow(window_);
}

void Game::renderGameOver() {
    // Render game world in background
    renderArena();
    renderBots();
    renderProjectiles();
    renderParticles();

    // Red overlay
    float cx = 0.0f, cy = 0.0f;
    Vertex box[] = {
        Vertex(Vec3(cx - 0.25f, cy - 0.18f, 0), Vec3(0.15f, 0, 0)),
        Vertex(Vec3(cx + 0.25f, cy - 0.18f, 0), Vec3(0.15f, 0, 0)),
        Vertex(Vec3(cx + 0.25f, cy + 0.18f, 0), Vec3(0.15f, 0, 0)),
        Vertex(Vec3(cx - 0.25f, cy + 0.18f, 0), Vec3(0.15f, 0, 0)),
    };
    renderer_->drawQuad(box);

    renderer_->endFrame();
    SDL_GL_SwapWindow(window_);
}
