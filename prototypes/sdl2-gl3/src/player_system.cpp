// player_system.cpp - Player movement and shooting implementation
// Extracted from Game::updatePlayer and Game::handleMouse in game.cpp.
// State stays on Game (player, keys, mouseX/Y, shootRequested, ...) —
// hud.cpp, savegame and the weapons free functions read those fields.
#include "player_system.h"
#include "game.h"
#include "config.h"
#include "weapons.h"
#include "coop.h"
#include "math.h"
#include <cmath>

void PlayerSystem::update(Game& game, float dt) {
    // Mouse look (was Game::handleMouse)
    if (fabsf(game.mouseX) > 80.0f) game.mouseX = 0.0f;
    if (fabsf(game.mouseY) > 80.0f) game.mouseY = 0.0f;
    if (game.mouseX != 0 || game.mouseY != 0) {
        game.player.yaw += game.mouseX * game.mouseSensitivity;
        game.player.pitch -= game.mouseY * game.mouseSensitivity;
        game.player.pitch = fmaxf(-1.2f, fminf(1.2f, game.player.pitch));
    }
    game.mouseX = 0;
    game.mouseY = 0;

    // Aim assist for gamepad: nudge toward nearest alive bot within view
    if (CoopManager::getGamepad().connected) {
        const float ASSIST = 0.04f;
        const float MAX_ANGLE = 0.35f;
        float best = MAX_ANGLE;
        Vec3 target(0,0,0);
        bool found = false;
        for (auto& b : game.bots) {
            if (!b.alive) continue;
            float dx = b.pos.x - game.player.pos.x;
            float dz = b.pos.z - game.player.pos.z;
            float dy = b.pos.y - game.player.pos.y;
            float dist = sqrtf(dx*dx + dz*dz);
            if (dist < 0.1f) continue;
            float targetYaw = atan2f(dx, -dz);
            float diff = targetYaw - game.player.yaw;
            while (diff > 3.14159f) diff -= 6.28318f;
            while (diff < -3.14159f) diff += 6.28318f;
            if (fabsf(diff) < best) {
                best = fabsf(diff);
                target = b.pos;
                found = true;
            }
        }
        if (found) {
            float dx = target.x - game.player.pos.x;
            float dz = target.z - game.player.pos.z;
            float dy = target.y - game.player.pos.y;
            float dist = sqrtf(dx*dx + dz*dz);
            float targetYaw = atan2f(dx, -dz);
            float targetPitch = atan2f(dy, dist);
            float yawDiff = targetYaw - game.player.yaw;
            while (yawDiff > 3.14159f) yawDiff -= 6.28318f;
            while (yawDiff < -3.14159f) yawDiff += 6.28318f;
            game.player.yaw += yawDiff * ASSIST;
            game.player.pitch += (targetPitch - game.player.pitch) * ASSIST;
            game.player.pitch = fmaxf(-1.2f, fminf(1.2f, game.player.pitch));
        }
    }

    float moveX = 0, moveZ = 0;
    if (game.keys[SDL_SCANCODE_W]) moveZ -= 1;
    if (game.keys[SDL_SCANCODE_S]) moveZ += 1;
    if (game.keys[SDL_SCANCODE_A]) moveX -= 1;
    if (game.keys[SDL_SCANCODE_D]) moveX += 1;

    // Solo pad: left stick moves with the facing, right stick looks.
    // Coop keeps the pad on player 2.
    if (!game.coopActive) {
        CoopManager::update(dt);
        const CoopManager::GamepadState& pad = CoopManager::getGamepad();
        if (pad.connected) {
            float fwdX = sinf(game.player.yaw);
            float fwdZ = -cosf(game.player.yaw);
            float rightX = cosf(game.player.yaw);
            float rightZ = sinf(game.player.yaw);
            moveX += rightX * pad.leftStickX - fwdX * pad.leftStickY;
            moveZ += rightZ * pad.leftStickX - fwdZ * pad.leftStickY;
            game.player.yaw += pad.rightStickX * PAD_LOOK_SPEED * dt;
            game.player.pitch -= pad.rightStickY * PAD_LOOK_SPEED * dt;
            game.player.pitch = fmaxf(-1.2f, fminf(1.2f, game.player.pitch));
        }
    }

    float speedMult = 1.0f;
    if (game.cloakTimer > 0.0f) speedMult = game.cloakSpeedBoost;

    if (moveX != 0 || moveZ != 0) {
        Vec3 moveDir = Vec3(moveX, 0, moveZ).normalized();
        game.player.pos.x += moveDir.x * game.playerSpeed * dt * speedMult;
        game.player.pos.z += moveDir.z * game.playerSpeed * dt * speedMult;
    }

    game.player.pos.y = game.playerHeight;

    if (game.player.pos.x < -game.arenaSize) game.player.pos.x = -game.arenaSize;
    if (game.player.pos.x > game.arenaSize) game.player.pos.x = game.arenaSize;
    if (game.player.pos.z < -game.arenaSize) game.player.pos.z = -game.arenaSize;
    if (game.player.pos.z > game.arenaSize) game.player.pos.z = game.arenaSize;

    if (game.shootRequested && game.currentWeapon == WeaponType::RAILGUN) {
        fireRailgun(game);
    }
    if (game.shootLightning && game.currentWeapon == WeaponType::LIGHTNING_GUN) {
        fireLightning(game);
    }
    if (game.shootRequested && game.currentWeapon == WeaponType::PLASMA_RIFLE) {
        firePlasma(game);
    }
    if (game.shootRequested && game.currentWeapon == WeaponType::GHOST_SNIPER && game.loadout == Loadout::GHOST) {
        fireGhost(game);
    }

    // Player 2 shooting (coop)
    if (game.coopActive && game.player2Shoot && game.player2ShootCooldown <= 0.0f) {
        // Player 2 fires railgun in facing direction
        Vec3 dir(
            sinf(game.player2.yaw),
            0.0f,
            -cosf(game.player2.yaw)
        );
        Vec3 muzzlePos = game.player2.pos + Vec3(0, 1.0f, 0);
        game.projectiles.push_back(Projectile(muzzlePos, dir, true, WeaponType::RAILGUN, 15.0f));
        game.player2ShootCooldown = RAILGUN_FIRE_RATE;
    }
    game.player2ShootCooldown -= dt;
    if (game.player2ShootCooldown < 0.0f) game.player2ShootCooldown = 0.0f;
}
