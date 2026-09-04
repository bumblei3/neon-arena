#include "playtest.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>

PlaytestSimulator::SimulationResult PlaytestSimulator::run(int numWaves, int startWave) {
    SimulationResult result = {};
    result.maxWaveReached = startWave - 1;
    
    const BalancingDB& db = BalancingDB::get();
    
    SimPlayer player;
    std::vector<SimBot> bots;
    
    float gameTime = 0;
    float dt = 1.0f / 60.0f; // 60 FPS simulation
    int wave = startWave;
    int upgradePoints = 0;
    int kills = 0;
    float totalDamageDealt = 0;
    float totalDamageTaken = 0;
    
    // Weapon stats from DB
    auto* railgun = db.getWeapon("Railgun");
    auto* lightning = db.getWeapon("Lightning Gun");
    auto* plasma = db.getWeapon("Plasma Rifle");
    
    // Bot stats
    auto* meleeBot = db.getBot("Melee");
    auto* shooterBot = db.getBot("Shooter");
    auto* tankBot = db.getBot("Tank");
    auto* flankerBot = db.getBot("Flanker");
    auto* bossBot = db.getBot("Boss");
    
    int totalBotSpawnTime = 0;
    int totalBotDeathTime = 0;
    int totalBotCount = 0;
    
    while (wave <= numWaves && result.deaths < 3) {
        auto waveCfg = db.getWave(wave);
        int botsToSpawn = waveCfg.botCount;
        int botsSpawned = 0;
        float waveStartTime = gameTime;
        
        // Spawn initial bots
        for (int i = 0; i < botsToSpawn; i++) {
            SimBot bot;
            float angle = (float)i / botsToSpawn * 6.28318f;
            float radius = 50.0f;
            bot.posX = cosf(angle) * radius;
            bot.posZ = sinf(angle) * radius;
            bot.alive = true;
            bot.spawnTime = gameTime;
            
            // Choose bot type
            float r = (float)rand() / RAND_MAX;
            if (waveCfg.isBoss && i == 0) {
                // Boss
                bot.type = 4;
                bot.health = (bossBot->baseHealth + bossBot->healthPerWave * wave) * waveCfg.healthMultiplier;
                bot.moveSpeed = bossBot->moveSpeed * waveCfg.speedMultiplier;
                bot.attackCooldown = bossBot->attackCooldown;
                bot.attackRange = bossBot->attackRange;
                bot.attackDamage = bossBot->attackDamage;
            } else if (r < 0.35f) {
                // Melee
                bot.type = 0;
                bot.health = (meleeBot->baseHealth + meleeBot->healthPerWave * wave) * waveCfg.healthMultiplier;
                bot.moveSpeed = meleeBot->moveSpeed * waveCfg.speedMultiplier;
                bot.attackCooldown = meleeBot->attackCooldown;
                bot.attackRange = meleeBot->attackRange;
                bot.attackDamage = meleeBot->attackDamage;
            } else if (r < 0.55f) {
                // Shooter
                bot.type = 1;
                bot.health = (shooterBot->baseHealth + shooterBot->healthPerWave * wave) * waveCfg.healthMultiplier;
                bot.moveSpeed = shooterBot->moveSpeed * waveCfg.speedMultiplier;
                bot.attackCooldown = shooterBot->attackCooldown;
                bot.attackRange = shooterBot->attackRange;
                bot.attackDamage = shooterBot->attackDamage;
            } else if (r < 0.75f) {
                // Tank
                bot.type = 2;
                bot.health = (tankBot->baseHealth + tankBot->healthPerWave * wave) * waveCfg.healthMultiplier;
                bot.moveSpeed = tankBot->moveSpeed * waveCfg.speedMultiplier;
                bot.attackCooldown = tankBot->attackCooldown;
                bot.attackRange = tankBot->attackRange;
                bot.attackDamage = tankBot->attackDamage;
            } else {
                // Flanker
                bot.type = 3;
                bot.health = (flankerBot->baseHealth + flankerBot->healthPerWave * wave) * waveCfg.healthMultiplier;
                bot.moveSpeed = flankerBot->moveSpeed * waveCfg.speedMultiplier;
                bot.attackCooldown = flankerBot->attackCooldown;
                bot.attackRange = flankerBot->attackRange;
                bot.attackDamage = flankerBot->attackDamage;
            }
            
            bots.push_back(bot);
            totalBotCount++;
        }
        
        // Simulate wave
        bool waveComplete = false;
        while (!waveComplete && player.alive && gameTime - waveStartTime < 120.0f) {
            gameTime += dt;
            
            // Update bots
            for (auto& bot : bots) {
                if (!bot.alive) continue;
                
                // Move toward player
                float dx = player.posX - bot.posX;
                float dz = player.posZ - bot.posZ;
                float dist = std::sqrt(dx * dx + dz * dz);
                
                if (dist > 0.1f) {
                    bot.posX += (dx / dist) * bot.moveSpeed * dt;
                    bot.posZ += (dz / dist) * bot.moveSpeed * dt;
                }
                
                // Attack player
                if (dist < bot.attackRange) {
                    bot.attackCooldown -= dt;
                    if (bot.attackCooldown <= 0.0f) {
                        player.health -= bot.attackDamage;
                        totalDamageTaken += bot.attackDamage;
                        bot.attackCooldown = bot.type == 4 ? 1.0f : 1.0f;
                        if (player.health <= 0) {
                            player.alive = false;
                            result.deaths++;
                        }
                    }
                }
            }
            
            // Player fires (auto-target nearest bot)
            if (player.alive) {
                // Find nearest bot
                SimBot* target = nullptr;
                float nearestDist = 999.0f;
                for (auto& bot : bots) {
                    if (!bot.alive) continue;
                    float dx = player.posX - bot.posX;
                    float dz = player.posZ - bot.posZ;
                    float dist = std::sqrt(dx * dx + dz * dz);
                    if (dist < nearestDist) {
                        nearestDist = dist;
                        target = &bot;
                    }
                }
                
                // Fire railgun
                player.railgunCooldown -= dt;
                if (player.railgunCooldown <= 0.0f && target && nearestDist < 50.0f) {
                    float dmg = railgun->damage * (1.0f + 0.15f * (result.railgunLevel - 1));
                    target->health -= dmg;
                    totalDamageDealt += dmg;
                    player.railgunCooldown = 1.0f / railgun->fireRate;
                    if (target->health <= 0) {
                        target->alive = false;
                        target->deathTime = gameTime;
                        kills++;
                        totalBotDeathTime += (int)(gameTime - target->spawnTime);
                    }
                }
                
                // Fire lightning
                player.lightningCooldown -= dt;
                if (player.lightningCooldown <= 0.0f && target && nearestDist < 15.0f) {
                    float dmg = lightning->damage * (1.0f + 0.12f * (result.lightningLevel - 1));
                    // Chain to nearby bots
                    int chains = 3;
                    SimBot* current = target;
                    for (int c = 0; c < chains && current; c++) {
                        current->health -= dmg * (1.0f - c * 0.2f);
                        totalDamageDealt += dmg * (1.0f - c * 0.2f);
                        if (current->health <= 0 && current->alive) {
                            current->alive = false;
                            current->deathTime = gameTime;
                            kills++;
                        }
                        // Find next nearest
                        SimBot* next = nullptr;
                        float nextDist = 999.0f;
                        for (auto& bot : bots) {
                            if (!bot.alive || &bot == current) continue;
                            float d = std::sqrt((bot.posX - current->posX) * (bot.posX - current->posX) +
                                                (bot.posZ - current->posZ) * (bot.posZ - current->posZ));
                            if (d < nextDist) {
                                nextDist = d;
                                next = &bot;
                            }
                        }
                        current = next;
                    }
                    player.lightningCooldown = 1.0f / lightning->fireRate;
                }
            }
            
            // Check wave complete
            waveComplete = true;
            for (auto& bot : bots) {
                if (bot.alive) {
                    waveComplete = false;
                    break;
                }
            }
            
            // Revive after 3 seconds
            if (!player.alive && result.deaths < 3) {
                static float deathTimer = 0;
                deathTimer += dt;
                if (deathTimer >= 3.0f) {
                    player.alive = true;
                    player.health = 50.0f;
                    deathTimer = 0;
                    result.revives++;
                }
            }
        }
        
        if (!player.alive && result.deaths >= 3) {
            break;
        }
        
        // Wave complete
        result.maxWaveReached = wave;
        upgradePoints += waveCfg.upgradeReward;
        result.upgradePointsEarned += waveCfg.upgradeReward;
        
        // Spend upgrade points (auto-balance)
        if (upgradePoints >= 5) {
            // Upgrade strongest weapon
            if (result.railgunLevel <= result.lightningLevel && result.railgunLevel <= result.plasmaLevel) {
                result.railgunLevel++;
            } else if (result.lightningLevel <= result.plasmaLevel) {
                result.lightningLevel++;
            } else {
                result.plasmaLevel++;
            }
            upgradePoints -= 5;
            result.upgradePointsSpent += 5;
        }
        
        wave++;
    }
    
    result.totalKills = kills;
    result.totalDamageDealt = totalDamageDealt;
    result.totalDamageTaken = totalDamageTaken;
    result.gameTimeSeconds = gameTime;
    result.railgunLevel = result.railgunLevel;
    result.lightningLevel = result.lightningLevel;
    result.plasmaLevel = result.plasmaLevel;
    result.upgradePointsEarned = result.upgradePointsEarned;
    result.upgradePointsSpent = result.upgradePointsSpent;
    if (totalBotCount > 0) {
        result.avgBotAliveTime = (float)totalBotDeathTime / totalBotCount;
    }
    float totalDamagePossible = totalDamageDealt + result.totalDamageTaken;
    result.dpsEfficiency = totalDamagePossible > 0 ? totalDamageDealt / totalDamagePossible : 0;
    
    return result;
}

