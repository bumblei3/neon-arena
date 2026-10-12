// input_system.h - Handles all player input (keyboard, mouse, gamepad)
#pragma once
#include <SDL.h>
#include "config.h"

class Game;

class InputSystem {
public:
    InputSystem();
    
    void update(Game& game, float dt);
    void handleEvent(Game& game, SDL_Event& event);
    
    // Input state queries
    bool isShootRequested() const { return shoot_requested_; }
    bool isShootLightning() const { return shoot_lightning_; }
    bool isADSHeld() const { return ads_held_; }
    float mouseDeltaX() const { return mouse_delta_x_; }
    float mouseDeltaY() const { return mouse_delta_y_; }
    
    void clearShootRequest() { shoot_requested_ = false; }
    void clearLightning() { shoot_lightning_ = false; }
    
    // Mouse state
    float mouseSensitivity() const { return mouse_sensitivity_; }
    void decreaseSensitivity() { 
        mouse_sensitivity_ -= MOUSE_SENSITIVITY_STEP;
        if (mouse_sensitivity_ < MIN_MOUSE_SENSITIVITY) 
            mouse_sensitivity_ = MIN_MOUSE_SENSITIVITY;
    }
    void increaseSensitivity() { 
        mouse_sensitivity_ += MOUSE_SENSITIVITY_STEP;
        if (mouse_sensitivity_ > MAX_MOUSE_SENSITIVITY) 
            mouse_sensitivity_ = MAX_MOUSE_SENSITIVITY;
    }
    
    // Key state
    bool isKeyPressed(SDL_Scancode key) const;
    bool isKeyHeld(SDL_Scancode key) const;
    void setKey(SDL_Scancode key, bool pressed);
    
    // Gamepad
    bool isGamepadConnected() const { return gamepad_ != nullptr; }
    void openGamepad();
    void closeGamepad();
    void pollGamepad(Game& game, float dt);
    
private:
    void handleKeyDown(Game& game, SDL_Event& event);
    void handleKeyUp(Game& game, SDL_Event& event);
    void handleMouseMotion(Game& game, SDL_Event& event);
    void handleMouseButton(Game& game, SDL_Event& event);
    void handleControllerButton(Game& game, SDL_Event& event);
    void handleControllerAxis(Game& game, SDL_Event& event);
    
    // State
    bool keys_[SDL_NUM_SCANCODES];
    bool shoot_requested_;
    bool shoot_lightning_;
    bool ads_held_;
    float mouse_delta_x_;
    float mouse_delta_y_;
    float mouse_sensitivity_;
    float player_yaw_;
    float player_pitch_;
    
    // Gamepad
    SDL_GameController* gamepad_;
    int deadzone_;
    
    // Controller state
    float ctrl_move_x_, ctrl_move_y_;
    float ctrl_look_x_, ctrl_look_y_;
    float ctrl_trigger_;
};
