#include "wave_editor.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>

std::vector<WaveEditor::WaveConfigEntry> WaveEditor::entries;
WaveEditor::GlobalConfig WaveEditor::global;
int WaveEditor::currentEditWave = 1;
bool WaveEditor::showEditor = false;
bool WaveEditor::showGlobal = false;

const char* WaveEditor::MODIFIER_NAMES[] = {
    "NONE", "SPEED_BOOST", "SHIELD", "REGENERATION", "SPLITTER",
    "FROST", "FRENZY", "VAMPIRE", "TIMELAPSE"
};
const uint32_t WaveEditor::MODIFIER_VALUES[] = {
    0, 1, 2, 4, 8, 16, 32, 64, 128
};
const int WaveEditor::MODIFIER_COUNT = 9;

// === Defaults ===

void WaveEditor::loadDefaults() {
    entries.clear();
    
    for (int w = 1; w <= global.totalWaves; w++) {
        WaveConfigEntry e;
        e.wave = w;
        e.botCount = w + 1;
        
        if (w >= 10) e.botCount = 10 + (w - 10) * 2;
        if (w >= 20) e.botCount = 20 + (w - 20);
        
        // Apply cap
        if (e.botCount > global.maxBots) e.botCount = global.maxBots;
        
        e.healthMultiplier = 1.0f + w * 0.1f;
        e.speedMultiplier = 1.0f + w * 0.05f;
        e.isBoss = (w % global.bossEvery == 0);
        e.bossCount = e.isBoss ? 1 : 0;
        e.minionCount = e.isBoss ? (1 + w / 10) : 0;
        e.spawnRadius = 35;
        e.spawnAngleOffset = 0;
        e.spawnFromCenter = false;
        e.pointsReward = w * 100;
        e.upgradeReward = 1 + w / 3;
        
        // Progressive modifiers
        e.modifiers = 0;
        if (w >= 3) e.modifiers |= 1;   // SPEED_BOOST
        if (w >= 6) e.modifiers |= 2;   // SHIELD
        if (w >= 9) e.modifiers |= 4;   // REGENERATION
        if (w >= 12) e.modifiers |= 8;  // SPLITTER
        if (w >= 15) e.modifiers |= 16; // FROST
        if (w >= 18) e.modifiers |= 32; // FRENZY
        if (w >= 21) e.modifiers |= 64; // VAMPIRE
        if (w >= 24) e.modifiers |= 128; // TIMELAPSE
        
        entries.push_back(e);
    }
}

// === Lookup ===

int WaveEditor::findEntry(int wave) {
    for (int i = 0; i < (int)entries.size(); i++) {
        if (entries[i].wave == wave) return i;
    }
    return -1;
}

void WaveEditor::applyEntry(int wave, int entryIdx) {
    if (entryIdx < 0 || entryIdx >= (int)entries.size()) return;
    // Currently no-op — placeholder for future integration with game state
}

void WaveEditor::clampEntry(WaveConfigEntry& e) {
    if (e.botCount < 0) e.botCount = 0;
    if (e.botCount > global.maxBots) e.botCount = global.maxBots;
    if (e.healthMultiplier < 0.1f) e.healthMultiplier = 0.1f;
    if (e.speedMultiplier < 0.1f) e.speedMultiplier = 0.1f;
    if (e.bossCount < 0) e.bossCount = 0;
    if (e.minionCount < 0) e.minionCount = 0;
    if (e.spawnRadius < 0) e.spawnRadius = 0;
    if (e.spawnRadius > 100) e.spawnRadius = 100;
    if (e.pointsReward < 0) e.pointsReward = 0;
    if (e.upgradeReward < 0) e.upgradeReward = 0;
}

// === Generation ===

int WaveEditor::generateWaveConfig(int currentWave) {
    int idx = findEntry(currentWave);
    if (idx >= 0) return entries[idx].botCount;
    return currentWave + 1;
}

// === Editor UI ===

void WaveEditor::updateEditor() {
    if (!showEditor) return;
    // Update logic for smooth transitions
}

