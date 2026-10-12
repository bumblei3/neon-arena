// wave_system.cpp - Wave spawning and progression implementation
// Extracted from bots.cpp (spawnWave) and Game::nextWave. The wave *state*
// (wave, waveComplete, waveBreak, modifierState) stays on Game — WaveSystem
// only owns the logic, so all other readers of those fields keep working.
#include "wave_system.h"
#include "game.h"
#include "wave_config.h"
#include "modifier.h"
#include "balancing.h"
#include "bot_ai.h"
#include "ghost_rules.h"
#include "bot_spawn_helpers.h"
#include <cmath>
#include <cstdio>

void WaveSystem::init(Game& game) {
    (void)game;
}

void WaveSystem::nextWave(Game& game) {
    game.waveComplete = false;
    game.waveBreak = 0;
    game.tookDamageThisWave = false;

    // Select and apply wave modifiers (from wave 5+)
    SelectModifiers(game.modifierState, game.wave + 1, game.dailyOffset,
                    game.modifierForce1, game.modifierForce2);
    ApplyModifiers(game.modifierState);

    spawnWave(game);
    game.waveAnnounceTimer = 2.0f;
    if (game.echoSystem) game.echoSystem->startRecording();
}

void WaveSystem::spawnWave(Game& game) {
    game.wave++;
    game.bots.clear();

    WaveConfig config = generateWaveConfig(game.wave);

    if (config.isBossWave) {
        spawnBossWave(game, game.wave, config.bossCount, config.minionCount);
    } else {
        spawnNormalWave(game, game.wave, config.baseBotCount);
    }
    game.waveComplete = false;
}

void WaveSystem::spawnBossWave(Game& game, int wave_num, int boss_count, int minion_count) {
    WaveConfig config = generateWaveConfig(wave_num);
    int totalCount = boss_count + minion_count;

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
            float health, speed;
            getBalancedBotStats(wave_num, 4, health, speed);
            bot.health = health;
            bot.moveSpeed = speed;
            bot.attackCooldown = 0;
            bot.aiState.personality = BotAI::Personality::BOSS;
        } else if (wave_num >= GhostRules::DETECTOR_WAVE && i == 1) {
            bot.botType = 6;
            float health, speed;
            getBalancedBotStats(wave_num, 6, health, speed);
            bot.health = health;
            bot.moveSpeed = speed;
            applyBotModifiers(bot, config.modifiers);
            initBotAI(bot);
        } else {
            bot.botType = 0;
            float health, speed;
            getBalancedBotStats(wave_num, 0, health, speed);
            bot.health = health;
            bot.moveSpeed = speed;
            applyBotModifiers(bot, config.modifiers);
            initBotAI(bot);
        }
        game.bots.push_back(bot);
    }
    printf("BOSS WAVE %d: Boss + %d minions! (modifiers: 0x%x)\n",
           wave_num, minion_count, static_cast<int>(config.modifiers));
}

void WaveSystem::spawnNormalWave(Game& game, int wave_num, int bot_count) {
    WaveConfig config = generateWaveConfig(wave_num);
    for (int i = 0; i < bot_count; i++) {
        Entity bot;
        float angle = (float)i / bot_count * 6.28318f;
        float radius = game.arenaSize * 0.7f;
        bot.pos = Vec3(cosf(angle) * radius, 0.5f, sinf(angle) * radius);
        bot.yaw = 0;
        bot.pitch = 0;
        bot.alive = true;
        bot.type = 1;

        if (wave_num >= 3 && i == 0) {
            bot.botType = 2;
            float health, speed;
            getBalancedBotStats(wave_num, 2, health, speed);
            bot.health = health;
            bot.moveSpeed = speed;
        } else if (wave_num >= 2 && i == bot_count - 1) {
            bot.botType = 3;
            float health, speed;
            getBalancedBotStats(wave_num, 3, health, speed);
            bot.health = health;
            bot.moveSpeed = speed;
        } else if (wave_num >= 4 && i % 3 == 1) {
            bot.botType = 1;
            float health, speed;
            getBalancedBotStats(wave_num, 1, health, speed);
            bot.health = health;
            bot.moveSpeed = speed;
        } else if (wave_num >= GhostRules::DETECTOR_WAVE && i == 1) {
            bot.botType = 6;
            float health, speed;
            getBalancedBotStats(wave_num, 6, health, speed);
            bot.health = health;
            bot.moveSpeed = speed;
        } else if (wave_num >= 6 && i % 5 == 2) {
            // Stealth Bot from wave 6+
            bot.botType = 5;
            float health, speed;
            getBalancedBotStats(wave_num, 5, health, speed);
            bot.health = health;
            bot.moveSpeed = speed * 1.2f;  // Stealth bots are faster
        } else {
            bot.botType = 0;
            float health, speed;
            getBalancedBotStats(wave_num, 0, health, speed);
            bot.health = health;
            bot.moveSpeed = speed;
        }
        applyBotModifiers(bot, config.modifiers);
        initBotAI(bot);
        game.bots.push_back(bot);
    }
    printf("Wave %d: %d bots spawned (modifiers: 0x%x)\n",
           wave_num, bot_count, static_cast<int>(config.modifiers));
}
