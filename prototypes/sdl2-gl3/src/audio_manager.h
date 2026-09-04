// audio_manager.h - Audio management for neon arena prototype
#pragma once
#include <SDL_mixer.h>
#include <string>
#include <vector>
#include <unordered_map>

enum class AudioCategory {
    SFX,
    MUSIC,
    UI
};

using SoundHandle = int;
constexpr SoundHandle INVALID_SOUND = -1;

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    bool init();
    void shutdown();

    // Sound loading
    SoundHandle loadSFX(const std::string& path, const std::string& name);
    SoundHandle loadMusic(const std::string& path);

    // Playback
    SoundHandle playSFX(SoundHandle sound, int loops = 0);
    SoundHandle playSFX(const std::string& name, int loops = 0);
    void playMusic(SoundHandle music, int loops = -1);
    void stopMusic();
    void stopChannel(SoundHandle channel);
    void stopAllSFX();

    // Volume control (0.0 to 1.0)
    void setMasterVolume(float volume);
    void setCategoryVolume(AudioCategory category, float volume);
    float getMasterVolume() const { return masterVolume; }
    float getCategoryVolume(AudioCategory category) const;

    // Spatial audio - pan based on relative position (-1 to 1)
    SoundHandle playSFXAt(SoundHandle sound, float x, float y, int loops = 0);

    bool isPlaying(SoundHandle channel) const;
    const char* getLastError() const { return lastError.c_str(); }

    // Procedural sound generation
    Mix_Chunk* generateSound(int type, float duration, float freqStart, float freqEnd, float amplitude);
    void generateAllSounds();

    // Convenience play methods for procedural sounds
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

private:
    std::unordered_map<std::string, SoundHandle> sfxMap;
    std::vector<Mix_Chunk*> sfxChunks;
    std::vector<Mix_Music*> musicTracks;
    std::vector<int> activeChannels;

    // Procedural sound handles
    SoundHandle sndShoot = INVALID_SOUND;
    SoundHandle sndLightning = INVALID_SOUND;
    SoundHandle sndExplosion = INVALID_SOUND;
    SoundHandle sndHit = INVALID_SOUND;
    SoundHandle sndKill = INVALID_SOUND;
    SoundHandle sndPowerUp = INVALID_SOUND;
    SoundHandle sndBossWarning = INVALID_SOUND;
    SoundHandle sndWaveComplete = INVALID_SOUND;
    SoundHandle sndGameOver = INVALID_SOUND;
    SoundHandle sndMenuClick = INVALID_SOUND;
    SoundHandle sndUpgrade = INVALID_SOUND;

    float masterVolume = 1.0f;
    float sfxVolume = 1.0f;
    float musicVolume = 0.7f;
    float uiVolume = 1.0f;

    std::string lastError;
    bool initialized = false;

    SoundHandle addGeneratedSound(Mix_Chunk* chunk, const std::string& name);
    int getFreeChannel();
    void cleanupFinishedChannels();
};

extern AudioManager* g_audio;
