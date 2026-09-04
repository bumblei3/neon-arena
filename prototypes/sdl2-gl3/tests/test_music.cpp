// Tests for Music Generator logic (no SDL audio needed)
#include <cstdio>
#include <cassert>
#include <cmath>
#include "../src/music_generator.h"

static int musicPassed = 0, musicFailed = 0;

#define MUSIC_TEST(name, expr) do { \
    printf("  Running %s... ", name); \
    fflush(stdout); \
    if (expr) { printf("PASSED\n"); musicPassed++; } \
    else { printf("FAILED\n"); musicFailed++; } \
} while(0)

void testMusicGenerator() {
    printf("\n[Music Generator Tests]\n");

    // Test construction/destruction
    {
        MusicGenerator gen;
        MUSIC_TEST("constructs", true);
    }

    // Test init
    {
        MusicGenerator gen;
        bool ok = gen.init(44100);
        MUSIC_TEST("init_succeeds", ok);
        MUSIC_TEST("is_playing_after_init", gen.isPlaying());
        MUSIC_TEST("default_scene_is_menu", gen.getCurrentScene() == MusicScene::MENU);
    }

    // Test scene changes
    {
        MusicGenerator gen;
        gen.init();
        gen.playScene(MusicScene::GAMEPLAY);
        MUSIC_TEST("scene_play_gameplay", gen.getCurrentScene() == MusicScene::GAMEPLAY);
        gen.playScene(MusicScene::BOSS);
        MUSIC_TEST("scene_play_boss", gen.getCurrentScene() == MusicScene::BOSS);
        gen.playScene(MusicScene::GAME_OVER);
        MUSIC_TEST("scene_play_gameover", gen.getCurrentScene() == MusicScene::GAME_OVER);
        gen.playScene(MusicScene::MENU);
        MUSIC_TEST("scene_play_menu", gen.getCurrentScene() == MusicScene::MENU);
    }

    // Test volume
    {
        MusicGenerator gen;
        gen.init();
        gen.setVolume(0.5f);
        MUSIC_TEST("volume_set_half", std::abs(gen.getVolume() - 0.5f) < 0.001f);
        gen.setVolume(1.5f); // Clamps to 1.0
        MUSIC_TEST("volume_clamps_high", std::abs(gen.getVolume() - 1.0f) < 0.001f);
        gen.setVolume(-1.0f); // Clamps to 0.0
        MUSIC_TEST("volume_clamps_low", std::abs(gen.getVolume() - 0.0f) < 0.001f);
    }

    // Test stop
    {
        MusicGenerator gen;
        gen.init();
        gen.stop();
        MUSIC_TEST("stop_works", !gen.isPlaying());
    }

    // Test note frequency calculation
    {
        MusicGenerator gen;
        gen.init();
        // A4 = 440Hz
        float freqA4 = gen.getNoteFrequency(69);
        MUSIC_TEST("freq_A4_is_440", std::abs(freqA4 - 440.0f) < 0.01f);
        // C4 = ~261.63Hz
        float freqC4 = gen.getNoteFrequency(60);
        MUSIC_TEST("freq_C4_is_261", std::abs(freqC4 - 261.63f) < 0.1f);
    }

    printf("\n[Music Generator Results] Passed: %d, Failed: %d\n", musicPassed, musicFailed);
}

int main() {
    testMusicGenerator();
    return musicFailed > 0 ? 1 : 0;
}
