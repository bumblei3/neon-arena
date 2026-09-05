#include "achievements.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

AchievementSystem::Achievement AchievementSystem::achievements[(int)ID::COUNT];
bool AchievementSystem::initialized = false;
std::vector<AchievementSystem::ID> AchievementSystem::newlyUnlocked_;

void AchievementSystem::initAchievements() {
    if (initialized) return;
    
    achievements[(int)ID::FIRST_BLOOD] = Achievement(ID::FIRST_BLOOD, "First Blood", "Kill your first bot", "ach_firstblood", 5);
    achievements[(int)ID::KILL_100] = Achievement(ID::KILL_100, "Bot Slayer", "Kill 100 bots", "ach_kill100", 10);
    achievements[(int)ID::KILL_1000] = Achievement(ID::KILL_1000, "Bot Annihilator", "Kill 1000 bots", "ach_kill1000", 25);
    achievements[(int)ID::RAILGUN_MASTER] = Achievement(ID::RAILGUN_MASTER, "Railgun Master", "Get 100 railgun kills", "ach_railgun", 15);
    achievements[(int)ID::LIGHTNING_MASTER] = Achievement(ID::LIGHTNING_MASTER, "Storm Bringer", "Get 100 lightning kills", "ach_lightning", 15);
    achievements[(int)ID::PLASMA_MASTER] = Achievement(ID::PLASMA_MASTER, "Plasma Specialist", "Get 100 plasma kills", "ach_plasma", 15);
    achievements[(int)ID::MULTIKILL_3] = Achievement(ID::MULTIKILL_3, "Triple Kill", "Kill 3 bots in 1 second", "ach_triple", 10);
    achievements[(int)ID::MULTIKILL_5] = Achievement(ID::MULTIKILL_5, "Pentakill", "Kill 5 bots in 1 second", "ach_penta", 20);
    
    achievements[(int)ID::WAVE_5] = Achievement(ID::WAVE_5, "Getting Started", "Survive to wave 5", "ach_wave5", 5);
    achievements[(int)ID::WAVE_10] = Achievement(ID::WAVE_10, "Veteran", "Survive to wave 10", "ach_wave10", 10);
    achievements[(int)ID::WAVE_20] = Achievement(ID::WAVE_20, "Survivor", "Survive to wave 20", "ach_wave20", 15);
    achievements[(int)ID::WAVE_30] = Achievement(ID::WAVE_30, "Endurance", "Survive to wave 30", "ach_wave30", 20);
    achievements[(int)ID::WAVE_50] = Achievement(ID::WAVE_50, "Marathon", "Survive to wave 50", "ach_wave50", 30);
    achievements[(int)ID::PERFECT_WAVE] = Achievement(ID::PERFECT_WAVE, "Untouchable", "Clear a wave without taking damage", "ach_perfect", 10);
    
    achievements[(int)ID::ALL_UPGRADES_LV5] = Achievement(ID::ALL_UPGRADES_LV5, "Maxed Out", "Max all upgrade types", "ach_upgrades", 20);
    achievements[(int)ID::ALL_WEAPONS] = Achievement(ID::ALL_WEAPONS, "Arsenal", "Use all weapons at least once", "ach_arsenal", 10);
    achievements[(int)ID::POINTS_100K] = Achievement(ID::POINTS_100K, "High Roller", "Accumulate 100k points", "ach_points", 15);
    
    achievements[(int)ID::NO_DAMAGE_W10] = Achievement(ID::NO_DAMAGE_W10, "Untouchable", "Reach wave 10 without damage", "ach_nodmg", 20);
    achievements[(int)ID::SPEEDRUN_5_MIN] = Achievement(ID::SPEEDRUN_5_MIN, "Speedrunner", "Reach wave 20 under 5 minutes", "ach_speed", 25);
    achievements[(int)ID::BOSS_RUSH] = Achievement(ID::BOSS_RUSH, "Boss Rush", "Kill 5 bosses in a row", "ach_boss", 20);
    achievements[(int)ID::COMBO_50] = Achievement(ID::COMBO_50, "Combo King", "Reach 50x combo", "ach_combo", 15);
    achievements[(int)ID::HEADSHOT_100] = Achievement(ID::HEADSHOT_100, "Sharpshooter", "Get 100 precision kills", "ach_headshot", 15);
    
    achievements[(int)ID::ECHO_CHAMPIOON] = Achievement(ID::ECHO_CHAMPIOON, "Echo Master", "Trigger echo-chaos 10 times", "ach_echo", 15, true);
    achievements[(int)ID::OVERCLOCKED] = Achievement(ID::OVERCLOCKED, "Overclocked", "Use 10 overclocks in one run", "ach_overclock", 15, true);
    achievements[(int)ID::FUSION_DISCOVERER] = Achievement(ID::FUSION_DISCOVERER, "Fusion Scholar", "Trigger all fusion types", "ach_fusion", 20, true);
    
    initialized = true;
}

const AchievementSystem::Achievement& AchievementSystem::getAchievement(ID id) {
    initAchievements();
    return achievements[(int)id];
}

