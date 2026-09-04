// playtest_tests.cpp - Balancing analysis and playtest simulation
#include <cstdio>
#include <cassert>
#include "../src/balancing.h"
#include "../src/playtest.h"

static int testsPassed = 0, testsFailed = 0;

#define TEST(name, expr) do { \
    printf("  [%s] %s... ", (expr) ? "PASS" : "FAIL", name); \
    fflush(stdout); \
    if (expr) { printf("PASSED\n"); testsPassed++; } \
    else { printf("FAILED\n"); testsFailed++; } \
} while(0)

void testBalancingDB() {
    printf("\n=== Balancing Database ===\n");
    
    const BalancingDB& db = BalancingDB::get();
    
    // Weapon tests
    {
        auto* railgun = db.getWeapon("Railgun");
        TEST("railgun_exists", railgun != nullptr);
        if (railgun) {
            TEST("railgun_dps", railgun->dps() > 100.0f);
            TEST("railgun_dps_value", railgun->dps() == 50.0f * 3.3f);
            TEST("railgun_range", railgun->range >= 80.0f);
        }
    }
    
    {
        auto* lightning = db.getWeapon("Lightning Gun");
        TEST("lightning_exists", lightning != nullptr);
        if (lightning) {
            TEST("lightning_fire_rate", lightning->fireRate >= 8.0f);
            TEST("lightning_range_low", lightning->range < 20.0f);
        }
    }
    
    {
        auto* plasma = db.getWeapon("Plasma Rifle");
        TEST("plasma_exists", plasma != nullptr);
        if (plasma) {
            TEST("plasma_high_damage", plasma->damage >= 70.0f);
        }
    }
    
    // Bot tests
    {
        auto* melee = db.getBot("Melee");
        TEST("melee_exists", melee != nullptr);
        if (melee) {
            TEST("melee_high_spawn", melee->spawnWeight >= 0.3f);
        }
    }
    
    {
        auto* boss = db.getBot("Boss");
        TEST("boss_exists", boss != nullptr);
        if (boss) {
            TEST("boss_high_health", boss->baseHealth >= 400.0f);
            TEST("boss_low_spawn", boss->spawnWeight <= 0.1f);
        }
    }
    
    // Wave tests
    {
        auto w1 = db.getWave(1);
        TEST("wave_1_not_boss", !w1.isBoss);
        TEST("wave_1_bot_count", w1.botCount >= 1);
        TEST("wave_1_health_mult", w1.healthMultiplier >= 1.0f);
    }
    
    {
        auto w5 = db.getWave(5);
        TEST("wave_5_is_boss", w5.isBoss);
        TEST("wave_5_boss_count", w5.bossCount >= 1);
        TEST("wave_5_minions", w5.minionCount >= 1);
    }
    
    {
        auto w10 = db.getWave(10);
        TEST("wave_10_is_boss", w10.isBoss);
        TEST("wave_10_more_health", w10.healthMultiplier > db.getWave(5).healthMultiplier);
    }
    
    {
        auto w20 = db.getWave(20);
        TEST("wave_20_is_boss", w20.isBoss);
        TEST("wave_20_high_health", w20.healthMultiplier >= 2.5f);
    }
    
    // Balance report
    {
        auto report = db.analyze();
        TEST("report_dps_positive", report.totalDpsP1 > 0);
        TEST("report_dps_w10_higher", report.totalDpsP10 > report.totalDpsP1);
        TEST("report_bot_health_increases", report.botHealthW10 > report.botHealthW1);
        TEST("report_ttk_positive", report.timeToKillW1 > 0);
        TEST("report_bot_cap_reasonable", report.recommendedBotCap >= 10 && report.recommendedBotCap <= 20);
    }
    
    // Difficulty curve
    {
        float d1 = db.getDifficulty(1);
        float d10 = db.getDifficulty(10);
        float d20 = db.getDifficulty(20);
        TEST("difficulty_increases", d10 > d1 && d20 > d10);
    }
    
    // Player power
    {
        float p1 = db.getPlayerPower(1, 1, 1, 1);
        float p10 = db.getPlayerPower(10, 3, 2, 2);
        TEST("player_power_increases", p10 > p1);
    }
}

void testPlaytestSimulation() {
    printf("\n=== Playtest Simulation ===\n");
    
    // Run simulation for 20 waves
    auto result = PlaytestSimulator::run(20);
    
    TEST("sim_completed", result.maxWaveReached >= 1);
    TEST("sim_has_kills", result.totalKills > 0);
    TEST("sim_damage_dealt", result.totalDamageDealt > 0);
    TEST("sim_game_time", result.gameTimeSeconds > 0);
    TEST("sim_dps_efficiency", result.dpsEfficiency > 0.0f && result.dpsEfficiency <= 1.0f);
    TEST("sim_no_nonsense_deaths", result.deaths <= 3);
    TEST("sim_upgrade_levels", result.railgunLevel >= 1 && result.plasmaLevel >= 1 && result.lightningLevel >= 1);
    
    // Print detailed report
    PlaytestSimulator::printReport(result);
}

int main() {
    testBalancingDB();
    testPlaytestSimulation();
    
    printf("\n=== Results: %d passed, %d failed ===\n", testsPassed, testsFailed);
    return testsFailed > 0 ? 1 : 0;
}
