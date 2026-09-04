// score.h - Score system for neon arena prototype
#pragma once
#include "game.h"

// Score / combo / multiplier
void updateScore(float dt, Game& game);
void addScore(Game& game, int points);
void saveHighScore(Game& game);
void loadHighScore(Game& game);

// Kill feed
void addKillFeed(Game& game, const std::string& text, Vec3 color);
void updateKillFeed(float dt, Game& game);
void renderKillFeed(Game& game);

// Damage numbers
void addDamageNumber(Game& game, Vec3 pos, int damage);
void updateDamageNumbers(float dt, Game& game);
void renderDamageNumbers(Game& game);
