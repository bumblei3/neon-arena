// input_system.cpp - Input handling implementation
#include "input_system.h"
#include "game.h"
#include "config.h"
#include <cstdio>
#include <cmath>

InputSystem::InputSystem()
    : shoot_requested_(false), shoot_lightning_(false), ads_held_(false),
      mouse_delta_x_(0), mouse_delta_y_(0),
      mouse_sensitivity_(DEFAULT_MOUSE_SENSITIVITY),
      player_yaw_(0), player_pitch_(0),
      gamepad_(nullptr), deadzone_(COOP_DEADZONE),
      ctrl_move_x_(0), ctrl_move_y_(0), ctrl_look_x_(0), ctrl_look_y_(0), ctrl_trigger_(0) {
    memset(keys_, 0, sizeof(keys_));
}

void InputSystem::update(Game& game, float dt) {
    // Continuous input (held keys)
    (void)dt;
    
    if (!game.player.alive) return;
    
    // Movement
    float moveX = 0.0f, moveZ = 0.0f;
    if (isKeyHeld(SDL_SCANCODE_W)) moveZ -= 1.0f;
    if (isKeyHeld(SDL_SCANCODE_S)) moveZ += 1.0f;
    if (isKeyHeld(SDL_SCANCODE_A)) moveX -= 1.0f;
    if (isKeyHeld(SDL_SCANCODE_D)) moveX += 1.0f;
    
    // Apply movement
    float speed = game.playerSpeed;
    if (moveX != 0.0f || moveZ != 0.0f) {
        float len = sqrtf(moveX * moveX + moveZ * moveZ);
        moveX /= len;
        moveZ /= len;
        game.player.pos.x += moveX * speed * dt;
        game.player.pos.z += moveZ * speed * dt;
    }
    
    // Clamp to arena bounds
    float halfArena = game.arenaSize * 0.5f;
    if (game.player.pos.x < -halfArena) game.player.pos.x = -halfArena;
    if (game.player.pos.x > halfArena) game.player.pos.x = halfArena;
    if (game.player.pos.z < -halfArena) game.player.pos.z = -halfArena;
    if (game.player.pos.z > halfArena) game.player.pos.z = halfArena;
    
    // Apply mouse look
    game.player.yaw += mouse_delta_x_ * mouse_sensitivity_;
    game.player.pitch += mouse_delta_y_ * mouse_sensitivity_;
    
    // Clamp pitch
    constexpr float MAX_PITCH = 1.5f;
    if (game.player.pitch > MAX_PITCH) game.player.pitch = MAX_PITCH;
    if (game.player.pitch < -MAX_PITCH) game.player.pitch = -MAX_PITCH;
    
    // Reset per-frame mouse delta
    mouse_delta_x_ = 0.0f;
    mouse_delta_y_ = 0.0f;
    
    // Handle continuous fire (lightning gun)
    if (shoot_lightning_ && game.lightningCooldown <= 0.0f) {
        fireLightning(game);
    }
}

void InputSystem::handleEvent(Game& game, SDL_Event& event) {
    switch (event.type) {
        case SDL_KEYDOWN:
            handleKeyDown(game, event);
            break;
        case SDL_KEYUP:
            handleKeyUp(game, event);
            break;
        case SDL_MOUSEMOTION:
            handleMouseMotion(game, event);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            handleMouseButton(game, event);
            break;
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            handleControllerButton(game, event);
            break;
        case SDL_CONTROLLERAXISMOTION:
            handleControllerAxis(game, event);
            break;
    }
}

void InputSystem::handleKeyDown(Game& game, SDL_Event& event) {
    if (event.key.repeat) return;
    
    setKey(event.key.keysym.scancode, true);
    
    switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
            if (game.state == GameState::PLAYING) {
                game.state = GameState::PAUSED;
            } else if (game.state == GameState::PAUSED) {
                game.state = GameState::PLAYING;
            }
            break;
            
        case SDLK_SPACE:
            if (game.waveComplete) {
                game.nextWave();
            } else if (game.gameOver) {
                game.resetGame();
            }
            break;
            
        case SDLK_1:
            switchWeapon(game, WeaponType::RAILGUN);
            break;
        case SDLK_2:
            switchWeapon(game, WeaponType::LIGHTNING_GUN);
            break;
        case SDLK_3:
            switchWeapon(game, WeaponType::PLASMA_RIFLE);
            break;
            
        case SDLK_q:
            if (game.loadout == Loadout::GHOST) {
                activateCloak(game);
            }
            break;
            
        case SDLK_e:
            if (game.loadout == Loadout::ARENA) {
                activateNuclearBlast(game);
            } else if (game.loadout == Loadout::GHOST) {
                activateScannerSweep(game);
            }
            break;
            
        case SDLK_r:
            if (game.loadout == Loadout::ARENA) {
                activateTimeSlow(game);
            } else if (game.loadout == Loadout::GHOST) {
                activateEMPBlast(game);
            }
            break;
            
        case SDLK_f:
            if (game.loadout == Loadout::ARENA) {
                activateShield(game);
            } else if (game.loadout == Loadout::GHOST) {
                activateTacNuke(game);
            }
            break;
            
        case SDLK_TAB:
            game.showUpgradeMenu = !game.showUpgradeMenu;
            break;
            
        default:
            break;
    }
}

