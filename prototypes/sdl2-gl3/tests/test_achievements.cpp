// AchievementSystem tests - unlock logic, categories, save/load
#include <cstdio>
#include <cassert>
#include <cstring>
#include "achievements.h"

static int testsPassed = 0;
static int testsFailed = 0;

#define TEST(name, expr) do { \
    if (expr) { \
        printf("  [PASS] %s\n", name); \
        testsPassed++; \
    } else { \
        printf("  [FAIL] %s\n", name); \
        testsFailed++; \
    } \
} while(0)

// === Test Initialization ===

void testInit() {
    printf("\n[Initialization Tests]\n");
    
    TEST("achievement_count", AchievementSystem::getAchievementCount() == (int)AchievementSystem::ID::COUNT);
    
    auto& first = AchievementSystem::getAchievement(AchievementSystem::ID::FIRST_BLOOD);
    TEST("first_blood_name", strcmp(first.name, "First Blood") == 0);
    TEST("first_blood_points", first.points == 5);
    TEST("first_blood_not_secret", !first.secret);
    
    auto& secret = AchievementSystem::getAchievement(AchievementSystem::ID::ECHO_CHAMPIOON);
    TEST("echo_secret", secret.secret);
    
    auto& marathon = AchievementSystem::getAchievement(AchievementSystem::ID::WAVE_50);
    TEST("wave50_name", strcmp(marathon.name, "Marathon") == 0);
}

// === Test Unlocking ===

void testUnlock() {
    printf("\n[Unlock Tests]\n");
    
    AchievementSystem::AchievementProgress p;
    
    TEST("not_unlocked_initially", !p.isUnlocked(AchievementSystem::ID::FIRST_BLOOD));
    
    AchievementSystem::unlock(p, AchievementSystem::ID::FIRST_BLOOD);
    TEST("unlocked", p.isUnlocked(AchievementSystem::ID::FIRST_BLOOD));
    TEST("count_1", p.totalAchievements == 1);
    TEST("points_5", p.totalPoints == 5);
    
    // Unlock another
    AchievementSystem::unlock(p, AchievementSystem::ID::WAVE_5);
    TEST("count_2", p.totalAchievements == 2);
    TEST("points_10", p.totalPoints == 10);
    
    // Unlock same again — should not duplicate
    AchievementSystem::unlock(p, AchievementSystem::ID::FIRST_BLOOD);
    TEST("no_duplicate", p.totalAchievements == 2);
}

// === Test Wave Achievements ===

void testWaveAchievements() {
    printf("\n[Wave Achievement Tests]\n");
    
    AchievementSystem::AchievementProgress p;
    
    // Wave 3 — no achievements
    AchievementSystem::checkWaveAchievements(p, 3, 120.0f, true);
    TEST("wave3_none", p.totalAchievements == 0);
    
    // Wave 5 — unlocks WAVE_5
    AchievementSystem::checkWaveAchievements(p, 5, 180.0f, true);
    TEST("wave5_unlocked", p.isUnlocked(AchievementSystem::ID::WAVE_5));
    TEST("wave5_count", p.totalAchievements == 1);
    
    // Wave 10 — unlocks WAVE_10, NO_DAMAGE_W10 (if no damage)
    AchievementSystem::checkWaveAchievements(p, 10, 350.0f, false);
    TEST("wave10_unlocked", p.isUnlocked(AchievementSystem::ID::WAVE_10));
    TEST("nodmg_w10", p.isUnlocked(AchievementSystem::ID::NO_DAMAGE_W10));
    
    // Wave 20 speedrun (under 5 min)
    AchievementSystem::AchievementProgress p2;
    AchievementSystem::checkWaveAchievements(p2, 20, 250.0f, true);
    TEST("speedrun", p2.isUnlocked(AchievementSystem::ID::SPEEDRUN_5_MIN));
    
    // Wave 50 marathon
    AchievementSystem::AchievementProgress p3;
    AchievementSystem::checkWaveAchievements(p3, 50, 1200.0f, true);
    TEST("wave50", p3.isUnlocked(AchievementSystem::ID::WAVE_50));
    
    // Perfect wave (no damage)
    AchievementSystem::AchievementProgress p4;
    AchievementSystem::checkWaveAchievements(p4, 3, 60.0f, false);
    TEST("perfect_wave", p4.isUnlocked(AchievementSystem::ID::PERFECT_WAVE));
}

// === Test Combat Achievements ===

