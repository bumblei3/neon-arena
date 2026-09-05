// audio_manager.cpp - Audio manager with procedural sound generation
#include "audio_manager.h"
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstdlib>

AudioManager* g_audio = nullptr;

AudioManager::AudioManager() {
    g_audio = this;
}

AudioManager::~AudioManager() {
    shutdown();
    g_audio = nullptr;
}

bool AudioManager::init() {
    if (initialized) return true;

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        lastError = std::string("Mix_OpenAudio failed: ") + Mix_GetError();
        fprintf(stderr, "%s\n", lastError.c_str());
        return false;
    }

    Mix_AllocateChannels(32);
    generateAllSounds();
    initialized = true;
    return true;
}

void AudioManager::shutdown() {
    if (!initialized) return;

    stopMusic();
    stopAllSFX();

    for (auto& chunk : sfxChunks) {
        if (chunk) Mix_FreeChunk(chunk);
    }
    sfxChunks.clear();
    sfxMap.clear();

    for (auto& music : musicTracks) {
        if (music) Mix_FreeMusic(music);
    }
    musicTracks.clear();

    // Cleanup generated buffers
    for (auto& buf : generatedBuffers) {
        delete[] buf;
    }
    generatedBuffers.clear();
    generatedChunks.clear();

    activeChannels.clear();

    Mix_CloseAudio();
    initialized = false;
}

// Procedural sound generation - generate unique game sounds
Mix_Chunk* AudioManager::generateSound(int type, float duration, float freqStart, float freqEnd, float amplitude) {
    int sampleRate = 44100;
    int numSamples = sampleRate * duration;
    short* samples = new short[numSamples];
    // Track for cleanup
    generatedBuffers.push_back(samples);

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
    // Track for cleanup
    generatedChunks.push_back(chunk);
    chunk->allocated = 1;
    chunk->abuf = (Uint8*)samples;
    chunk->alen = numSamples * sizeof(short);
    chunk->volume = 64;
    return chunk;
}

SoundHandle AudioManager::loadSFX(const std::string& path, const std::string& name) {
    if (!initialized) return INVALID_SOUND;

    auto it = sfxMap.find(name);
    if (it != sfxMap.end()) return it->second;

    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (!chunk) {
        lastError = std::string("Failed to load SFX '") + path + "': " + Mix_GetError();
        fprintf(stderr, "%s\n", lastError.c_str());
        return INVALID_SOUND;
    }

    SoundHandle handle = static_cast<SoundHandle>(sfxChunks.size());
    sfxChunks.push_back(chunk);
    sfxMap[name] = handle;
    return handle;
}

SoundHandle AudioManager::loadMusic(const std::string& path) {
    if (!initialized) return INVALID_SOUND;

    Mix_Music* music = Mix_LoadMUS(path.c_str());
    if (!music) {
        lastError = std::string("Failed to load music '") + path + "': " + Mix_GetError();
        fprintf(stderr, "%s\n", lastError.c_str());
        return INVALID_SOUND;
    }

    SoundHandle handle = static_cast<SoundHandle>(musicTracks.size());
    musicTracks.push_back(music);
    return handle;
}

SoundHandle AudioManager::playSFX(SoundHandle sound, int loops) {
    if (!initialized || sound < 0 || sound >= (int)sfxChunks.size()) {
        return INVALID_SOUND;
    }

    cleanupFinishedChannels();

    int channel = getFreeChannel();
    if (channel == -1) return INVALID_SOUND;

    int vol = static_cast<int>(MIX_MAX_VOLUME * masterVolume * sfxVolume);
    Mix_Volume(channel, vol);

    Mix_PlayChannel(channel, sfxChunks[sound], loops);
    activeChannels.push_back(channel);

    return channel;
}

SoundHandle AudioManager::playSFX(const std::string& name, int loops) {
    auto it = sfxMap.find(name);
    if (it == sfxMap.end()) {
        lastError = std::string("SFX '") + name + "' not found";
        return INVALID_SOUND;
    }
    return playSFX(it->second, loops);
}

void AudioManager::playMusic(SoundHandle music, int loops) {
    if (!initialized || music < 0 || music >= (int)musicTracks.size()) return;

    int vol = static_cast<int>(MIX_MAX_VOLUME * masterVolume * musicVolume);
    Mix_VolumeMusic(vol);

    Mix_PlayMusic(musicTracks[music], loops);
}

void AudioManager::stopMusic() {
    Mix_HaltMusic();
}

void AudioManager::stopChannel(SoundHandle channel) {
    if (channel < 0) return;
    Mix_HaltChannel(channel);
    auto it = std::find(activeChannels.begin(), activeChannels.end(), channel);
    if (it != activeChannels.end()) activeChannels.erase(it);
}

void AudioManager::stopAllSFX() {
    for (int channel : activeChannels) {
        Mix_HaltChannel(channel);
    }
    activeChannels.clear();
}

void AudioManager::setMasterVolume(float volume) {
    masterVolume = std::clamp(volume, 0.0f, 1.0f);
}

void AudioManager::setCategoryVolume(AudioCategory category, float volume) {
    volume = std::clamp(volume, 0.0f, 1.0f);
    switch (category) {
        case AudioCategory::SFX: sfxVolume = volume; break;
        case AudioCategory::MUSIC: musicVolume = volume; break;
        case AudioCategory::UI: uiVolume = volume; break;
    }
}

