// player_system.h - Player movement and shooting (extracted from game.cpp)
#pragma once

class Game;

// PlayerSystem owns the player update *logic* (movement, look, firing).
// State stays on Game (player, keys, mouseX/Y, shootRequested, ...) —
// hud.cpp, savegame and the weapons free functions read those fields
// directly. Same state-on-Game split WaveSystem and InputSystem use.
class PlayerSystem {
public:
    void update(Game& game, float dt);
};
