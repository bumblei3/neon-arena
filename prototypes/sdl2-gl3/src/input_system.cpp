// input_system.cpp - Input handling implementation
// Extracted from Game::handleInput / handleKeyDown / handleKeyUp /
// handleMouseMotion / handleMouseButton / handlePlayingKeyDown /
// handleOptionsKeyDown in game.cpp. The input state (keys[], mouseX/Y,
// shootRequested, adsHeld, mouseSensitivity) stays on Game — updatePlayer()
// and hud.cpp read it directly.
#include "input_system.h"
#include "game.h"
#include "config.h"
#include "specials.h"
#include "weapons.h"
#include "savegame.h"
#include "replay_recorder.h"
#include "wave_editor.h"
#include <cstdio>

void InputSystem::pollEvents(Game& game) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            if (game.state == GameState::PLAYING || game.state == GameState::PAUSED) {
                SavegameManager::save(game);
            }
            game.running = false;
        } else if (event.type == SDL_KEYDOWN) {
            handleKeyDown(game, event);
        } else if (event.type == SDL_KEYUP) {
            handleKeyUp(game, event);
        } else if (event.type == SDL_MOUSEMOTION) {
            handleMouseMotion(game, event);
        } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
            handleMouseButton(game, event);
        } else if (event.type == SDL_CONTROLLERAXISMOTION) {
            if (!game.gamepad_) return;
            const int DEAD = 8000;
            int16_t v = event.caxis.value;
            if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
                game.keys[SDL_SCANCODE_A] = v < -DEAD;
                game.keys[SDL_SCANCODE_D] = v > DEAD;
            } else if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
                game.keys[SDL_SCANCODE_W] = v < -DEAD;
                game.keys[SDL_SCANCODE_S] = v > DEAD;
            } else if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
                game.mouseX += (float)v / 32767.0f * 4.0f;
            } else if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
                game.mouseY += (float)v / 32767.0f * 4.0f;
            }
        } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
            if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A) game.shootRequested = true;
            if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B) game.keys[SDL_SCANCODE_SPACE] = true;
        } else if (event.type == SDL_CONTROLLERBUTTONUP) {
            if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A) game.shootRequested = false;
            if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B) game.keys[SDL_SCANCODE_SPACE] = false;
        } else if (event.type == SDL_CONTROLLERDEVICEADDED || event.type == SDL_CONTROLLERDEVICEREMOVED) {
            CoopManager::handleDeviceEvent(event);
        }
    }
}

void InputSystem::handleKeyDown(Game& game, SDL_Event& event) {
    game.keys[event.key.keysym.scancode] = true;
    if (event.key.repeat) return;

    if (game.state == GameState::MENU) {
        game.handleMenuInput(event);
    } else if (game.state == GameState::PAUSED) {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
            game.state = GameState::PLAYING;
            SDL_SetRelativeMouseMode(SDL_TRUE);
        } else if (event.key.keysym.sym == SDLK_q) {
            game.state = GameState::MENU;
            SDL_SetRelativeMouseMode(SDL_FALSE);
            game.updateMenuItems();
        }
    } else if (game.state == GameState::PLAYING) {
        handlePlayingKeyDown(game, event);
    } else if (game.state == GameState::OPTIONS) {
        handleOptionsKeyDown(game, event);
    } else if (game.state == GameState::GAME_OVER) {
        if (event.key.keysym.sym == SDLK_SPACE) {
            game.startNewRun(game.loadout);
        } else if (event.key.keysym.sym == SDLK_ESCAPE) {
            game.state = GameState::MENU;
            SDL_SetRelativeMouseMode(SDL_FALSE);
            game.updateMenuItems();
        }
    }
}

void InputSystem::handleKeyUp(Game& game, SDL_Event& event) {
    game.keys[event.key.keysym.scancode] = false;
}

void InputSystem::handleMouseMotion(Game& game, SDL_Event& event) {
    if (game.state != GameState::PLAYING) return;
    game.mouseX = (float)event.motion.xrel;
    game.mouseY = (float)event.motion.yrel;
}