float AudioManager::getCategoryVolume(AudioCategory category) const {
    switch (category) {
        case AudioCategory::SFX: return sfxVolume;
        case AudioCategory::MUSIC: return musicVolume;
        case AudioCategory::UI: return uiVolume;
    }
    return 1.0f;
}

SoundHandle AudioManager::playSFXAt(SoundHandle sound, float x, float y, int loops) {
    if (!initialized || sound < 0 || sound >= (int)sfxChunks.size()) {
        return INVALID_SOUND;
    }

    cleanupFinishedChannels();

    int channel = getFreeChannel();
    if (channel == -1) return INVALID_SOUND;

    int vol = static_cast<int>(MIX_MAX_VOLUME * masterVolume * sfxVolume);
    Mix_Volume(channel, vol);

    // Spatial audio: pan based on x position (-1 to 1)
    Uint8 left = static_cast<Uint8>(255.0f * (0.5f - x * 0.5f));
    Uint8 right = static_cast<Uint8>(255.0f * (0.5f + x * 0.5f));
    Mix_SetPanning(channel, left, right);

    // Vertical position affects volume
    float vertAttn = 1.0f - std::max(0.0f, y) * 0.3f;
    Mix_Volume(channel, static_cast<int>(vol * vertAttn));

    Mix_PlayChannel(channel, sfxChunks[sound], loops);
    activeChannels.push_back(channel);

    return channel;
}

bool AudioManager::isPlaying(SoundHandle channel) const {
    if (channel < 0) return false;
    return Mix_Playing(channel) != 0;
}

SoundHandle AudioManager::addGeneratedSound(Mix_Chunk* chunk, const std::string& name) {
    SoundHandle handle = static_cast<SoundHandle>(sfxChunks.size());
    sfxChunks.push_back(chunk);
    sfxMap[name] = handle;
    return handle;
}

void AudioManager::generateAllSounds() {
    addGeneratedSound(generateSound(2, 0.15f, 2000.0f, 200.0f, 0.8f), "shoot");
    addGeneratedSound(generateSound(2, 0.3f, 1000.0f, 100.0f, 0.9f), "lightning");
    addGeneratedSound(generateSound(2, 0.5f, 200.0f, 50.0f, 1.0f), "explosion");
    addGeneratedSound(generateSound(3, 0.08f, 800.0f, 200.0f, 0.7f), "hit");
    addGeneratedSound(generateSound(1, 0.2f, 400.0f, 800.0f, 0.6f), "kill");
    addGeneratedSound(generateSound(0, 0.3f, 300.0f, 1200.0f, 0.7f), "powerup");
    addGeneratedSound(generateSound(1, 1.0f, 200.0f, 600.0f, 0.9f), "boss_warning");
    addGeneratedSound(generateSound(1, 0.5f, 400.0f, 1000.0f, 0.7f), "wave_complete");
    addGeneratedSound(generateSound(0, 0.8f, 400.0f, 100.0f, 0.8f), "game_over");
    addGeneratedSound(generateSound(3, 0.05f, 1000.0f, 500.0f, 0.5f), "menu_click");
    addGeneratedSound(generateSound(1, 0.4f, 500.0f, 1500.0f, 0.7f), "upgrade");
    
    sndShoot = sfxMap["shoot"];
    sndLightning = sfxMap["lightning"];
    sndExplosion = sfxMap["explosion"];
    sndHit = sfxMap["hit"];
    sndKill = sfxMap["kill"];
    sndPowerUp = sfxMap["powerup"];
    sndBossWarning = sfxMap["boss_warning"];
    sndWaveComplete = sfxMap["wave_complete"];
    sndGameOver = sfxMap["game_over"];
    sndMenuClick = sfxMap["menu_click"];
    sndUpgrade = sfxMap["upgrade"];
}

void AudioManager::playShoot() { if (sndShoot >= 0) playSFX(sndShoot); }
void AudioManager::playLightning() { if (sndLightning >= 0) playSFX(sndLightning); }
void AudioManager::playExplosion() { if (sndExplosion >= 0) playSFX(sndExplosion); }
void AudioManager::playHit() { if (sndHit >= 0) playSFX(sndHit); }
void AudioManager::playKill() { if (sndKill >= 0) playSFX(sndKill); }
void AudioManager::playPowerUp() { if (sndPowerUp >= 0) playSFX(sndPowerUp); }
void AudioManager::playBossWarning() { if (sndBossWarning >= 0) playSFX(sndBossWarning); }
void AudioManager::playWaveComplete() { if (sndWaveComplete >= 0) playSFX(sndWaveComplete); }
void AudioManager::playGameOver() { if (sndGameOver >= 0) playSFX(sndGameOver); }
void AudioManager::playMenuClick() { if (sndMenuClick >= 0) playSFX(sndMenuClick); }
void AudioManager::playUpgrade() { if (sndUpgrade >= 0) playSFX(sndUpgrade); }

int AudioManager::getFreeChannel() {
    for (int i = 0; i < 32; i++) {
        if (!Mix_Playing(i)) return i;
    }
    return -1;
}

void AudioManager::cleanupFinishedChannels() {
    activeChannels.erase(
        std::remove_if(activeChannels.begin(), activeChannels.end(),
            [](int ch) { return !Mix_Playing(ch); }),
        activeChannels.end());
}
