#include "savegame.h"
#include "game.h"
#include "bots.h"
#include "weapons.h"
#include "powerups.h"
#include "specials.h"
#include "score.h"
#include <cstdio>
#include <cstring>

std::string SavegameManager::lastError_;

// Helper: write a value safely
template<typename T>
static bool writeVal(FILE* f, const T& val) {
    return fwrite(&val, sizeof(T), 1, f) == 1;
}

template<typename T>
static bool readVal(FILE* f, T& val) {
    return fread(&val, sizeof(T), 1, f) == 1;
}

// Helper: write vector size + elements
template<typename T>
static bool writeVec(FILE* f, const std::vector<T>& vec) {
    size_t s = vec.size();
    if (!writeVal(f, s)) return false;
    if (s > 0) {
        if (fwrite(vec.data(), sizeof(T), s, f) != s) return false;
    }
    return true;
}

template<typename T>
static bool readVec(FILE* f, std::vector<T>& vec) {
    size_t s = 0;
    if (!readVal(f, s)) return false;
    vec.resize(s);
    if (s > 0) {
        if (fread(vec.data(), sizeof(T), s, f) != s) return false;
    }
    return true;
}

bool SavegameManager::exists(const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");
    if (f) { fclose(f); return true; }
    return false;
}

const std::string& SavegameManager::getLastError() { return lastError_; }
void SavegameManager::clearError() { lastError_.clear(); }

bool SavegameManager::save(const Game& game, const std::string& path) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) {
        lastError_ = "Cannot open file for writing: " + path;
        return false;
    }

    // Magic number + version
    fwrite(SAVEGAME_MAGIC, strlen(SAVEGAME_MAGIC), 1, f);
    writeVal(f, SAVEGAME_VERSION);

    // Game state
    writeVal(f, game.wave);
    writeVal(f, game.score);
    writeVal(f, game.highScore);
    writeVal(f, game.kills);
    writeVal(f, game.gameTime);
    writeVal(f, game.gameOver);
    writeVal(f, game.waveComplete);
    writeVal(f, game.waveBreak);

    // Player
    writeVal(f, game.player.pos);
    writeVal(f, game.player.health);
    writeVal(f, game.player.yaw);
    writeVal(f, game.player.pitch);

    // Config
    writeVal(f, game.playerSpeed);
    writeVal(f, game.playerHeight);
    writeVal(f, game.mouseSensitivity);
    writeVal(f, game.maxHealth);
    writeVal(f, game.arenaSize);

    // Weapon system
    int weaponIdx = static_cast<int>(game.currentWeapon);
    writeVal(f, weaponIdx);
    writeVal(f, game.railgunCooldown);
    writeVal(f, game.lightningCooldown);
    writeVal(f, game.plasmaCooldown);
    writeVal(f, game.lightningRange);
    writeVal(f, game.railgunLevel);
    writeVal(f, game.lightningLevel);

    // Specials
    writeVal(f, game.nuclearBlastCooldown);
    writeVal(f, game.timeSlowCooldown);
    writeVal(f, game.shieldCooldown);
    writeVal(f, game.timeSlowTimer);
    writeVal(f, game.shieldTimer);
    writeVal(f, game.hasShield);

    // Upgrades
    writeVal(f, game.upgradePoints);
    writeVal(f, game.healthLevel);
    writeVal(f, game.speedLevel);

    // Score system
    writeVal(f, game.scoreMultiplier);
    writeVal(f, game.killStreak);
    writeVal(f, game.comboCount);
    writeVal(f, game.multiplierTimer);

    // HUD state
    writeVal(f, game.hitFeedbackTimer);
    writeVal(f, game.waveAnnounceTimer);

    // Bots (save alive bots with their full state)
    // Note: We need to save bot-specific fields. Since Entity has splitters and botType,
    // we need to save those too.
    size_t botCount = game.bots.size();
    writeVal(f, botCount);
    for (const auto& bot : game.bots) {
        writeVal(f, bot.pos);
        writeVal(f, bot.health);
        writeVal(f, bot.alive);
        writeVal(f, bot.yaw);
        writeVal(f, bot.botType);
        writeVal(f, bot.moveSpeed);
        writeVal(f, bot.splitters);
    }

    // Projectiles
    writeVec(f, game.projectiles);

    // Power-ups
    writeVec(f, game.powerUps);
    writeVal(f, game.damageBoostTimer);

    // Kill Feed
    writeVec(f, game.killFeed);

    // Damage Numbers
    writeVec(f, game.damageNumbers);

    fclose(f);
    return true;
}

