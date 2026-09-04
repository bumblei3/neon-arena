// audio.cpp - Erweitertes Audio-System
#include "audio.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

AudioSystem::AudioSystem() {}

AudioSystem::~AudioSystem() {
    shutdown();
}

bool AudioSystem::init() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "SDL_Mixer init failed: %s\\n", Mix_GetError());
        return false;
    }
    Mix_AllocateChannels(32);
    generateAllSounds();
    return true;
}

void AudioSystem::shutdown() {
    if (sndShoot) { Mix_FreeChunk(sndShoot); sndShoot = nullptr; }
    if (sndLightning) { Mix_FreeChunk(sndLightning); sndLightning = nullptr; }
    if (sndExplosion) { Mix_FreeChunk(sndExplosion); sndExplosion = nullptr; }
    if (sndHit) { Mix_FreeChunk(sndHit); sndHit = nullptr; }
    if (sndKill) { Mix_FreeChunk(sndKill); sndKill = nullptr; }
    if (sndPowerUp) { Mix_FreeChunk(sndPowerUp); sndPowerUp = nullptr; }
    if (sndBossWarning) { Mix_FreeChunk(sndBossWarning); sndBossWarning = nullptr; }
    if (sndWaveComplete) { Mix_FreeChunk(sndWaveComplete); sndWaveComplete = nullptr; }
    if (sndGameOver) { Mix_FreeChunk(sndGameOver); sndGameOver = nullptr; }
    if (sndMenuClick) { Mix_FreeChunk(sndMenuClick); sndMenuClick = nullptr; }
    if (sndUpgrade) { Mix_FreeChunk(sndUpgrade); sndUpgrade = nullptr; }
    Mix_CloseAudio();
}

Mix_Chunk* AudioSystem::generateSound(int type, float duration, float freqStart, float freqEnd, float amplitude) {
    int sampleRate = 44100;
    int numSamples = sampleRate * duration;
    short* samples = new short[numSamples];

    for (int i = 0; i < numSamples; i++) {
        float t = (float)i / sampleRate;
        float progress = (float)i / numSamples;
        float freq = freqStart + (freqEnd - freqStart) * progress;
        float amp = amplitude * (1.0f - progress);

        float sample = 0.0f;
        switch (type) {
            case 0: sample = sinf(2.0f * M_PI * freq * t); break;
            case 1:
                sample = sinf(2.0f * M_PI * freq * t) * 0.6f;
                sample += sinf(2.0f * M_PI * freq * 2.0f * t) * 0.3f;
                sample += sinf(2.0f * M_PI * freq * 3.0f * t) * 0.1f;
                break;
            case 2:
                sample = sinf(2.0f * M_PI * freq * t) * 0.7f;
                sample += ((rand() % 100) / 100.0f - 0.5f) * 0.3f;
                break;
            case 3:
                sample = sinf(2.0f * M_PI * freq * t);
                sample *= (1.0f + sinf(2.0f * M_PI * 10.0f * t)) * 0.5f;
                break;
            default: sample = sinf(2.0f * M_PI * freq * t); break;
        }
        samples[i] = (short)(sample * amp * 32767 * 0.5f);
    }

    Mix_Chunk* chunk = new Mix_Chunk();
    chunk->allocated = 1;
    chunk->abuf = (Uint8*)samples;
    chunk->alen = numSamples * sizeof(short);
    chunk->volume = 64;
    return chunk;
}

void AudioSystem::generateAllSounds() {
    sndShoot = generateSound(2, 0.15f, 2000.0f, 200.0f, 0.8f);
    sndLightning = generateSound(2, 0.3f, 1000.0f, 100.0f, 0.9f);
    sndExplosion = generateSound(2, 0.5f, 200.0f, 50.0f, 1.0f);
    sndHit = generateSound(3, 0.08f, 800.0f, 200.0f, 0.7f);
    sndKill = generateSound(1, 0.2f, 400.0f, 800.0f, 0.6f);
    sndPowerUp = generateSound(0, 0.3f, 300.0f, 1200.0f, 0.7f);
    sndBossWarning = generateSound(1, 1.0f, 200.0f, 600.0f, 0.9f);
    sndWaveComplete = generateSound(1, 0.5f, 400.0f, 1000.0f, 0.7f);
    sndGameOver = generateSound(0, 0.8f, 400.0f, 100.0f, 0.8f);
    sndMenuClick = generateSound(3, 0.05f, 1000.0f, 500.0f, 0.5f);
    sndUpgrade = generateSound(1, 0.4f, 500.0f, 1500.0f, 0.7f);
}

void AudioSystem::playShoot() { if (sndShoot) Mix_PlayChannel(-1, sndShoot, 0); }
void AudioSystem::playLightning() { if (sndLightning) Mix_PlayChannel(-1, sndLightning, 0); }
void AudioSystem::playExplosion() { if (sndExplosion) Mix_PlayChannel(-1, sndExplosion, 0); }
void AudioSystem::playHit() { if (sndHit) Mix_PlayChannel(-1, sndHit, 0); }
void AudioSystem::playKill() { if (sndKill) Mix_PlayChannel(-1, sndKill, 0); }
void AudioSystem::playPowerUp() { if (sndPowerUp) Mix_PlayChannel(-1, sndPowerUp, 0); }
void AudioSystem::playBossWarning() { if (sndBossWarning) Mix_PlayChannel(-1, sndBossWarning, 0); }
void AudioSystem::playWaveComplete() { if (sndWaveComplete) Mix_PlayChannel(-1, sndWaveComplete, 0); }
void AudioSystem::playGameOver() { if (sndGameOver) Mix_PlayChannel(-1, sndGameOver, 0); }
void AudioSystem::playMenuClick() { if (sndMenuClick) Mix_PlayChannel(-1, sndMenuClick, 0); }
void AudioSystem::playUpgrade() { if (sndUpgrade) Mix_PlayChannel(-1, sndUpgrade, 0); }

void AudioSystem::setSfxVolume(int vol) {
    sfxVolume = vol;
    Mix_Volume(-1, vol);
}
