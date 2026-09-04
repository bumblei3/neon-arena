// audio.h - Erweitertes Audio-System
#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include <string>

class AudioSystem {
public:
    AudioSystem();
    ~AudioSystem();

    bool init();
    void shutdown();

    // Sound Effects
    void playShoot();
    void playLightning();
    void playExplosion();
    void playHit();
    void playKill();
    void playPowerUp();
    void playBossWarning();
    void playWaveComplete();
    void playGameOver();
    void playMenuClick();
    void playUpgrade();

    void setSfxVolume(int vol);

private:
    Mix_Chunk* sndShoot = nullptr;
    Mix_Chunk* sndLightning = nullptr;
    Mix_Chunk* sndExplosion = nullptr;
    Mix_Chunk* sndHit = nullptr;
    Mix_Chunk* sndKill = nullptr;
    Mix_Chunk* sndPowerUp = nullptr;
    Mix_Chunk* sndBossWarning = nullptr;
    Mix_Chunk* sndWaveComplete = nullptr;
    Mix_Chunk* sndGameOver = nullptr;
    Mix_Chunk* sndMenuClick = nullptr;
    Mix_Chunk* sndUpgrade = nullptr;

    int sfxVolume = 64;

    Mix_Chunk* generateSound(int type, float duration, float freqStart, float freqEnd, float amplitude);
    void generateAllSounds();
};
