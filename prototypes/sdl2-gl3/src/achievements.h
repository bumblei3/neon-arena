#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <cstdio>

// AchievementSystem: Track and notify player achievements
// Categories: Combat, Wave, Collection, Skill, Secret
// Storage: savegame-integrated (bit flags for unlocked achievements)
// Notification: server → client via CVar

class AchievementSystem {
public:
    enum class ID : uint32_t {
        // Combat
        FIRST_BLOOD = 0,        // Kill first bot
        KILL_100,               // Kill 100 bots total
        KILL_1000,              // Kill 1000 bots total
        RAILGUN_MASTER,         // 100 railgun kills
        LIGHTNING_MASTER,       // 100 lightning kills
        PLASMA_MASTER,          // 100 plasma kills
        MULTIKILL_3,            // 3 kills in 1 second
        MULTIKILL_5,            // 5 kills in 1 second
        
        // Wave
        WAVE_5,                 // Survive to wave 5
        WAVE_10,                // Survive to wave 10
        WAVE_20,                // Survive to wave 20
        WAVE_30,                // Survive to wave 30
        WAVE_50,                // Survive to wave 50
        PERFECT_WAVE,           // Clear wave without taking damage
        
        // Collection
        ALL_UPGRADES_LV5,       // Max all upgrade types
        ALL_WEAPONS,            // Use all weapons at least once
        POINTS_100K,            // Accumulate 100k points
        
        // Skill
        NO_DAMAGE_W10,          // Reach wave 10 without damage
        SPEEDRUN_5_MIN,         // Reach wave 20 under 5 minutes
        BOSS_RUSH,              // Kill 5 bosses in a row
        COMBO_50,               // 50x combo
        HEADSHOT_100,           // 100 precision kills
        
        // Secret
        ECHO_CHAMPIOON,         // Trigger echo-chaos 10 times
        OVERCLOCKED,            // Use 10 overclocks in one run
        FUSION_DISCOVERER,      // Trigger all fusion types
        
        COUNT
    };
    
    struct Achievement {
        ID id;
        const char* name;
        const char* description;
        const char* icon;           // Icon identifier (for UI)
        int points;                 // Achievement points
        bool secret;                // Hidden until unlocked
        
        Achievement() : id(ID::FIRST_BLOOD), name(nullptr), description(nullptr), icon(nullptr), points(0), secret(false) {}
        Achievement(ID i, const char* n, const char* d, const char* ic, int p, bool s = false)
            : id(i), name(n), description(d), icon(ic), points(p), secret(s) {}
    };
    
    struct AchievementProgress {
        uint32_t unlocked[(uint32_t(ID::COUNT) + 31) / 32];  // Bitmask
        int totalPoints;
        int totalAchievements;
        
        AchievementProgress() : totalPoints(0), totalAchievements(0) {
            memset(unlocked, 0, sizeof(unlocked));
        }
        
        bool isUnlocked(ID id) const {
            int idx = (int)id;
            return (unlocked[idx / 32] & (1 << (idx % 32))) != 0;
        }
        
        void unlock(ID id) {
            int idx = (int)id;
            if (!isUnlocked(id)) {
                unlocked[idx / 32] |= (1 << (idx % 32));
                totalAchievements++;
                totalPoints += getAchievement(id).points;
            }
        }
    };
    
    // Get achievement definition
    static const Achievement& getAchievement(ID id);
    static const Achievement& getAchievement(int idx) { return getAchievement((ID)idx); }
    static int getAchievementCount() { return (int)ID::COUNT; }
    
    // Check if achievement is unlocked
    static bool isUnlocked(const AchievementProgress& p, ID id) { return p.isUnlocked(id); }
    
    // Unlock achievement
    static void unlock(AchievementProgress& p, ID id);
    
    // Check wave-based achievements
    static void checkWaveAchievements(AchievementProgress& p, int currentWave, float runTimeSec, bool tookDamage);
    
    // Check combat achievements
    static void checkCombatAchievements(AchievementProgress& p, const char* weapon, bool isHeadshot, int comboCount, int multikillCount);
    
    // Check skill achievements
    static void checkSkillAchievements(AchievementProgress& p, int echoCount, int overclockCount, int fusionTypes);
    
    // Save/load progress
    static void saveProgress(const AchievementProgress& p, FILE* f);
    static bool loadProgress(AchievementProgress& p, FILE* f);
    
    // Get icon for achievement (returns placeholder string)
    static const char* getIconName(ID id);
    
    // Print all achievements
    static void printAchievements();
    static void printProgress(const AchievementProgress& p);
    
    // Get newly unlocked since last check
    static int getNewlyUnlocked(AchievementProgress& p, ID* outArray, int maxCount);
    
private:
    static Achievement achievements[(int)ID::COUNT];
    static bool initialized;
    static void initAchievements();
};