void WaveEditor::renderEditor() {
    if (!showEditor) return;
    
    if (entries.empty()) loadDefaults();
    
    printf("\n\033[1;36m=== WAVE EDITOR ===\033[0m\n");
    printf("Editing Wave %d | Global: maxBots=%d bossEvery=%d diffCurve=%.1f totalWaves=%d\n",
           currentEditWave, global.maxBots, global.bossEvery, global.difficultyCurve, global.totalWaves);
    
    int idx = findEntry(currentEditWave);
    if (idx >= 0) {
        auto& e = entries[idx];
        printf("  Bot Count:    %d\n", e.botCount);
        printf("  Health Mult:  %.2f\n", e.healthMultiplier);
        printf("  Speed Mult:   %.2f\n", e.speedMultiplier);
        printf("  Is Boss:      %s\n", e.isBoss ? "yes" : "no");
        printf("  Boss Count:   %d\n", e.bossCount);
        printf("  Minions:      %d\n", e.minionCount);
        printf("  Spawn Radius: %d\n", e.spawnRadius);
        printf("  Spawn Angle:  %.1f\n", e.spawnAngleOffset);
        printf("  Spawn Center: %s\n", e.spawnFromCenter ? "yes" : "no");
        printf("  Points:       %d\n", e.pointsReward);
        printf("  Upgrades:     %d\n", e.upgradeReward);
        printf("  Modifiers:    %s\n", modifierFlagsToString(e.modifiers).c_str());
    } else {
        printf("  [No entry for wave %d — use 'add %d']\n", currentEditWave, currentEditWave);
    }
    
    if (showGlobal) {
        printf("\n\033[1;33m--- GLOBAL CONFIG ---\033[0m\n");
        printf("  Max Bots:         %d\n", global.maxBots);
        printf("  Boss Every:       %d waves\n", global.bossEvery);
        printf("  Difficulty Curve: %.2f\n", global.difficultyCurve);
        printf("  Speed Penalty:    %.2f\n", global.speedPenalty);
        printf("  Total Waves:      %d\n", global.totalWaves);
    }
    
    printf("\033[1;36m====================\033[0m\n\n");
}

// === Interactive Commands ===

void WaveEditor::cmdSet(int wave, const char* field, const char* value) {
    int idx = findEntry(wave);
    if (idx < 0) {
        printf("Wave %d not found. Use 'add %d' first.\n", wave, wave);
        return;
    }
    
    auto& e = entries[idx];
    
    if (strcmp(field, "botCount") == 0) e.botCount = atoi(value);
    else if (strcmp(field, "healthMult") == 0) e.healthMultiplier = (float)atof(value);
    else if (strcmp(field, "speedMult") == 0) e.speedMultiplier = (float)atof(value);
    else if (strcmp(field, "isBoss") == 0) e.isBoss = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
    else if (strcmp(field, "bossCount") == 0) e.bossCount = atoi(value);
    else if (strcmp(field, "minions") == 0) e.minionCount = atoi(value);
    else if (strcmp(field, "spawnRadius") == 0) e.spawnRadius = atoi(value);
    else if (strcmp(field, "spawnAngle") == 0) e.spawnAngleOffset = (float)atof(value);
    else if (strcmp(field, "spawnCenter") == 0) e.spawnFromCenter = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
    else if (strcmp(field, "points") == 0) e.pointsReward = atoi(value);
    else if (strcmp(field, "upgrades") == 0) e.upgradeReward = atoi(value);
    else if (strcmp(field, "modifiers") == 0) e.modifiers = stringToModifierFlags(value);
    else {
        printf("Unknown field: %s\n", field);
        return;
    }
    
    clampEntry(e);
    printf("Wave %d: %s = %s\n", wave, field, value);
}

void WaveEditor::cmdAdd(int wave) {
    if (findEntry(wave) >= 0) {
        printf("Wave %d already exists.\n", wave);
        return;
    }
    
    WaveConfigEntry e;
    e.wave = wave;
    e.botCount = wave + 1;
    if (e.botCount > global.maxBots) e.botCount = global.maxBots;
    e.healthMultiplier = 1.0f + wave * 0.1f;
    e.speedMultiplier = 1.0f + wave * 0.05f;
    e.isBoss = (wave % global.bossEvery == 0);
    e.bossCount = e.isBoss ? 1 : 0;
    e.minionCount = e.isBoss ? (1 + wave / 10) : 0;
    e.spawnRadius = 35;
    e.pointsReward = wave * 100;
    e.upgradeReward = 1 + wave / 3;
    
    entries.push_back(e);
    
    // Sort by wave number
    std::sort(entries.begin(), entries.end(), [](const WaveConfigEntry& a, const WaveConfigEntry& b) {
        return a.wave < b.wave;
    });
    
    printf("Added wave %d.\n", wave);
}

