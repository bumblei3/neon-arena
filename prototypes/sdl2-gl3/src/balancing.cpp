#include "balancing.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

BalancingDB::BalancingDB() {
    initWeapons();
    initBots();
}

void BalancingDB::initWeapons() {
    weapons = {
        // Name, Damage, FireRate, Range, Speed, MaxLvl, UpgradeMult
        {"Railgun",      50.0f,  3.3f, 100.0f, 100.0f, 5, 0.15f},
        {"Lightning Gun", 8.0f, 10.0f,  15.0f,  50.0f, 5, 0.12f},
        {"Plasma Rifle", 80.0f,  2.0f,  80.0f,  30.0f, 5, 0.18f},
    };
}

void BalancingDB::initBots() {
    bots = {
    // Name,     BaseHP, HP/Wave, Spd,  Range, DPS,  CD,   Weight
        {"Melee",    70.0f,  7.0f,  3.5f,  2.0f, 12.0f, 1.0f, 0.35f},
        {"Shooter",  55.0f,  6.0f,  2.5f, 15.0f, 10.0f, 1.5f, 0.25f},
        {"Tank",    280.0f, 20.0f,  1.5f,  2.5f, 18.0f, 1.0f, 0.15f},
        {"Flanker",  40.0f,  4.0f,  5.0f,  2.0f, 10.0f, 0.8f, 0.20f},
        {"Boss",    750.0f, 55.0f,  2.0f, 20.0f, 25.0f, 1.0f, 0.05f},
    };
}

const WeaponBalance* BalancingDB::getWeapon(const char* name) const {
    for (auto& w : weapons) {
        if (strcmp(w.name, name) == 0) return &w;
    }
    return nullptr;
}

const BotBalance* BalancingDB::getBot(const char* name) const {
    for (auto& b : bots) {
        if (strcmp(b.name, name) == 0) return &b;
    }
    return nullptr;
}

WaveBalance BalancingDB::getWave(int wave) const {
    WaveBalance cfg;
    cfg.wave = wave;
    cfg.isBoss = (wave % 5 == 0);
    cfg.botCount = cfg.isBoss ? (1 + wave / 10) : (wave + 1);
    cfg.healthMultiplier = 1.0f + wave * 0.1f;
    cfg.speedMultiplier = 1.0f + wave * 0.02f;
    cfg.bossCount = cfg.isBoss ? 1 : 0;
    cfg.minionCount = cfg.isBoss ? (1 + wave / 5) : 0;
    cfg.pointsReward = wave * 100;
    cfg.upgradeReward = 1 + wave / 3;
    return cfg;
}

BalancingDB::BalanceReport BalancingDB::analyze() const {
    BalanceReport r = {};
    
    // Player DPS calculations
    auto* railgun = getWeapon("Railgun");
    auto* lightning = getWeapon("Lightning Gun");
    auto* plasma = getWeapon("Plasma Rifle");
    
    if (railgun) {
        r.totalDpsP1 += railgun->effectiveDps(1);
        r.totalDpsP10 += railgun->effectiveDps(3);
    }
    if (lightning) {
        r.totalDpsP1 += lightning->effectiveDps(1) * 0.7f; // Chain factor
        r.totalDpsP10 += lightning->effectiveDps(3) * 0.7f;
    }
    if (plasma) {
        r.totalDpsP1 += plasma->effectiveDps(1);
        r.totalDpsP10 += plasma->effectiveDps(3);
    }
    
    // Bot health
    auto* meleeBot = getBot("Melee");
    auto* shooterBot = getBot("Shooter");
    auto* tankBot = getBot("Tank");
    auto* bossBot = getBot("Boss");
    
    if (meleeBot) {
        r.botHealthW1 = meleeBot->baseHealth + meleeBot->healthPerWave;
        r.botHealthW10 = meleeBot->baseHealth + meleeBot->healthPerWave * 10.0f;
        r.botHealthW20 = meleeBot->baseHealth + meleeBot->healthPerWave * 20.0f;
    }
    if (bossBot) {
        r.bossHealthW5 = (bossBot->baseHealth + bossBot->healthPerWave * 5.0f) * 1.5f;
        r.bossHealthW10 = (bossBot->baseHealth + bossBot->healthPerWave * 10.0f) * 2.0f;
    }
    
    // Time to kill (using player DPS at wave 1)
    if (meleeBot && r.totalDpsP1 > 0) {
        r.timeToKillW1 = r.botHealthW1 / r.totalDpsP1;
        r.timeToKillW10 = r.botHealthW10 / r.totalDpsP10;
    }
    
    // Survival score (seconds of sustained damage needed)
    // Assumes player can dodge 50% of attacks
    if (meleeBot) {
        float avgDpsW10 = meleeBot->attackDamage / meleeBot->attackCooldown * 3; // 3 bots attacking
        r.survivalScoreW10 = 100.0f / (avgDpsW10 * 0.5f);
        
        float avgDpsW20 = avgDpsW10 * 1.5f; // More bots
        r.survivalScoreW20 = 100.0f / (avgDpsW20 * 0.5f);
    }
    
    // Spawn rate
    r.avgSpawnsPerSec = 0.5f; // 1 bot every 2 seconds
    r.recommendedBotCap = 15;
    
    return r;
}

float BalancingDB::getDifficulty(int wave) const {
    auto cfg = getWave(wave);
    float difficulty = cfg.botCount * cfg.healthMultiplier * cfg.speedMultiplier;
    if (cfg.isBoss) difficulty *= 1.5f;
    return difficulty;
}

float BalancingDB::getPlayerPower(int wave, int railgunLvl, int lightningLvl, int plasmaLvl) const {
    float power = 0;
    auto* rg = getWeapon("Railgun");
    auto* lg = getWeapon("Lightning Gun");
    auto* pl = getWeapon("Plasma Rifle");
    
    if (rg) power += rg->effectiveDps(railgunLvl);
    if (lg) power += lg->effectiveDps(lightningLvl) * 0.7f;
    if (pl) power += pl->effectiveDps(plasmaLvl);
    
    // Upgrade levels provide additional power
    power *= 1.0f + 0.05f * (railgunLvl + lightningLvl + plasmaLvl);
    
    return power;
}
