#include <cstdio>
#include <cassert>
#include <vector>
#include <string>
#include <algorithm>

// Mock Game for testing
struct MockGame {
    float playerSpeed = 10.0f;
    float maxHealth = 100.0f;
    float playerHealth = 100.0f;
    float railgunFireRate = 0.3f;
    float plasmaFireRate = 0.5f;
    float plasmaDamage = 80.0f;
    int   lightningChainCount = 3;
    float shieldMaxCooldown = 15.0f;
    float shieldCrashChance = 0.0f;
    float railgunFeedbackChance = 0.0f;
    float lightningBacklashChance = 0.0f;
    float plasmaOverheatPenalty = 0.0f;
    int   splitterVirusLevel = 0;
    bool  splitterFriendlyFire = false;
    float scoreMultiplierFloat = 1.0f;
    float scoreDecayRate = 0.0f;
    float phaseGlitchChance = 0.0f;
    int   phaseShiftKills = 0;
    int   upgradePoints = 5;
    int   score = 0;
};

// Simplified OverclockManager for testing (without lambda)
struct Upgrade {
    std::string name;
    std::string bugName;
    int maxLevel = 3;
    int level = 0;
};

class TestOverclock {
public:
    std::vector<Upgrade> upgrades;
    float glitchMeter = 0.0f;

    TestOverclock() {
        upgrades.resize(8);
        upgrades[0] = {"Railgun Overclock", "Feedback Loop", 3};
        upgrades[1] = {"Plasma Overheat", "Overheat Crash", 3};
        upgrades[2] = {"Lightning Chain+", "Backlash", 3};
        upgrades[3] = {"Adrenaline Rush", "Fragile", 3};
        upgrades[4] = {"Shield Glitch", "System Crash", 3};
        upgrades[5] = {"Splitter Virus", "Friendly Fire", 2};
        upgrades[6] = {"Risky Multiplier", "Score Decay", 2};
        upgrades[7] = {"Phase Shift", "Phase Glitch", 2};
    }

    bool apply(MockGame& g, int idx) {
        if (idx < 0 || idx >= (int)upgrades.size()) return false;
        if (g.upgradePoints <= 0) return false;
        if (upgrades[idx].level >= upgrades[idx].maxLevel) return false;

        upgrades[idx].level++;
        g.upgradePoints--;
        glitchMeter += 5.0f * upgrades[idx].level;

        // Apply buff + bug based on index
        switch (idx) {
            case 0: // Railgun Overclock
                g.railgunFireRate *= 0.85f;
                g.railgunFeedbackChance += 0.10f;
                break;
            case 1: // Plasma Overheat
                g.plasmaFireRate *= 0.85f;
                g.plasmaOverheatPenalty += 1.0f;
                break;
            case 2: // Lightning Chain+
                g.lightningChainCount++;
                g.lightningBacklashChance += 0.08f;
                break;
            case 3: // Adrenaline Rush
                g.playerSpeed *= 1.15f;
                g.maxHealth *= 0.90f;
                if (g.playerHealth > g.maxHealth) g.playerHealth = g.maxHealth;
                break;
            case 4: // Shield Glitch
                g.shieldMaxCooldown *= 0.75f;
                g.shieldCrashChance += 0.05f;
                break;
            case 5: // Splitter Virus
                g.splitterVirusLevel++;
                g.splitterFriendlyFire = true;
                break;
            case 6: // Risky Multiplier
                g.scoreMultiplierFloat *= 1.5f;
                g.scoreDecayRate += 0.01f;
                break;
            case 7: // Phase Shift
                g.phaseShiftKills++;
                g.phaseGlitchChance += 0.05f;
                break;
        }
        return true;
    }

    void reset() {
        for (auto& u : upgrades) u.level = 0;
        glitchMeter = 0.0f;
    }
};

static int ocPassed = 0, ocFailed = 0;

#define OC_TEST(name, expr) do { \
    printf("  Running %s... ", name); \
    fflush(stdout); \
    if (expr) { printf("PASSED\n"); ocPassed++; } \
    else { printf("FAILED\n"); ocFailed++; } \
} while(0)

