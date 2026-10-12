// menu_system.h - Menu navigation and item management (extracted from game.cpp)
#pragma once
#include <SDL.h>
#include <vector>
#include <string>

class Game;

// MenuSystem owns menu navigation logic (item list, selection, activation).
// State (menuItems, menuSelection) stays on Game — renderer and savegame
// read those fields directly. Same state-on-Game split as the other
// extracted systems.
class MenuSystem {
public:
    void updateItems(Game& game);
    void handleInput(Game& game, SDL_Event& event);
};