void InputSystem::handleKeyUp(Game& game, SDL_Event& event) {
    setKey(event.key.keysym.scancode, false);
    (void)game;
}

void InputSystem::handleMouseMotion(Game& game, SDL_Event& event) {
    if (game.state != GameState::PLAYING) return;
    mouse_delta_x_ = (float)event.motion.xrel;
    mouse_delta_y_ = (float)event.motion.yrel;
}

void InputSystem::handleMouseButton(Game& game, SDL_Event& event) {
    if (game.state != GameState::PLAYING) return;
    
    if (event.button.button == SDL_BUTTON_LEFT) {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            shoot_requested_ = true;
        }
    } else if (event.button.button == SDL_BUTTON_RIGHT) {
        ads_held_ = (event.type == SDL_MOUSEBUTTONDOWN);
    }
}

void InputSystem::handleControllerButton(Game& game, SDL_Event& event) {
    (void)game;
    // TODO: Map controller buttons
}

void InputSystem::handleControllerAxis(Game& game, SDL_Event& event) {
    (void)game;
    // TODO: Map controller axes
}

bool InputSystem::isKeyPressed(SDL_Scancode key) const {
    return keys_[key];
}

bool InputSystem::isKeyHeld(SDL_Scancode key) const {
    return keys_[key];
}

void InputSystem::setKey(SDL_Scancode key, bool pressed) {
    if (key >= 0 && key < SDL_NUM_SCANCODES) {
        keys_[key] = pressed;
    }
}

void InputSystem::openGamepad() {
    if (gamepad_) return;
    
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            gamepad_ = SDL_GameControllerOpen(i);
            if (gamepad_) {
                printf("[Input] Gamepad opened: %s\n", SDL_GameControllerName(gamepad_));
                break;
            }
        }
    }
}

void InputSystem::closeGamepad() {
    if (gamepad_) {
        SDL_GameControllerClose(gamepad_);
        gamepad_ = nullptr;
    }
}

void InputSystem::pollGamepad(Game& game, float dt) {
    if (!gamepad_) return;
    
    // Read sticks
    ctrl_move_x_ = (float)SDL_GameControllerGetAxis(gamepad_, SDL_CONTROLLER_AXIS_LEFTX) / 32768.0f;
    ctrl_move_y_ = (float)SDL_GameControllerGetAxis(gamepad_, SDL_CONTROLLER_AXIS_LEFTY) / 32768.0f;
    ctrl_look_x_ = (float)SDL_GameControllerGetAxis(gamepad_, SDL_CONTROLLER_AXIS_RIGHTX) / 32768.0f;
    ctrl_look_y_ = (float)SDL_GameControllerGetAxis(gamepad_, SDL_CONTROLLER_AXIS_RIGHTY) / 32768.0f;
    ctrl_trigger_ = (float)SDL_GameControllerGetAxis(gamepad_, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32768.0f;
    
    // Apply deadzone
    auto applyDeadzone = [this](float& x, float& y) {
        float mag = sqrtf(x * x + y * y);
        if (mag < (float)deadzone_ / 32768.0f) {
            x = 0.0f;
            y = 0.0f;
        }
    };
    applyDeadzone(ctrl_move_x_, ctrl_move_y_);
    applyDeadzone(ctrl_look_x_, ctrl_look_y_);
    
    // Apply to game
    if (game.state == GameState::PLAYING && game.player.alive) {
        // Movement
        if (ctrl_move_x_ != 0.0f || ctrl_move_y_ != 0.0f) {
            game.player.pos.x += ctrl_move_x_ * game.playerSpeed * dt;
            game.player.pos.z += ctrl_move_y_ * game.playerSpeed * dt;
        }
        
        // Look
        game.player.yaw += ctrl_look_x_ * mouse_sensitivity_ * 2.0f * dt;
        game.player.pitch -= ctrl_look_y_ * mouse_sensitivity_ * 2.0f * dt;
        
        // Clamp pitch
        constexpr float MAX_PITCH = 1.5f;
        if (game.player.pitch > MAX_PITCH) game.player.pitch = MAX_PITCH;
        if (game.player.pitch < -MAX_PITCH) game.player.pitch = -MAX_PITCH;
        
        // Trigger fire
        if (ctrl_trigger_ > 0.5f) {
            shoot_requested_ = true;
        }
    }
}
