#include "specials.h"
#include "game.h"

// ------------------------------------------------------------
// Special ability implementations
// ------------------------------------------------------------

void activateNuclearBlast(Game& game) {
    if (game.nuclearBlastCooldown > 0.0f) return;

    for (auto& bot : game.bots) {
        if (bot.alive) {
            bot.alive = false;
            game.score += 50;
        }
    }
    game.nuclearBlastCooldown = 30.0f;
}

void activateTimeSlow(Game& game) {
    if (game.timeSlowCooldown > 0.0f) return;

    game.timeSlowTimer = 3.0f;
    game.timeSlowCooldown = 20.0f;
}

void activateShield(Game& game) {
    if (game.shieldCooldown > 0.0f) return;

    game.shieldTimer = 5.0f;
    game.hasShield = true;
    game.shieldCooldown = 15.0f;
}

void updateSpecials(float dt, Game& game) {
    // Decrement cooldowns
    if (game.nuclearBlastCooldown > 0.0f)
        game.nuclearBlastCooldown -= dt;
    if (game.timeSlowCooldown > 0.0f)
        game.timeSlowCooldown -= dt;
    if (game.shieldCooldown > 0.0f)
        game.shieldCooldown -= dt;

    // Decrement time-slow timer
    if (game.timeSlowTimer > 0.0f)
        game.timeSlowTimer -= dt;

    // Decrement shield timer; disable shield when expired
    if (game.shieldTimer > 0.0f) {
        game.shieldTimer -= dt;
        if (game.shieldTimer <= 0.0f)
            game.hasShield = false;
    }
}

void renderSpecialEffects(Game& game) {
    if (!game.hasShield) return;

    // Draw shield glow around player
    glColor4f(0.2f, 0.6f, 1.0f, 0.4f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i += 10) {
        float angle = static_cast<float>(i) * 3.14159f / 180.0f;
        float x = game.player.pos.x + cosf(angle) * 30.0f;
        float y = game.player.pos.y + sinf(angle) * 30.0f;
        glVertex2f(x, y);
    }
    glEnd();
}

void renderSpecialsHUD(Game& game) {
    // Cooldown boxes for E / R / F keys
    struct Box { const char* label; float cooldown; float maxCooldown; };
    Box boxes[] = {
        {"E", game.nuclearBlastCooldown, 30.0f},
        {"R", game.timeSlowCooldown, 20.0f},
        {"F", game.shieldCooldown, 15.0f},
    };

    float startX = 20.0f;
    float y = 40.0f;
    float boxSize = 40.0f;
    float gap = 10.0f;

    for (int i = 0; i < 3; i++) {
        float x = startX + i * (boxSize + gap);

        // Background
        glColor4f(0.1f, 0.1f, 0.1f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + boxSize, y);
        glVertex2f(x + boxSize, y + boxSize);
        glVertex2f(x, y + boxSize);
        glEnd();

        // Cooldown fill (dimmed if on cooldown)
        if (boxes[i].cooldown > 0.0f) {
            float ratio = boxes[i].cooldown / boxes[i].maxCooldown;
            glColor4f(0.3f, 0.3f, 0.3f, 0.5f);
            glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + boxSize, y);
            glVertex2f(x + boxSize, y + boxSize * ratio);
            glVertex2f(x, y + boxSize * ratio);
            glEnd();
        }
    }
}
