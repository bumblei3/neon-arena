// main.cpp — NeonArena CLI Tools
// Unified command-line interface for wave editing, map validation, and more

#include <cstdio>
#include <cstring>
#include <string>
#include "wave_editor.h"
#include "map_validator.h"

void printUsage() {
    printf("\n\033[1;36mNeonArena Tools v0.60\033[0m\n");
    printf("Usage: neon-tools <command> [options]\n\n");
    printf("\033[1;33mCommands:\033[0m\n");
    printf("  wave-edit       Interactive wave configuration editor\n");
    printf("  wave-generate   Generate default wave config (JSON)\n");
    printf("  map-validate    Validate arena map data\n");
    printf("  map-generate    Generate default valid map (JSON)\n");
    printf("  help            Show this help\n\n");
}

int cmdWaveEdit(int argc, char* argv[]) {
    WaveEditor::cmdHelp();
    
    if (argc > 2) {
        // Load initial file
        if (!WaveEditor::load(argv[2])) {
            printf("Failed to load %s\n", argv[2]);
        }
    }
    
    char line[256];
    printf("\n> ");
    fflush(stdout);
    
    while (fgets(line, sizeof(line), stdin)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) {
            break;
        } else if (strncmp(line, "set ", 4) == 0) {
            int wave;
            char field[32], value[64];
            if (sscanf(line + 4, "%d %s %s", &wave, field, value) == 3) {
                WaveEditor::cmdSet(wave, field, value);
            } else {
                printf("Usage: set <wave> <field> <value>\n");
            }
        } else if (strncmp(line, "add ", 4) == 0) {
            int wave;
            if (sscanf(line + 4, "%d", &wave) == 1) {
                WaveEditor::cmdAdd(wave);
            }
        } else if (strncmp(line, "remove ", 7) == 0) {
            int wave;
            if (sscanf(line + 7, "%d", &wave) == 1) {
                WaveEditor::cmdRemove(wave);
            }
        } else if (strncmp(line, "generate ", 9) == 0) {
            int count;
            if (sscanf(line + 9, "%d", &count) == 1) {
                WaveEditor::cmdGenerate(count);
            }
        } else if (strncmp(line, "show ", 5) == 0) {
            int wave;
            if (sscanf(line + 5, "%d", &wave) == 1) {
                WaveEditor::cmdShow(wave);
            }
        } else if (strcmp(line, "list") == 0) {
            WaveEditor::cmdList();
        } else if (strncmp(line, "save ", 5) == 0) {
            WaveEditor::save(line + 5);
        } else if (strncmp(line, "load ", 5) == 0) {
            WaveEditor::load(line + 5);
        } else if (strcmp(line, "global") == 0) {
            WaveEditor::isGlobalVisible() = !WaveEditor::isGlobalVisible();
            WaveEditor::renderEditor();
        } else if (strcmp(line, "defaults") == 0) {
            WaveEditor::cmdGenerate(30);
        } else if (strcmp(line, "help") == 0) {
            WaveEditor::cmdHelp();
        } else if (line[0] == '\0') {
            // empty line
        } else {
            printf("Unknown command: %s\n", line);
        }
        
        printf("\n> ");
        fflush(stdout);
    }
    
    printf("\nGoodbye!\n");
    return 0;
}

int cmdWaveGenerate(int argc, char* argv[]) {
    int count = 30;
    if (argc > 2) {
        // wave-generate 50 output.json
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 0;
    }
    
    const char* cmd = argv[1];
    
    if (strcmp(cmd, "wave-edit") == 0) {
        return cmdWaveEdit(argc, argv);
    } else if (strcmp(cmd, "wave-generate") == 0) {
        WaveEditor::cmdGenerate(30);
        if (argc > 2) {
            WaveEditor::save(argv[2]);
        } else {
            WaveEditor::cmdList();
        }
        return 0;
    } else if (strcmp(cmd, "map-validate") == 0) {
        printf("Map validator: use map-generate to create a test map\n");
        return 0;
    } else if (strcmp(cmd, "map-generate") == 0) {
        auto map = MapValidator::generateDefaultMap("test_arena", 50.0f);
        auto result = MapValidator::validate(map);
        MapValidator::printReport(result);
        return 0;
    } else if (strcmp(cmd, "help") == 0) {
        printUsage();
        return 0;
    } else {
        printf("Unknown command: %s\n", cmd);
        printUsage();
        return 1;
    }
}
