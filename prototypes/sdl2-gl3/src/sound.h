// sound.h - SDL2 Mixer sound system
#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include <string>
#include <vector>
#include <unordered_map>

class Sound {
public:
    Sound();
    ~Sound();

    bool init();
    void shutdown();

    // Load sound effects
    bool loadSound(const std::string& name, const std::string& path);
    bool loadMusic(const std::string& path);

    // Play sounds
    void playSound(const std::string& name, int loops = 0);
    void playSoundAt(const std::string& name, float x, float y, float z, float maxDist = 50.0f);
    void playMusic();
    void stopMusic();

    // Volume control (0-128)
    void setSoundVolume(int vol);
    void setMusicVolume(int vol);

    // Update listener position (for 3D audio)
    void setListener(float x, float y, float z, float yaw);

private:
    std::unordered_map<std::string, Mix_Chunk*> sounds_;
    Mix_Music* music_ = nullptr;
    int soundVolume_ = 64;
    int musicVolume_ = 32;
    float listenerX_ = 0, listenerY_ = 0, listenerZ_ = 0;
};
