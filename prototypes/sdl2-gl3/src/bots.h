// bots.h - Bot logic for neon arena prototype
#pragma once
#include "game.h"

void spawnWave(Game& game);
void updateBots(Game& game, float dt);
void renderBots(Game& game);
void renderSolidBots(Game& game);
void spawnExplosion(Game& game, Vec3 pos, Vec3 color, int count);
