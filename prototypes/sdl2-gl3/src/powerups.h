// powerups.h - Power-up system for neon arena prototype
#pragma once
#include "game.h"

// Spawns a power-up at the given position with the specified type
// type: 0 = health, 1 = score bonus, 2 = damage boost
void spawnPowerUp(Game& game, Vec3 pos, int type);

// Updates all power-ups: decrements life, advances rotation, removes dead
void updatePowerUps(float dt, Game& game);

// Renders all active power-ups as spinning diamonds with type-based colors
// Green = health, Gold = score, Red = damage boost
void renderPowerUps(Game& game);

// Collects the power-up at the given index, applying its effect to the player
void collectPowerUp(Game& game, int index);
