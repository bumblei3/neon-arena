#include "audio_polish.h"
#include <cmath>
#include <cstring>

float AudioPolish::reverbAmount = 0.0f;
float AudioPolish::intensity = 0.5f;
float AudioPolish::listenerX = 0.0f;
float AudioPolish::listenerZ = 0.0f;

// Simple delay-line reverb
struct DelayLine {
    static const int MAX_SAMPLES = 44100; // 1 second at 44.1kHz
    float buffer[MAX_SAMPLES] = {};
    int writePos = 0;
    int delaySamples = 22050; // 500ms default
    float feedback = 0.3f;
    float wetLevel = 0.3f;
    
    float process(float input) {
        int readPos = writePos - delaySamples;
        if (readPos < 0) readPos += MAX_SAMPLES;
        
        float output = buffer[readPos];
        buffer[writePos] = input + output * feedback;
        writePos++;
        if (writePos >= MAX_SAMPLES) writePos = 0;
        
        return input + output * wetLevel;
    }
    
    void setDelay(float seconds, int sampleRate) {
        delaySamples = (int)(seconds * sampleRate);
        if (delaySamples >= MAX_SAMPLES) delaySamples = MAX_SAMPLES - 1;
    }
};

static DelayLine reverbDelay;

void AudioPolish::init() {
    reverbDelay.setDelay(0.3f, 44100);
    reverbDelay.feedback = 0.3f;
    reverbDelay.wetLevel = 0.3f;
}

void AudioPolish::shutdown() {
}

void AudioPolish::update(float dt, int aliveBots, int wave, bool bossActive, bool playerInLargeArena) {
    // Calculate intensity based on game state
    float targetIntensity = 0.3f;
    
    if (aliveBots > 0) {
        targetIntensity += aliveBots * 0.05f;
    }
    if (bossActive) {
        targetIntensity += 0.3f;
    }
    if (wave > 10) {
        targetIntensity += 0.1f;
    }
    
    if (targetIntensity > 1.0f) targetIntensity = 1.0f;
    
    // Smooth transition
    if (intensity < targetIntensity) {
        intensity += dt * 0.5f;
        if (intensity > targetIntensity) intensity = targetIntensity;
    } else if (intensity > targetIntensity) {
        intensity -= dt * 0.3f;
        if (intensity < targetIntensity) intensity = targetIntensity;
    }
    
    // Adjust reverb based on arena size
    if (playerInLargeArena) {
        reverbAmount = 0.3f;
        reverbDelay.wetLevel = 0.3f;
    } else {
        reverbAmount = 0.1f;
        reverbDelay.wetLevel = 0.15f;
    }
}

void AudioPolish::setIntensity(float i) {
    intensity = (i < 0.0f) ? 0.0f : ((i > 1.0f) ? 1.0f : i);
}

void AudioPolish::setListenerPosition(float x, float z) {
    listenerX = x;
    listenerZ = z;
}

float AudioPolish::calculateOcclusion(float soundX, float soundZ) {
    // Simple occlusion: check if sound is behind walls
    // For now, just use distance-based attenuation
    float dx = soundX - listenerX;
    float dz = soundZ - listenerZ;
    float dist = std::sqrt(dx * dx + dz * dz);
    
    // No occlusion within 5m
    if (dist < 5.0f) return 1.0f;
    
    // Gradual attenuation with distance
    float occlusion = 1.0f / (1.0f + dist * 0.02f);
    return occlusion;
}

float AudioPolish::calculateReverb(float soundX, float soundZ) {
    float dx = soundX - listenerX;
    float dz = soundZ - listenerZ;
    float dist = std::sqrt(dx * dx + dz * dz);
    
    // More reverb for distant sounds
    if (dist < 10.0f) return reverbAmount * 0.5f;
    return reverbAmount;
}

void AudioPolish::playSFX3D(const std::string& name, float x, float z) {
    float occlusion = calculateOcclusion(x, z);
    float reverb = calculateReverb(x, z);
    
    // Adjust volume based on occlusion
    int vol = (int)(occlusion * MIX_MAX_VOLUME);
    if (vol < 0) vol = 0;
    if (vol > MIX_MAX_VOLUME) vol = MIX_MAX_VOLUME;
    
    // Play sound with adjusted volume
    // Note: This is a simplified version - in production you'd use Mix_Volume()
    // For now, just play the sound normally
    if (g_audio) {
        Mix_Chunk* chunk = nullptr;
        auto handle = g_audio->playSFX(name);
        if (handle != INVALID_SOUND) {
            Mix_Volume(handle, vol);
        }
    }
}

void AudioPolish::setReverbAmount(float amount) {
    reverbAmount = (amount < 0.0f) ? 0.0f : ((amount > 1.0f) ? 1.0f : amount);
    reverbDelay.wetLevel = reverbAmount;
}
