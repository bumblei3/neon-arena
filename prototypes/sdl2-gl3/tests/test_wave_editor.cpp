// WaveEditor tests - JSON load/save, commands, modifier parsing
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include "wave_editor.h"

static int testsPassed = 0;
static int testsFailed = 0;

#define TEST(name, expr) do { \
    if (expr) { \
        printf("  [PASS] %s\n", name); \
        testsPassed++; \
    } else { \
        printf("  [FAIL] %s\n", name); \
        testsFailed++; \
    } \
} while(0)

// === Test Modifier Flag Conversion ===

void testModifierFlags() {
    printf("\n[Modifier Flag Tests]\n");
    
    // String to flags
    TEST("none_flags", WaveEditor::stringToModifierFlags("NONE") == 0);
    TEST("speed_boost", WaveEditor::stringToModifierFlags("SPEED_BOOST") == 1);
    TEST("shield", WaveEditor::stringToModifierFlags("SHIELD") == 2);
    TEST("regen", WaveEditor::stringToModifierFlags("REGENERATION") == 4);
    TEST("splitter", WaveEditor::stringToModifierFlags("SPLITTER") == 8);
    TEST("frost", WaveEditor::stringToModifierFlags("FROST") == 16);
    TEST("frenzy", WaveEditor::stringToModifierFlags("FRENZY") == 32);
    TEST("vampire", WaveEditor::stringToModifierFlags("VAMPIRE") == 64);
    TEST("timelapse", WaveEditor::stringToModifierFlags("TIMELAPSE") == 128);
    
    // Combined
    TEST("speed_shield", WaveEditor::stringToModifierFlags("SPEED_BOOST|SHIELD") == 3);
    TEST("all_mods", WaveEditor::stringToModifierFlags("SPEED_BOOST|SHIELD|REGENERATION|SPLITTER|FROST|FRENZY|VAMPIRE|TIMELAPSE") == 255);
    
    // Hex parsing
    TEST("hex_0x0F", WaveEditor::stringToModifierFlags("0x0F") == 15);
    TEST("hex_0xFF", WaveEditor::stringToModifierFlags("0xFF") == 255);
    
    // Flags to string
    TEST("flags_to_none", strcmp(WaveEditor::modifierFlagsToString(0).c_str(), "NONE") == 0);
    TEST("flags_to_speed", strcmp(WaveEditor::modifierFlagsToString(1).c_str(), "SPEED_BOOST") == 0);
    TEST("flags_to_combo", strcmp(WaveEditor::modifierFlagsToString(3).c_str(), "SPEED_BOOST|SHIELD") == 0);
}

// === Test JSON Save ===

void testJSONSave() {
    printf("\n[JSON Save Tests]\n");
    
    // Generate default waves
    WaveEditor::cmdGenerate(5);
    
    // Save to file
    const char* testFile = "/tmp/test_wave_editor.json";
    bool saved = WaveEditor::save(testFile);
    TEST("save_file", saved);
    
    // Check file exists and has content
    FILE* f = fopen(testFile, "r");
    TEST("file_exists", f != nullptr);
    if (f) {
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        TEST("file_has_content", size > 0);
        fclose(f);
    }
}

// === Test JSON Load ===

void testJSONLoad() {
    printf("\n[JSON Load Tests]\n");
    
    const char* testFile = "/tmp/test_wave_editor.json";
    
    // Clear entries
    WaveEditor::getEntries().clear();
    
    // Load from file
    bool loaded = WaveEditor::load(testFile);
    TEST("load_file", loaded);
    TEST("loaded_5_waves", WaveEditor::getEntries().size() == 5);
    
    if (WaveEditor::getEntries().size() >= 1) {
        auto& w1 = WaveEditor::getEntries()[0];
        TEST("wave1_number", w1.wave == 1);
        TEST("wave1_botCount", w1.botCount == 2);
    }
    
    if (WaveEditor::getEntries().size() >= 5) {
        auto& w5 = WaveEditor::getEntries()[4];
        TEST("wave5_isBoss", w5.isBoss == true);
    }
}

// === Test JSON Roundtrip ===

