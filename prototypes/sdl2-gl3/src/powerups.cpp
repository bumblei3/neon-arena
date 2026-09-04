// powerups.cpp - Power-up system implementation for neon arena prototype
#include "powerups.h"
#include <algorithm>

void spawnPowerUp(Game& game, Vec3 pos, int type) {
    PowerUp pu;
    pu.pos = pos;
    pu.type = type;
    pu.life = 15.0f;  // 15 seconds to collect
    pu.rotation = 0;
    game.powerUps.push_back(pu);
}

void updatePowerUps(float dt, Game& game) {
    for (auto& pu : game.powerUps) {
        pu.life -= dt;
        pu.rotation += dt * 2.0f;
    }
    game.powerUps.erase(
        std::remove_if(game.powerUps.begin(), game.powerUps.end(),
            [](const PowerUp& p) { return p.life <= 0.0f; }),
        game.powerUps.end());
}

void renderPowerUps(Game& game) {
    for (const auto& pu : game.powerUps) {
        float pulse = 1.0f + sinf(pu.rotation * 3.0f) * 0.2f;
        float alpha = pu.life / 15.0f;
        Vec3 color;
        switch (pu.type) {
            case 0: color = Vec3(0.0f, 1.0f, 0.3f); break;  // Health - green
            case 1: color = Vec3(1.0f, 0.8f, 0.0f); break;  // Score - gold
            case 2: color = Vec3(1.0f, 0.3f, 0.0f); break;  // Damage - red
            default: color = Vec3(1.0f, 1.0f, 1.0f); break;
        }
        color = color * alpha;

        // Draw as spinning diamond
        float s = 0.3f * pulse;
        float h = s * 1.5f;
        Vec3 top(pu.pos.x, pu.pos.y + h, pu.pos.z);
        Vec3 bottom(pu.pos.x, pu.pos.y - h, pu.pos.z);
        Vec3 right(pu.pos.x + s, pu.pos.y, pu.pos.z);
        Vec3 left(pu.pos.x - s, pu.pos.y, pu.pos.z);
        Vec3 front(pu.pos.x, pu.pos.y, pu.pos.z + s);
        Vec3 back(pu.pos.x, pu.pos.y, pu.pos.z - s);

        Vertex tri1[] = { Vertex(top, color), Vertex(right, color), Vertex(front, color) };
        game.renderer_->drawLineLoop(tri1, 3, color);
        Vertex tri2[] = { Vertex(top, color), Vertex(front, color), Vertex(left, color) };
        game.renderer_->drawLineLoop(tri2, 3, color);
        Vertex tri3[] = { Vertex(bottom, color), Vertex(right, color), Vertex(front, color) };
        game.renderer_->drawLineLoop(tri3, 3, color);
        Vertex tri4[] = { Vertex(bottom, color), Vertex(front, color), Vertex(left, color) };
        game.renderer_->drawLineLoop(tri4, 3, color);
    }
}

void collectPowerUp(Game& game, int index) {
    if (index < 0 || index >= (int)game.powerUps.size()) return;
    PowerUp pu = game.powerUps[index];
    switch (pu.type) {
        case 0:  // Health
            game.player.health = game.maxHealth;
            break;
        case 1:  // Score bonus
            addScore(game, 500);
            break;
        case 2:  // Damage boost
            game.damageBoostTimer = 10.0f;
            break;
    }
    game.powerUps.erase(game.powerUps.begin() + index);
}
