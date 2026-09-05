// weapons.h - Weapon system for neon arena prototype
#pragma once
#include "game.h"

// Weapon functions - operate on Game reference
void fireRailgun(Game& game);
void fireLightning(Game& game);
void firePlasma(Game& game);
void fireGhost(Game& game);
void updateWeapons(float dt, Game& game);
void renderLightning(Game& game);
void switchWeapon(Game& game, WeaponType w);
void findLightningTargets(Vec3 pos, Game& game, std::vector<Entity*>& targets);