void WaveEditor::cmdRemove(int wave) {
    int idx = findEntry(wave);
    if (idx < 0) {
        printf("Wave %d not found.\n", wave);
        return;
    }
    entries.erase(entries.begin() + idx);
    printf("Removed wave %d.\n", wave);
}

void WaveEditor::cmdGenerate(int count) {
    entries.clear();
    global.totalWaves = count;
    loadDefaults();
    printf("Generated %d waves.\n", count);
}

void WaveEditor::cmdList() {
    printf("\n\033[1;36m%-5s %-6s %-8s %-8s %-5s %-5s %-5s %-6s %-5s\033[0m\n",
           "Wave", "Bots", "HP Mult", "Spd Mult", "Boss", "Boss#", "Min", "Pts", "Upgr");
    printf("------------------------------------------------------------------------\n");
    for (auto& e : entries) {
        printf("%-5d %-6d %-8.2f %-8.2f %-5s %-5d %-5d %-6d %-5d\n",
               e.wave, e.botCount, e.healthMultiplier, e.speedMultiplier,
               e.isBoss ? "Y" : "N", e.bossCount, e.minionCount,
               e.pointsReward, e.upgradeReward);
    }
    printf("\n");
}

void WaveEditor::cmdShow(int wave) {
    currentEditWave = wave;
    renderEditor();
}

void WaveEditor::cmdHelp() {
    printf("\n\033[1;36mWave Editor Commands:\033[0m\n");
    printf("  set <wave> <field> <value>  — Set a field value\n");
    printf("  add <wave>                  — Add a wave entry\n");
    printf("  remove <wave>               — Remove a wave entry\n");
    printf("  generate <count>            — Regenerate all waves\n");
    printf("  list                        — List all waves\n");
    printf("  show <wave>                 — Show wave details\n");
    printf("  global                      — Toggle global config display\n");
    printf("  save <filename>             — Save to JSON file\n");
    printf("  load <filename>             — Load from JSON file\n");
    printf("  defaults                    — Load default wave config\n");
    printf("  quit                        — Exit editor\n");
    printf("\n\033[1;33mFields:\033[0m botCount, healthMult, speedMult, isBoss, bossCount,\n");
    printf("  minions, spawnRadius, spawnAngle, spawnCenter, points, upgrades, modifiers\n");
    printf("\n\033[1;33mModifier flags:\033[0m SPEED_BOOST, SHIELD, REGENERATION, SPLITTER,\n");
    printf("  FROST, FRENZY, VAMPIRE, TIMELAPSE (comma-separated or hex like 0x0F)\n");
    printf("\n");
}

// === JSON Parsing ===

const char* WaveEditor::skipWhitespace(const char* p) {
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    return p;
}

const char* WaveEditor::parseString(const char* p, std::string& out) {
    out.clear();
    if (*p != '"') return p;
    p++;
    while (*p && *p != '"') {
        if (*p == '\\') {
            p++;
            switch (*p) {
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                case '/': out += '/'; break;
                case 'n': out += '\n'; break;
                case 't': out += '\t'; break;
                default: out += *p; break;
            }
        } else {
            out += *p;
        }
        p++;
    }
    if (*p == '"') p++;
    return p;
}

const char* WaveEditor::parseNumber(const char* p, float& out) {
    char* end;
    out = strtof(p, &end);
    return end;
}

const char* WaveEditor::parseInt(const char* p, int& out) {
    char* end;
    out = (int)strtol(p, &end, 10);
    return end;
}

std::string WaveEditor::escapeString(const char* s) {
    std::string out;
    for (const char* p = s; *p; p++) {
        switch (*p) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            default: out += *p; break;
        }
    }
    return out;
}

std::string WaveEditor::modifierFlagsToString(uint32_t flags) {
    if (flags == 0) return "NONE";
    std::string result;
    for (int i = 1; i < MODIFIER_COUNT; i++) {
        if (flags & MODIFIER_VALUES[i]) {
            if (!result.empty()) result += "|";
            result += MODIFIER_NAMES[i];
        }
    }
    return result;
}

