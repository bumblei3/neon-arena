// bots.cpp - Bot logic with State Machine AI
#include "game.h"
#include "wave_config.h"
#include "bot_ai.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

static void applyModifiers(Entity& bot, EnemyModifier modifiers) {
    if (hasModifier(modifiers, EnemyModifier::SPEED_BOOST)) {
        bot.moveSpeed *= 1.2f;
    }
    if (hasModifier(modifiers, EnemyModifier::SHIELD)) {
        bot.health += 50.0f;
    }
    if (hasModifier(modifiers, EnemyModifier::REGENERATION)) {
        bot.vy = 1.0f;
    }
    if (hasModifier(modifiers, EnemyModifier::SPLITTER)) {
        bot.splitters = 2;
    }
}

// Initialize AI state based on bot type
static void initBotAI(Entity& bot) {
    switch (bot.botType) {
        case 0: // Melee
            bot.aiState.personality = BotAI::Personality::AGGRESSIVE;
            break;
        case 1: // Shooter
            bot.aiState.personality = BotAI::Personality::DEFENSIVE;
            break;
        case 2: // Tank
            bot.aiState.personality = BotAI::Personality::SWARM;
            break;
        case 3: // Fast
            bot.aiState.personality = BotAI::Personality::FLANKER;
            break;
        case 4: // Boss
            bot.aiState.personality = BotAI::Personality::BOSS;
            break;
        default:
            bot.aiState.personality = BotAI::Personality::AGGRESSIVE;
            break;
    }
    bot.aiState.state = BotAI::State::IDLE;
}

void spawnWave(Game& game) {
    game.wave++;
    game.bots.clear();

    WaveConfig config = generateWaveConfig(game.wave);

    if (config.isBossWave) {
        int totalCount = config.bossCount + config.minionCount;
        for (int i = 0; i < totalCount; i++) {
            Entity bot;
            float angle = (float)i / totalCount * 6.28318f;
            float radius = game.arenaSize * 0.6f;
            bot.pos = Vec3(cosf(angle) * radius, 0.5f, sinf(angle) * radius);
            bot.yaw = 0;
            bot.pitch = 0;
            bot.alive = true;
            bot.type = 1;

            if (i == 0) {
                bot.botType = 4;
                bot.isBoss = true;
                bot.health = (500.0f + game.wave * 50) * config.healthMultiplier;
                bot.moveSpeed = 2.0f;
                bot.attackCooldown = 0;
                bot.aiState.personality = BotAI::Personality::BOSS;
            } else {
                bot.botType = 0;
                bot.health = (50.0f + game.wave * 5) * config.healthMultiplier;
                bot.moveSpeed = 4.0f;
                applyModifiers(bot, config.modifiers);
                initBotAI(bot);
            }
            game.bots.push_back(bot);
        }
        printf("BOSS WAVE %d: Boss + %d minions! (modifiers: 0x%x)\n",
               game.wave, config.minionCount, static_cast<int>(config.modifiers));
    } else {
        int botCount = config.baseBotCount;
        for (int i = 0; i < botCount; i++) {
            Entity bot;
            float angle = (float)i / botCount * 6.28318f;
            float radius = game.arenaSize * 0.7f;
            bot.pos = Vec3(cosf(angle) * radius, 0.5f, sinf(angle) * radius);
            bot.yaw = 0;
            bot.pitch = 0;
            bot.alive = true;
            bot.type = 1;

            if (game.wave >= 3 && i == 0) {
                bot.botType = 2;
                bot.health = (200.0f + game.wave * 20) * config.healthMultiplier;
                bot.moveSpeed = 1.5f;
            } else if (game.wave >= 2 && i == botCount - 1) {
                bot.botType = 3;
                bot.health = (50.0f + game.wave * 5) * config.healthMultiplier;
                bot.moveSpeed = 6.0f;
            } else if (game.wave >= 4 && i % 3 == 1) {
                bot.botType = 1;
                bot.health = (80.0f + game.wave * 8) * config.healthMultiplier;
                bot.moveSpeed = 2.5f;
            } else {
                bot.botType = 0;
                bot.health = (100.0f + game.wave * 10) * config.healthMultiplier;
                bot.moveSpeed = 3.0f;
            }
            applyModifiers(bot, config.modifiers);
            initBotAI(bot);
            game.bots.push_back(bot);
        }
        printf("Wave %d: %d bots spawned (modifiers: 0x%x)\n",
               game.wave, botCount, static_cast<int>(config.modifiers));
    }
    game.waveComplete = false;
}

