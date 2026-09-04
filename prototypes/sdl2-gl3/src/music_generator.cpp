#include "music_generator.h"
#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

MusicGenerator* MusicGenerator::instance_ = nullptr;

MusicGenerator::MusicGenerator() {
    instance_ = this;
}

MusicGenerator::~MusicGenerator() {
    shutdown();
    instance_ = nullptr;
}

float MusicGenerator::getNoteFrequency(int midiNote) {
    return 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f);
}

void MusicGenerator::initPattern(Pattern& pat, const std::vector<int>& notes, int steps) {
    pat.notes = notes;
    pat.triggers.resize(steps, false);
    pat.steps = steps;
    pat.currentStep = 0;
}

bool MusicGenerator::init(int sr) {
    sampleRate = sr;
    beatIncrement = bpm / 60.0f / sampleRate;

    // Initialize patterns
    // Bass: C2, G2, A2, F2 (synthwave bassline)
    initPattern(bass, {36, 36, 43, 43, 36, 36, 41, 41}, 8);
    // Arp: C4, E4, G4, C5 (arpeggio)
    initPattern(arp, {60, 64, 67, 72, 67, 64, 60, 64}, 8);
    // Drums
    initPattern(drums, {-1, -1, 36, -1, -1, -1, 36, -1}, 8);
    drums.triggers = {true, false, true, false, true, false, true, false};

    playing = true;
    return true;
}

void MusicGenerator::shutdown() {
    playing = false;
}

void MusicGenerator::playScene(MusicScene scene) {
    targetScene = scene;
    crossfadeTo(scene);
    currentScene = scene;
}

void MusicGenerator::stop() {
    playing = false;
}

void MusicGenerator::setVolume(float vol) {
    volume = (vol < 0.0f) ? 0.0f : ((vol > 1.0f) ? 1.0f : vol);
}

void MusicGenerator::crossfadeTo(MusicScene scene) {
    switch (scene) {
        case MusicScene::MENU:
            bpm = 100.0f;
            initPattern(bass, {36, 43, 36, 41, 36, 43, 36, 48}, 8);
            initPattern(arp, {60, 64, 67, 72, 76, 72, 67, 64}, 8);
            drums.triggers = {true, false, false, false, true, false, false, false};
            break;
        case MusicScene::GAMEPLAY:
            bpm = 128.0f;
            initPattern(bass, {36, 36, 39, 41, 36, 36, 39, 43}, 8);
            initPattern(arp, {60, 64, 67, 72, 67, 64, 60, 64}, 8);
            drums.triggers = {true, false, true, false, true, false, true, true};
            break;
        case MusicScene::BOSS:
            bpm = 150.0f;
            initPattern(bass, {36, 31, 36, 31, 34, 29, 34, 29}, 8);
            initPattern(arp, {57, 60, 63, 65, 63, 60, 57, 53}, 8);
            drums.triggers = {true, true, true, true, true, true, true, true};
            break;
        case MusicScene::GAME_OVER:
            bpm = 80.0f;
            initPattern(bass, {36, 39, 36, 34, 32, 29, 27, 24}, 8);
            initPattern(arp, {57, 60, 63, 60, 57, 53, 50, 48}, 8);
            drums.triggers = {true, false, false, true, false, false, true, false};
            break;
    }
    beatIncrement = bpm / 60.0f / sampleRate;
}

void MusicGenerator::mixCallback(void* udata, Uint8* stream, int len) {
    MusicGenerator* gen = instance_;
    if (!gen || !gen->playing) {
        std::memset(stream, 0, len);
        return;
    }
    // Stereo, 16-bit signed
    int numSamples = len / 4; // 4 bytes per stereo sample (2 channels * 2 bytes)
    int16_t* buf = reinterpret_cast<int16_t*>(stream);
    gen->synthesize(buf, numSamples * 2); // stereo interleaved
}

void MusicGenerator::synthesize(int16_t* buffer, int length) {
    // length is total samples (stereo interleaved)
    for (int i = 0; i < length; i += 2) {
        float sample = 0.0f;

        // Update beat position
        beatPosition += beatIncrement;
        if (beatPosition >= 1.0f) {
            beatPosition -= 1.0f;

            // Advance sequencer
            bass.currentStep = (bass.currentStep + 1) % bass.steps;
            arp.currentStep = (arp.currentStep + 1) % arp.steps;
            drums.currentStep = (drums.currentStep + 1) % drums.steps;

            // Trigger envelopes
            if (bass.notes[bass.currentStep] >= 0) bassEnv = 1.0f;
            if (arp.notes[arp.currentStep] >= 0) arpEnv = 1.0f;
            if (drums.triggers[drums.currentStep]) drumEnv = 1.0f;
        }

        // Bass synth (saw with filter)
        if (bass.notes[bass.currentStep] >= 0 && bassEnv > 0.0f) {
            float freq = getNoteFrequency(bass.notes[bass.currentStep]);
            float t = beatPosition;
            // Saw wave
            float saw = 2.0f * (t * freq - std::floor(t * freq + 0.5f));
            // Simple low-pass filter
            filterMem += 0.1f * (saw - filterMem);
            sample += filterMem * bassEnv * 0.3f;
            bassEnv *= 0.999f; // slow decay
        }

        // Arp synth (square with decay)
        if (arp.notes[arp.currentStep] >= 0 && arpEnv > 0.0f) {
            float freq = getNoteFrequency(arp.notes[arp.currentStep]);
            float t = beatPosition;
            float square = (std::sin(2.0f * M_PI * freq * t) > 0.0f) ? 0.3f : -0.3f;
            sample += square * arpEnv * 0.2f;
            arpEnv *= 0.995f; // faster decay for staccato
        }

        // Drums (kick/snare from noise + sine)
        if (drums.triggers[drums.currentStep] && drumEnv > 0.0f) {
            // Kick drum: low sine with fast decay
            float kickFreq = 55.0f;
            float kickT = beatPosition;
            float kick = std::sin(2.0f * M_PI * kickFreq * kickT) * drumEnv * 0.4f;
            // Add some noise for snare-ish sound
            float noise = ((rand() % 100) / 100.0f - 0.5f) * drumEnv * 0.2f;
            sample += kick + noise;
            drumEnv *= 0.99f;
        }

        // Master volume + clip
        sample *= volume;
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;

        int16_t s = static_cast<int16_t>(sample * 32767.0f);
        buffer[i] = s;      // left
        buffer[i + 1] = s;  // right (mono to stereo)
    }
}
