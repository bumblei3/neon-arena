// playtest.h - Headless playtest simulation
#pragma once
#include "balancing.h"

class PlaytestSimulator {
public:
    struct SimulationResult {
        int maxWaveReached;
        int totalKills;
        float totalDamageDealt;
        float totalDamageTaken;
        float gameTimeSeconds;
        int railgunLevel;
        int lightningLevel;
        int plasmaLevel;
        int upgradePointsEarned;
        int upgradePointsSpent;
        int deaths;
        int revives;
        bool bossKills[10];  // Boss kills per 5-wave interval
        float avgBotAliveTime;
        float dpsEfficiency;  // damage dealt / damage possible
    };

    static SimulationResult run(int numWaves = 50, int startWave = 1);
    
    static void printReport(const SimulationResult& r);
    
private:
    struct SimBot {
        float health;
        float posX, posZ;
        float moveSpeed;
        float attackCooldown;
        float attackRange;
        float attackDamage;
        float spawnTime;
        float deathTime;
        bool alive;
        int type; // 0=melee, 1=shooter, 2=tank, 3=flanker, 4=boss
    };
    
    struct SimPlayer {
        float health = 100;
        float posX = 0, posZ = 0;
        float railgunCooldown = 0;
        float lightningCooldown = 0;
        float plasmaCooldown = 0;
        bool alive = true;
    };
};