void PlaytestSimulator::printReport(const SimulationResult& r) {
    printf("\n=========================================\n");
    printf("   PLAYTEST SIMULATION REPORT\n");
    printf("=========================================\n");
    printf("Max Wave Reached:    %d\n", r.maxWaveReached);
    printf("Total Kills:         %d\n", r.totalKills);
    printf("Deaths:              %d\n", r.deaths);
    printf("Revives:             %d\n", r.revives);
    printf("Game Time:           %.1f seconds (%.1f min)\n", r.gameTimeSeconds, r.gameTimeSeconds / 60.0f);
    printf("Damage Dealt:        %.0f\n", r.totalDamageDealt);
    printf("Damage Taken:        %.0f\n", r.totalDamageTaken);
    printf("DPS Efficiency:      %.1f%%\n", r.dpsEfficiency * 100.0f);
    printf("Avg Bot Alive Time:  %.2f seconds\n", r.avgBotAliveTime);
    printf("\n");
    printf("Final Levels:        Railgun=%d, Lightning=%d, Plasma=%d\n",
           r.railgunLevel, r.lightningLevel, r.plasmaLevel);
    printf("Upgrade Points:      Earned=%d, Spent=%d\n", r.upgradePointsEarned, r.upgradePointsSpent);
    printf("=========================================\n\n");
}
