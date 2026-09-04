// hud.h - HUD rendering functions for neon arena prototype
#pragma once
#include "game.h"

void renderHUD(Game& game);
void renderMinimap(Game& game);
void renderUpgradeMenu(Game& game);
void handleUpgradeInput(Game& game, SDL_Event& event);
void applyUpgrade(Game& game, int selection);
void resetUpgrades(Game& game);
