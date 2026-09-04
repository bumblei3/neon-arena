// NEON ARENA - Vulkan + SDL2 Prototype
// Audio system: sound effects and music playback
#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

class AudioSystem {
public:
    static const int MAX_SOUNDS = 16;

    bool initialized = false;

    // Sound effects
    Mix_Chunk* shootSound = nullptr;
    Mix_Chunk* killSound = nullptr;
    Mix_Chunk* waveClearSound = nullptr;
    Mix_Chunk* playerHurtSound = nullptr;
    Mix_Chunk* gameOverSound = nullptr;

    // Music
    Mix_Music* backgroundMusic = nullptr;

    // Volume (0-128)
    int sfxVolume = 64;
    int musicVolume = 32;

    void init();
    void cleanup();

    void playShoot();
    void playKill();
    void playWaveClear();
    void playPlayerHurt();
    void playGameOver();
    void playBackgroundMusic();
    void stopBackgroundMusic();

    // Generate procedural sounds (no external files needed)
    void generateProceduralSounds();

private:
    Mix_Chunk* createBeep(float frequency, float durationMs, float volume = 0.5f);
    Mix_Chunk* createNoise(float durationMs, float volume = 0.3f);
};
