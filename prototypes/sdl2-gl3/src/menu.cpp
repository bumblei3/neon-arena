// menu.cpp - Menu system for neon arena prototype
#include "game.h"

void Game::updateMenu(float dt) {
    stateTimer += dt;
}

void Game::renderMenu() {
    // Use beginFrame()/endFrame() for proper rendering
    renderer_->beginFrame();
    renderer_->clear(0.02f, 0.02f, 0.05f, 1.0f);

    // Set up camera for menu background
    Vec3 eye(0, 5, 20);
    Vec3 center(0, 0, 0);
    Vec3 up(0, 1, 0);
    Mat4 view = Mat4::lookAt(eye, center, up);
    float aspect = (float)renderer_->getWidth() / renderer_->getHeight();
    Mat4 proj = Mat4::perspective(1.1f, aspect, 0.1f, 200.0f);
    renderer_->setView(view);
    renderer_->setProjection(proj);
    renderer_->setViewPos(eye);

    // Render plasma background
    renderArena();

    renderer_->endFrame();

    // Overlay box (drawn after post-processing)
    float cx = 0.0f, cy = 0.0f;
    Vertex box[] = {
        Vertex(Vec3(cx - 0.35f, cy - 0.3f, 0), Vec3(0, 0, 0.15f)),
        Vertex(Vec3(cx + 0.35f, cy - 0.3f, 0), Vec3(0, 0, 0.15f)),
        Vertex(Vec3(cx + 0.35f, cy + 0.3f, 0), Vec3(0, 0, 0.15f)),
        Vertex(Vec3(cx - 0.35f, cy + 0.3f, 0), Vec3(0, 0, 0.15f)),
    };
    renderer_->drawQuad(box);

    // Title
    text_.drawTextCentered("NEON ARENA", 0.25f, 2.0f, Vec3(0.0f, 0.8f, 1.0f));
    text_.drawTextCentered("WAVE SURVIVAL", 0.18f, 1.0f, Vec3(0.5f, 0.5f, 0.6f));

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
        text_.drawText(menuItems[i], -0.13f, yPos - 0.025f, 1.2f, itemColor);
    }

    // Controls
    text_.drawTextCentered("WASD: MOVE  MOUSE: AIM  LMB: RAILGUN  RMB: LIGHTNING", -0.32f, 0.7f, Vec3(0.3f, 0.3f, 0.4f));
    text_.drawTextCentered("1/2/Q: SWITCH WEAPON  SPACE: NEXT WAVE", -0.38f, 0.7f, Vec3(0.3f, 0.3f, 0.4f));

    SDL_GL_SwapWindow(window_);
}

void Game::renderPauseMenu() {
    // Use beginFrame()/endFrame() for proper rendering
    renderer_->beginFrame();
    renderer_->clear(0.02f, 0.02f, 0.05f, 1.0f);

    Vec3 forward(
        sinf(player.yaw) * cosf(player.pitch),
        -sinf(player.pitch),
        -cosf(player.yaw) * cosf(player.pitch)
    );
    Vec3 eye = player.pos;
    Vec3 center = eye + forward;
    Vec3 up(0, 1, 0);
    Mat4 view = Mat4::lookAt(eye, center, up);
    float aspect = (float)renderer_->getWidth() / renderer_->getHeight();
    Mat4 proj = Mat4::perspective(1.1f, aspect, 0.1f, 200.0f);
    renderer_->setView(view);
    renderer_->setProjection(proj);
    renderer_->setViewPos(eye);

    // Render game world in background
    renderArena();
    renderBots(*this);
    renderProjectiles();
    renderParticles();
    renderLightning(*this);

    renderer_->endFrame();

    // Dark overlay (drawn after post-processing)
    float cx = 0.0f, cy = 0.0f;
    Vertex box[] = {
        Vertex(Vec3(cx - 0.2f, cy - 0.12f, 0), Vec3(0, 0, 0.25f)),
        Vertex(Vec3(cx + 0.2f, cy - 0.12f, 0), Vec3(0, 0, 0.25f)),
        Vertex(Vec3(cx + 0.2f, cy + 0.12f, 0), Vec3(0, 0, 0.25f)),
        Vertex(Vec3(cx - 0.2f, cy + 0.12f, 0), Vec3(0, 0, 0.25f)),
    };
    renderer_->drawQuad(box);

    text_.drawTextCentered("PAUSED", 0.05f, 1.5f, Vec3(0.0f, 0.8f, 1.0f));
    text_.drawTextCentered("ESC: RESUME    Q: QUIT TO MENU", -0.08f, 0.8f, Vec3(0.4f, 0.4f, 0.5f));

    SDL_GL_SwapWindow(window_);
}

