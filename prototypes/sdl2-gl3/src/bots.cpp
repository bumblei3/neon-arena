// bots.cpp - Bot logic implementation for neon arena prototype
#include "game.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

void spawnWave(Game& game) {
    game.wave++;
    game.bots.clear();

    // Boss wave every 5 waves
    if (game.wave % 5 == 0) {
        // Boss wave - 1 strong boss + a few minions
        int botCount = 1 + game.wave / 5;  // Boss + minions
        for (int i = 0; i < botCount; i++) {
            Entity bot;
            float angle = (float)i / botCount * 6.28318f;
            float radius = game.arenaSize * 0.6f;
            bot.pos = Vec3(
                cosf(angle) * radius,
                0.5f,
                sinf(angle) * radius
            );
            bot.yaw = 0;
            bot.pitch = 0;
            bot.alive = true;
            bot.type = 1;

            if (i == 0) {
                // Boss
                bot.botType = 4;
                bot.isBoss = true;
                bot.health = 500.0f + game.wave * 50;
                bot.moveSpeed = 2.0f;
                bot.attackCooldown = 0;
            } else {
                // Minions
                bot.botType = 0;
                bot.health = 50.0f + game.wave * 5;
                bot.moveSpeed = 4.0f;
            }
            game.bots.push_back(bot);
        }
        printf("BOSS WAVE %d: Boss + %d minions!\n", game.wave, botCount - 1);
    } else {
        // Normal wave
        int botCount = game.wave + 1;
        for (int i = 0; i < botCount; i++) {
            Entity bot;
            float angle = (float)i / botCount * 6.28318f;
            float radius = game.arenaSize * 0.7f;
            bot.pos = Vec3(
                cosf(angle) * radius,
                0.5f,
                sinf(angle) * radius
            );
            bot.yaw = 0;
            bot.pitch = 0;
            bot.alive = true;
            bot.type = 1;

            if (game.wave >= 3 && i == 0) {
                bot.botType = 2;
                bot.health = 200.0f + game.wave * 20;
                bot.moveSpeed = 1.5f;
            } else if (game.wave >= 2 && i == botCount - 1) {
                bot.botType = 3;
                bot.health = 50.0f + game.wave * 5;
                bot.moveSpeed = 6.0f;
            } else if (game.wave >= 4 && i % 3 == 1) {
                bot.botType = 1;
                bot.health = 80.0f + game.wave * 8;
                bot.moveSpeed = 2.5f;
            } else {
                bot.botType = 0;
                bot.health = 100.0f + game.wave * 10;
                bot.moveSpeed = 3.0f;
            }
            game.bots.push_back(bot);
        }
        printf("Wave %d: %d bots spawned\n", game.wave, botCount);
    }
    game.waveComplete = false;
}