void testOverclock() {
    printf("\n[Overclock Upgrade Tests]\n");

    // Test apply upgrade
    {
        TestOverclock oc;
        MockGame g;
        bool ok = oc.apply(g, 0); // Railgun Overclock
        OC_TEST("apply_railgun_upgrade", ok);
        OC_TEST("upgrade_level_increased", oc.upgrades[0].level == 1);
        OC_TEST("upgrade_points_decreased", g.upgradePoints == 4);
        OC_TEST("glitch_meter_increased", oc.glitchMeter > 0.0f);
    }

    // Test railgun buff + bug
    {
        TestOverclock oc;
        MockGame g;
        float baseFireRate = g.railgunFireRate;
        oc.apply(g, 0);
        OC_TEST("railgun_fire_rate_increased", g.railgunFireRate < baseFireRate);
        OC_TEST("railgun_feedback_chance_set", g.railgunFeedbackChance == 0.10f);
    }

    // Test max level cap
    {
        TestOverclock oc;
        MockGame g;
        g.upgradePoints = 10;
        oc.apply(g, 0);
        oc.apply(g, 0);
        oc.apply(g, 0);
        OC_TEST("upgrade_level_3", oc.upgrades[0].level == 3);
        bool ok = oc.apply(g, 0); // Should fail - max level
        OC_TEST("max_level_blocks_fourth", !ok);
    }

    // Test upgrade points required
    {
        TestOverclock oc;
        MockGame g;
        g.upgradePoints = 0;
        bool ok = oc.apply(g, 0);
        OC_TEST("no_points_blocks_upgrade", !ok);
    }

    // Test all upgrades
    {
        TestOverclock oc;
        MockGame g;
        g.upgradePoints = 20;
        for (int i = 0; i < 8; i++) {
            oc.apply(g, i);
        }
        OC_TEST("all_upgrades_applied", oc.upgrades[7].level == 1);
    }

    // Test plasma overheat
    {
        TestOverclock oc;
        MockGame g;
        oc.apply(g, 1);
        OC_TEST("plasma_fire_rate_increased", g.plasmaFireRate < 0.5f);
        OC_TEST("plasma_overheat_penalty", g.plasmaOverheatPenalty == 1.0f);
    }

    // Test lightning chain
    {
        TestOverclock oc;
        MockGame g;
        oc.apply(g, 2);
        OC_TEST("lightning_chain_increased", g.lightningChainCount == 4);
        OC_TEST("lightning_backlash_set", g.lightningBacklashChance == 0.08f);
    }

    // Test adrenaline rush (speed + fragile)
    {
        TestOverclock oc;
        MockGame g;
        oc.apply(g, 3);
        OC_TEST("speed_increased", g.playerSpeed > 10.0f);
        OC_TEST("max_hp_decreased", g.maxHealth < 100.0f);
    }

    // Test shield glitch
    {
        TestOverclock oc;
        MockGame g;
        oc.apply(g, 4);
        OC_TEST("shield_cooldown_faster", g.shieldMaxCooldown < 15.0f);
        OC_TEST("shield_crash_chance", g.shieldCrashChance == 0.05f);
    }

    // Test splitter virus
    {
        TestOverclock oc;
        MockGame g;
        oc.apply(g, 5);
        OC_TEST("splitter_virus_level", g.splitterVirusLevel == 1);
        OC_TEST("splitter_friendly_fire", g.splitterFriendlyFire);
    }

    // Test risky multiplier
    {
        TestOverclock oc;
        MockGame g;
        oc.apply(g, 6);
        OC_TEST("score_multiplier_increased", g.scoreMultiplierFloat == 1.5f);
        OC_TEST("score_decay_set", g.scoreDecayRate == 0.01f);
    }

    // Test phase shift
    {
        TestOverclock oc;
        MockGame g;
        oc.apply(g, 7);
        OC_TEST("phase_shift_kills", g.phaseShiftKills == 1);
        OC_TEST("phase_glitch_chance", g.phaseGlitchChance == 0.05f);
    }

    // Test reset
    {
        TestOverclock oc;
        MockGame g;
        g.upgradePoints = 10;
        oc.apply(g, 0);
        oc.apply(g, 1);
        oc.apply(g, 2);
        oc.reset();
        OC_TEST("reset_levels_zero", oc.upgrades[0].level == 0);
        OC_TEST("reset_glitch_zero", oc.glitchMeter == 0.0f);
    }

    // Test glitch meter accumulation
    {
        TestOverclock oc;
        MockGame g;
        g.upgradePoints = 10;
        oc.apply(g, 0); // +5 glitch
        oc.apply(g, 1); // +5 glitch
        OC_TEST("glitch_accumulates", oc.glitchMeter == 10.0f);
    }

    // Test multiple levels on same upgrade
    {
        TestOverclock oc;
        MockGame g;
        g.upgradePoints = 10;
        oc.apply(g, 0); // Level 1
        oc.apply(g, 0); // Level 2
        OC_TEST("railgun_level_2", oc.upgrades[0].level == 2);
        OC_TEST("feedback_chance_level2", g.railgunFeedbackChance == 0.20f);
    }

    printf("\n[Overclock Results] Passed: %d, Failed: %d\n", ocPassed, ocFailed);
}

int main() {
    testOverclock();
    return ocFailed > 0 ? 1 : 0;
}
