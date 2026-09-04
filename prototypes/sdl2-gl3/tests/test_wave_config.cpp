// WaveConfig tests - standalone tests for wave generation
#include <cstdio>
#include <cassert>
#include <cstring>
#include "wave_config.h"

static int waveTestsPassed = 0;
static int waveTestsFailed = 0;

#define WAVE_TEST(name, expr) do { \
    if (expr) { \
        printf("  Running %s... PASSED\n", name); \
        waveTestsPassed++; \
    } else { \
        printf("  Running %s... FAILED\n", name); \
        waveTestsFailed++; \
    } \
} while(0)

void testWaveConfig() {
    printf("\n[Wave Config Tests]\n");
    
    // Test wave 1
    {
        WaveConfig cfg = generateWaveConfig(1);
        WAVE_TEST("wave_1_no_boss", !cfg.isBossWave);
        WAVE_TEST("wave_1_base_count", cfg.baseBotCount == 2);
        WAVE_TEST("wave_1_no_modifiers", cfg.modifiers == EnemyModifier::NONE);
        WAVE_TEST("wave_1_health_mult", cfg.healthMultiplier == 1.05f);
    }
    
    // Test wave 2
    {
        WaveConfig cfg = generateWaveConfig(2);
        WAVE_TEST("wave_2_no_boss", !cfg.isBossWave);
        WAVE_TEST("wave_2_base_count", cfg.baseBotCount == 3);
        WAVE_TEST("wave_2_no_modifiers", cfg.modifiers == EnemyModifier::NONE);
    }
    
    // Test wave 3
    {
        WaveConfig cfg = generateWaveConfig(3);
        WAVE_TEST("wave_3_speed_boost", (cfg.modifiers & EnemyModifier::SPEED_BOOST) != 0);
        WAVE_TEST("wave_3_no_shield", (cfg.modifiers & EnemyModifier::SHIELD) == 0);
    }
    
    // Test wave 5
    {
        WaveConfig cfg = generateWaveConfig(5);
        WAVE_TEST("wave_5_is_boss", cfg.isBossWave);
        WAVE_TEST("wave_5_one_boss", cfg.bossCount == 1);
        WAVE_TEST("wave_5_minions", cfg.minionCount == 2);
        WAVE_TEST("wave_5_no_modifiers", cfg.modifiers == EnemyModifier::NONE);
        WAVE_TEST("wave_5_health_mult", cfg.healthMultiplier == 1.5f);
    }
    
    // Test wave 6
    {
        WaveConfig cfg = generateWaveConfig(6);
        WAVE_TEST("wave_6_speed_shield", (cfg.modifiers & EnemyModifier::SPEED_BOOST) != 0);
        WAVE_TEST("wave_6_speed_shield_2", (cfg.modifiers & EnemyModifier::SHIELD) != 0);
        WAVE_TEST("wave_6_no_regen", (cfg.modifiers & EnemyModifier::REGENERATION) == 0);
    }
    
    // Test wave 10
    {
        WaveConfig cfg = generateWaveConfig(10);
        WAVE_TEST("wave_10_is_boss", cfg.isBossWave);
        WAVE_TEST("wave_10_shield", (cfg.modifiers & EnemyModifier::SHIELD) != 0);
        WAVE_TEST("wave_10_regen", (cfg.modifiers & EnemyModifier::REGENERATION) != 0);
        WAVE_TEST("wave_10_no_speed", (cfg.modifiers & EnemyModifier::SPEED_BOOST) == 0);
    }
    
    // Test wave 12
    {
        WaveConfig cfg = generateWaveConfig(12);
        WAVE_TEST("wave_12_speed", (cfg.modifiers & EnemyModifier::SPEED_BOOST) != 0);
        WAVE_TEST("wave_12_shield", (cfg.modifiers & EnemyModifier::SHIELD) != 0);
        WAVE_TEST("wave_12_regen", (cfg.modifiers & EnemyModifier::REGENERATION) != 0);
        WAVE_TEST("wave_12_splitter", (cfg.modifiers & EnemyModifier::SPLITTER) != 0);
    }
    
    // Test wave 20
    {
        WaveConfig cfg = generateWaveConfig(20);
        WAVE_TEST("wave_20_is_boss", cfg.isBossWave);
        WAVE_TEST("wave_20_speed", (cfg.modifiers & EnemyModifier::SPEED_BOOST) != 0);
        WAVE_TEST("wave_20_shield", (cfg.modifiers & EnemyModifier::SHIELD) != 0);
        WAVE_TEST("wave_20_regen", (cfg.modifiers & EnemyModifier::REGENERATION) != 0);
    }
    
    // Test wave 25
    {
        WaveConfig cfg = generateWaveConfig(25);
        WAVE_TEST("wave_25_is_boss", cfg.isBossWave);
        WAVE_TEST("wave_25_minions", cfg.minionCount == 6);
        WAVE_TEST("wave_25_health_mult", cfg.healthMultiplier == 3.5f);
    }
    
    printf("\n[Wave Config Results] Passed: %d, Failed: %d\n", waveTestsPassed, waveTestsFailed);
}