void updateBots(Game& game, float dt) {
    for (auto& bot : game.bots) {
        if (!bot.alive) continue;

        // Move towards player
        Vec3 toPlayer = Vec3(
            game.player.pos.x - bot.pos.x,
            0,
            game.player.pos.z - bot.pos.z
        );

        float dist = toPlayer.length();

        // Different behavior based on bot type
        float preferredDist = 3.0f;
        if (bot.botType == 1) preferredDist = 12.0f;
        if (bot.botType == 3) preferredDist = 2.0f;
        if (bot.botType == 4) preferredDist = 5.0f;  // Boss keeps medium distance

        if (dist > preferredDist) {
            toPlayer = toPlayer.normalized();
            bot.pos.x += toPlayer.x * bot.moveSpeed * dt;
            bot.pos.z += toPlayer.z * bot.moveSpeed * dt;
        } else if (bot.botType == 1 && dist < preferredDist - 2.0f) {
            toPlayer = toPlayer.normalized();
            bot.pos.x -= toPlayer.x * bot.moveSpeed * 0.5f * dt;
            bot.pos.z -= toPlayer.z * bot.moveSpeed * 0.5f * dt;
        }

        // Rotate towards player
        bot.yaw = atan2f(toPlayer.x, -toPlayer.z);

        // Hover animation
        float hoverOffset = sinf(game.gameTime * 2.0f + bot.pos.x * 0.1f + bot.pos.z * 0.1f) * 0.3f;
        if (bot.botType == 4) hoverOffset *= 2.0f;  // Boss hovers more
        bot.pos.y = hoverOffset;

        // Keep bots in bounds
        if (bot.pos.x < -game.arenaSize + 2) bot.pos.x = -game.arenaSize + 2;
        if (bot.pos.x > game.arenaSize - 2) bot.pos.x = game.arenaSize - 2;
        if (bot.pos.z < -game.arenaSize + 2) bot.pos.z = -game.arenaSize + 2;
        if (bot.pos.z > game.arenaSize - 2) bot.pos.z = game.arenaSize - 2;

        // Bot attacks
        bot.attackCooldown -= dt;
        if (bot.botType == 0 || bot.botType == 2 || bot.botType == 3) {
            if (dist < 4.0f && bot.attackCooldown <= 0.0f) {
                game.player.health -= 10.0f;
                bot.attackCooldown = 1.0f;
            }
        }
        if (bot.botType == 1) {
            if (dist < 20.0f && bot.attackCooldown <= 0.0f) {
                Vec3 toPlayerDir = Vec3(
                    game.player.pos.x - bot.pos.x,
                    0,
                    game.player.pos.z - bot.pos.z
                ).normalized();
                Vec3 muzzlePos = bot.pos + Vec3(0, 1.0f, 0);
                game.projectiles.push_back(Projectile(muzzlePos, toPlayerDir, false, WeaponType::RAILGUN, 15.0f));
                bot.attackCooldown = 2.0f;
            }
        }
        if (bot.botType == 4) {
            // Boss: multiple attack patterns
            if (bot.attackCooldown <= 0.0f) {
                bot.bossPhase += 1.0f;
                if (bot.bossPhase > 3.0f) bot.bossPhase = 0.0f;

                if (bot.bossPhase < 1.0f) {
                    // Spread shot - 5 projectiles in a fan
                    for (int i = 0; i < 5; i++) {
                        float angle = (i - 2) * 0.3f;
                        Vec3 dir = Vec3(sinf(angle), 0, -cosf(angle));
                        Vec3 muzzlePos = bot.pos + Vec3(0, 2.0f, 0);
                        game.projectiles.push_back(Projectile(muzzlePos, dir, false, WeaponType::RAILGUN, 20.0f));
                    }
                    bot.attackCooldown = 3.0f;
                } else {
                    // Aimed shot at player
                    Vec3 toPlayerDir = Vec3(
                        game.player.pos.x - bot.pos.x,
                        0,
                        game.player.pos.z - bot.pos.z
                    ).normalized();
                    Vec3 muzzlePos = bot.pos + Vec3(0, 2.0f, 0);
                    game.projectiles.push_back(Projectile(muzzlePos, toPlayerDir, false, WeaponType::RAILGUN, 30.0f));
                    bot.attackCooldown = 2.0f;
                }
            }
        }
    }
}

