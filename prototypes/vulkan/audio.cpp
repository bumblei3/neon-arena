// NEON ARENA - Vulkan + SDL2 Prototype
// Audio system implementation
#include "audio.h"
#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void AudioSystem::init() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "SDL_mixer initialization failed: %s\n", Mix_GetError());
        fprintf(stderr, "Audio will be disabled.\n");
        return;
    }

    Mix_AllocateChannels(8);
    initialized = true;

    generateProceduralSounds();
    printf("Audio system initialized\n");
}

void AudioSystem::cleanup() {
    if (!initialized) return;

    if (shootSound) Mix_FreeChunk(shootSound);
    if (killSound) Mix_FreeChunk(killSound);
    if (waveClearSound) Mix_FreeChunk(waveClearSound);
    if (playerHurtSound) Mix_FreeChunk(playerHurtSound);
    if (gameOverSound) Mix_FreeChunk(gameOverSound);
    if (backgroundMusic) Mix_FreeMusic(backgroundMusic);

    Mix_CloseAudio();
    initialized = false;
}

Mix_Chunk* AudioSystem::createBeep(float frequency, float durationMs, float volume) {
    int sampleRate = 44100;
    int samples = (int)(sampleRate * durationMs / 1000.0f);
    int bufferSize = samples * 2; // 16-bit mono

    Uint8* buffer = new Uint8[bufferSize];
    memset(buffer, 0, bufferSize);

    float phase = 0.0f;
    float phaseIncrement = 2.0f * M_PI * frequency / sampleRate;

    for (int i = 0; i < samples; i++) {
        float sample = sinf(phase) * volume;
        // Apply simple envelope
        float envelope = 1.0f - (float)i / samples;
        sample *= envelope;

        Sint16 sample16 = (Sint16)(sample * 32767);
        buffer[i * 2] = sample16 & 0xFF;
        buffer[i * 2 + 1] = (sample16 >> 8) & 0xFF;

        phase += phaseIncrement;
        if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
    }

    // Convert to SDL_mixer chunk (WAV format in memory)
    SDL_RWops* rw = SDL_RWFromMem(buffer, bufferSize);
    Mix_Chunk* chunk = Mix_LoadWAV_RW(rw, 0);
    delete[] buffer;

    return chunk;
}

Mix_Chunk* AudioSystem::createNoise(float durationMs, float volume) {
    int sampleRate = 44100;
    int samples = (int)(sampleRate * durationMs / 1000.0f);
    int bufferSize = samples * 2;

    Uint8* buffer = new Uint8[bufferSize];
    memset(buffer, 0, bufferSize);

    for (int i = 0; i < samples; i++) {
        float sample = ((float)rand() / RAND_MAX - 0.5f) * volume;
        float envelope = 1.0f - (float)i / samples;
        sample *= envelope;

        Sint16 sample16 = (Sint16)(sample * 32767);
        buffer[i * 2] = sample16 & 0xFF;
        buffer[i * 2 + 1] = (sample16 >> 8) & 0xFF;
    }

    SDL_RWops* rw = SDL_RWFromMem(buffer, bufferSize);
    Mix_Chunk* chunk = Mix_LoadWAV_RW(rw, 0);
    delete[] buffer;

    return chunk;
}

void AudioSystem::generateProceduralSounds() {
    // Shoot sound: short high-pitched beep
    shootSound = createBeep(880.0f, 0.1f, 0.4f);

    // Kill sound: lower pitched, slightly longer
    killSound = createBeep(440.0f, 0.15f, 0.5f);

    // Wave clear: ascending tone
    waveClearSound = createBeep(660.0f, 0.3f, 0.4f);

    // Player hurt: harsh noise
    playerHurtSound = createNoise(0.2f, 0.5f);

    // Game over: descending tone
    gameOverSound = createBeep(220.0f, 0.5f, 0.5f);

    printf("Generated procedural sounds\n");
}

void AudioSystem::playShoot() {
    if (!initialized || !shootSound) return;
    Mix_PlayChannel(-1, shootSound, 0);
}

void AudioSystem::playKill() {
    if (!initialized || !killSound) return;
    Mix_PlayChannel(-1, killSound, 0);
}

void AudioSystem::playWaveClear() {
    if (!initialized || !waveClearSound) return;
    Mix_PlayChannel(-1, waveClearSound, 0);
}

void AudioSystem::playPlayerHurt() {
    if (!initialized || !playerHurtSound) return;
    Mix_PlayChannel(-1, playerHurtSound, 0);
}

void AudioSystem::playGameOver() {
    if (!initialized || !gameOverSound) return;
    Mix_PlayChannel(-1, gameOverSound, 0);
}

void AudioSystem::playBackgroundMusic() {
    if (!initialized || !backgroundMusic) return;
    Mix_PlayMusic(backgroundMusic, -1);
}

void AudioSystem::stopBackgroundMusic() {
    if (!initialized) return;
    Mix_HaltMusic();
}
