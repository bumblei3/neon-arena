// Tests for AudioManager logic (volume math, no SDL dependency)
#include <cstdio>
#include <cassert>
#include <cmath>

// Mock volume logic matching AudioManager's category system
enum class AudioCategory { Master, SFX, Music, UI };

struct AudioState {
    float volumes[4] = {1.0f, 1.0f, 1.0f, 0.7f}; // Master, SFX, Music, UI defaults

    void setVolume(AudioCategory cat, float v) {
        int idx = static_cast<int>(cat);
        volumes[idx] = (v < 0.0f) ? 0.0f : ((v > 1.0f) ? 1.0f : v);
    }

    float getVolume(AudioCategory cat) const {
        return volumes[static_cast<int>(cat)];
    }

    // Effective volume = Master * Category
    float getEffectiveVolume(AudioCategory cat) const {
        return volumes[0] * volumes[static_cast<int>(cat)];
    }

    // Spatial panning: returns left/right volume for position (-1 to 1)
    void getPan(float x, float posX, float& left, float& right) {
        float relX = x - posX; // negative = left, positive = right
        float pan = (relX < -20.0f) ? -1.0f : ((relX > 20.0f) ? 1.0f : relX / 20.0f);
        float dist = std::abs(relX);
        float attenuation = (dist > 30.0f) ? 0.0f : (1.0f - dist / 30.0f);
        float vol = getEffectiveVolume(AudioCategory::SFX) * attenuation;
        left = vol * ((pan < 0) ? 1.0f : 1.0f - pan);
        right = vol * ((pan > 0) ? 1.0f : 1.0f + pan);
    }
};

static int audioPassed = 0, audioFailed = 0;

#define AUDIO_TEST(name, expr) do { \
    printf("  Running %s... ", name); \
    fflush(stdout); \
    if (expr) { printf("PASSED\n"); audioPassed++; } \
    else { printf("FAILED\n"); audioFailed++; } \
} while(0)

void testAudioLogic() {
    printf("\n[Audio Manager Logic Tests]\n");

    // Volume defaults
    {
        AudioState a;
        AUDIO_TEST("default_master_full", a.getVolume(AudioCategory::Master) == 1.0f);
        AUDIO_TEST("default_sfx_full", a.getVolume(AudioCategory::SFX) == 1.0f);
        AUDIO_TEST("default_music_full", a.getVolume(AudioCategory::Music) == 1.0f);
        AUDIO_TEST("default_ui_70pct", a.getVolume(AudioCategory::UI) == 0.7f);
    }

    // Volume clamping
    {
        AudioState a;
        a.setVolume(AudioCategory::SFX, 1.5f);
        AUDIO_TEST("clamp_high", a.getVolume(AudioCategory::SFX) == 1.0f);
        a.setVolume(AudioCategory::SFX, -0.5f);
        AUDIO_TEST("clamp_low", a.getVolume(AudioCategory::SFX) == 0.0f);
    }

    // Volume setting
    {
        AudioState a;
        a.setVolume(AudioCategory::Music, 0.5f);
        AUDIO_TEST("set_music_half", a.getVolume(AudioCategory::Music) == 0.5f);
    }

    // Effective volume chain
    {
        AudioState a;
        a.setVolume(AudioCategory::Master, 0.5f);
        a.setVolume(AudioCategory::SFX, 0.8f);
        float eff = a.getEffectiveVolume(AudioCategory::SFX);
        AUDIO_TEST("effective_is_product", std::abs(eff - 0.4f) < 0.001f);
    }

    // Master zero = silence
    {
        AudioState a;
        a.setVolume(AudioCategory::Master, 0.0f);
        AUDIO_TEST("master_zero_silences_sfx", a.getEffectiveVolume(AudioCategory::SFX) == 0.0f);
    }

    // Spatial panning - center
    {
        AudioState a;
        float left, right;
        a.getPan(0, 0, left, right);
        AUDIO_TEST("center_pan_equal", std::abs(left - right) < 0.001f);
    }

    // Spatial panning - far left
    {
        AudioState a;
        float left, right;
        a.getPan(-25, 0, left, right);
        AUDIO_TEST("left_pan_left_louder", left > right);
    }

    // Spatial panning - far right
    {
        AudioState a;
        float left, right;
        a.getPan(25, 0, left, right);
        AUDIO_TEST("right_pan_right_louder", right > left);
    }

    // Distance attenuation
    {
        AudioState a;
        float left1, right1, left2, right2;
        a.getPan(5, 0, left1, right1);
        a.getPan(50, 0, left2, right2);
        AUDIO_TEST("closer_is_louder", left1 > left2);
    }

    // Max distance = silence
    {
        AudioState a;
        float left, right;
        a.getPan(100, 0, left, right);
        AUDIO_TEST("far_silent", left == 0.0f && right == 0.0f);
    }

    // Closeness = full volume
    {
        AudioState a;
        float left, right;
        a.getPan(0, 0, left, right);
        AUDIO_TEST("zero_dist_full_vol", std::abs(left - 1.0f) < 0.01f && std::abs(right - 1.0f) < 0.01f);
    }

    printf("\n[Audio Logic Results] Passed: %d, Failed: %d\n", audioPassed, audioFailed);
}

int main() {
    testAudioLogic();
    return audioFailed > 0 ? 1 : 0;
}
