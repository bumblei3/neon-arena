// wave_config.h - Wave configuration and modifiers for neon arena
#pragma once
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>

// Enemy modifiers that can be applied to waves (bit flags)
enum EnemyModifier : uint32_t {
    NONE = 0,
    SPEED_BOOST = 1,      // +20% move speed
    SHIELD = 2,           // Absorbs one hit (extra HP)
    REGENERATION = 4,     // +1 HP/sec
    SPLITTER = 8,         // Spawns 2 mini-bots on death
    FROST = 16,           // Slows player on hit
    FRENZY = 32,          // Bots attack faster
    VAMPIRE = 64,         // Bots heal on player hit
    TIMELAPSE = 128       // Arena shrinks over time
};

// Fusion effects - emerge from modifier combinations
enum class WaveFusion : uint32_t {
    NONE = 0,
    PERMAFROST = 1,       // FROST + SHIELD: Bots freezable, player -20% speed
    BLOOD_MOON = 2,       // VAMPIRE + SPLITTER: Kills heal, bots faster
    BULLET_HELL = 3,      // FRENZY + TIMELAPSE: +50% fire rate, arena shrinks
    UNSTOPPABLE = 4,      // SPEED_BOOST + REGENERATION: Bots fast + regen
    GLASS_RAIN = 5,       // FRENZY + SPLITTER: Explosive splitter chaos
    ETERNAL_WINTER = 6,   // FROST + TIMELAPSE: Slow + shrinking arena
    LEECH_SWARM = 7,      // VAMPIRE + SPLITTER: Splitters heal on hit
    FRENZY_FROST = 8,     // FRENZY + FROST: Fast + slow player
    OVERCHARGE = 9,       // SHIELD + REGENERATION: Double HP regen
    TIMESHIFT = 10        // SPEED_BOOST + TIMELAPSE: Speed boost accelerates
};

// Fusion effect data structure
struct FusionEffect {
    WaveFusion fusion;
    const char* name;
    const char* description;
    float playerSpeedMult;      // Multiplier for player speed
    float botSpeedMult;         // Multiplier for bot speed
    float playerDamageMult;     // Multiplier for player damage
    float botDamageMult;        // Multiplier for bot damage
    float arenaShrinkRate;      // Arena shrink per second (0 = no shrink)
    int bonusPoints;            // Bonus points for surviving this fusion
};

inline const FusionEffect* getFusionEffect(WaveFusion fusion) {
    static const FusionEffect effects[] = {
        { WaveFusion::NONE, "None", "No fusion active", 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0 },
        { WaveFusion::PERMAFROST, "Permafrost", "Bots freezable, player -20% speed", 0.8f, 1.0f, 1.0f, 1.0f, 0.0f, 50 },
        { WaveFusion::BLOOD_MOON, "Blood Moon", "Kills heal you, bots become faster", 1.0f, 1.3f, 1.0f, 1.2f, 0.0f, 75 },
        { WaveFusion::BULLET_HELL, "Bullet Hell", "+50% fire rate, arena shrinks!", 1.0f, 1.0f, 1.5f, 1.0f, 0.5f, 100 },
        { WaveFusion::UNSTOPPABLE, "Unstoppable", "Fast bots that regenerate", 1.0f, 1.5f, 1.0f, 1.0f, 0.0f, 60 },
        { WaveFusion::GLASS_RAIN, "Glass Rain", "Explosive splitter chaos", 1.0f, 1.0f, 1.0f, 1.3f, 0.0f, 80 },
        { WaveFusion::ETERNAL_WINTER, "Eternal Winter", "Slow + shrinking arena", 0.7f, 0.8f, 1.0f, 1.0f, 0.3f, 90 },
        { WaveFusion::LEECH_SWARM, "Leech Swarm", "Splitters heal on hit", 1.0f, 1.0f, 1.0f, 1.1f, 0.0f, 70 },
        { WaveFusion::FRENZY_FROST, "Frost Frenzy", "Fast bots, slow player", 0.75f, 1.4f, 1.0f, 1.0f, 0.0f, 65 },
        { WaveFusion::OVERCHARGE, "Overcharge", "Double HP regen on bots", 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 55 },
        { WaveFusion::TIMESHIFT, "Timeshift", "Speed boost accelerates over time", 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 85 }
    };
    
    for (const auto& e : effects) {
        if (e.fusion == fusion) return &e;
    }
    return &effects[0];
}

inline bool hasModifier(EnemyModifier flags, EnemyModifier mod) {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(mod)) != 0;
}

