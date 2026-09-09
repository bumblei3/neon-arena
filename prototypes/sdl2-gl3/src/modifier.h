// modifier.h - Wave-Survival Modifier System (15 modifiers + 6 synergy pairs)
// Ported from OA-Mod g_neonwave.c (NW_MOD_*)
#pragma once

#include <cstring>

// --- Modifier Types (15 total) ---
enum ModifierType {
    MOD_NONE = 0,
    MOD_GLASS,       // 1: drones die to one hit, +2 skill aggression
    MOD_SWARM,       // 2: double drone count, skill capped lower
    MOD_LOWGRAV,     // 3: gravity halved
    MOD_DOUBLEPTS,   // 4: wave clear grants x2 upgrade points
    MOD_TIMEWARP,    // 5: player speed scaled up
    MOD_VAMPIRE,     // 6: each kill heals player (lifesteal)
    MOD_FRENZY,      // 7: damage multiplier boosted
    MOD_OVERSHIELD,  // 8: bonus armor at wave start
    MOD_MIRROR,      // 9: bots' damage partially reflected back
    MOD_REGEN,       // 10: player regenerates HP at wave start
    MOD_SURGE,       // 11: tougher drones, x3 upgrade points
    MOD_FROST,       // 12: slowed player, frosty drones
    MOD_CHAOS,       // 13: chaotic spawns (random skill per drone)
    MOD_MIMIC,       // 14: drones copy random upgrade from human
    MOD_SHIELD,      // 15: temporary invulnerability at wave start
    MOD_POOL_SIZE    // 16: count
};

// --- Synergy Pair ---
struct SynergyPair {
    ModifierType a;
    ModifierType b;
    const char* name;
    bool anti; // true = anti-synergy (penalty), false = synergy (bonus)
};

// --- Modifier State (per-wave) ---
struct ModifierState {
    ModifierType slot1 = MOD_NONE;
    ModifierType slot2 = MOD_NONE;       // second modifier slot (wave >= 8)
    int synergyIndex = -1;               // index into synergy table, -1 = none
    unsigned int modifiersSeen = 0;      // bitmask of modifiers encountered this run

    // Active effect values (computed when modifiers applied)
    float gravityScale = 1.0f;
    float speedScale = 1.0f;
    float damageMultiplier = 1.0f;
    int bonusArmor = 0;
    int vampiricHeal = 0;
    int mirrorDivisor = 3;               // damage reflected = botDmg / mirrorDivisor
    int pointsMultiplier = 1;
    bool shieldActive = false;
    float shieldDuration = 0.0f;
    bool regenActive = false;
    bool chaosActive = false;
    bool surgeActive = false;
    bool frostActive = false;
    bool mimicActive = false;
    bool glassActive = false;
    bool swarmActive = false;

    // Synergy effects
    float synergyGravityScale = 1.0f;
    float synergySpeedScale = 1.0f;
    float synergyDamageMultiplier = 1.0f;
    int synergyVampiricHeal = 0;
    int synergyMirrorDivisor = 3;
    int synergyPointsMultiplier = 1;
    int synergyBonusArmor = 0;

    ModifierState() { memset(this, 0, sizeof(*this)); }
};

// --- Functions ---

// Get display name for a modifier
const char* ModifierName(ModifierType mod);

// Check if a modifier is active in current state
bool IsModifierActive(const ModifierState& state, ModifierType mod);

// Select modifiers for a given wave (slot1 always, slot2 from wave 8+)
// dailyOffset: rotation offset for daily challenges
// forceMod1/forceMod2: test hooks (-1 = random)
void SelectModifiers(ModifierState& state, int wave, int dailyOffset = 0,
                     int forceMod1 = -1, int forceMod2 = -1);

// Detect synergy pair from active modifiers
void DetectSynergy(ModifierState& state);

// Apply modifier effects to game state (called at wave start)
// Returns the number of extra bots to spawn (SWARM doubles)
int ApplyModifiers(ModifierState& state);

// Update per-frame modifier effects (shield timer, etc.)
void UpdateModifiers(ModifierState& state, float dt);

// Get synergy display name (empty string if none)
const char* GetSynergyName(const ModifierState& state);

// Check if current synergy is anti-synergy
bool IsAntiSynergy(const ModifierState& state);
