// input_system.h - Input handling (extracted from game.cpp)
#pragma once
#include <SDL.h>

class Game;

// InputSystem owns the input *logic* (event dispatch + key/mouse/gamepad
// handlers). The input *state* (keys[], mouseX/Y, shootRequested, adsHeld,
// mouseSensitivity) stays on Game — updatePlayer() and hud.cpp read those
// fields directly, so moving the state would force changes across files.
// WaveSystem used the same split.
class InputSystem {
public:
    void pollEvents(Game& game);
    void handleKeyDown(Game& game, SDL_Event& event);
    void handleKeyUp(Game& game, SDL_Event& event);
    void handleMouseMotion(Game& game, SDL_Event& event);
    void handleMouseButton(Game& game, SDL_Event& event);
    void handlePlayingKeyDown(Game& game, SDL_Event& event);
    void handleOptionsKeyDown(Game& game, SDL_Event& event);
};