bool SavegameManager::load(Game& game, const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) {
        lastError_ = "Cannot open file for reading: " + path;
        return false;
    }

    // Check magic
    char magic[10] = {0};
    if (fread(magic, strlen(SAVEGAME_MAGIC), 1, f) != 1 ||
        memcmp(magic, SAVEGAME_MAGIC, strlen(SAVEGAME_MAGIC)) != 0) {
        lastError_ = "Invalid savegame file (bad magic)";
        fclose(f);
        return false;
    }

    int version = 0;
    if (!readVal(f, version) || version != SAVEGAME_VERSION) {
        lastError_ = "Incompatible savegame version";
        fclose(f);
        return false;
    }

    // Game state
    readVal(f, game.wave);
    readVal(f, game.score);
    readVal(f, game.highScore);
    readVal(f, game.kills);
    readVal(f, game.gameTime);
    readVal(f, game.gameOver);
    readVal(f, game.waveComplete);
    readVal(f, game.waveBreak);

    // Player
    readVal(f, game.player.pos);
    readVal(f, game.player.health);
    readVal(f, game.player.yaw);
    readVal(f, game.player.pitch);

    // Config
    readVal(f, game.playerSpeed);
    readVal(f, game.playerHeight);
    readVal(f, game.mouseSensitivity);
    readVal(f, game.maxHealth);
    readVal(f, game.arenaSize);

    // Weapon system
    int weaponIdx = 0;
    readVal(f, weaponIdx);
    game.currentWeapon = static_cast<WeaponType>(weaponIdx);
    readVal(f, game.railgunCooldown);
    readVal(f, game.lightningCooldown);
    readVal(f, game.plasmaCooldown);
    readVal(f, game.lightningRange);
    readVal(f, game.railgunLevel);
    readVal(f, game.lightningLevel);

    // Specials
    readVal(f, game.nuclearBlastCooldown);
    readVal(f, game.timeSlowCooldown);
    readVal(f, game.shieldCooldown);
    readVal(f, game.timeSlowTimer);
    readVal(f, game.shieldTimer);
    readVal(f, game.hasShield);

    // Upgrades
    readVal(f, game.upgradePoints);
    readVal(f, game.healthLevel);
    readVal(f, game.speedLevel);

    // Score system
    readVal(f, game.scoreMultiplier);
    readVal(f, game.killStreak);
    readVal(f, game.comboCount);
    readVal(f, game.multiplierTimer);

    // HUD state
    readVal(f, game.hitFeedbackTimer);
    readVal(f, game.waveAnnounceTimer);

    // Bots
    size_t botCount = 0;
    readVal(f, botCount);
    game.bots.resize(botCount);
    for (auto& bot : game.bots) {
        readVal(f, bot.pos);
        readVal(f, bot.health);
        readVal(f, bot.alive);
        readVal(f, bot.yaw);
        readVal(f, bot.botType);
        readVal(f, bot.moveSpeed);
        readVal(f, bot.splitters);
    }

    // Projectiles
    readVec(f, game.projectiles);

    // Power-ups
    readVec(f, game.powerUps);
    readVal(f, game.damageBoostTimer);

    // Kill Feed
    readVec(f, game.killFeed);

    // Damage Numbers
    readVec(f, game.damageNumbers);

    fclose(f);
    return true;
}

bool SavegameManager::remove(const std::string& path) {
    return ::remove(path.c_str()) == 0;
}
