// WaveConfig tests - standalone tests for wave generation
// Tests generateWaveConfig without game.h dependency
#include <cstdio>
#include <cassert>
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
    
    // Test wave 1 - should be simple
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
    
    // Test wave 3 - first modifiers appear
    {
        WaveConfig cfg = generateWaveConfig(3);
        WAVE_TEST("wave_3_speed_boost", hasModifier(cfg.modifiers, EnemyModifier::SPEED_BOOST));
        WAVE_TEST("wave_3_no_shield", !hasModifier(cfg.modifiers, EnemyModifier::SHIELD));
    }
    
    // Test wave 5 - first boss wave
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
        WAVE_TEST("wave_6_speed_shield", hasModifier(cfg.modifiers, EnemyModifier::SPEED_BOOST));
        WAVE_TEST("wave_6_speed_shield_2", hasModifier(cfg.modifiers, EnemyModifier::SHIELD));
        WAVE_TEST("wave_6_no_regen", !hasModifier(cfg.modifiers, EnemyModifier::REGENERATION));
    }
    
    // Test wave 10 - boss with modifiers
    {
        WaveConfig cfg = generateWaveConfig(10);
        WAVE_TEST("wave_10_is_boss", cfg.isBossWave);
        WAVE_TEST("wave_10_shield", hasModifier(cfg.modifiers, EnemyModifier::SHIELD));
        WAVE_TEST("wave_10_regen", hasModifier(cfg.modifiers, EnemyModifier::REGENERATION));
        WAVE_TEST("wave_10_no_speed", !hasModifier(cfg.modifiers, EnemyModifier::SPEED_BOOST));
    }
    
    // Test wave 12 - all modifiers on normal wave
    {
        WaveConfig cfg = generateWaveConfig(12);
        WAVE_TEST("wave_12_speed", hasModifier(cfg.modifiers, EnemyModifier::SPEED_BOOST));
        WAVE_TEST("wave_12_shield", hasModifier(cfg.modifiers, EnemyModifier::SHIELD));
        WAVE_TEST("wave_12_regen", hasModifier(cfg.modifiers, EnemyModifier::REGENERATION));
        WAVE_TEST("wave_12_splitter", hasModifier(cfg.modifiers, EnemyModifier::SPLITTER));
    }
    
    // Test wave 20 - boss with all modifiers
    {
        WaveConfig cfg = generateWaveConfig(20);
        WAVE_TEST("wave_20_is_boss", cfg.isBossWave);
        WAVE_TEST("wave_20_speed", hasModifier(cfg.modifiers, EnemyModifier::SPEED_BOOST));
        WAVE_TEST("wave_20_shield", hasModifier(cfg.modifiers, EnemyModifier::SHIELD));
        WAVE_TEST("wave_20_regen", hasModifier(cfg.modifiers, EnemyModifier::REGENERATION));
    }
    
    // Test wave 25 - deep boss
    {
        WaveConfig cfg = generateWaveConfig(25);
        WAVE_TEST("wave_25_is_boss", cfg.isBossWave);
        WAVE_TEST("wave_25_minions", cfg.minionCount == 6);
        WAVE_TEST("wave_25_health_mult", cfg.healthMultiplier == 3.5f);
    }
    
    // Test modifier bitwise operations
    {
        EnemyModifier m = EnemyModifier::SPEED_BOOST | EnemyModifier::SHIELD;
        WAVE_TEST("modifier_or_speed", hasModifier(m, EnemyModifier::SPEED_BOOST));
        WAVE_TEST("modifier_or_shield", hasModifier(m, EnemyModifier::SHIELD));
        WAVE_TEST("modifier_or_no_regen", !hasModifier(m, EnemyModifier::REGENERATION));
    }
    
    printf("\n[Wave Config Results] Passed: %d, Failed: %d\n", waveTestsPassed, waveTestsFailed);
}

int main() {
    testWaveConfig();
    return waveTestsFailed > 0 ? 1 : 0;
}