void testWaveFusions() {
    printf("\n[Wave Fusion Tests]\n");
    
    // Wave 14: SPEED + SHIELD + REGEN + SPLITTER (no frost/frenzy/timelapse/vampire)
    {
        WaveConfig cfg = generateWaveConfig(14);
        WaveFusion f = detectFusion(cfg.modifiers);
        WAVE_TEST("fusion_wave14_none", f == WaveFusion::NONE);
    }
    
    // Wave 16: FROST + FRENZY + VAMPIRE + TIMELAPSE
    {
        WaveConfig cfg = generateWaveConfig(16);
        WaveFusion f = detectFusion(cfg.modifiers);
        WAVE_TEST("fusion_wave16_eternal_winter", f == WaveFusion::ETERNAL_WINTER);
    }
    
    // Wave 19: FROST + FRENZY + VAMPIRE + TIMELAPSE
    {
        WaveConfig cfg = generateWaveConfig(19);
        WaveFusion f = detectFusion(cfg.modifiers);
        WAVE_TEST("fusion_wave19_eternal_winter", f == WaveFusion::ETERNAL_WINTER);
    }
    
    // Wave 21: FROST + FRENZY + VAMPIRE + TIMELAPSE
    {
        WaveConfig cfg = generateWaveConfig(21);
        WaveFusion f = detectFusion(cfg.modifiers);
        WAVE_TEST("fusion_wave21_bullet_hell", f == WaveFusion::BULLET_HELL);
    }
    
    // Wave 24: FROST + FRENZY + VAMPIRE + TIMELAPSE
    {
        WaveConfig cfg = generateWaveConfig(24);
        WaveFusion f = detectFusion(cfg.modifiers);
        WAVE_TEST("fusion_wave24_bullet_hell", f == WaveFusion::BULLET_HELL);
    }
    
    // Test fusion effects
    {
        const FusionEffect* eff = getFusionEffect(WaveFusion::NONE);
        WAVE_TEST("fusion_none_effect", eff->bonusPoints == 0);
    }
    
    // Test all fusion effects are defined
    {
        for (uint32_t i = 0; i < 11; i++) {
            const FusionEffect* eff = getFusionEffect((WaveFusion)i);
            if (!eff || eff->fusion != (WaveFusion)i) {
                printf("ERROR: fusion %u not defined!\n", i);
            }
        }
    }
    
    printf("\n[Fusion Test Results] Passed: %d, Failed: %d\n", waveTestsPassed, waveTestsFailed);
}

int main() {
    printf("=== Wave Config Test Suite ===\n");
    testWaveConfig();
    testWaveFusions();
    printf("\n=== Test Suite Complete ===\n");
    
    return 0;
}