void Game::renderOptions() {
    // Use beginFrame()/endFrame() for proper rendering
    renderer_->beginFrame();
    renderer_->clear(0.02f, 0.02f, 0.05f, 1.0f);

    // Set up camera for options background
    Vec3 eye(0, 5, 20);
    Vec3 center(0, 0, 0);
    Vec3 up(0, 1, 0);
    Mat4 view = Mat4::lookAt(eye, center, up);
    float aspect = (float)renderer_->getWidth() / renderer_->getHeight();
    Mat4 proj = Mat4::perspective(1.1f, aspect, 0.1f, 200.0f);
    renderer_->setView(view);
    renderer_->setProjection(proj);
    renderer_->setViewPos(eye);

    // Render plasma background
    renderArena();

    renderer_->endFrame();

    // Overlay box (drawn after post-processing)
    Vertex box[] = {
        Vertex(Vec3(-0.35f, -0.35f, 0), Vec3(0, 0, 0.15f)),
        Vertex(Vec3(0.35f, -0.35f, 0), Vec3(0, 0, 0.15f)),
        Vertex(Vec3(0.35f, 0.35f, 0), Vec3(0, 0, 0.15f)),
        Vertex(Vec3(-0.35f, 0.35f, 0), Vec3(0, 0, 0.15f)),
    };
    renderer_->drawQuad(box);

    // Title
    text_.drawTextCentered("OPTIONS", 0.28f, 1.8f, Vec3(0.0f, 0.8f, 1.0f));

    // Options items
    Vec3 selColor(0.0f, 1.0f, 0.8f);
    Vec3 normColor(0.5f, 0.5f, 0.5f);

    // Sensitivity
    Vec3 sensColor = (menuSelection == 0) ? selColor : normColor;
    text_.drawText("SENSITIVITY: ", -0.2f, 0.1f, 1.0f, sensColor);
    // Show value bar
    float sensPct = mouseSensitivity / 0.005f;
    Vertex sensBar[] = {
        Vertex(Vec3(0.1f, 0.09f, 0), sensColor),
        Vertex(Vec3(0.1f + sensPct * 0.3f, 0.09f, 0), sensColor),
    };
    renderer_->drawLineLoop(sensBar, 2, sensColor);

    // Bloom intensity
    Vec3 bloomColor = (menuSelection == 1) ? selColor : normColor;
    text_.drawText("BLOOM: ", -0.2f, -0.02f, 1.0f, bloomColor);

    // Back
    Vec3 backColor = (menuSelection == 2) ? selColor : normColor;
    text_.drawTextCentered("BACK", -0.2f, 1.0f, backColor);

    // Controls
    text_.drawTextCentered("LEFT/RIGHT: CHANGE    ESC: BACK", -0.32f, 0.7f, Vec3(0.3f, 0.3f, 0.4f));

    SDL_GL_SwapWindow(window_);
}

void Game::renderGameOver() {
    // Use beginFrame()/endFrame() for proper rendering
    renderer_->beginFrame();
    renderer_->clear(0.02f, 0.02f, 0.05f, 1.0f);

    Vec3 forward(
        sinf(player.yaw) * cosf(player.pitch),
        -sinf(player.pitch),
        -cosf(player.yaw) * cosf(player.pitch)
    );
    Vec3 eye = player.pos;
    Vec3 center = eye + forward;
    Vec3 up(0, 1, 0);
    Mat4 view = Mat4::lookAt(eye, center, up);
    float aspect = (float)renderer_->getWidth() / renderer_->getHeight();
    Mat4 proj = Mat4::perspective(1.1f, aspect, 0.1f, 200.0f);
    renderer_->setView(view);
    renderer_->setProjection(proj);
    renderer_->setViewPos(eye);

    // Render game world in background
    renderArena();
    renderBots(*this);
    renderProjectiles();
    renderParticles();
    renderLightning(*this);

    renderer_->endFrame();

    // Red overlay (drawn after post-processing)
    float cx = 0.0f, cy = 0.0f;
    Vertex box[] = {
        Vertex(Vec3(cx - 0.25f, cy - 0.18f, 0), Vec3(0.15f, 0, 0)),
        Vertex(Vec3(cx + 0.25f, cy - 0.18f, 0), Vec3(0.15f, 0, 0)),
        Vertex(Vec3(cx + 0.25f, cy + 0.18f, 0), Vec3(0.15f, 0, 0)),
        Vertex(Vec3(cx - 0.25f, cy + 0.18f, 0), Vec3(0.15f, 0, 0)),
    };
    renderer_->drawQuad(box);

    text_.drawTextCentered("GAME OVER", 0.12f, 1.8f, Vec3(1.0f, 0.2f, 0.2f));
    text_.drawTextCentered("SCORE: " + std::to_string(score), 0.0f, 1.0f, Vec3(1.0f, 0.8f, 0.0f));
    text_.drawTextCentered("HIGH:  " + std::to_string(highScore), -0.08f, 1.0f, Vec3(0.6f, 0.6f, 0.6f));
    text_.drawTextCentered("WAVES: " + std::to_string(wave - 1) + "  KILLS: " + std::to_string(kills), -0.16f, 0.9f, Vec3(0.5f, 0.5f, 0.5f));
    text_.drawTextCentered("SPACE: RETRY    ESC: MENU", -0.24f, 0.8f, Vec3(0.5f, 0.5f, 0.5f));

    SDL_GL_SwapWindow(window_);
}
