#pragma once
// savegame.h - Spielstand speichern und laden
#include <string>

class Game;

// Savegame Format Version (für Kompatibilität)
constexpr int SAVEGAME_VERSION = 1;
constexpr const char* SAVEGAME_MAGIC = "NEONARENA";

class SavegameManager {
public:
    // Prüft ob ein Savegame existiert
    static bool exists(const std::string& path = "savegame.dat");

    // Speichert den aktuellen Spielstand
    static bool save(const Game& game, const std::string& path = "savegame.dat");

    // Lädt einen Spielstand
    static bool load(Game& game, const std::string& path = "savegame.dat");

    // Löscht das Savegame
    static bool remove(const std::string& path = "savegame.dat");

    // Letzte Fehlermeldung
    static const std::string& getLastError();
    static void clearError();

private:
    static std::string lastError_;
};