void renderSolidBots(Game& game) {
    for (auto& bot : game.bots) {
        if (!bot.alive) continue;

        float pulse = 1.0f + sinf(game.gameTime * 4.0f + bot.pos.x * 0.5f) * 0.15f;
        float s = 0.8f * pulse;
        float sizeMult = 1.0f;
        if (bot.botType == 4) sizeMult = 2.5f;  // Boss ist größer
        s *= sizeMult;

        float healthPct = bot.health / (100.0f + game.wave * 10);
        if (healthPct > 1.0f) healthPct = 1.0f;
        if (healthPct < 0.0f) healthPct = 0.0f;

        // Verschiedene Farben für verschiedene Bot-Typen
        Vec3 botColor;
        switch (bot.botType) {
            case 0: // Melee - grün/cyan
                botColor = Vec3((1.0f - healthPct) * 0.8f, healthPct * 1.0f, healthPct * 0.5f);
                break;
            case 1: // Shooter - orange/gelb
                botColor = Vec3(1.0f, 0.5f + healthPct * 0.5f, (1.0f - healthPct) * 0.3f);
                break;
            case 2: // Tank - lila/magenta
                botColor = Vec3(0.6f + healthPct * 0.4f, (1.0f - healthPct) * 0.3f, 0.8f);
                break;
            case 3: // Fast - rot/pink
                botColor = Vec3(1.0f, (1.0f - healthPct) * 0.5f, (1.0f - healthPct) * 0.3f);
                break;
            case 4: // Boss - gold/rot
                botColor = Vec3(1.0f, 0.3f + healthPct * 0.4f, 0.0f);
                break;
            default:
                botColor = Vec3((1.0f - healthPct) * 0.8f, healthPct * 1.0f, healthPct * 0.5f);
                break;
        }

        float rotOffset = game.gameTime * 1.5f + bot.pos.x * 0.3f;
        float cosR = cosf(rotOffset);
        float sinR = sinf(rotOffset);

        // Icosahedron-ähnliche Form (12 Ecken, 20 Dreiecke)
        float phi = 1.6180339887f;  // Golden ratio
        float h = s * phi;

        // Obere Pyramide
        Vec3 top(bot.pos.x, bot.pos.y + h, bot.pos.z);
        Vec3 mid1(bot.pos.x - s * cosR, bot.pos.y, bot.pos.z - s * sinR);
        Vec3 mid2(bot.pos.x + s * sinR, bot.pos.y, bot.pos.z - s * cosR);
        Vec3 mid3(bot.pos.x + s * cosR, bot.pos.y, bot.pos.z + s * sinR);
        Vec3 mid4(bot.pos.x - s * sinR, bot.pos.y, bot.pos.z + s * cosR);
        Vec3 bottom(bot.pos.x, bot.pos.y - h * 0.6f, bot.pos.z);

        // Obere Dreiecke (4)
        Vertex tri1[] = { Vertex(top, Vec3(0,1,0), botColor), Vertex(mid1, Vec3(-1,0.5f,-1), botColor), Vertex(mid2, Vec3(1,0.5f,-1), botColor) };
        game.renderer_->drawTriangles(tri1, 3, botColor);
        Vertex tri2[] = { Vertex(top, Vec3(0,1,0), botColor), Vertex(mid2, Vec3(1,0.5f,-1), botColor), Vertex(mid3, Vec3(1,0.5f,1), botColor) };
        game.renderer_->drawTriangles(tri2, 3, botColor);
        Vertex tri3[] = { Vertex(top, Vec3(0,1,0), botColor), Vertex(mid3, Vec3(1,0.5f,1), botColor), Vertex(mid4, Vec3(-1,0.5f,1), botColor) };
        game.renderer_->drawTriangles(tri3, 3, botColor);
        Vertex tri4[] = { Vertex(top, Vec3(0,1,0), botColor), Vertex(mid4, Vec3(-1,0.5f,1), botColor), Vertex(mid1, Vec3(-1,0.5f,-1), botColor) };
        game.renderer_->drawTriangles(tri4, 3, botColor);

        // Untere Dreiecke (4)
        Vertex tri5[] = { Vertex(bottom, Vec3(0,-1,0), botColor), Vertex(mid1, Vec3(-1,-0.5f,-1), botColor), Vertex(mid2, Vec3(1,-0.5f,-1), botColor) };
        game.renderer_->drawTriangles(tri5, 3, botColor);
        Vertex tri6[] = { Vertex(bottom, Vec3(0,-1,0), botColor), Vertex(mid2, Vec3(1,-0.5f,-1), botColor), Vertex(mid3, Vec3(1,-0.5f,1), botColor) };
        game.renderer_->drawTriangles(tri6, 3, botColor);
        Vertex tri7[] = { Vertex(bottom, Vec3(0,-1,0), botColor), Vertex(mid3, Vec3(1,-0.5f,1), botColor), Vertex(mid4, Vec3(-1,-0.5f,1), botColor) };
        game.renderer_->drawTriangles(tri7, 3, botColor);
        Vertex tri8[] = { Vertex(bottom, Vec3(0,-1,0), botColor), Vertex(mid4, Vec3(-1,-0.5f,1), botColor), Vertex(mid1, Vec3(-1,-0.5f,-1), botColor) };
        game.renderer_->drawTriangles(tri8, 3, botColor);
    }
}

