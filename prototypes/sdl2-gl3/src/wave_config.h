// wave_config.h - Wave configuration and modifiers for neon arena
#pragma once
#include <vector>

// Enemy modifiers that can be applied to waves
enum class EnemyModifier {
    NONE = 0,
    SPEED_BOOST = 1,      // +20% move speed
    SHIELD = 2,           // Absorbs one hit (extra HP)
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
inline WaveConfig generateWaveConfig(int wave) {
    WaveConfig config;
    
    if (wave % 5 == 0) {
        config.isBossWave = true;
        config.bossCount = 1;
        config.minionCount = 1 + wave / 5;
        config.healthMultiplier = 1.0f + (wave * 0.1f);
        
        if (wave >= 10) {
            config.modifiers = EnemyModifier::SHIELD | EnemyModifier::REGENERATION;
        }
        if (wave >= 20) {
            config.modifiers = config.modifiers | EnemyModifier::SPEED_BOOST;
        }
    } else {
        config.baseBotCount = wave + 1;
        config.healthMultiplier = 1.0f + (wave * 0.05f);
        
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
