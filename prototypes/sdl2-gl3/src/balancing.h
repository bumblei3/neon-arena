// balancing.h - Balancing data for weapons, bots, and waves
#pragma once
#include <vector>
#include <cmath>

struct WeaponBalance {
    const char* name;
    float damage;
    float fireRate;       // Shots per second
    float range;
    float speed;          // Projectile speed
    int upgradeMax;
    float upgradeMult;    // Damage multiplier per upgrade level
    
    float dps() const { return damage * fireRate; }
    float effectiveDps(int level) const { return damage * (1.0f + upgradeMult * (level - 1)) * fireRate; }
};

struct BotBalance {
    const char* name;
    float baseHealth;
    float healthPerWave;
    float moveSpeed;
    float attackRange;
    float attackDamage;
    float attackCooldown;
    float spawnWeight;    // Relative spawn probability
};

struct WaveBalance {
    int wave;
    int botCount;
    float healthMultiplier;
    float speedMultiplier;
    bool isBoss;
    int bossCount;
    int minionCount;
    int pointsReward;
    int upgradeReward;
};

class BalancingDB {
public:
    static const BalancingDB& get() {
        static BalancingDB instance;
        return instance;
    }
    
    // Weapon data
    const WeaponBalance* getWeapon(const char* name) const;
    const std::vector<WeaponBalance>& getWeapons() const { return weapons; }
    
    // Bot data
    const BotBalance* getBot(const char* name) const;
    const std::vector<BotBalance>& getBots() const { return bots; }
    
    // Wave data (procedural generation based on parameters)
    WaveBalance getWave(int wave) const;
    
    // Balance analysis
    struct BalanceReport {
        float totalDpsP1;        // Total player DPS at wave 1
        float totalDpsP10;       // Total player DPS at wave 10
        float botHealthW1;       // Average bot health wave 1
        float botHealthW10;      // Average bot health wave 10
        float botHealthW20;      // Average bot health wave 20
        float bossHealthW5;      // Boss health wave 5
        float bossHealthW10;     // Boss health wave 10
        float timeToKillW1;      // Seconds to kill avg bot wave 1
        float timeToKillW10;     // Seconds to kill avg bot wave 10
        float survivalScoreW10;  // Estimated survival time at wave 10
        float survivalScoreW20;  // Estimated survival time at wave 20
        float avgSpawnsPerSec;   // Average bot spawn rate
        int recommendedBotCap;   // Recommended bot count limit
    };
    
    BalanceReport analyze() const;
    
    // Difficulty curve
    float getDifficulty(int wave) const;
    float getPlayerPower(int wave, int railgunLvl, int lightningLvl, int plasmaLvl) const;
    
private:
    BalancingDB();
    
    std::vector<WeaponBalance> weapons;
    std::vector<BotBalance> bots;
    
    void initWeapons();
    void initBots();
};