void AchievementSystem::unlock(AchievementProgress& p, ID id) {
    initAchievements();
    if (!p.isUnlocked(id)) {
        newlyUnlocked_.push_back(id);
    }
    p.unlock(id);
}

void AchievementSystem::checkWaveAchievements(AchievementProgress& p, int currentWave, float runTimeSec, bool tookDamage) {
    initAchievements();
    
    if (currentWave >= 5) unlock(p, ID::WAVE_5);
    if (currentWave >= 10) unlock(p, ID::WAVE_10);
    if (currentWave >= 20) unlock(p, ID::WAVE_20);
    if (currentWave >= 30) unlock(p, ID::WAVE_30);
    if (currentWave >= 50) unlock(p, ID::WAVE_50);
    
    if (!tookDamage && currentWave > 1) {
        unlock(p, ID::PERFECT_WAVE);
    }
    
    // Speedrun: wave 20 under 5 minutes
    if (currentWave >= 20 && runTimeSec < 300.0f) {
        unlock(p, ID::SPEEDRUN_5_MIN);
    }
    
    // No damage to wave 10
    if (currentWave >= 10 && !tookDamage) {
        unlock(p, ID::NO_DAMAGE_W10);
    }
}

void AchievementSystem::checkCombatAchievements(AchievementProgress& p, const char* weapon, bool isHeadshot, int comboCount, int multikillCount) {
    initAchievements();
    
    // First blood
    unlock(p, ID::FIRST_BLOOD);
    
    // Weapon masters
    if (weapon) {
        if (strcmp(weapon, "Railgun") == 0) unlock(p, ID::RAILGUN_MASTER);
        else if (strcmp(weapon, "Lightning Gun") == 0) unlock(p, ID::LIGHTNING_MASTER);
        else if (strcmp(weapon, "Plasma Rifle") == 0) unlock(p, ID::PLASMA_MASTER);
    }
    
    // Multikills
    if (multikillCount >= 3) unlock(p, ID::MULTIKILL_3);
    if (multikillCount >= 5) unlock(p, ID::MULTIKILL_5);
    
    // Combo
    if (comboCount >= 50) unlock(p, ID::COMBO_50);
    
    // Headshots
    if (isHeadshot) unlock(p, ID::HEADSHOT_100);
}

void AchievementSystem::checkSkillAchievements(AchievementProgress& p, int echoCount, int overclockCount, int fusionTypes) {
    initAchievements();
    
    if (echoCount >= 10) unlock(p, ID::ECHO_CHAMPIOON);
    if (overclockCount >= 10) unlock(p, ID::OVERCLOCKED);
    if (fusionTypes >= 11) unlock(p, ID::FUSION_DISCOVERER); // All 11 fusions
}

void AchievementSystem::saveProgress(const AchievementProgress& p, FILE* f) {
    if (!f) return;
    fwrite(p.unlocked, sizeof(uint32_t), (uint32_t(ID::COUNT) + 31) / 32, f);
    fwrite(&p.totalPoints, sizeof(int), 1, f);
    fwrite(&p.totalAchievements, sizeof(int), 1, f);
}

bool AchievementSystem::loadProgress(AchievementProgress& p, FILE* f) {
    if (!f) return false;
    
    size_t n1 = fread(p.unlocked, sizeof(uint32_t), (uint32_t(ID::COUNT) + 31) / 32, f);
    size_t n2 = fread(&p.totalPoints, sizeof(int), 1, f);
    size_t n3 = fread(&p.totalAchievements, sizeof(int), 1, f);
    
    return n1 == (uint32_t(ID::COUNT) + 31) / 32 && n2 == 1 && n3 == 1;
}

const char* AchievementSystem::getIconName(ID id) {
    initAchievements();
    return achievements[(int)id].icon;
}

void AchievementSystem::printAchievements() {
    initAchievements();
    printf("\n=== Achievements (%d total) ===\n", (int)ID::COUNT);
    for (int i = 0; i < (int)ID::COUNT; i++) {
        auto& a = achievements[i];
        printf("  [%c] %s: %s (%d pts)%s\n", 
               a.secret ? '?' : ' ', a.name, a.description, a.points,
               a.secret ? " [SECRET]" : "");
    }
    printf("===============================\n");
}

void AchievementSystem::printProgress(const AchievementProgress& p) {
    initAchievements();
    printf("\n=== Achievement Progress ===\n");
    printf("  Unlocked: %d / %d\n", p.totalAchievements, (int)ID::COUNT);
    printf("  Points: %d\n", p.totalPoints);
    printf("  Unlocked: ");
    for (int i = 0; i < (int)ID::COUNT; i++) {
        if (p.isUnlocked((ID)i)) {
            printf("%s ", achievements[i].name);
        }
    }
    printf("\n============================\n");
}

int AchievementSystem::consumeNewlyUnlocked(ID* outArray, int maxCount) {
    initAchievements();
    int count = 0;
    for (auto id : newlyUnlocked_) {
        if (count >= maxCount) break;
        outArray[count++] = id;
    }
    newlyUnlocked_.clear();
    return count;
}

void AchievementSystem::clearNewlyUnlocked() {
    newlyUnlocked_.clear();
}
