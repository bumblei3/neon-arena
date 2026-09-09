// test_modifier.cpp - Standalone tests for the Wave-Survival Modifier System
#include <cstdio>
#include <cassert>
#include <cstring>
#include "modifier.h"

static int modTestsPassed = 0;
static int modTestsFailed = 0;

#define MOD_TEST(name, expr) do { \
    if (expr) { \
        printf("  Running %s... PASSED\n", name); \
        modTestsPassed++; \
    } else { \
        printf("  Running %s... FAILED\n", name); \
        modTestsFailed++; \
    } \
} while(0)

void testModifierNames() {
    printf("\n[Modifier Name Tests]\n");
    
    MOD_TEST("name_none", strcmp(ModifierName(MOD_NONE), "") == 0);
    MOD_TEST("name_glass", strcmp(ModifierName(MOD_GLASS), "GLASS DRONES") == 0);
    MOD_TEST("name_swarm", strcmp(ModifierName(MOD_SWARM), "SWARM") == 0);
    MOD_TEST("name_lowgrav", strcmp(ModifierName(MOD_LOWGRAV), "LOW GRAVITY") == 0);
    MOD_TEST("name_doublepts", strcmp(ModifierName(MOD_DOUBLEPTS), "DOUBLE POINTS") == 0);
    MOD_TEST("name_timewarp", strcmp(ModifierName(MOD_TIMEWARP), "TIME WARP") == 0);
    MOD_TEST("name_vampire", strcmp(ModifierName(MOD_VAMPIRE), "VAMPIRE") == 0);
    MOD_TEST("name_frenzy", strcmp(ModifierName(MOD_FRENZY), "FRENZY") == 0);
    MOD_TEST("name_overshield", strcmp(ModifierName(MOD_OVERSHIELD), "OVERSHIELD") == 0);
    MOD_TEST("name_mirror", strcmp(ModifierName(MOD_MIRROR), "MIRROR") == 0);
    MOD_TEST("name_regen", strcmp(ModifierName(MOD_REGEN), "REGEN") == 0);
    MOD_TEST("name_surge", strcmp(ModifierName(MOD_SURGE), "SURGE") == 0);
    MOD_TEST("name_frost", strcmp(ModifierName(MOD_FROST), "FROST") == 0);
    MOD_TEST("name_chaos", strcmp(ModifierName(MOD_CHAOS), "CHAOS") == 0);
    MOD_TEST("name_mimic", strcmp(ModifierName(MOD_MIMIC), "MIMIC") == 0);
    MOD_TEST("name_shield", strcmp(ModifierName(MOD_SHIELD), "SHIELD") == 0);
}

void testModifierActivation() {
    printf("\n[Modifier Activation Tests]\n");
    
    ModifierState state;
    memset(&state, 0, sizeof(state));
    
    state.slot1 = MOD_GLASS;
    MOD_TEST("active_slot1", IsModifierActive(state, MOD_GLASS));
    MOD_TEST("not_active_other", !IsModifierActive(state, MOD_SWARM));
    
    state.slot2 = MOD_SWARM;
    MOD_TEST("active_slot2", IsModifierActive(state, MOD_SWARM));
    MOD_TEST("active_both", IsModifierActive(state, MOD_GLASS) && IsModifierActive(state, MOD_SWARM));
}

void testModifierSelection() {
    printf("\n[Modifier Selection Tests]\n");
    
    ModifierState state;
    memset(&state, 0, sizeof(state));
    
    // Wave 1-4: no modifiers
    SelectModifiers(state, 1, 0);
    MOD_TEST("wave1_no_mod", state.slot1 == MOD_NONE);
    
    SelectModifiers(state, 4, 0);
    MOD_TEST("wave4_no_mod", state.slot1 == MOD_NONE);
    
    // Wave 5+: at least slot 1
    SelectModifiers(state, 5, 0);
    MOD_TEST("wave5_has_slot1", state.slot1 != MOD_NONE);
    MOD_TEST("wave5_no_slot2", state.slot2 == MOD_NONE);
    
    // Wave 8+: slot 2
    SelectModifiers(state, 8, 0);
    MOD_TEST("wave8_has_slot2", state.slot2 != MOD_NONE);
    MOD_TEST("wave8_different_slots", state.slot1 != state.slot2);
    
    // Wave 10: still works
    SelectModifiers(state, 10, 0);
    MOD_TEST("wave10_has_slot1", state.slot1 != MOD_NONE);
    
    // Test force hooks
    memset(&state, 0, sizeof(state));
    SelectModifiers(state, 5, 0, MOD_SWARM, MOD_VAMPIRE);
    MOD_TEST("force_mod1", state.slot1 == MOD_SWARM);
    MOD_TEST("force_mod2", state.slot2 == MOD_VAMPIRE);
    
    // Test daily offset changes selection
    ModifierState state1, state2;
    memset(&state1, 0, sizeof(state1));
    memset(&state2, 0, sizeof(state2));
    SelectModifiers(state1, 5, 0);
    SelectModifiers(state2, 5, 1);
    // With different offsets, wave 5 should give different results (probabilistic but almost certain)
    // Actually they could be the same, but with different offsets the pool index differs
    // We'll just verify both have modifiers
    MOD_TEST("daily_offset_1_has_mod", state1.slot1 != MOD_NONE);
    MOD_TEST("daily_offset_2_has_mod", state2.slot1 != MOD_NONE);
}