void InputSystem::handleMouseButton(Game& game, SDL_Event& event) {
    if (game.state != GameState::PLAYING) return;

    if (event.button.button == SDL_BUTTON_LEFT) {
        game.shootRequested = (event.type == SDL_MOUSEBUTTONDOWN);
    }
    if (event.button.button == SDL_BUTTON_RIGHT) {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (game.loadout == Loadout::GHOST) game.adsHeld = true;
            else game.shootLightning = true;
        } else {
            game.shootLightning = false;
            game.adsHeld = false;
        }
    }
}

void InputSystem::handlePlayingKeyDown(Game& game, SDL_Event& event) {
    if (event.key.keysym.sym == SDLK_ESCAPE) {
        game.state = GameState::PAUSED;
        SDL_SetRelativeMouseMode(SDL_FALSE);
        SavegameManager::save(game);
    }
    if (event.key.keysym.sym == SDLK_F9) {
        if (ReplayRecorder::isRecording()) {
            ReplayRecorder::stop();
            ReplayRecorder::saveToFile("replay.nrp");
        } else {
            ReplayRecorder::start();
        }
    }
    if (event.key.keysym.sym == SDLK_F10) {
        WaveEditor::isVisible() = !WaveEditor::isVisible();
        WaveEditor::renderEditor();
    }
    if (game.showUpgradeMenu) {
        handleUpgradeInput(game, event);
    } else if (event.key.keysym.sym == SDLK_SPACE && game.waveComplete) {
        game.nextWave();
    }

    // Weapon switching
    if (game.loadout == Loadout::GHOST) {
        if (event.key.keysym.sym == SDLK_1 || event.key.keysym.sym == SDLK_4) {
            game.currentWeapon = WeaponType::GHOST_SNIPER;
        }
        if (event.key.keysym.sym == SDLK_g) activateScannerSweep(game);
        if (event.key.keysym.sym == SDLK_h) activateEMPBlast(game);
        if (event.key.keysym.sym == SDLK_i || event.key.keysym.sym == SDLK_n) activateTacNuke(game);
        if (event.key.keysym.sym == SDLK_j) activateCloak(game);
    } else {
        if (event.key.keysym.sym == SDLK_1) game.currentWeapon = WeaponType::RAILGUN;
        if (event.key.keysym.sym == SDLK_2) game.currentWeapon = WeaponType::LIGHTNING_GUN;
        if (event.key.keysym.sym == SDLK_3) game.currentWeapon = WeaponType::PLASMA_RIFLE;
        if (event.key.keysym.sym == SDLK_q) game.currentWeapon = WeaponType::LIGHTNING_GUN;
        if (event.key.keysym.sym == SDLK_e) activateNuclearBlast(game);
        if (event.key.keysym.sym == SDLK_r) activateTimeSlow(game);
        if (event.key.keysym.sym == SDLK_f) activateShield(game);
    }
}

void InputSystem::handleOptionsKeyDown(Game& game, SDL_Event& event) {
    if (event.key.keysym.sym == SDLK_ESCAPE) {
        game.state = GameState::MENU;
    }
    if (event.key.keysym.sym == SDLK_LEFT && game.menuSelection == 0) {
        game.mouseSensitivity -= MOUSE_SENSITIVITY_STEP;
        if (game.mouseSensitivity < MIN_MOUSE_SENSITIVITY) game.mouseSensitivity = MIN_MOUSE_SENSITIVITY;
    }
    if (event.key.keysym.sym == SDLK_RIGHT && game.menuSelection == 0) {
        game.mouseSensitivity += MOUSE_SENSITIVITY_STEP;
        if (game.mouseSensitivity > MAX_MOUSE_SENSITIVITY) game.mouseSensitivity = MAX_MOUSE_SENSITIVITY;
    }
    if (event.key.keysym.sym == SDLK_UP) {
        game.menuSelection--;
        if (game.menuSelection < 0) game.menuSelection = 2;
    }
    if (event.key.keysym.sym == SDLK_DOWN) {
        game.menuSelection++;
        if (game.menuSelection > 2) game.menuSelection = 0;
    }
}
