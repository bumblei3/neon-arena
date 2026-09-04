// score.cpp - Score system implementation for neon arena prototype
#include "score.h"
#include <algorithm>
#include <cstdio>

void updateScore(float dt, Game& game) {
    // Decay multiplier over time
    if (game.scoreMultiplier > 1) {
        game.multiplierTimer -= dt;
        if (game.multiplierTimer <= 0.0f) {
            game.scoreMultiplier = 1;
            game.comboCount = 0;
        }
    }
    // Decay damage boost
    if (game.damageBoostTimer > 0.0f) {
        game.damageBoostTimer -= dt;
    }
}

void addScore(Game& game, int points) {
    // Increase combo
    game.comboCount++;
    game.multiplierTimer = game.multiplierDecay;
    if (game.comboCount >= 10) {
        game.scoreMultiplier = 5;
    } else if (game.comboCount >= 5) {
        game.scoreMultiplier = 3;
    } else if (game.comboCount >= 3) {
        game.scoreMultiplier = 2;
    }
    game.score += points * game.scoreMultiplier;
    if (game.score > game.highScore) {
        game.highScore = game.score;
    }
}

void saveHighScore(Game& game) {
    FILE* f = fopen("highscore.dat", "w");
    if (f) {
        fprintf(f, "%d", game.highScore);
        fclose(f);
    }
}

void loadHighScore(Game& game) {
    FILE* f = fopen("highscore.dat", "r");
    if (f) {
        fscanf(f, "%d", &game.highScore);
        fclose(f);
    }
}

// ──────────────────────────────────────────────────────────
// Kill feed
// ──────────────────────────────────────────────────────────

void addKillFeed(Game& game, const std::string& text, Vec3 color) {
    KillFeedEntry entry;
    entry.text = text;
    entry.color = color;
    entry.life = 3.0f;
    game.killFeed.push_back(entry);
    // Max 5 entries
    if (game.killFeed.size() > 5) {
        game.killFeed.erase(game.killFeed.begin());
    }
}

void updateKillFeed(float dt, Game& game) {
    for (auto& entry : game.killFeed) {
        entry.life -= dt;
    }
    game.killFeed.erase(
        std::remove_if(game.killFeed.begin(), game.killFeed.end(),
            [](const KillFeedEntry& e) { return e.life <= 0.0f; }),
        game.killFeed.end());
}

void renderKillFeed(Game& game) {
    float x = -0.95f;
    float y = 0.5f;
    for (const auto& entry : game.killFeed) {
        float alpha = entry.life / 3.0f;
        Vec3 color = entry.color * alpha;
        game.text_.drawText(entry.text, x, y, 0.7f, color);
        y -= 0.06f;
    }
}

// ──────────────────────────────────────────────────────────
// Damage numbers
// ──────────────────────────────────────────────────────────

void addDamageNumber(Game& game, Vec3 pos, int damage) {
    DamageNumber dn;
    dn.pos = pos;
    dn.damage = damage;
    dn.life = 1.0f;
    dn.vy = 2.0f;
    game.damageNumbers.push_back(dn);
}

void updateDamageNumbers(float dt, Game& game) {
    for (auto& dn : game.damageNumbers) {
        dn.life -= dt;
        dn.pos.y += dn.vy * dt;
        dn.vy -= 5.0f * dt;
    }
    game.damageNumbers.erase(
        std::remove_if(game.damageNumbers.begin(), game.damageNumbers.end(),
            [](const DamageNumber& d) { return d.life <= 0.0f; }),
        game.damageNumbers.end());
}

void renderDamageNumbers(Game& game) {
    for (const auto& dn : game.damageNumbers) {
        float alpha = dn.life;
        Vec3 color = Vec3(1.0f, 0.3f, 0.0f) * alpha;
        std::string text = std::to_string(dn.damage);
        game.text_.drawText(text, dn.pos.x, dn.pos.y, 0.8f, color);
    }
}
