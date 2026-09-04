// audio_polish.h - Dynamic music layers, reverb, occlusion
#pragma once
#include "audio_manager.h"
#include "music_generator.h"

class AudioPolish {
public:
    static void init();
    static void shutdown();
    static void update(float dt, int aliveBots, int wave, bool bossActive, bool playerInLargeArena);
    
    // Music intensity control
    static void setIntensity(float i);  // 0.0 = calm, 1.0 = full chaos
    
    // Spatial effects
    static void setListenerPosition(float x, float z);
    static void playSFX3D(const std::string& name, float x, float z);
    
    // Reverb (echo for large arenas)
    static void setReverbAmount(float amount);  // 0.0 = dry, 1.0 = max echo
    
private:
    static float reverbAmount;
    static float intensity;
    static float listenerX, listenerZ;
    
    // Occlusion
    static float calculateOcclusion(float soundX, float soundZ);
    static float calculateReverb(float soundX, float soundZ);
};