void renderBots(Game& game) {
    // Render solid bots first (with lighting)
    renderSolidBots(game);

    for (auto& bot : game.bots) {
        if (!bot.alive) continue;

        // Draw wireframe overlay for glow effect
        // Pulsating size based on game time
        float pulse = 1.0f + sinf(game.gameTime * 4.0f + bot.pos.x * 0.5f) * 0.15f;
        float sizeMult = 1.0f;
        if (bot.botType == 4) sizeMult = 2.5f;
        float s = 0.8f * pulse * sizeMult;  // Size with pulse

        // Color based on health (green-cyan when healthy, red when damaged)
        float healthPct = bot.health / (100.0f + game.wave * 10);
        Vec3 botColor(
            (1.0f - healthPct) * 0.8f,
            healthPct * 1.0f,
            healthPct * 0.5f
        );

        // Rotation offset based on game time
        float rotOffset = game.gameTime * 1.5f + bot.pos.x * 0.3f;
        float cosR = cosf(rotOffset);
        float sinR = sinf(rotOffset);

        // Top pyramid (rotated)
        Vertex top[] = {
            Vertex(Vec3(bot.pos.x, bot.pos.y + s * 2, bot.pos.z), botColor),
            Vertex(Vec3(bot.pos.x - s * cosR, bot.pos.y, bot.pos.z - s * sinR), botColor),
            Vertex(Vec3(bot.pos.x + s * sinR, bot.pos.y, bot.pos.z - s * cosR), botColor),
        };
        game.renderer_->drawLineLoop(top, 3, botColor);

        Vertex top2[] = {
            Vertex(Vec3(bot.pos.x, bot.pos.y + s * 2, bot.pos.z), botColor),
            Vertex(Vec3(bot.pos.x + s * sinR, bot.pos.y, bot.pos.z - s * cosR), botColor),
            Vertex(Vec3(bot.pos.x + s * cosR, bot.pos.y, bot.pos.z + s * sinR), botColor),
        };
        game.renderer_->drawLineLoop(top2, 3, botColor);

        Vertex top3[] = {
            Vertex(Vec3(bot.pos.x, bot.pos.y + s * 2, bot.pos.z), botColor),
            Vertex(Vec3(bot.pos.x + s * cosR, bot.pos.y, bot.pos.z + s * sinR), botColor),
            Vertex(Vec3(bot.pos.x - s * sinR, bot.pos.y, bot.pos.z + s * cosR), botColor),
        };
        game.renderer_->drawLineLoop(top3, 3, botColor);

        Vertex top4[] = {
            Vertex(Vec3(bot.pos.x, bot.pos.y + s * 2, bot.pos.z), botColor),
            Vertex(Vec3(bot.pos.x - s * sinR, bot.pos.y, bot.pos.z + s * cosR), botColor),
            Vertex(Vec3(bot.pos.x - s * cosR, bot.pos.y, bot.pos.z - s * sinR), botColor),
        };
        game.renderer_->drawLineLoop(top4, 3, botColor);

        // Bottom pyramid (rotated opposite direction)
        float rotOffset2 = -rotOffset * 0.7f;
        float cosR2 = cosf(rotOffset2);
        float sinR2 = sinf(rotOffset2);

        Vertex bot1[] = {
            Vertex(Vec3(bot.pos.x, bot.pos.y - s, bot.pos.z), botColor),
            Vertex(Vec3(bot.pos.x - s * cosR2, bot.pos.y, bot.pos.z - s * sinR2), botColor),
            Vertex(Vec3(bot.pos.x + s * sinR2, bot.pos.y, bot.pos.z - s * cosR2), botColor),
        };
        game.renderer_->drawLineLoop(bot1, 3, botColor);

        Vertex bot2[] = {
            Vertex(Vec3(bot.pos.x, bot.pos.y - s, bot.pos.z), botColor),
            Vertex(Vec3(bot.pos.x + s * sinR2, bot.pos.y, bot.pos.z - s * cosR2), botColor),
            Vertex(Vec3(bot.pos.x + s * cosR2, bot.pos.y, bot.pos.z + s * sinR2), botColor),
        };
        game.renderer_->drawLineLoop(bot2, 3, botColor);

        Vertex bot3[] = {
            Vertex(Vec3(bot.pos.x, bot.pos.y - s, bot.pos.z), botColor),
            Vertex(Vec3(bot.pos.x + s * cosR2, bot.pos.y, bot.pos.z + s * sinR2), botColor),
            Vertex(Vec3(bot.pos.x - s * sinR2, bot.pos.y, bot.pos.z + s * cosR2), botColor),
        };
        game.renderer_->drawLineLoop(bot3, 3, botColor);

        Vertex bot4[] = {
            Vertex(Vec3(bot.pos.x, bot.pos.y - s, bot.pos.z), botColor),
            Vertex(Vec3(bot.pos.x - s * cosR2, bot.pos.y, bot.pos.z - s * sinR2), botColor),
            Vertex(Vec3(bot.pos.x - s * sinR2, bot.pos.y, bot.pos.z + s * cosR2), botColor),
        };
        game.renderer_->drawLineLoop(bot4, 3, botColor);

        // Health bar above bot
        Vec3 hpColor(1.0f - healthPct, healthPct, 0.0f);
        if (bot.botType == 4) hpColor = Vec3(1.0f, 0.8f, 0.0f);  // Boss HP is gold
        float hbWidth = 1.5f * sizeMult;
        float hbY = bot.pos.y + 2.5f * sizeMult;

        // Background
        Vertex bg[] = {
            Vertex(Vec3(bot.pos.x - hbWidth * 0.5f, hbY, bot.pos.z), Vec3(0.2f, 0.2f, 0.2f)),
            Vertex(Vec3(bot.pos.x + hbWidth * 0.5f, hbY, bot.pos.z), Vec3(0.2f, 0.2f, 0.2f)),
        };
        game.renderer_->drawLineLoop(bg, 2, Vec3(0.2f, 0.2f, 0.2f));

        // Fill
        Vertex hp[] = {
            Vertex(Vec3(bot.pos.x - hbWidth * 0.5f * healthPct, hbY, bot.pos.z), hpColor),
            Vertex(Vec3(bot.pos.x + hbWidth * 0.5f * healthPct, hbY, bot.pos.z), hpColor),
        };
        game.renderer_->drawLineLoop(hp, 2, hpColor);
    }
}

void spawnExplosion(Game& game, Vec3 pos, Vec3 color, int count) {
    for (int i = 0; i < count; i++) {
        float angle = (float)i / count * 6.28318f;
        float speed = 3.0f + (rand() % 100) / 100.0f * 5.0f;
        Vec3 vel(
            sinf(angle) * speed,
            (rand() % 100) / 100.0f * 8.0f,
            cosf(angle) * speed
        );
        float life = 0.5f + (rand() % 100) / 200.0f;
        float size = 0.1f + (rand() % 100) / 500.0f;
        game.particles.push_back(Particle(pos, vel, color, life, size));
    }
    // Add some smoke particles
    for (int i = 0; i < count / 2; i++) {
        Vec3 vel(
            (rand() % 100 - 50) / 50.0f * 2.0f,
            (rand() % 100) / 100.0f * 3.0f,
            (rand() % 100 - 50) / 50.0f * 2.0f
        );
        float life = 1.0f + (rand() % 100) / 100.0f;
        float size = 0.2f + (rand() % 100) / 300.0f;
        game.particles.push_back(Particle(pos, vel, Vec3(0.3f, 0.3f, 0.3f), life, size));
    }
}