void testCombatAchievements() {
    printf("\n[Combat Achievement Tests]\n");
    
    AchievementSystem::AchievementProgress p;
    
    AchievementSystem::checkCombatAchievements(p, "Railgun", false, 1, 1);
    TEST("first_blood", p.isUnlocked(AchievementSystem::ID::FIRST_BLOOD));
    TEST("railgun_master", p.isUnlocked(AchievementSystem::ID::RAILGUN_MASTER));
    
    AchievementSystem::AchievementProgress p2;
    AchievementSystem::checkCombatAchievements(p2, "Lightning Gun", false, 1, 1);
    TEST("lightning_master", p2.isUnlocked(AchievementSystem::ID::LIGHTNING_MASTER));
    
    AchievementSystem::AchievementProgress p3;
    AchievementSystem::checkCombatAchievements(p3, "Plasma Rifle", false, 1, 1);
    TEST("plasma_master", p3.isUnlocked(AchievementSystem::ID::PLASMA_MASTER));
    
    // Multikill
    AchievementSystem::AchievementProgress p4;
    AchievementSystem::checkCombatAchievements(p4, nullptr, false, 1, 3);
    TEST("triple_kill", p4.isUnlocked(AchievementSystem::ID::MULTIKILL_3));
    TEST("no_penta", !p4.isUnlocked(AchievementSystem::ID::MULTIKILL_5));
    
    AchievementSystem::AchievementProgress p5;
    AchievementSystem::checkCombatAchievements(p5, nullptr, false, 1, 5);
    TEST("pentakill", p5.isUnlocked(AchievementSystem::ID::MULTIKILL_5));
    
    // Combo
    AchievementSystem::AchievementProgress p6;
    AchievementSystem::checkCombatAchievements(p6, nullptr, false, 50, 1);
    TEST("combo_50", p6.isUnlocked(AchievementSystem::ID::COMBO_50));
    
    // Headshot
    AchievementSystem::AchievementProgress p7;
    AchievementSystem::checkCombatAchievements(p7, nullptr, true, 1, 1);
    TEST("headshot", p7.isUnlocked(AchievementSystem::ID::HEADSHOT_100));
}

// === Test Skill Achievements ===

void testSkillAchievements() {
    printf("\n[Skill Achievement Tests]\n");
    
    AchievementSystem::AchievementProgress p;
    
    // Not enough
    AchievementSystem::checkSkillAchievements(p, 5, 3, 5);
    TEST("echo_not_enough", !p.isUnlocked(AchievementSystem::ID::ECHO_CHAMPIOON));
    
    // Enough echoes
    AchievementSystem::checkSkillAchievements(p, 15, 3, 5);
    TEST("echo_champion", p.isUnlocked(AchievementSystem::ID::ECHO_CHAMPIOON));
    
    // Enough overclocks
    AchievementSystem::checkSkillAchievements(p, 15, 12, 5);
    TEST("overclocked", p.isUnlocked(AchievementSystem::ID::OVERCLOCKED));
    
    // All fusions
    AchievementSystem::checkSkillAchievements(p, 15, 12, 11);
    TEST("fusion_discoverer", p.isUnlocked(AchievementSystem::ID::FUSION_DISCOVERER));
}

// === Test Save/Load ===

void testSaveLoad() {
    printf("\n[Save/Load Tests]\n");
    
    AchievementSystem::AchievementProgress p;
    AchievementSystem::unlock(p, AchievementSystem::ID::FIRST_BLOOD);
    AchievementSystem::unlock(p, AchievementSystem::ID::WAVE_10);
    AchievementSystem::unlock(p, AchievementSystem::ID::COMBO_50);
    
    // Save
    const char* testFile = "/tmp/test_achievements.bin";
    FILE* f = fopen(testFile, "wb");
    TEST("file_open", f != nullptr);
    if (f) {
        AchievementSystem::saveProgress(p, f);
        fclose(f);
    }
    
    // Load
    AchievementSystem::AchievementProgress p2;
    f = fopen(testFile, "rb");
    if (f) {
        AchievementSystem::loadProgress(p2, f);
        fclose(f);
    }
    
    TEST("loaded_first_blood", p2.isUnlocked(AchievementSystem::ID::FIRST_BLOOD));
    TEST("loaded_wave10", p2.isUnlocked(AchievementSystem::ID::WAVE_10));
    TEST("loaded_combo50", p2.isUnlocked(AchievementSystem::ID::COMBO_50));
    TEST("loaded_count", p2.totalAchievements == 3);
    TEST("loaded_points", p2.totalPoints == 30);
}

// === Test Total Count ===

void testTotalCount() {
    printf("\n[Total Count Tests]\n");
    
    // COUNT should be 25
    TEST("count_25", (int)AchievementSystem::ID::COUNT == 25);
}

// === Test Secret Achievements ===

void testSecretAchievements() {
    printf("\n[Secret Achievement Tests]\n");
    
    auto& echo = AchievementSystem::getAchievement(AchievementSystem::ID::ECHO_CHAMPIOON);
    auto& overclock = AchievementSystem::getAchievement(AchievementSystem::ID::OVERCLOCKED);
    auto& fusion = AchievementSystem::getAchievement(AchievementSystem::ID::FUSION_DISCOVERER);
    
    TEST("echo_secret", echo.secret);
    TEST("overclock_secret", overclock.secret);
    TEST("fusion_secret", fusion.secret);
    
    // Non-secret
    auto& first = AchievementSystem::getAchievement(AchievementSystem::ID::FIRST_BLOOD);
    TEST("first_blood_not_secret", !first.secret);
}

// === Test Print ===

void testPrint() {
    printf("\n[Print Test]\n");
    
    AchievementSystem::printAchievements();
    
    AchievementSystem::AchievementProgress p;
    AchievementSystem::unlock(p, AchievementSystem::ID::FIRST_BLOOD);
    AchievementSystem::printProgress(p);
    
    TEST("print_no_crash", true);
}

// === Main ===

int main() {
    printf("=== Achievement System Test Suite ===\n");
    
    testInit();
    testUnlock();
    testWaveAchievements();
    testCombatAchievements();
    testSkillAchievements();
    testSaveLoad();
    testTotalCount();
    testSecretAchievements();
    testPrint();
    
    printf("\n=== Results: %d passed, %d failed ===\n", testsPassed, testsFailed);
    
    return testsFailed > 0 ? 1 : 0;
}