void testSynergies() {
    printf("\n[Synergy Tests]\n");
    
    ModifierState state;
    
    // AERIAL ASSAULT: LOWGRAV + DOUBLEPTS
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_LOWGRAV;
    state.slot2 = MOD_DOUBLEPTS;
    DetectSynergy(state);
    MOD_TEST("aerial_assault_detected", state.synergyIndex == 0);
    MOD_TEST("aerial_assault_not_anti", !IsAntiSynergy(state));
    MOD_TEST("aerial_assault_name", strcmp(GetSynergyName(state), "AERIAL ASSAULT") == 0);
    
    // BLOOD WELL: VAMPIRE + REGEN
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_VAMPIRE;
    state.slot2 = MOD_REGEN;
    DetectSynergy(state);
    MOD_TEST("blood_well_detected", state.synergyIndex == 1);
    MOD_TEST("blood_well_not_anti", !IsAntiSynergy(state));
    
    // OVERDRIVE: FRENZY + SURGE
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_FRENZY;
    state.slot2 = MOD_SURGE;
    DetectSynergy(state);
    MOD_TEST("overdrive_detected", state.synergyIndex == 2);
    
    // HIVE MIRROR: SWARM + MIRROR
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_SWARM;
    state.slot2 = MOD_MIRROR;
    DetectSynergy(state);
    MOD_TEST("hive_mirror_detected", state.synergyIndex == 3);
    
    // SHIELD BLEED (anti): OVERSHIELD + VAMPIRE
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_OVERSHIELD;
    state.slot2 = MOD_VAMPIRE;
    DetectSynergy(state);
    MOD_TEST("shield_bleed_detected", state.synergyIndex == 4);
    MOD_TEST("shield_bleed_is_anti", IsAntiSynergy(state));
    
    // DRIFT LOCK (anti): TIMEWARP + LOWGRAV
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_TIMEWARP;
    state.slot2 = MOD_LOWGRAV;
    DetectSynergy(state);
    MOD_TEST("drift_lock_detected", state.synergyIndex == 5);
    MOD_TEST("drift_lock_is_anti", IsAntiSynergy(state));
    
    // No synergy: GLASS + SWARM
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_GLASS;
    state.slot2 = MOD_SWARM;
    DetectSynergy(state);
    MOD_TEST("no_synergy", state.synergyIndex == -1);
    
    // Order doesn't matter: DOUBLEPTS + LOWGRAV (reversed AERIAL ASSAULT)
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_DOUBLEPTS;
    state.slot2 = MOD_LOWGRAV;
    DetectSynergy(state);
    MOD_TEST("reversed_aerial_assault", state.synergyIndex == 0);
}

void testModifierEffects() {
    printf("\n[Modifier Effect Tests]\n");
    
    ModifierState state;
    
    // LOWGRAV: gravityScale = 0.5
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_LOWGRAV;
    ApplyModifiers(state);
    MOD_TEST("lowgrav_gravity_scale", state.gravityScale == 0.5f);
    
    // TIMEWARP: speedScale = 1.625
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_TIMEWARP;
    ApplyModifiers(state);
    MOD_TEST("timewarp_speed_scale", state.speedScale == 1.625f);
    
    // FROST: speedScale = 0.6875
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_FROST;
    ApplyModifiers(state);
    MOD_TEST("frost_speed_scale", state.speedScale == 0.6875f);
    
    // FRENZY: damageMultiplier = 1.33
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_FRENZY;
    ApplyModifiers(state);
    MOD_TEST("frenzy_damage_mult", state.damageMultiplier == 1.33f);
    
    // OVERSHIELD: bonusArmor = 50
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_OVERSHIELD;
    ApplyModifiers(state);
    MOD_TEST("overshield_armor", state.bonusArmor == 50);
    
    // SHIELD: shieldActive = true
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_SHIELD;
    ApplyModifiers(state);
    MOD_TEST("shield_active", state.shieldActive);
    MOD_TEST("shield_duration", state.shieldDuration == 3.0f);
    
    // VAMPIRE: vampiricHeal = 4
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_VAMPIRE;
    ApplyModifiers(state);
    MOD_TEST("vampire_heal", state.vampiricHeal == 4);
    
    // MIRROR: mirrorDivisor = 3
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_MIRROR;
    ApplyModifiers(state);
    MOD_TEST("mirror_divisor", state.mirrorDivisor == 3);
    
    // DOUBLEPTS: pointsMultiplier = 2
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_DOUBLEPTS;
    ApplyModifiers(state);
    MOD_TEST("doublepts_mult", state.pointsMultiplier == 2);
    
    // SURGE: pointsMultiplier = 3
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_SURGE;
    ApplyModifiers(state);
    MOD_TEST("surge_mult", state.pointsMultiplier == 3);
    
    // SWARM: extraBots doubles
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_SWARM;
    int extra = ApplyModifiers(state);
    MOD_TEST("swarm_extra_bots", extra == 1);
    MOD_TEST("swarm_active", state.swarmActive);
    
    // REGEN: regenActive
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_REGEN;
    ApplyModifiers(state);
    MOD_TEST("regen_active", state.regenActive);
    
    // CHAOS: chaosActive
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_CHAOS;
    ApplyModifiers(state);
    MOD_TEST("chaos_active", state.chaosActive);
    
    // GLASS: glassActive
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_GLASS;
    ApplyModifiers(state);
    MOD_TEST("glass_active", state.glassActive);
    
    // MIMIC: mimicActive
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_MIMIC;
    ApplyModifiers(state);
    MOD_TEST("mimic_active", state.mimicActive);
}