void testJSONRoundtrip() {
    printf("\n[JSON Roundtrip Tests]\n");
    
    // Create and save
    WaveEditor::getEntries().clear();
    WaveEditor::cmdGenerate(3);
    
    // Modify a wave
    WaveEditor::cmdSet(1, "botCount", "10");
    WaveEditor::cmdSet(1, "healthMult", "2.5");
    WaveEditor::cmdSet(1, "modifiers", "SPEED_BOOST|SHIELD");
    
    const char* testFile = "/tmp/test_roundtrip.json";
    WaveEditor::save(testFile);
    
    // Clear and reload
    WaveEditor::getEntries().clear();
    WaveEditor::load(testFile);
    
    TEST("roundtrip_3_waves", WaveEditor::getEntries().size() == 3);
    
    // Check wave 1
    auto& entries = WaveEditor::getEntries();
    bool found = false;
    for (auto& e : entries) {
        if (e.wave == 1) {
            TEST("roundtrip_botCount", e.botCount == 10);
            TEST("roundtrip_healthMult", e.healthMultiplier == 2.5f);
            TEST("roundtrip_modifiers", e.modifiers == 3);
            found = true;
        }
    }
    TEST("roundtrip_found_wave1", found);
}

// === Test Commands ===

void testCommands() {
    printf("\n[Command Tests]\n");
    
    // Generate
    WaveEditor::getEntries().clear();
    WaveEditor::cmdGenerate(10);
    TEST("cmd_generate_10", WaveEditor::getEntries().size() == 10);
    
    // Add
    WaveEditor::cmdAdd(15);
    TEST("cmd_add_15", WaveEditor::getEntries().size() == 11);
    
    // Set (value must be under maxBots=15 to avoid clamping)
    WaveEditor::cmdSet(5, "botCount", "10");
    bool found = false;
    for (auto& e : WaveEditor::getEntries()) {
        if (e.wave == 5) {
            TEST("cmd_set_botCount", e.botCount == 10);
            found = true;
        }
    }
    TEST("cmd_set_found", found);
    
    // Remove
    WaveEditor::cmdRemove(15);
    TEST("cmd_remove_15", WaveEditor::getEntries().size() == 10);
    
    // Generate more
    WaveEditor::cmdGenerate(20);
    TEST("cmd_generate_20", WaveEditor::getEntries().size() == 20);
}

// === Test Clamping ===

void testClamping() {
    printf("\n[Clamping Tests]\n");
    
    WaveEditor::getEntries().clear();
    WaveEditor::cmdAdd(1);
    
    // Set invalid values
    WaveEditor::cmdSet(1, "botCount", "-5");
    WaveEditor::cmdSet(1, "healthMult", "-1.0");
    WaveEditor::cmdSet(1, "spawnRadius", "999");
    
    for (auto& e : WaveEditor::getEntries()) {
        if (e.wave == 1) {
            TEST("clamp_botCount_nonneg", e.botCount >= 0);
            TEST("clamp_healthMin", e.healthMultiplier >= 0.1f);
            TEST("clamp_spawnRadius", e.spawnRadius <= 100);
        }
    }
    
    // Test maxBots cap
    WaveEditor::getGlobal().maxBots = 20;
    WaveEditor::cmdSet(1, "botCount", "100");
    for (auto& e : WaveEditor::getEntries()) {
        if (e.wave == 1) {
            TEST("clamp_maxBots", e.botCount <= 20);
        }
    }
}

// === Test Global Config ===

void testGlobalConfig() {
    printf("\n[Global Config Tests]\n");
    
    auto& g = WaveEditor::getGlobal();
    int origMaxBots = g.maxBots;
    
    g.maxBots = 25;
    WaveEditor::getEntries().clear();
    WaveEditor::cmdGenerate(30);
    
    // Check no wave exceeds cap
    bool allValid = true;
    for (auto& e : WaveEditor::getEntries()) {
        if (e.botCount > 25) {
            allValid = false;
            break;
        }
    }
    TEST("global_maxBots_applied", allValid);
    
    // Reset
    g.maxBots = origMaxBots;
}

// === Test Error Handling ===

void testErrorHandling() {
    printf("\n[Error Handling Tests]\n");
    
    // Load non-existent file
    bool loaded = WaveEditor::load("/tmp/nonexistent_file_xyz.json");
    TEST("load_nonexistent_fails", !loaded);
    
    // Save to invalid path
    bool saved = WaveEditor::save("/invalid_path/xyz.json");
    TEST("save_invalid_path_fails", !saved);
    
    // Set unknown field
    WaveEditor::getEntries().clear();
    WaveEditor::cmdAdd(1);
    WaveEditor::cmdSet(1, "unknownField", "123");  // Should print error, not crash
    TEST("unknown_field_no_crash", true);
}

// === Main ===

int main() {
    printf("=== Wave Editor Test Suite ===\n");
    
    testModifierFlags();
    testJSONSave();
    testJSONLoad();
    testJSONRoundtrip();
    testCommands();
    testClamping();
    testGlobalConfig();
    testErrorHandling();
    
    printf("\n=== Results: %d passed, %d failed ===\n", testsPassed, testsFailed);
    
    return testsFailed > 0 ? 1 : 0;
}