uint32_t WaveEditor::stringToModifierFlags(const char* s) {
    if (!s || !*s) return 0;
    if (strcmp(s, "NONE") == 0) return 0;
    
    // Try hex (0x...)
    if (strncmp(s, "0x", 2) == 0 || strncmp(s, "0X", 2) == 0) {
        return (uint32_t)strtol(s, nullptr, 16);
    }
    
    uint32_t flags = 0;
    // Parse pipe-delimited list
    std::string input(s);
    size_t pos = 0;
    while (pos < input.size()) {
        size_t pipe = input.find('|', pos);
        std::string token = (pipe == std::string::npos) ? input.substr(pos) : input.substr(pos, pipe - pos);
        
        for (int i = 0; i < MODIFIER_COUNT; i++) {
            if (token == MODIFIER_NAMES[i]) {
                flags |= MODIFIER_VALUES[i];
                break;
            }
        }
        
        if (pipe == std::string::npos) break;
        pos = pipe + 1;
    }
    return flags;
}

// === JSON Load ===

bool WaveEditor::load(const char* filename) {
    return parseJSON(filename);
}

bool WaveEditor::parseJSON(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Cannot open file: %s\n", filename);
        return false;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(f);
        return false;
    }
    
    size_t read = fread(buffer, 1, size, f);
    buffer[read] = '\0';
    fclose(f);
    
    const char* p = skipWhitespace(buffer);
    
    if (*p != '{') {
        printf("Expected '{' at start of JSON\n");
        free(buffer);
        return false;
    }
    p++;
    
    entries.clear();
    
    while (*p) {
        p = skipWhitespace(p);
        if (*p == '}') break;
        
        if (*p == ',') {
            p++;
            continue;
        }
        
        // Parse key
        std::string key;
        p = parseString(p, key);
        p = skipWhitespace(p);
        
        if (*p != ':') {
            printf("Expected ':' after key\n");
            free(buffer);
            return false;
        }
        p++;
        p = skipWhitespace(p);
        
        if (key == "global") {
            // Parse global config object
            if (*p != '{') { printf("Expected '{' for global\n"); free(buffer); return false; }
            p++;
            while (*p) {
                p = skipWhitespace(p);
                if (*p == '}') break;
                if (*p == ',') { p++; continue; }
                
                std::string gkey;
                p = parseString(p, gkey);
                p = skipWhitespace(p);
                if (*p != ':') { free(buffer); return false; }
                p++;
                p = skipWhitespace(p);
                
                float val;
                if (gkey == "maxBots") { p = parseInt(p, global.maxBots); }
                else if (gkey == "bossEvery") { p = parseInt(p, global.bossEvery); }
                else if (gkey == "difficultyCurve") { p = parseNumber(p, val); global.difficultyCurve = val; }
                else if (gkey == "speedPenalty") { p = parseNumber(p, val); global.speedPenalty = val; }
                else if (gkey == "totalWaves") { p = parseInt(p, global.totalWaves); }
                else {
                    // Skip unknown
                    while (*p && *p != ',' && *p != '}') p++;
                }
            }
            if (*p == '}') p++;
        }
        else if (key == "waves") {
            // Parse array of wave entries
            if (*p != '[') { printf("Expected '[' for waves\n"); free(buffer); return false; }
            p++;
            while (*p) {
                p = skipWhitespace(p);
                if (*p == ']') break;
                if (*p == ',') { p++; continue; }
                
                if (*p != '{') { printf("Expected '{' for wave entry\n"); free(buffer); return false; }
                p++;
                
                WaveConfigEntry e;
                
                while (*p) {
                    p = skipWhitespace(p);
                    if (*p == '}') break;
                    if (*p == ',') { p++; continue; }
                    
                    std::string wkey;
                    p = parseString(p, wkey);
                    p = skipWhitespace(p);
                    if (*p != ':') { free(buffer); return false; }
                    p++;
                    p = skipWhitespace(p);
                    
                    float val;
                    if (wkey == "wave") { p = parseInt(p, e.wave); }
                    else if (wkey == "botCount") { p = parseInt(p, e.botCount); }
                    else if (wkey == "healthMultiplier") { p = parseNumber(p, val); e.healthMultiplier = val; }
                    else if (wkey == "speedMultiplier") { p = parseNumber(p, val); e.speedMultiplier = val; }
                    else if (wkey == "isBoss") { 
                        if (*p == 't') { e.isBoss = true; p += 4; }
                        else { e.isBoss = false; p += 5; }
                    }
                    else if (wkey == "bossCount") { p = parseInt(p, e.bossCount); }
                    else if (wkey == "minionCount") { p = parseInt(p, e.minionCount); }
                    else if (wkey == "spawnRadius") { p = parseInt(p, e.spawnRadius); }
                    else if (wkey == "spawnAngleOffset") { p = parseNumber(p, val); e.spawnAngleOffset = val; }
                    else if (wkey == "spawnFromCenter") {
                        if (*p == 't') { e.spawnFromCenter = true; p += 4; }
                        else { e.spawnFromCenter = false; p += 5; }
                    }
                    else if (wkey == "pointsReward") { p = parseInt(p, e.pointsReward); }
                    else if (wkey == "upgradeReward") { p = parseInt(p, e.upgradeReward); }
                    else if (wkey == "modifiers") {
                        std::string modStr;
                        p = parseString(p, modStr);
                        e.modifiers = stringToModifierFlags(modStr.c_str());
                    }
                    else {
                        // Skip unknown value
                        while (*p && *p != ',' && *p != '}') p++;
                    }
                }
                
                clampEntry(e);
                entries.push_back(e);
                
                if (*p == '}') p++;
            }
            if (*p == ']') p++;
        }
        else {
            // Skip unknown key
            while (*p && *p != ',' && *p != '}') p++;
        }
    }
    
    free(buffer);
    
    // Sort entries by wave number
    std::sort(entries.begin(), entries.end(), [](const WaveConfigEntry& a, const WaveConfigEntry& b) {
        return a.wave < b.wave;
    });
    
    printf("Loaded %d waves from %s\n", (int)entries.size(), filename);
    return true;
}

