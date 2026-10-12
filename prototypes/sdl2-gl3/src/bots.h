// bots.h - Bot logic for neon arena prototype
// Wave spawning moved to wave_system.h (WaveSystem); bots.h keeps the
// update/render free functions.
#pragma once
#include "game.h"

void updateBots(Game& game, float dt);
void renderBots(Game& game);
void renderSolidBots(Game& game);
void spawnExplosion(Game& game, Vec3 pos, Vec3 color, int count);
