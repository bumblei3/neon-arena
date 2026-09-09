// modifier.cpp - Wave-Survival Modifier System implementation
#include "modifier.h"
#include <cstdio>
#include <cstring>
#include <cmath>

// --- Synergy Table (6 pairs, matching OA-Mod nwSynergies[]) ---
static const SynergyPair synergyTable[] = {
    { MOD_LOWGRAV,   MOD_DOUBLEPTS, "AERIAL ASSAULT", false },
    { MOD_VAMPIRE,   MOD_REGEN,     "BLOOD WELL",     false },
    { MOD_FRENZY,    MOD_SURGE,     "OVERDRIVE",      false },
    { MOD_SWARM,     MOD_MIRROR,    "HIVE MIRROR",    false },
    { MOD_OVERSHIELD, MOD_VAMPIRE,  "SHIELD BLEED",   true  },
    { MOD_TIMEWARP,   MOD_LOWGRAV,  "DRIFT LOCK",     true  },
};
#define SYNERGY_COUNT (int)(sizeof(synergyTable) / sizeof(synergyTable[0]))

const char* ModifierName(ModifierType mod) {
    switch (mod) {
        case MOD_GLASS:      return "GLASS DRONES";
        case MOD_SWARM:      return "SWARM";
        case MOD_LOWGRAV:    return "LOW GRAVITY";
        case MOD_DOUBLEPTS:  return "DOUBLE POINTS";
        case MOD_TIMEWARP:   return "TIME WARP";
        case MOD_VAMPIRE:    return "VAMPIRE";
        case MOD_FRENZY:     return "FRENZY";
        case MOD_OVERSHIELD: return "OVERSHIELD";
        case MOD_MIRROR:     return "MIRROR";
        case MOD_REGEN:      return "REGEN";
        case MOD_SURGE:      return "SURGE";
        case MOD_FROST:      return "FROST";
        case MOD_CHAOS:      return "CHAOS";
        case MOD_MIMIC:      return "MIMIC";
        case MOD_SHIELD:     return "SHIELD";
        default:             return "";
    }
}

bool IsModifierActive(const ModifierState& state, ModifierType mod) {
    return (state.slot1 == mod || state.slot2 == mod);
}

void SelectModifiers(ModifierState& state, int wave, int dailyOffset,
                     int forceMod1, int forceMod2) {
    // Reset state
    state.slot1 = MOD_NONE;
    state.slot2 = MOD_NONE;
    state.synergyIndex = -1;
    state.shieldActive = false;
    state.regenActive = false;
    state.chaosActive = false;
    state.surgeActive = false;
    state.frostActive = false;
    state.mimicActive = false;
    state.glassActive = false;
    state.swarmActive = false;

    // Modifiers start from wave 5 through max-1 (including boss waves)
    if (wave < 5) return;

    // Test hooks: force specific modifiers
    if (forceMod1 >= 0 && forceMod1 < MOD_POOL_SIZE) {
        state.slot1 = (ModifierType)forceMod1;
        if (forceMod2 >= 0 && forceMod2 < MOD_POOL_SIZE) {
            state.slot2 = (ModifierType)forceMod2;
        }
    } else {
        // Random selection with daily offset rotation
        // Pool: MOD_GLASS (1) through MOD_SHIELD (15), excluding MOD_NONE (0)
        int poolSize = MOD_POOL_SIZE - 1; // 15 modifiers
        int idx = (dailyOffset + wave * 7) % poolSize;
        state.slot1 = (ModifierType)(1 + idx);

        // Second modifier from wave 8 onward (different from slot 1)
        if (wave >= 8) {
            int idx2 = (dailyOffset + wave * 13 + 3) % poolSize;
            state.slot2 = (ModifierType)(1 + idx2);
            if (state.slot2 == state.slot1) {
                state.slot2 = (ModifierType)(1 + (idx2 + 1) % poolSize);
            }
        }
    }

    // Track modifiers seen this run
    if (state.slot1 != MOD_NONE) state.modifiersSeen |= (1u << state.slot1);
    if (state.slot2 != MOD_NONE) state.modifiersSeen |= (1u << state.slot2);

    // Detect synergy pair
    DetectSynergy(state);

    // Log
    if (state.slot1 != MOD_NONE) {
        printf("[Modifier] Wave %d: %s", wave, ModifierName(state.slot1));
        if (state.slot2 != MOD_NONE) {
            printf(" + %s", ModifierName(state.slot2));
        }
        if (state.synergyIndex >= 0) {
            printf(" -> SYNERGY: %s%s",
                   IsAntiSynergy(state) ? "ANTI-" : "",
                   GetSynergyName(state));
        }
        printf("\n");
    }
}

void DetectSynergy(ModifierState& state) {
    state.synergyIndex = -1;
    if (state.slot2 == MOD_NONE) return;

    for (int i = 0; i < SYNERGY_COUNT; i++) {
        ModifierType a = synergyTable[i].a;
        ModifierType b = synergyTable[i].b;
        if ((state.slot1 == a && state.slot2 == b) ||
            (state.slot1 == b && state.slot2 == a)) {
            state.synergyIndex = i;
            printf("[Modifier] Synergy detected: %s%s\n",
                   synergyTable[i].anti ? "ANTI-" : "",
                   synergyTable[i].name);
            return;
        }
    }
}

