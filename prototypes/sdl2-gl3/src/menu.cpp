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

    // Title
    text_.drawTextCentered(renderer_, "NEON ARENA", 0.25f, 2.0f, Vec3(0.0f, 0.8f, 1.0f));
    text_.drawTextCentered(renderer_, "WAVE SURVIVAL", 0.18f, 1.0f, Vec3(0.5f, 0.5f, 0.6f));

    // Menu items with selection highlight
    for (int i = 0; i < (int)menuItems.size(); i++) {
        Vec3 itemColor = (i == menuSelection) ?
            Vec3(0.0f, 1.0f, 0.8f) : Vec3(0.5f, 0.5f, 0.5f);

        float yPos = 0.05f - i * 0.12f;

        // Selection arrow
        if (i == menuSelection) {
            Vertex arrow[] = {
                Vertex(Vec3(-0.18f, yPos - 0.015f, 0), itemColor),
                Vertex(Vec3(-0.15f, yPos, 0), itemColor),
                Vertex(Vec3(-0.18f, yPos + 0.015f, 0), itemColor),
            };
            renderer_->drawLineLoop(arrow, 3, itemColor);
        }

        // Draw text
        text_.drawText(renderer_, menuItems[i], -0.13f, yPos - 0.025f, 1.2f, itemColor);
    }

    // Controls
    text_.drawTextCentered(renderer_, "UP/DOWN: SELECT    ENTER: CONFIRM", -0.25f, 0.7f, Vec3(0.3f, 0.3f, 0.4f));

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

    text_.drawTextCentered(renderer_, "PAUSED", 0.05f, 1.5f, Vec3(0.0f, 0.8f, 1.0f));
    text_.drawTextCentered(renderer_, "ESC: RESUME    Q: QUIT TO MENU", -0.08f, 0.8f, Vec3(0.4f, 0.4f, 0.5f));

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

    text_.drawTextCentered(renderer_, "GAME OVER", 0.12f, 1.8f, Vec3(1.0f, 0.2f, 0.2f));
    text_.drawTextCentered(renderer_, "SPACE: RETRY    ESC: MENU", -0.05f, 0.8f, Vec3(0.5f, 0.5f, 0.5f));

    renderer_->endFrame();
    SDL_GL_SwapWindow(window_);
}
