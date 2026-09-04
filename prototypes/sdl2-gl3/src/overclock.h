#pragma once
// overclock.h - Roguelite upgrade system with bugs
// Each upgrade has a buff AND a bug (negative side effect)
#include <functional>
#include <string>
#include <vector>

class Game;

// Buff/Bug effect functions
using UpgradeEffect = std::function<void(Game&)>;

struct UpgradeDef {
    std::string name;
    std::string description;
    std::string bugName;
    std::string bugDescription;
    int maxLevel = 3;
    UpgradeEffect buff;
    UpgradeEffect bug;
};

// Active upgrade instance
struct ActiveUpgrade {
    int defIndex;
    int level = 1;
};

class OverclockManager {
public:
    OverclockManager();

    // Initialize upgrade definitions
    void init();

    // Get available upgrades for selection
    const std::vector<UpgradeDef>& getUpgrades() const { return upgrades; }

    // Apply an upgrade (buff + bug)
    void applyUpgrade(Game& game, int upgradeIndex);

    // Get upgrade level
    int getLevel(int upgradeIndex) const;

    // Reset all upgrades
    void reset();

    // Glitch meter (fills with bugs collected)
    float getGlitchMeter() const { return glitchMeter; }
    void addGlitch(float amount);
    bool spendGlitch(float amount); // Returns true if spent

    // Get active upgrades
    const std::vector<ActiveUpgrade>& getActiveUpgrades() const { return activeUpgrades; }

    // Count total bugs (sum of all upgrade levels * bug multiplier)
    int getTotalBugCount() const;

private:
    std::vector<UpgradeDef> upgrades;
    std::vector<ActiveUpgrade> activeUpgrades;
    float glitchMeter = 0.0f;
    float glitchDecay = 0.1f; // Glitch meter decays over time

    // Find active upgrade index, -1 if not found
    int findActive(int defIndex) const;
};
