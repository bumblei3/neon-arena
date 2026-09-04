#include "coop.h"
#include <cstdio>
#include <cmath>

CoopManager::GamepadState CoopManager::gamepad;
CoopManager::ReviveState CoopManager::revive;
bool CoopManager::coopActive = false;
SDL_GameController* CoopManager::controller = nullptr;

void CoopManager::init() {
    // Initialize SDL GameController subsystem
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
        printf("Warning: Failed to init GameController: %s\n", SDL_GetError());
        return;
    }
    
    // Check for connected controllers
    int numJoysticks = SDL_NumJoysticks();
    printf("Found %d joystick(s)\n", numJoysticks);
    
    for (int i = 0; i < numJoysticks; i++) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) {
                printf("Controller %d connected: %s\n", i, SDL_GameControllerName(controller));
                gamepad.connected = true;
            } else {
                printf("Failed to open controller %d: %s\n", i, SDL_GetError());
            }
        } else {
            printf("Joystick %d is not a game controller\n", i);
        }
    }
    
    printf("Coop initialized: controller %s\n", gamepad.connected ? "connected" : "not found");
}

void CoopManager::shutdown() {
    if (controller) {
        SDL_GameControllerClose(controller);
        controller = nullptr;
    }
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    coopActive = false;
}

void CoopManager::update(float dt) {
    if (!gamepad.connected) return;
    
    // Poll controller state
    gamepad.leftStickX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
    gamepad.leftStickY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
    gamepad.rightStickX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
    gamepad.rightStickY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);
    gamepad.rightTrigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
    gamepad.leftTrigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    
    // Apply deadzone
    auto applyDeadzone = [](float val, int deadzone) -> float {
        if (val > -deadzone && val < deadzone) return 0.0f;
        return val / 32767.0f;
    };
    
    gamepad.leftStickX = applyDeadzone(gamepad.leftStickX, gamepad.deadzone);
    gamepad.leftStickY = applyDeadzone(gamepad.leftStickY, gamepad.deadzone);
    gamepad.rightStickX = applyDeadzone(gamepad.rightStickX, gamepad.deadzone);
    gamepad.rightStickY = applyDeadzone(gamepad.rightStickY, gamepad.deadzone);
    gamepad.rightTrigger = gamepad.rightTrigger / 32767.0f;
    gamepad.leftTrigger = gamepad.leftTrigger / 32767.0f;
    
    // Update revive timer
    if (revive.reviving) {
        revive.reviveTimer -= dt;
        if (revive.reviveTimer <= 0.0f) {
            revive.reviving = false;
            revive.playerDown = false;
            revive.reviveTimer = 0.0f;
        }
    }
}

void CoopManager::calculateSharedCamera(Vec3& outEye, Vec3& outTarget, float& outFov,
                                       const Vec3& p1Pos, const Vec3& p2Pos,
                                       float arenaSize, float aspect) {
    // Center between both players
    float centerX = (p1Pos.x + p2Pos.x) * 0.5f;
    float centerZ = (p1Pos.z + p2Pos.z) * 0.5f;
    
    // Distance between players
    float dx = p2Pos.x - p1Pos.x;
    float dz = p2Pos.z - p1Pos.z;
    float playerDist = std::sqrt(dx * dx + dz * dz);
    
    // Camera height and distance based on player spread
    float cameraHeight = 25.0f + playerDist * 0.8f;
    float cameraDist = 35.0f + playerDist * 0.6f;
    
    // Clamp to arena
    float maxDist = arenaSize * 1.5f;
    if (cameraDist > maxDist) cameraDist = maxDist;
    
    outEye = Vec3(centerX, cameraHeight, centerZ - cameraDist);
    outTarget = Vec3(centerX, 0.0f, centerZ);
    
    // FOV widens when players are far apart
    outFov = 60.0f + playerDist * 1.5f;
    if (outFov > 90.0f) outFov = 90.0f;
}

void CoopManager::updatePlayer2(Game& game, float dt) {
    if (!gamepad.connected || !coopActive) return;
    
    // Player 2 movement (left stick)
    float moveX = gamepad.leftStickX;
    float moveZ = gamepad.leftStickY;
    
    // Normalize if needed
    float mag = std::sqrt(moveX * moveX + moveZ * moveZ);
    if (mag > 1.0f) {
        moveX /= mag;
        moveZ /= mag;
    }
    
    float speed = game.playerSpeed * 0.9f; // Slightly slower
    game.player2.pos.x += moveX * speed * dt;
    game.player2.pos.z += moveZ * speed * dt;
    
    // Player 2 aiming (right stick)
    float aimX = gamepad.rightStickX;
    float aimZ = gamepad.rightStickY;
    
    // Player 2 facing direction
    float aimMag = std::sqrt(aimX * aimX + aimZ * aimZ);
    if (aimMag > 0.3f) {
        game.player2.yaw = atan2f(aimX, -aimZ);
    }
    
    // Player 2 shooting (right trigger)
    game.player2Shoot = gamepad.rightTrigger > 0.5f;
    
    // Keep in bounds
    if (game.player2.pos.x < -game.arenaSize) game.player2.pos.x = -game.arenaSize;
    if (game.player2.pos.x > game.arenaSize) game.player2.pos.x = game.arenaSize;
    if (game.player2.pos.z < -game.arenaSize) game.player2.pos.z = -game.arenaSize;
    if (game.player2.pos.z > game.arenaSize) game.player2.pos.z = game.arenaSize;
    
    game.player2.pos.y = game.playerHeight;
}

void CoopManager::startRevive() {
    if (!revive.playerDown) return;
    revive.reviving = true;
    revive.reviveTimer = revive.reviveDuration;
}

void CoopManager::cancelRevive() {
    revive.reviving = false;
    revive.reviveTimer = 0.0f;
}

void CoopManager::completeRevive(Game& game) {
    // Revive player 1 with half health
    game.player.alive = true;
    game.player.health = game.maxHealth * 0.5f;
    revive.playerDown = false;
    revive.reviving = false;
    revive.reviveTimer = 0.0f;
    
    // Teleport to player 2
    game.player.pos = game.player2.pos + Vec3(2.0f, 0, 2.0f);
}