// === JSON Save ===

bool WaveEditor::save(const char* filename) {
    return writeJSON(filename);
}

bool WaveEditor::writeJSON(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Cannot create file: %s\n", filename);
        return false;
    }
    
    fprintf(f, "{\n");
    
    // Global config
    fprintf(f, "  \"global\": {\n");
    fprintf(f, "    \"maxBots\": %d,\n", global.maxBots);
    fprintf(f, "    \"bossEvery\": %d,\n", global.bossEvery);
    fprintf(f, "    \"difficultyCurve\": %.2f,\n", global.difficultyCurve);
    fprintf(f, "    \"speedPenalty\": %.2f,\n", global.speedPenalty);
    fprintf(f, "    \"totalWaves\": %d\n", global.totalWaves);
    fprintf(f, "  },\n");
    
    // Waves array
    fprintf(f, "  \"waves\": [\n");
    
    for (int i = 0; i < (int)entries.size(); i++) {
        auto& e = entries[i];
        fprintf(f, "    {\n");
        fprintf(f, "      \"wave\": %d,\n", e.wave);
        fprintf(f, "      \"botCount\": %d,\n", e.botCount);
        fprintf(f, "      \"healthMultiplier\": %.2f,\n", e.healthMultiplier);
        fprintf(f, "      \"speedMultiplier\": %.2f,\n", e.speedMultiplier);
        fprintf(f, "      \"isBoss\": %s,\n", e.isBoss ? "true" : "false");
        fprintf(f, "      \"bossCount\": %d,\n", e.bossCount);
        fprintf(f, "      \"minionCount\": %d,\n", e.minionCount);
        fprintf(f, "      \"spawnRadius\": %d,\n", e.spawnRadius);
        fprintf(f, "      \"spawnAngleOffset\": %.1f,\n", e.spawnAngleOffset);
        fprintf(f, "      \"spawnFromCenter\": %s,\n", e.spawnFromCenter ? "true" : "false");
        fprintf(f, "      \"pointsReward\": %d,\n", e.pointsReward);
        fprintf(f, "      \"upgradeReward\": %d,\n", e.upgradeReward);
        fprintf(f, "      \"modifiers\": \"%s\"\n", modifierFlagsToString(e.modifiers).c_str());
        fprintf(f, "    }%s\n", (i < (int)entries.size() - 1) ? "," : "");
    }
    
    fprintf(f, "  ]\n");
    fprintf(f, "}\n");
    
    fclose(f);
    printf("Saved %d waves to %s\n", (int)entries.size(), filename);
    return true;
}
