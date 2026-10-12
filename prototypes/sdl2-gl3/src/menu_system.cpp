// menu_system.cpp - Menu navigation implementation
// Extracted from Game::updateMenuItems and Game::handleMenuInput in game.cpp.
// State (menuItems, menuSelection) stays on Game.
#include "menu_system.h"
#include "game.h"
#include "savegame.h"

void MenuSystem::updateItems(Game& game) {
    game.menuItems.clear();
    if (SavegameManager::exists()) {
        game.menuItems.push_back("Continue");
        game.menuItems.push_back("New Arena");
        game.menuItems.push_back("New Ghost");
        game.menuItems.push_back("Sensitivity");
        game.menuItems.push_back("Quit");
    } else {
        game.menuItems.push_back("Start Arena");
        game.menuItems.push_back("Start Ghost");
        game.menuItems.push_back("Sensitivity");
        game.menuItems.push_back("Quit");
    }
}

void MenuSystem::handleInput(Game& game, SDL_Event& event) {
    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) {
        game.menuSelection--;
        if (game.menuSelection < 0) game.menuSelection = (int)game.menuItems.size() - 1;
    } else if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) {
        game.menuSelection++;
        if (game.menuSelection >= (int)game.menuItems.size()) game.menuSelection = 0;
    } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
        bool hasSave = SavegameManager::exists();
        int continueIdx = hasSave ? 0 : -1;
        int arenaIdx = hasSave ? 1 : 0;
        int ghostIdx = hasSave ? 2 : 1;
        int optionsIdx = hasSave ? 3 : 2;
        int quitIdx = hasSave ? 4 : 3;

        if (game.menuSelection == continueIdx && continueIdx >= 0) {
            if (SavegameManager::load(game)) {
                game.loadout = (game.currentWeapon == WeaponType::GHOST_SNIPER)
                    ? Loadout::GHOST : Loadout::ARENA;
                game.adsHeld = false;
                game.state = GameState::PLAYING;
                SDL_SetRelativeMouseMode(SDL_TRUE);
            }
        } else if (game.menuSelection == arenaIdx) {
            if (hasSave) SavegameManager::remove();
            game.startNewRun(Loadout::ARENA);
        } else if (game.menuSelection == ghostIdx) {
            if (hasSave) SavegameManager::remove();
            game.startNewRun(Loadout::GHOST);
        } else if (game.menuSelection == optionsIdx) {
            game.state = GameState::OPTIONS;
        } else if (game.menuSelection == quitIdx) {
            game.running = false;
        }
    }
}