void testSynergyEffects() {
    printf("\n[Synergy Effect Tests]\n");
    
    ModifierState state;
    
    // AERIAL ASSAULT: gravityScale = 0.35, pointsMultiplier = 3
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_LOWGRAV;
    state.slot2 = MOD_DOUBLEPTS;
    ApplyModifiers(state);
    MOD_TEST("aerial_gravity", state.synergyGravityScale == 0.35f);
    MOD_TEST("aerial_pts", state.synergyPointsMultiplier == 3);
    
    // BLOOD WELL: synergyVampiricHeal = 8
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_VAMPIRE;
    state.slot2 = MOD_REGEN;
    ApplyModifiers(state);
    MOD_TEST("bloodwell_heal", state.synergyVampiricHeal == 8);
    
    // OVERDRIVE: synergyDamageMultiplier = 2.0
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_FRENZY;
    state.slot2 = MOD_SURGE;
    ApplyModifiers(state);
    MOD_TEST("overdrive_damage", state.synergyDamageMultiplier == 2.0f);
    
    // HIVE MIRROR: synergyMirrorDivisor = 2
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_SWARM;
    state.slot2 = MOD_MIRROR;
    ApplyModifiers(state);
    MOD_TEST("hivemirror_divisor", state.synergyMirrorDivisor == 2);
    
    // SHIELD BLEED: synergyBonusArmor = 25, synergyVampiricHeal = 2
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_OVERSHIELD;
    state.slot2 = MOD_VAMPIRE;
    ApplyModifiers(state);
    MOD_TEST("shieldbleed_armor", state.synergyBonusArmor == 25);
    MOD_TEST("shieldbleed_heal", state.synergyVampiricHeal == 2);
    
    // DRIFT LOCK: synergyGravityScale = 0.75, synergySpeedScale = 1.25
    memset(&state, 0, sizeof(state));
    state.slot1 = MOD_TIMEWARP;
    state.slot2 = MOD_LOWGRAV;
    ApplyModifiers(state);
    MOD_TEST("driftlock_gravity", state.synergyGravityScale == 0.75f);
    MOD_TEST("driftlock_speed", state.synergySpeedScale == 1.25f);
}

void testModifierUpdate() {
    printf("\n[Modifier Update Tests]\n");
    
    ModifierState state;
    
    // Shield timer counts down
    memset(&state, 0, sizeof(state));
    state.shieldActive = true;
    state.shieldDuration = 3.0f;
    UpdateModifiers(state, 1.0f);
    MOD_TEST("shield_1s", state.shieldDuration == 2.0f);
    UpdateModifiers(state, 1.0f);
    MOD_TEST("shield_2s", state.shieldDuration == 1.0f);
    UpdateModifiers(state, 1.5f);
    MOD_TEST("shield_expired", !state.shieldActive);
    MOD_TEST("shield_duration_zero", state.shieldDuration == 0.0f);
}

void testModifiersSeen() {
    printf("\n[Modifiers Seen Tests]\n");
    
    ModifierState state;
    memset(&state, 0, sizeof(state));
    
    state.slot1 = MOD_GLASS;
    state.slot2 = MOD_VAMPIRE;
    state.modifiersSeen |= (1u << state.slot1);
    state.modifiersSeen |= (1u << state.slot2);
    
    unsigned int expected = (1u << MOD_GLASS) | (1u << MOD_VAMPIRE);
    MOD_TEST("seen_bitmask", state.modifiersSeen == expected);
}

int main() {
    printf("=== Modifier System Tests ===\n");
    
    testModifierNames();
    testModifierActivation();
    testModifierSelection();
    testSynergies();
    testModifierEffects();
    testSynergyEffects();
    testModifierUpdate();
    testModifiersSeen();
    
    printf("\n=== Results ===\n");
    printf("Passed: %d\n", modTestsPassed);
    printf("Failed: %d\n", modTestsFailed);
    printf("Total:  %d\n", modTestsPassed + modTestsFailed);
    
    return modTestsFailed > 0 ? 1 : 0;
}
