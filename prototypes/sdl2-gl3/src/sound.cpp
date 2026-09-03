// sound.cpp - SDL2 Mixer sound system implementation
#include "sound.h"
#include <cmath>
#include <cstdio>

Sound::Sound() : music_(nullptr), soundVolume_(64), musicVolume_(32) {}

Sound::~Sound() {
    shutdown();
}

bool Sound::init() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "SDL_Mixer init failed: %s\n", Mix_GetError());
        return false;
    }
    Mix_AllocateChannels(16);
    return true;
}

void Sound::shutdown() {
    for (auto& pair : sounds_) {
        Mix_FreeChunk(pair.second);
    }
    sounds_.clear();

    if (music_) {
        Mix_FreeMusic(music_);
        music_ = nullptr;
    }

    Mix_CloseAudio();
}

bool Sound::loadSound(const std::string& name, const std::string& path) {
    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (!chunk) {
        fprintf(stderr, "Failed to load sound %s: %s\n", path.c_str(), Mix_GetError());
        return false;
    }
    sounds_[name] = chunk;
    return true;
}

bool Sound::loadMusic(const std::string& path) {
    music_ = Mix_LoadMUS(path.c_str());
    if (!music_) {
        fprintf(stderr, "Failed to load music %s: %s\n", path.c_str(), Mix_GetError());
        return false;
    }
    return true;
}

void Sound::playSound(const std::string& name, int loops) {
    auto it = sounds_.find(name);
    if (it == sounds_.end()) return;

    Mix_VolumeChunk(it->second, soundVolume_);
    Mix_PlayChannel(-1, it->second, loops);
}

void Sound::playSoundAt(const std::string& name, float x, float y, float z, float maxDist) {
    auto it = sounds_.find(name);
    if (it == sounds_.end()) return;

    float dx = x - listenerX_;
    float dy = y - listenerY_;
    float dz = z - listenerZ_;
    float dist = sqrtf(dx * dx + dy * dy + dz * dz);

    if (dist > maxDist) return;

    // Distance attenuation
    float volume = 1.0f - (dist / maxDist);
    volume = volume * volume; // Quadratic falloff
    int vol = (int)(volume * soundVolume_);
    if (vol < 0) vol = 0;
    if (vol > 128) vol = 128;

    // Stereo panning based on X position
    int panLeft = 255;
    int panRight = 255;
    if (dist > 0.1f) {
        float angle = atan2f(dx, dz);
        panLeft = (int)(255 * (0.5f + 0.5f * sinf(angle)));
        panRight = (int)(255 * (0.5f - 0.5f * sinf(angle)));
    }

    int channel = Mix_PlayChannel(-1, it->second, 0);
    if (channel >= 0) {
        Mix_Volume(channel, vol);
        Mix_SetPanning(channel, panLeft, panRight);
    }
}

void Sound::playMusic() {
    if (!music_) return;
    Mix_VolumeMusic(musicVolume_);
    Mix_PlayMusic(music_, -1); // Loop forever
}

void Sound::stopMusic() {
    Mix_HaltMusic();
}

void Sound::setSoundVolume(int vol) {
    soundVolume_ = vol;
}

void Sound::setMusicVolume(int vol) {
    musicVolume_ = vol;
}

void Sound::setListener(float x, float y, float z, float yaw) {
    listenerX_ = x;
    listenerY_ = y;
    listenerZ_ = z;
}
