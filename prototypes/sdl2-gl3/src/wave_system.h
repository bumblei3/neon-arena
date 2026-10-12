// wave_system.h - Wave spawning and progression (extracted from bots.cpp/game.cpp)
#pragma once

class Game;

// WaveSystem owns wave spawning/progression. It operates directly on the
// Game fields (wave, waveComplete, bots, modifierState, ...) via friend
// access — the fields stay on Game because ~12 other modules (hud, savegame,
// specials, wave_editor, tests) read them. WaveSystem only owns the *logic*,
// not the state, so this extraction is complete without touching those files.
class WaveSystem {
public:
    void init(Game& game);
    void nextWave(Game& game);

private:
    void spawnWave(Game& game);
    void spawnBossWave(Game& game, int wave_num, int boss_count, int minion_count);
    void spawnNormalWave(Game& game, int wave_num, int bot_count);
};
