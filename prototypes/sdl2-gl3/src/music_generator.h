#pragma once
// music_generator.h - Procedural music synthesizer for NeonArena
#include <SDL_mixer.h>
#include <cmath>
#include <vector>
#include <cstdint>

enum class MusicScene {
    MENU,
    GAMEPLAY,
    BOSS,
    GAME_OVER
};

class MusicGenerator {
public:
    MusicGenerator();
    ~MusicGenerator();

    bool init(int sampleRate = 44100);
    void shutdown();

    void playScene(MusicScene scene);
    void stop();
    void setVolume(float vol);

    bool isPlaying() const { return playing; }
    MusicScene getCurrentScene() const { return currentScene; }
    float getVolume() const { return volume; }

    // Music generation callback (called from SDL2's audio thread)
    static void mixCallback(void* udata, Uint8* stream, int len);

    // Note frequency calculation (MIDI note to Hz)
    float getNoteFrequency(int midiNote);

private:
    static MusicGenerator* instance_;

    void synthesize(int16_t* buffer, int length);

    MusicScene currentScene = MusicScene::MENU;
    MusicScene targetScene = MusicScene::MENU;
    float volume = 0.7f;
    bool playing = false;

    int sampleRate;
    float bpm = 120.0f;
    float beatPosition = 0.0f;
    float beatIncrement = 0.0f;

    struct Pattern {
        std::vector<int> notes;
        std::vector<bool> triggers;
        int steps;
        int currentStep;
    };

    Pattern bass;
    Pattern arp;
    Pattern drums;

    float bassEnv = 0.0f;
    float arpEnv = 0.0f;
    float drumEnv = 0.0f;
    float filterMem = 0.0f;

    void initPattern(Pattern& pat, const std::vector<int>& notes, int steps);
    void crossfadeTo(MusicScene scene);
};
