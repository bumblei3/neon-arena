// main.cpp - SDL2 + OpenGL 3.3+ Core Profile Prototype
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_mixer.h>
#include <cstdio>
#include <cmath>
#include "game.h"

// Procedural sound generation
Mix_Chunk* generateSound(int type) {
    // type: 0=shoot, 1=explosion, 2=wave, 3=gameover
    int sampleRate = 44100;
    int duration;
    if (type == 0) duration = 1500;      // 1.5s shoot
    else if (type == 1) duration = 3000; // 3s explosion
    else if (type == 2) duration = 4000; // 4s wave
    else duration = 5000;                // 5s game over

    int numSamples = sampleRate * duration / 1000;
    short* samples = new short[numSamples];

    for (int i = 0; i < numSamples; i++) {
        float t = (float)i / sampleRate;
        float amplitude = 0.0f;

        if (type == 0) {
            // Shoot: descending sine with noise
            float freq = 800.0f - t * 2000.0f;
            amplitude = sinf(2.0f * M_PI * freq * t);
            amplitude *= (1.0f - (float)i / numSamples); // fade out
            // Add some noise for "swoosh"
            amplitude += ((rand() % 100) / 100.0f - 0.5f) * 0.3f;
        } else if (type == 1) {
            // Explosion: white noise with low rumble
            amplitude = ((rand() % 200) / 100.0f - 1.0f);
            amplitude *= (1.0f - (float)i / numSamples); // fade out
            // Add low rumble
            amplitude += sinf(2.0f * M_PI * 60.0f * t) * 0.5f;
        } else if (type == 2) {
            // Wave complete: rising chime
            float freq = 400.0f + t * 600.0f;
            amplitude = sinf(2.0f * M_PI * freq * t) * 0.3f;
            amplitude += sinf(2.0f * M_PI * freq * 1.5f * t) * 0.2f;
            amplitude *= (1.0f - (float)i / numSamples);
        } else {
            // Game over: descending tone
            float freq = 300.0f - t * 40.0f;
            amplitude = sinf(2.0f * M_PI * freq * t) * 0.4f;
            amplitude *= (1.0f - (float)i / numSamples);
        }

        samples[i] = (short)(amplitude * 32767 * 0.5f);
    }

    Mix_Chunk* chunk = new Mix_Chunk();
    chunk->allocated = 1;
    chunk->abuf = (Uint8*)samples;
    chunk->alen = numSamples * sizeof(short);
    chunk->volume = 64;
    return chunk;
}

// Global sound pointers (for game.cpp)
Mix_Chunk* g_sndShoot = nullptr;
Mix_Chunk* g_sndExplosion = nullptr;
Mix_Chunk* g_sndWave = nullptr;
Mix_Chunk* g_sndGameOver = nullptr;

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // Set OpenGL attributes BEFORE creating window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "NEON ARENA - OpenGL 3.3 Core Profile",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280,
        720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create OpenGL context
    SDL_GLContext ctx = SDL_GL_CreateContext(window);
    if (!ctx) {
        fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Make context current
    SDL_GL_MakeCurrent(window, ctx);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "glewInit failed\n");
        SDL_GL_DeleteContext(ctx);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    printf("OpenGL version: %s\n", (const char*)glGetString(GL_VERSION));
    printf("GLSL version: %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("Vendor: %s\n", (const char*)glGetString(GL_VENDOR));
    printf("Renderer: %s\n", (const char*)glGetString(GL_RENDERER));

    // Enable VSync
    SDL_GL_SetSwapInterval(1);

    // Enable multisampling
    glEnable(GL_MULTISAMPLE);

    // Initialize SDL_Mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "SDL_Mixer init failed: %s\n", Mix_GetError());
        SDL_GL_DeleteContext(ctx);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    Mix_AllocateChannels(16);

    // Generate procedural sounds
    printf("Generating procedural sounds...\n");
    Mix_Chunk* sndShoot = generateSound(0);
    Mix_Chunk* sndExplosion = generateSound(1);
    Mix_Chunk* sndWave = generateSound(2);
    Mix_Chunk* sndGameOver = generateSound(3);
    printf("Sounds generated.\n");

    // Set global sounds for game
    g_sndShoot = sndShoot;
    g_sndExplosion = sndExplosion;
    g_sndWave = sndWave;
    g_sndGameOver = sndGameOver;

    // Create game
    Game game;
    if (!game.init(window)) {
        fprintf(stderr, "Failed to initialize game\n");
        delete[] sndShoot->abuf;
        delete sndShoot;
        delete[] sndExplosion->abuf;
        delete sndExplosion;
        delete[] sndWave->abuf;
        delete sndWave;
        delete[] sndGameOver->abuf;
        delete sndGameOver;
        SDL_GL_DeleteContext(ctx);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    printf("\n=== NEON ARENA - OpenGL Prototype ===\n");
    printf("Controls:\n");
    printf("  WASD - Move\n");
    printf("  Mouse - Look around\n");
    printf("  Left Click - Shoot (Railgun)\n");
    printf("  SPACE - Start next wave\n");
    printf("  ESC - Quit\n\n");

    // Main loop
    game.run();

    // Cleanup
    game.shutdown();
    delete[] sndShoot->abuf;
    delete sndShoot;
    delete[] sndExplosion->abuf;
    delete sndExplosion;
    delete[] sndWave->abuf;
    delete sndWave;
    delete[] sndGameOver->abuf;
    delete sndGameOver;
    Mix_CloseAudio();
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
