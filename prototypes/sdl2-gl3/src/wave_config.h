// wave_config.h - Wave configuration and modifiers for neon arena
#pragma once
#include "game.h"
#include <vector>

// Enemy modifiers that can be applied to waves
enum class EnemyModifier {
    NONE = 0,
    SPEED_BOOST = 1,      // +20% move speed
    SHIELD = 2,           // Absorbs one hit
    REGENERATION = 4,     // +1 HP/sec
    SPLITTER = 8          // Spawns 2 mini-bots on death
};

inline EnemyModifier operator|(EnemyModifier a, EnemyModifier b) {
    return static_cast<EnemyModifier>(static_cast<int>(a) | static_cast<int>(b));
}

inline bool hasModifier(EnemyModifier flags, EnemyModifier mod) {
    return (static_cast<int>(flags) & static_cast<int>(mod)) != 0;
}

// Wave configuration - defines what spawns in a wave
struct WaveConfig {
    int baseBotCount = 0;
    float healthMultiplier = 1.0f;
    EnemyModifier modifiers = EnemyModifier::NONE;
    bool isBossWave = false;
    int bossCount = 0;
    int minionCount = 0;
};

// Generate wave config for a given wave number
// Returns increasingly difficult configurations
inline WaveConfig generateWaveConfig(int wave) {
    WaveConfig config;
    
    // Boss wave every 5 waves
    if (wave % 5 == 0) {
        config.isBossWave = true;
        config.bossCount = 1;
        config.minionCount = 1 + wave / 5;
        config.healthMultiplier = 1.0f + (wave * 0.1f);
        
        // Boss waves get modifiers starting at wave 10
        if (wave >= 10) {
            config.modifiers = EnemyModifier::SHIELD | EnemyModifier::REGENERATION;
        }
        if (wave >= 20) {
            config.modifiers = config.modifiers | EnemyModifier::SPEED_BOOST;
        }
    } else {
        // Normal wave
        config.baseBotCount = wave + 1;
        config.healthMultiplier = 1.0f + (wave * 0.05f);
        
        // Add modifiers at certain waves
        if (wave >= 3) {
            config.modifiers = EnemyModifier::SPEED_BOOST;
        }
        if (wave >= 6) {
            config.modifiers = EnemyModifier::SPEED_BOOST | EnemyModifier::SHIELD;
        }
        if (wave >= 9) {
            config.modifiers = EnemyModifier::SPEED_BOOST | EnemyModifier::SHIELD | EnemyModifier::REGENERATION;
        }
        if (wave >= 12) {
            config.modifiers = EnemyModifier::SPEED_BOOST | EnemyModifier::SHIELD | EnemyModifier::REGENERATION | EnemyModifier::SPLITTER;
        }
    }
    
    return config;
}

// Apply modifiers to a bot
inline void applyModifiers(Entity& bot, EnemyModifier modifiers) {
    if (hasModifier(modifiers, EnemyModifier::SPEED_BOOST)) {
        bot.moveSpeed *= 1.2f;
    }
    if (hasModifier(modifiers, EnemyModifier::SHIELD)) {
        // Shield: bot survives one hit (handled in collision)
        // Mark with negative health threshold
        bot.health += 50.0f;  // Extra health as shield
    }
    if (hasModifier(modifiers, EnemyModifier::REGENERATION)) {
        // Regeneration: bots heal over time (handled in update)
        // Mark with a special flag - we use vy for this
        bot.vy = 1.0f;  // Regen flag
    }
    if (hasModifier(modifiers, EnemyModifier::SPLITTER)) {
        // Splitter: spawns mini-bots on death
        bot.vz = 1.0f;  // Splitter flag
    }
}
