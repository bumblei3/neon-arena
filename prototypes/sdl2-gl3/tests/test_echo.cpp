#include <cstdio>
#include <cassert>
#include <cmath>
#include "../src/echo.h"

static int echoPassed = 0, echoFailed = 0;

#define ECHO_TEST(name, expr) do { \
    printf("  Running %s... ", name); \
    fflush(stdout); \
    if (expr) { printf("PASSED\n"); echoPassed++; } \
    else { printf("FAILED\n"); echoFailed++; } \
} while(0)

void testEchoSystem() {
    printf("\n[Echo System Tests]\n");

    // Construction
    {
        EchoSystem echo;
        echo.init();
        ECHO_TEST("constructs", true);
        ECHO_TEST("starts_inactive", !echo.isActive());
        ECHO_TEST("starts_not_recording", !echo.isRecording());
        ECHO_TEST("starts_not_playing", !echo.isPlaying());
    }

    // Recording
    {
        EchoSystem echo;
        echo.init();
        echo.startRecording();
        ECHO_TEST("recording_active", echo.isRecording());

        // Record 10 frames
        for (int i = 0; i < 10; i++) {
            echo.recordFrame(i * 1.0f, 0, 0, 5.0f, 0, 0, 0, 0, i * 0.016f);
        }
        ECHO_TEST("frames_recorded", echo.getActiveFrameCount() == 10);
    }

    // Stop recording
    {
        EchoSystem echo;
        echo.init();
        echo.startRecording();
        for (int i = 0; i < 5; i++) {
            echo.recordFrame(i, 0, 0, 0, 0, 0, 0, 0, i * 0.016f);
        }
        echo.stopRecording();
        ECHO_TEST("stop_recording", !echo.isRecording());
        ECHO_TEST("frames_preserved", echo.getActiveFrameCount() == 5);
    }

    // Play
    {
        EchoSystem echo;
        echo.init();
        echo.startRecording();
        for (int i = 0; i < 10; i++) {
            echo.recordFrame(i, 0, 0, 0, 0, 0, 0, 0, i * 0.016f);
        }
        echo.stopRecording();
        echo.play();
        ECHO_TEST("play_active", echo.isPlaying());
        ECHO_TEST("echo_active", echo.isActive());
    }

    // Update advances playback
    {
        EchoSystem echo;
        echo.init();
        echo.startRecording();
        for (int i = 0; i < 100; i++) {
            echo.recordFrame(i * 0.1f, 0, 0, 1.0f, 0, 0, 0, 0, i * 0.016f);
        }
        echo.stopRecording();
        echo.play();

        echo.update(0.016f);
        ECHO_TEST("update_advances", echo.isPlaying());
    }

    // Boost not active initially
    {
        EchoSystem echo;
        echo.init();
        ECHO_TEST("no_boost_initially", !echo.isBoostActive());
        ECHO_TEST("boost_remaining_zero", echo.getBoostRemaining() == 0.0f);
    }

    // Boost multiplier
    {
        EchoSystem echo;
        echo.init();
        ECHO_TEST("boost_multiplier_value", echo.getBoostMultiplier() == 2.0f);
    }

    // Reset
    {
        EchoSystem echo;
        echo.init();
        echo.startRecording();
        for (int i = 0; i < 50; i++) {
            echo.recordFrame(i, 0, 0, 0, 0, 0, 0, 0, i * 0.016f);
        }
        echo.stopRecording();
        echo.play();
        echo.reset();
        ECHO_TEST("reset_clears_frames", echo.getActiveFrameCount() == 0);
        ECHO_TEST("reset_deactivates", !echo.isActive());
        ECHO_TEST("reset_stops_playback", !echo.isPlaying());
    }

    // Ghost inactive when echo inactive
    {
        EchoSystem echo;
        echo.init();
        const EchoGhost& ghost = echo.getGhost();
        ECHO_TEST("ghost_inactive_initially", !ghost.active);
    }

    // Player boost interaction (basic)
    {
        EchoSystem echo;
        echo.init();
        echo.startRecording();
        // Record frames at origin
        for (int i = 0; i < 60; i++) {
            echo.recordFrame(0, 0, 0, 0, 0, 0, 0, 0, i * 0.016f);
        }
        echo.stopRecording();
        echo.play();

        // Create a mock game with player position above echo
        struct MockGame {
            struct { float x = 0, y = 10, z = 0; } pos;
            struct { float x = 0, y = 1.7f, z = 0; } player;
            float playerSpeed = 10.0f;
        } game;

        // Player above echo position (y=10 > ghost y=0, within radius)
        float speed = 10.0f;
        bool boosted = echo.checkPlayerBoost(0, 10, 0, speed);
        // Should activate boost
        ECHO_TEST("player_boost_activates", boosted);
        ECHO_TEST("boost_multiplier_applied", speed == 20.0f);
    }

    // Bot collision interaction
    {
        EchoSystem echo;
        echo.init();
        echo.startRecording();
        for (int i = 0; i < 60; i++) {
            echo.recordFrame(0, 0, 0, 0, 0, 0, 0, 0, i * 0.016f);
        }
        echo.stopRecording();
        echo.play();

        // Bot at echo position
        echo.checkBotCollision(0, 0, 0, 0);
        ECHO_TEST("bot_collision_consumes_ghost", !echo.getGhost().active);
    }

    // Lifetime expiry
    {
        EchoSystem echo;
        echo.init();
        echo.startRecording();
        for (int i = 0; i < 10; i++) {
            echo.recordFrame(0, 0, 0, 0, 0, 0, 0, 0, i * 0.016f);
        }
        echo.stopRecording();
        echo.play();

        // Update for longer than lifetime
        for (int i = 0; i < 1000; i++) {
            echo.update(0.016f);
        }
        ECHO_TEST("echo_expires", !echo.isActive());
    }

    // Ring buffer wrap
    {
        EchoSystem echo;
        echo.init();
        echo.startRecording();
        // Record more than MAX_ECHO_FRAMES
        for (int i = 0; i < EchoSystem::MAX_ECHO_FRAMES + 100; i++) {
            echo.recordFrame(i * 0.01f, 0, 0, 0, 0, 0, 0, 0, i * 0.016f);
        }
        echo.stopRecording();
        // Should not crash and should have valid frame count
        ECHO_TEST("ring_buffer_no_crash", echo.getActiveFrameCount() <= EchoSystem::MAX_ECHO_FRAMES);
    }

    // Effects update
    {
        EchoSystem echo;
        echo.init();
        echo.startRecording();
        for (int i = 0; i < 10; i++) {
            echo.recordFrame(0, 0, 0, 0, 0, 0, 0, 0, i * 0.016f);
        }
        echo.stopRecording();
        echo.play();

        // Trigger an effect via bot collision
        echo.checkBotCollision(0, 0, 0, 0);
        ECHO_TEST("effect_created", echo.getEffects().size() > 0);
    }

    printf("\n[Echo Results] Passed: %d, Failed: %d\n", echoPassed, echoFailed);
}

int main() {
    testEchoSystem();
    return echoFailed > 0 ? 1 : 0;
}