// Detect fusion from modifier combination
// Returns the FIRST matching fusion (priority order as defined)
inline WaveFusion detectFusion(EnemyModifier mods) {
    uint32_t flags = static_cast<uint32_t>(mods);
    bool frost = (flags & EnemyModifier::FROST) != 0;
    bool frenzy = (flags & EnemyModifier::FRENZY) != 0;
    bool vampire = (flags & EnemyModifier::VAMPIRE) != 0;
    bool timelapse = (flags & EnemyModifier::TIMELAPSE) != 0;
    bool shield = (flags & EnemyModifier::SHIELD) != 0;
    bool regen = (flags & EnemyModifier::REGENERATION) != 0;
    bool speed = (flags & EnemyModifier::SPEED_BOOST) != 0;
    bool splitter = (flags & EnemyModifier::SPLITTER) != 0;
    
    // Priority order as defined in FusionEffect enum
    if (frenzy && timelapse) return WaveFusion::BULLET_HELL;
    if (frost && timelapse) return WaveFusion::ETERNAL_WINTER;
    if (vampire && splitter) return WaveFusion::BLOOD_MOON;
    if (frenzy && splitter) return WaveFusion::GLASS_RAIN;
    if (speed && timelapse) return WaveFusion::TIMESHIFT;
    if (vampire && timelapse) return WaveFusion::BLOOD_MOON;
    if (vampire && frost) return WaveFusion::BLOOD_MOON;
    if (vampire && frenzy) return WaveFusion::BLOOD_MOON;
    if (speed && regen) return WaveFusion::UNSTOPPABLE;
    if (shield && regen) return WaveFusion::OVERCHARGE;
    if (frost && shield) return WaveFusion::PERMAFROST;
    if (frenzy && frost) return WaveFusion::FRENZY_FROST;
    if (speed && frost) return WaveFusion::ETERNAL_WINTER;
    if (speed && frost && splitter) return WaveFusion::GLASS_RAIN;
    
    return WaveFusion::NONE;
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
            config.modifiers = static_cast<EnemyModifier>(EnemyModifier::SHIELD | EnemyModifier::REGENERATION);
        }
        if (wave >= 20) {
            config.modifiers = static_cast<EnemyModifier>(config.modifiers | EnemyModifier::SPEED_BOOST);
        }
    } else {
        config.baseBotCount = wave + 1;
        config.healthMultiplier = 1.0f + (wave * 0.05f);
        
        if (wave >= 3) {
            config.modifiers = EnemyModifier::SPEED_BOOST;
        }
        if (wave >= 6) {
            config.modifiers = static_cast<EnemyModifier>(EnemyModifier::SPEED_BOOST | EnemyModifier::SHIELD);
        }
        if (wave >= 9) {
            config.modifiers = static_cast<EnemyModifier>(EnemyModifier::SPEED_BOOST | EnemyModifier::SHIELD | EnemyModifier::REGENERATION);
        }
        if (wave >= 12) {
            config.modifiers = static_cast<EnemyModifier>(EnemyModifier::SPEED_BOOST | EnemyModifier::SHIELD | EnemyModifier::REGENERATION | EnemyModifier::SPLITTER);
        }
        if (wave >= 14) {
            // SPEED+SHIELD+SPLITTER only — no fusion (avoids UNSTOPPABLE from SPEED+REGEN)
            config.modifiers = static_cast<EnemyModifier>(EnemyModifier::SPEED_BOOST | EnemyModifier::SHIELD | EnemyModifier::SPLITTER);
        }
        if (wave >= 16 && wave < 21) {
            // All except FRENZY → ETERNAL_WINTER (FROST+TIMELAPSE) wins
            config.modifiers = static_cast<EnemyModifier>(EnemyModifier::SPEED_BOOST | EnemyModifier::SHIELD | EnemyModifier::REGENERATION | EnemyModifier::SPLITTER | EnemyModifier::FROST | EnemyModifier::VAMPIRE | EnemyModifier::TIMELAPSE);
        }
        if (wave >= 21) {
            // All except FROST → BULLET_HELL (FRENZY+TIMELAPSE) wins
            config.modifiers = static_cast<EnemyModifier>(EnemyModifier::SPEED_BOOST | EnemyModifier::SHIELD | EnemyModifier::REGENERATION | EnemyModifier::SPLITTER | EnemyModifier::FRENZY | EnemyModifier::VAMPIRE | EnemyModifier::TIMELAPSE);
        }
    }
    
    return config;
}

// Static array of modifier names for debugging
const char* const ENEMY_MODIFIERS[] = {
    "NONE", "SPEED_BOOST", "SHIELD", "REGENERATION", "SPLITTER",
    "FROST", "FRENZY", "VAMPIRE", "TIMELAPSE"
};