int ApplyModifiers(ModifierState& state) {
    // Reset effect values to defaults
    state.gravityScale = 1.0f;
    state.speedScale = 1.0f;
    state.damageMultiplier = 1.0f;
    state.bonusArmor = 0;
    state.vampiricHeal = 0;
    state.mirrorDivisor = 3;
    state.pointsMultiplier = 1;
    state.shieldActive = false;
    state.shieldDuration = 0.0f;
    state.regenActive = false;
    state.chaosActive = false;
    state.surgeActive = false;
    state.frostActive = false;
    state.mimicActive = false;
    state.glassActive = false;
    state.swarmActive = false;

    // Reset synergy effects
    state.synergyGravityScale = 1.0f;
    state.synergySpeedScale = 1.0f;
    state.synergyDamageMultiplier = 1.0f;
    state.synergyVampiricHeal = 0;
    state.synergyMirrorDivisor = 3;
    state.synergyPointsMultiplier = 1;
    state.synergyBonusArmor = 0;

    // Detect synergy pair from active modifiers
    DetectSynergy(state);

    int extraBots = 0;

    // --- Apply individual modifiers ---
    if (IsModifierActive(state, MOD_LOWGRAV)) {
        state.gravityScale = 0.5f;
    }
    if (IsModifierActive(state, MOD_TIMEWARP)) {
        state.speedScale = 1.625f; // 520/320
    }
    if (IsModifierActive(state, MOD_FROST)) {
        state.speedScale = 0.6875f; // 220/320
        state.frostActive = true;
    }
    if (IsModifierActive(state, MOD_FRENZY)) {
        state.damageMultiplier = 1.33f; // quadfactor 4/3
    }
    if (IsModifierActive(state, MOD_OVERSHIELD)) {
        state.bonusArmor = 50;
    }
    if (IsModifierActive(state, MOD_SHIELD)) {
        state.shieldActive = true;
        state.shieldDuration = 3.0f;
    }
    if (IsModifierActive(state, MOD_VAMPIRE)) {
        state.vampiricHeal = 4;
    }
    if (IsModifierActive(state, MOD_MIRROR)) {
        state.mirrorDivisor = 3;
    }
    if (IsModifierActive(state, MOD_DOUBLEPTS)) {
        state.pointsMultiplier = 2;
    }
    if (IsModifierActive(state, MOD_SURGE)) {
        state.surgeActive = true;
        state.pointsMultiplier *= 3;
    }
    if (IsModifierActive(state, MOD_REGEN)) {
        state.regenActive = true;
    }
    if (IsModifierActive(state, MOD_CHAOS)) {
        state.chaosActive = true;
    }
    if (IsModifierActive(state, MOD_GLASS)) {
        state.glassActive = true;
    }
    if (IsModifierActive(state, MOD_SWARM)) {
        state.swarmActive = true;
        extraBots = 1; // doubles the count (caller multiplies by 2)
    }
    if (IsModifierActive(state, MOD_MIMIC)) {
        state.mimicActive = true;
    }

    // --- Apply synergy effects ---
    if (state.synergyIndex >= 0) {
        switch (state.synergyIndex) {
            case 0: // AERIAL ASSAULT: lower grav, x3 points
                state.synergyGravityScale = 0.35f; // 280/800
                state.synergyPointsMultiplier = 3;
                break;
            case 1: // BLOOD WELL: double lifesteal
                state.synergyVampiricHeal = 8;
                break;
            case 2: // OVERDRIVE: harder hits
                state.synergyDamageMultiplier = 2.0f; // quadfactor 6/3
                break;
            case 3: // HIVE MIRROR: stronger reflect
                state.synergyMirrorDivisor = 2;
                break;
            case 4: // SHIELD BLEED (anti): weaker defense + heal
                state.synergyBonusArmor = 25;
                state.synergyVampiricHeal = 2;
                break;
            case 5: // DRIFT LOCK (anti): clamped movement
                state.synergyGravityScale = 0.75f; // 600/800
                state.synergySpeedScale = 1.25f; // 400/320
                break;
        }
    }

    return extraBots;
}

void UpdateModifiers(ModifierState& state, float dt) {
    if (state.shieldActive) {
        state.shieldDuration -= dt;
        if (state.shieldDuration <= 0.0f) {
            state.shieldActive = false;
            state.shieldDuration = 0.0f;
        }
    }
}

const char* GetSynergyName(const ModifierState& state) {
    if (state.synergyIndex < 0 || state.synergyIndex >= SYNERGY_COUNT) return "";
    return synergyTable[state.synergyIndex].name;
}

bool IsAntiSynergy(const ModifierState& state) {
    if (state.synergyIndex < 0 || state.synergyIndex >= SYNERGY_COUNT) return false;
    return synergyTable[state.synergyIndex].anti;
}
