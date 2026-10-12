// bot_spawn_helpers.h - shared bot-spawn helpers (used by bots.cpp and
// wave_system.cpp). Extracted from the former static functions in bots.cpp
// during the WaveSystem extraction.
#pragma once
#include "game.h"
#include "balancing.h"
#include "modifier.h"
#include "bot_ai.h"

inline void getBalancedBotStats(int wave, int botType, float& outHealth, float& outMoveSpeed) {
    const BalancingDB& db = BalancingDB::get();
    auto* melee = db.getBot("Melee");
    auto* shooter = db.getBot("Shooter");
    auto* tank = db.getBot("Tank");
    auto* flanker = db.getBot("Flanker");
    auto* boss = db.getBot("Boss");

    float healthMult = 1.0f + wave * 0.1f;
    float speedMult = 1.0f + wave * 0.02f;

    switch (botType) {
        case 0: // Melee
            outHealth = (melee->baseHealth + melee->healthPerWave * wave) * healthMult;
            outMoveSpeed = melee->moveSpeed * speedMult;
            break;
        case 1: // Shooter
            outHealth = (shooter->baseHealth + shooter->healthPerWave * wave) * healthMult;
            outMoveSpeed = shooter->moveSpeed * speedMult;
            break;
        case 2: // Tank
            outHealth = (tank->baseHealth + tank->healthPerWave * wave) * healthMult;
            outMoveSpeed = tank->moveSpeed * speedMult;
            break;
        case 3: // Flanker
            outHealth = (flanker->baseHealth + flanker->healthPerWave * wave) * healthMult;
            outMoveSpeed = flanker->moveSpeed * speedMult;
            break;
        case 4: // Boss
            outHealth = (boss->baseHealth + boss->healthPerWave * wave) * healthMult * 1.5f;
            outMoveSpeed = boss->moveSpeed * speedMult;
            break;
        case 5: // Stealth
            outHealth = (flanker->baseHealth * 0.6f + flanker->healthPerWave * wave * 0.8f) * healthMult;
            outMoveSpeed = flanker->moveSpeed * 1.4f * speedMult;
            break;
        case 6: // Detector
            outHealth = (shooter->baseHealth + shooter->healthPerWave * wave) * healthMult;
            outMoveSpeed = shooter->moveSpeed * 0.85f * speedMult;
            break;
        default:
            outHealth = 50.0f;
            outMoveSpeed = 3.0f;
            break;
    }
}

inline void applyBotModifiers(Entity& bot, EnemyModifier modifiers) {
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
inline void initBotAI(Entity& bot) {
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
        case 5: // Stealth
            bot.aiState.personality = BotAI::Personality::FLANKER;
            break;
        case 6: // Detector
            bot.aiState.personality = BotAI::Personality::DEFENSIVE;
            break;
        default:
            bot.aiState.personality = BotAI::Personality::AGGRESSIVE;
            break;
    }
    bot.aiState.state = BotAI::State::IDLE;
}

inline void getBotMaxHealth(int wave, int botType, float& outMaxHealth) {
    const BalancingDB& db = BalancingDB::get();
    float healthMult = 1.0f + wave * 0.1f;
    
    switch (botType) {
        case 0: outMaxHealth = (db.getBot("Melee")->baseHealth + db.getBot("Melee")->healthPerWave * wave) * healthMult; break;
        case 1: outMaxHealth = (db.getBot("Shooter")->baseHealth + db.getBot("Shooter")->healthPerWave * wave) * healthMult; break;
        case 2: outMaxHealth = (db.getBot("Tank")->baseHealth + db.getBot("Tank")->healthPerWave * wave) * healthMult; break;
        case 3: outMaxHealth = (db.getBot("Flanker")->baseHealth + db.getBot("Flanker")->healthPerWave * wave) * healthMult; break;
        case 4: outMaxHealth = (db.getBot("Boss")->baseHealth + db.getBot("Boss")->healthPerWave * wave) * healthMult * 1.5f; break;
        case 5: outMaxHealth = (db.getBot("Flanker")->baseHealth * 0.6f + db.getBot("Flanker")->healthPerWave * wave * 0.8f) * healthMult; break;
        case 6: outMaxHealth = (db.getBot("Shooter")->baseHealth + db.getBot("Shooter")->healthPerWave * wave) * healthMult; break;
        default: outMaxHealth = 100.0f + wave * 10.0f;
    }
}
