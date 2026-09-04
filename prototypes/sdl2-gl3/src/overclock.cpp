#include "overclock.h"
#include "game.h"

OverclockManager::OverclockManager() {
    init();
}

void OverclockManager::init() {
    // === RAILGUN OVERCLOCK ===
    upgrades.push_back({
        "Railgun Overclock",
        "+30% Damage per level",
        "Feedback Loop",
        "10% Self-Damage on Miss per level",
        3,
        // Buff: increase railgun damage
        [](Game& g) {
            // Base damage already includes railgunLevel
            // We add a multiplier on top
            g.railgunLevel++;
        },
        // Bug: track feedback chance (stored in game via a field)
        [](Game& g) {
            // Feedback chance tracked in game struct
            g.railgunFeedbackChance += 0.10f;
        }
    });

    // === PLASMA OVERHEAT ===
    upgrades.push_back({
        "Plasma Overheat",
        "+20% Fire Rate per level",
        "Overheat Crash",
        "+1s Overheat cooldown per level",
        3,
        [](Game& g) {
            g.plasmaFireRate *= 0.85f; // Faster = lower cooldown
        },
        [](Game& g) {
            g.plasmaOverheatPenalty += 1.0f;
        }
    });

    // === LIGHTNING CHAIN+ ===
    upgrades.push_back({
        "Lightning Chain+",
        "+1 Chain Target per level",
        "Backlash",
        "8% Self-hit chance per level",
        3,
        [](Game& g) {
            g.lightningChainCount++;
        },
        [](Game& g) {
            g.lightningBacklashChance += 0.08f;
        }
    });

    // === SPEED BOOST ===
    upgrades.push_back({
        "Adrenaline Rush",
        "+15% Move Speed per level",
        "Fragile",
        "-10% Max HP per level",
        3,
        [](Game& g) {
            g.playerSpeed *= 1.15f;
        },
        [](Game& g) {
            g.maxHealth *= 0.90f;
            if (g.player.health > g.maxHealth) g.player.health = g.maxHealth;
        }
    });

    // === SHIELD GLITCH ===
    upgrades.push_back({
        "Shield Glitch",
        "Shield regenerates 50% faster",
        "System Crash",
        "5% Shield crash (10s stun) per level",
        3,
        [](Game& g) {
            g.shieldMaxCooldown *= 0.75f;
        },
        [](Game& g) {
            g.shieldCrashChance += 0.05f;
        }
    });

    // === SPLITTER VIRUS ===
    upgrades.push_back({
        "Splitter Virus",
        "Bots spawn +1 Mini-Bot",
        "Friendly Fire",
        "Mini-bots can damage player",
        2,
        [](Game& g) {
            g.splitterVirusLevel++;
        },
        [](Game& g) {
            g.splitterFriendlyFire = true;
        }
    });

    // === SCORE MULTIPLIER ===
    upgrades.push_back({
        "Risky Multiplier",
        "x1.5 Score multiplier",
        "Score Decay",
        "Lose 1% score per second",
        2,
        [](Game& g) {
            g.scoreMultiplierFloat *= 1.5f;
        },
        [](Game& g) {
            g.scoreDecayRate += 0.01f;
        }
    });

    // === INVULN CHARGE ===
    upgrades.push_back({
        "Phase Shift",
        "1s Invulnerability on kill",
        "Phase Glitch",
        "5% chance to phase through floor",
        2,
        [](Game& g) {
            g.phaseShiftKills++;
        },
        [](Game& g) {
            g.phaseGlitchChance += 0.05f;
        }
    });
}

void OverclockManager::applyUpgrade(Game& game, int upgradeIndex) {
    if (upgradeIndex < 0 || upgradeIndex >= (int)upgrades.size()) return;
    if (game.upgradePoints <= 0) return;

    auto& def = upgrades[upgradeIndex];

    // Find existing
    int activeIdx = findActive(upgradeIndex);
    if (activeIdx >= 0) {
        if (activeUpgrades[activeIdx].level >= def.maxLevel) return;
        activeUpgrades[activeIdx].level++;
    } else {
        activeUpgrades.push_back({upgradeIndex, 1});
    }

    // Apply buff
    if (def.buff) def.buff(game);

    // Apply bug
    if (def.bug) def.bug(game);

    game.upgradePoints--;

    // Add glitch for taking the upgrade
    addGlitch(5.0f * activeUpgrades[findActive(upgradeIndex)].level);
}

int OverclockManager::getLevel(int upgradeIndex) const {
    int idx = findActive(upgradeIndex);
    return (idx >= 0) ? activeUpgrades[idx].level : 0;
}

void OverclockManager::reset() {
    activeUpgrades.clear();
    glitchMeter = 0.0f;
}

void OverclockManager::addGlitch(float amount) {
    glitchMeter += amount;
    if (glitchMeter > 100.0f) glitchMeter = 100.0f;
}

bool OverclockManager::spendGlitch(float amount) {
    if (glitchMeter >= amount) {
        glitchMeter -= amount;
        return true;
    }
    return false;
}

int OverclockManager::findActive(int defIndex) const {
    for (int i = 0; i < (int)activeUpgrades.size(); i++) {
        if (activeUpgrades[i].defIndex == defIndex) return i;
    }
    return -1;
}

int OverclockManager::getTotalBugCount() const {
    int total = 0;
    for (auto& active : activeUpgrades) {
        total += active.level;
    }
    return total;
}