void updateBots(Game& game, float dt) {
    // Collect bot states for swarm AI
    std::vector<BotAI::BotState*> botStates;
    for (auto& bot : game.bots) {
        if (!bot.alive) continue;
        botStates.push_back(&bot.aiState);
    }

    // Swarm coordination (only if enough bots)
    if (botStates.size() >= 3) {
        std::vector<BotAI::BotState> botStatesValues;
        for (auto* ptr : botStates) botStatesValues.push_back(*ptr);
        BotAI::swarmUpdate(botStatesValues, game.player.pos.x, game.player.pos.z);
    }

    bool playerIsMoving = (std::abs(game.player.vx) > 0.1f || std::abs(game.player.vz) > 0.1f);

    for (auto& bot : game.bots) {
        if (!bot.alive) continue;

        // Regeneration modifier
        if (bot.vy > 0.5f) {
            bot.health += 1.0f * dt;
        }

        // Apply fusion effects to bot AI
        BotAI::applyFusion(bot.aiState, game.currentFusion, dt);

        // Update AI state machine
        BotAI::update(bot.aiState, dt,
                      game.player.pos.x, game.player.pos.z,
                      bot.pos.x, bot.pos.z,
                      bot.health, 100.0f + game.wave * 10,
                      game.arenaSize, game.wave, playerIsMoving);

        // Pathfinding: use waypoint if path is available
        float targetX = bot.aiState.targetX;
        float targetZ = bot.aiState.targetZ;
        
        if (!bot.aiState.path.empty()) {
            // Follow path waypoints
            auto& wp = bot.aiState.path[0];
            float wdx = wp.first - bot.pos.x;
            float wdz = wp.second - bot.pos.z;
            float wdist = std::sqrt(wdx * wdx + wdz * wdz);
            if (wdist < 2.0f && bot.aiState.path.size() > 1) {
                bot.aiState.path.erase(bot.aiState.path.begin());
            }
            targetX = bot.aiState.path.empty() ? targetX : bot.aiState.path[0].first;
            targetZ = bot.aiState.path.empty() ? targetZ : bot.aiState.path[0].second;
        }

        // Move towards target
        float dx = targetX - bot.pos.x;
        float dz = targetZ - bot.pos.z;
        float dist = std::sqrt(dx * dx + dz * dz);

        if (dist > 0.5f) {
            float speed = bot.moveSpeed;
            // Slow down when close to target
            if (dist < 2.0f) speed *= 0.5f;
            bot.pos.x += (dx / dist) * speed * dt;
            bot.pos.z += (dz / dist) * speed * dt;
        }

        // Rotate towards movement direction or player
        if (bot.aiState.state == BotAI::State::HUNT || bot.aiState.state == BotAI::State::ATTACK) {
            float targetYaw = atan2f(game.player.pos.x - bot.pos.x, -(game.player.pos.z - bot.pos.z));
            bot.yaw = targetYaw;
        } else {
            float moveYaw = atan2f(dx, -dz);
            bot.yaw = moveYaw;
        }

        // Hover animation
        float hoverOffset = sinf(game.gameTime * 2.0f + bot.pos.x * 0.1f + bot.pos.z * 0.1f) * 0.3f;
        if (bot.botType == 4) hoverOffset *= 2.0f;
        bot.pos.y = hoverOffset;

        // Keep bots in bounds
        if (bot.pos.x < -game.arenaSize + 2) bot.pos.x = -game.arenaSize + 2;
        if (bot.pos.x > game.arenaSize - 2) bot.pos.x = game.arenaSize - 2;
        if (bot.pos.z < -game.arenaSize + 2) bot.pos.z = -game.arenaSize + 2;
        if (bot.pos.z > game.arenaSize - 2) bot.pos.z = game.arenaSize - 2;

        // Bot attacks based on AI decision
        bot.attackCooldown -= dt;
        if (BotAI::shouldAttack(bot.aiState, dist, bot.botType) && bot.attackCooldown <= 0.0f) {
            // Frenzy modifier: faster fire rate
            float fireRateMult = (bot.aiState.frenzyTimer > 0.0f) ? 0.6f : 1.0f;
            
            if (bot.botType == 0 || bot.botType == 2 || bot.botType == 3) {
                game.player.health -= 10.0f;
                bot.attackCooldown = 1.0f * fireRateMult;
            }
            if (bot.botType == 1) {
                Vec3 toPlayerDir = Vec3(
                    game.player.pos.x - bot.pos.x,
                    0,
                    game.player.pos.z - bot.pos.z
                ).normalized();
                Vec3 muzzlePos = bot.pos + Vec3(0, 1.0f, 0);
                game.projectiles.push_back(Projectile(muzzlePos, toPlayerDir, false, WeaponType::RAILGUN, 15.0f));
                bot.attackCooldown = 2.0f * fireRateMult;
            }
            if (bot.botType == 4) {
                // Boss multi-phase behavior
                int phase = BotAI::getBossPhase(bot.aiState, bot.health / (500.0f + game.wave * 50), bot.bossPhase);
                
                if (phase == 1) {
                    // Spread shot
                    for (int i = 0; i < 5; i++) {
                        float angle = (i - 2) * 0.3f;
                        Vec3 dir = Vec3(sinf(angle), 0, -cosf(angle));
                        Vec3 muzzlePos = bot.pos + Vec3(0, 2.0f, 0);
                        game.projectiles.push_back(Projectile(muzzlePos, dir, false, WeaponType::RAILGUN, 20.0f));
                    }
                    bot.attackCooldown = 3.0f * fireRateMult;
                } else if (phase == 2) {
                    // Aimed double shot
                    Vec3 toPlayerDir = Vec3(
                        game.player.pos.x - bot.pos.x,
                        0,
                        game.player.pos.z - bot.pos.z
                    ).normalized();
                    Vec3 muzzlePos = bot.pos + Vec3(0, 2.0f, 0);
                    game.projectiles.push_back(Projectile(muzzlePos, toPlayerDir, false, WeaponType::RAILGUN, 25.0f));
                    game.projectiles.push_back(Projectile(muzzlePos + Vec3(1,0,0), toPlayerDir, false, WeaponType::RAILGUN, 25.0f));
                    bot.attackCooldown = 2.5f * fireRateMult;
                } else if (phase == 3) {
                    // Frenzy: rapid aimed shots
                    Vec3 toPlayerDir = Vec3(
                        game.player.pos.x - bot.pos.x,
                        0,
                        game.player.pos.z - bot.pos.z
                    ).normalized();
                    Vec3 muzzlePos = bot.pos + Vec3(0, 2.0f, 0);
                    game.projectiles.push_back(Projectile(muzzlePos, toPlayerDir, false, WeaponType::RAILGUN, 20.0f));
                    bot.attackCooldown = 1.5f * fireRateMult;
                } else {
                    // Desperation: rain of projectiles
                    for (int i = 0; i < 8; i++) {
                        float angle = (float)i / 8 * 6.28318f;
                        Vec3 dir = Vec3(sinf(angle), 0, -cosf(angle));
                        Vec3 muzzlePos = bot.pos + Vec3(0, 2.0f, 0);
                        game.projectiles.push_back(Projectile(muzzlePos, dir, false, WeaponType::RAILGUN, 30.0f));
                    }
                    bot.attackCooldown = 2.0f * fireRateMult;
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
        if (bot.botType == 4) sizeMult = 2.5f;
        s *= sizeMult;

        float healthPct = bot.health / (100.0f + game.wave * 10);
        if (healthPct > 1.0f) healthPct = 1.0f;
        if (healthPct < 0.0f) healthPct = 0.0f;

        Vec3 botColor;
        switch (bot.botType) {
            case 0: botColor = Vec3((1.0f - healthPct) * 0.8f, healthPct * 1.0f, healthPct * 0.5f); break;
            case 1: botColor = Vec3(1.0f, 0.5f + healthPct * 0.5f, (1.0f - healthPct) * 0.3f); break;
            case 2: botColor = Vec3(0.6f + healthPct * 0.4f, (1.0f - healthPct) * 0.3f, 0.8f); break;
            case 3: botColor = Vec3(1.0f, (1.0f - healthPct) * 0.5f, (1.0f - healthPct) * 0.3f); break;
            case 4: botColor = Vec3(1.0f, 0.3f + healthPct * 0.4f, 0.0f); break;
            default: botColor = Vec3((1.0f - healthPct) * 0.8f, healthPct * 1.0f, healthPct * 0.5f); break;
        }

        if (bot.vy > 0.5f) botColor = botColor * 0.7f + Vec3(0.0f, 0.3f, 0.0f);
        if (bot.vz > 0.5f) botColor = botColor * 0.7f + Vec3(0.3f, 0.1f, 0.0f);

        float rotOffset = game.gameTime * 1.5f + bot.pos.x * 0.3f;
        float cosR = cosf(rotOffset);
        float sinR = sinf(rotOffset);
        float phi = 1.6180339887f;
        float h = s * phi;

        Vec3 top(bot.pos.x, bot.pos.y + h, bot.pos.z);
        Vec3 mid1(bot.pos.x - s * cosR, bot.pos.y, bot.pos.z - s * sinR);
        Vec3 mid2(bot.pos.x + s * sinR, bot.pos.y, bot.pos.z - s * cosR);
        Vec3 mid3(bot.pos.x + s * cosR, bot.pos.y, bot.pos.z + s * sinR);
        Vec3 mid4(bot.pos.x - s * sinR, bot.pos.y, bot.pos.z + s * cosR);
        Vec3 bottom(bot.pos.x, bot.pos.y - h * 0.6f, bot.pos.z);

        Vertex tri1[] = { Vertex(top, Vec3(0,1,0), botColor), Vertex(mid1, Vec3(-1,0.5f,-1), botColor), Vertex(mid2, Vec3(1,0.5f,-1), botColor) };
        game.renderer_->drawTriangles(tri1, 3, botColor);
        Vertex tri2[] = { Vertex(top, Vec3(0,1,0), botColor), Vertex(mid2, Vec3(1,0.5f,-1), botColor), Vertex(mid3, Vec3(1,0.5f,1), botColor) };
        game.renderer_->drawTriangles(tri2, 3, botColor);
        Vertex tri3[] = { Vertex(top, Vec3(0,1,0), botColor), Vertex(mid3, Vec3(1,0.5f,1), botColor), Vertex(mid4, Vec3(-1,0.5f,1), botColor) };
        game.renderer_->drawTriangles(tri3, 3, botColor);
        Vertex tri4[] = { Vertex(top, Vec3(0,1,0), botColor), Vertex(mid4, Vec3(-1,0.5f,1), botColor), Vertex(mid1, Vec3(-1,0.5f,-1), botColor) };
        game.renderer_->drawTriangles(tri4, 3, botColor);

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
    renderSolidBots(game);

    for (auto& bot : game.bots) {
        if (!bot.alive) continue;

        float pulse = 1.0f + sinf(game.gameTime * 4.0f + bot.pos.x * 0.5f) * 0.15f;
        float sizeMult = 1.0f;
        if (bot.botType == 4) sizeMult = 2.5f;
        float s = 0.8f * pulse * sizeMult;

        float healthPct = bot.health / (100.0f + game.wave * 10);
        Vec3 botColor((1.0f - healthPct) * 0.8f, healthPct * 1.0f, healthPct * 0.5f);

        float rotOffset = game.gameTime * 1.5f + bot.pos.x * 0.3f;
        float cosR = cosf(rotOffset);
        float sinR = sinf(rotOffset);

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
        if (bot.botType == 4) hpColor = Vec3(1.0f, 0.8f, 0.0f);
        float hbWidth = 1.5f * sizeMult;
        float hbY = bot.pos.y + 2.5f * sizeMult;

        Vertex bg[] = {
            Vertex(Vec3(bot.pos.x - hbWidth * 0.5f, hbY, bot.pos.z), Vec3(0.2f, 0.2f, 0.2f)),
            Vertex(Vec3(bot.pos.x + hbWidth * 0.5f, hbY, bot.pos.z), Vec3(0.2f, 0.2f, 0.2f)),
        };
        game.renderer_->drawLineLoop(bg, 2, Vec3(0.2f, 0.2f, 0.2f));

        Vertex hp[] = {
            Vertex(Vec3(bot.pos.x - hbWidth * 0.5f * healthPct, hbY, bot.pos.z), hpColor),
            Vertex(Vec3(bot.pos.x + hbWidth * 0.5f * healthPct, hbY, bot.pos.z), hpColor),
        };
        game.renderer_->drawLineLoop(hp, 2, hpColor);

        // AI state indicator (small colored dot above health bar)
        Vec3 stateColor;
        switch (bot.aiState.state) {
            case BotAI::State::IDLE: stateColor = Vec3(0.5f, 0.5f, 0.5f); break;
            case BotAI::State::HUNT: stateColor = Vec3(1.0f, 0.5f, 0.0f); break;
            case BotAI::State::ATTACK: stateColor = Vec3(1.0f, 0.0f, 0.0f); break;
            case BotAI::State::RETREAT: stateColor = Vec3(0.0f, 0.0f, 1.0f); break;
            case BotAI::State::FLANK: stateColor = Vec3(1.0f, 0.0f, 1.0f); break;
            case BotAI::State::STUNNED: stateColor = Vec3(1.0f, 1.0f, 0.0f); break;
            default: stateColor = Vec3(0.5f, 0.5f, 0.5f); break;
        }

        Vertex stateDot[] = {
            Vertex(Vec3(bot.pos.x - 0.1f, hbY + 0.3f, bot.pos.z), stateColor),
            Vertex(Vec3(bot.pos.x + 0.1f, hbY + 0.3f, bot.pos.z), stateColor),
        };
        game.renderer_->drawLineLoop(stateDot, 2, stateColor);
    }
}

void spawnExplosion(Game& game, Vec3 pos, Vec3 color, int count) {
    if (!game.particleSystem) return;

    // Main explosion burst (sparks)
    ParticleBurst burst;
    burst.posX = pos.x;
    burst.posY = pos.y;
    burst.posZ = pos.z;
    burst.dirX = 0.0f;
    burst.dirY = 1.0f;
    burst.dirZ = 0.0f;
    burst.speed = 8.0f;
    burst.spread = 1.0f;
    burst.life = 0.75f;
    burst.size = 0.2f;
    burst.sizeEnd = 0.05f;
    burst.r = color.x;
    burst.g = color.y;
    burst.b = color.z;
    burst.a = 1.0f;
    burst.count = count;
    burst.type = ParticleType::SPARK;
    game.particleSystem->spawnBurst(burst);

    // Smoke secondary effect
    ParticleBurst smoke;
    smoke.posX = pos.x;
    smoke.posY = pos.y;
    smoke.posZ = pos.z;
    smoke.dirX = 0.0f;
    smoke.dirY = 1.0f;
    smoke.dirZ = 0.0f;
    smoke.speed = 3.0f;
    smoke.spread = 1.0f;
    smoke.life = 1.5f;
    smoke.size = 0.3f;
    smoke.sizeEnd = 0.1f;
    smoke.r = 0.3f;
    smoke.g = 0.3f;
    smoke.b = 0.3f;
    smoke.a = 1.0f;
    smoke.count = count / 2;
    smoke.type = ParticleType::SMOKE;
    game.particleSystem->spawnBurst(smoke);
}
