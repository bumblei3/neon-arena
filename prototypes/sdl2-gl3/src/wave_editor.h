#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// WaveEditor: Per-wave configuration editor for NeonArena
// Allows tuning: bot count, modifiers, boss waves, spawn patterns
// Loads/Saves JSON (minimal parser, no external deps)

class WaveEditor {
public:
    struct WaveConfigEntry {
        int wave = 0;
        int botCount = 0;            // Number of bots to spawn
        float healthMultiplier = 1.0f;
        float speedMultiplier = 1.0f;
        bool isBoss = false;
        int bossCount = 0;
        int minionCount = 0;
        int spawnRadius = 35;        // Arena radius for spawning (0 = default)
        float spawnAngleOffset = 0;  // Offset for angular spawn distribution
        bool spawnFromCenter = false; // Bots spawn from center outward
        int pointsReward = 0;        // Points awarded for clearing wave
        int upgradeReward = 0;       // Upgrade points awarded
        
        // Modifier flags (bitmask, see EnemyModifier)
        uint32_t modifiers = 0;
    };
    
    struct GlobalConfig {
        int maxBots = 15;            // Hard cap on bots per wave
        int bossEvery = 5;           // Boss wave interval
        float difficultyCurve = 1.0f; // Overall difficulty multiplier
        float speedPenalty = 0.0f;   // Speed reduction for players (coop slowdown)
        int totalWaves = 30;         // Total waves to generate
    };
    
    // Accessors
    static GlobalConfig& getGlobal() { return global; }
    static std::vector<WaveConfigEntry>& getEntries() { return entries; }
    
    // Load from JSON file (custom minimal parser)
    static bool load(const char* filename);
    static bool save(const char* filename);
    
    // Generate wave config from editor settings
    static int generateWaveConfig(int currentWave);
    
    // Editor UI helpers
    static void updateEditor();
    static void renderEditor();
    
    // Editor state
    static bool& isVisible() { return showEditor; }
    static bool& isGlobalVisible() { return showGlobal; }
    static int& currentWave() { return currentEditWave; }
    
    // Commands for interactive mode
    static void cmdSet(int wave, const char* field, const char* value);
    static void cmdAdd(int wave);
    static void cmdRemove(int wave);
    static void cmdGenerate(int count);
    static void cmdList();
    static void cmdShow(int wave);
    static void cmdHelp();
    
    static std::string modifierFlagsToString(uint32_t flags);
    static uint32_t stringToModifierFlags(const char* s);
    
private:
    static std::vector<WaveConfigEntry> entries;
    static GlobalConfig global;
    static int currentEditWave;
    static bool showEditor;
    static bool showGlobal;
    
    static void loadDefaults();
    static int findEntry(int wave);
    static void applyEntry(int wave, int entryIdx);
    static void clampEntry(WaveConfigEntry& e);
    
    // JSON parsing helpers
    static bool parseJSON(const char* filename);
    static bool writeJSON(const char* filename);
    static const char* skipWhitespace(const char* p);
    static const char* parseString(const char* p, std::string& out);
    static const char* parseNumber(const char* p, float& out);
    static const char* parseInt(const char* p, int& out);
    static std::string escapeString(const char* s);
    
    // Modifier names
    static const char* MODIFIER_NAMES[];
    static const uint32_t MODIFIER_VALUES[];
    static const int MODIFIER_COUNT;
};
