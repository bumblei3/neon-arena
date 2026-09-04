// coop.h - Coop mode: shared screen, gamepad support, revive system
#pragma once
#include "game.h"

class CoopManager {
public:
    struct GamepadState {
        float leftStickX = 0, leftStickY = 0;    // Movement
        float rightStickX = 0, rightStickY = 0;  // Aiming
        float rightTrigger = 0;                  // Fire
        float leftTrigger = 0;                   // Alt fire
        bool connected = false;
        int deadzone = 8000;
    };
    
    struct ReviveState {
        bool playerDown = false;
        bool reviving = false;
        float reviveTimer = 0.0f;
        float reviveDuration = 5.0f;
        float reviveRange = 5.0f;
    };

    static void init();
    static void shutdown();
    static void update(float dt);
    
    static GamepadState& getGamepad() { return gamepad; }
    static ReviveState& getRevive() { return revive; }
    static bool isCoopActive() { return coopActive; }
    static void setCoopActive(bool active) { coopActive = active; }
    
    // Shared screen camera
    static void calculateSharedCamera(Vec3& outEye, Vec3& outTarget, float& outFov,
                                     const Vec3& player1Pos, const Vec3& player2Pos,
                                     float arenaSize, float aspect);
    
    // Player 2 input handling
    static void updatePlayer2(Game& game, float dt);
    
    // Revive system
    static void startRevive();
    static void cancelRevive();
    static void completeRevive(Game& game);
    
private:
    static GamepadState gamepad;
    static ReviveState revive;
    static bool coopActive;
    static SDL_GameController* controller;
};
