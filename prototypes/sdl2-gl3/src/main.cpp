// main.cpp - SDL2 + OpenGL 3.3+ Core Profile Prototype
#include <SDL.h>
#include <GL/glew.h>
#include <cstdio>
#include <cstring>
#include "game.h"
#include "audio_manager.h"
#include "music_generator.h"

// Global music generator
MusicGenerator* g_music = nullptr;

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

    // Initialize Audio Manager
    AudioManager audio;
    if (!audio.init()) {
        fprintf(stderr, "Audio system init failed\n");
        SDL_GL_DeleteContext(ctx);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialize Music Generator (procedural synthwave)
    MusicGenerator music;
    if (!music.init()) {
        fprintf(stderr, "Music generator init failed\n");
        SDL_GL_DeleteContext(ctx);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    g_music = &music;

    // Hook music generator into SDL2's audio system
    Mix_HookMusic(MusicGenerator::mixCallback, nullptr);
    music.setVolume(0.5f); // Start at 50% music volume

    // Play menu music
    music.playScene(MusicScene::MENU);

    // Create game
    Game game;
    if (!game.init(window)) {
        fprintf(stderr, "Failed to initialize game\n");
        SDL_GL_DeleteContext(ctx);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool ghost = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--ghost") == 0) ghost = true;
    }
    if (ghost) {
        game.startNewRun(Loadout::GHOST);
        printf("\n=== NEON ARENA — GHOST PROTOCOL ===\n");
        printf("  WASD move   Mouse look   LMB snipe   RMB ADS\n");
        printf("  G scan   H EMP   N/I nuke   J cloak   SPACE next wave   ESC pause\n\n");
    } else {
        printf("\n=== NEON ARENA - OpenGL Prototype ===\n");
        printf("Controls:\n");
        printf("  WASD - Move\n");
        printf("  Mouse - Look around\n");
        printf("  Left Click - Shoot (Railgun)\n");
        printf("  SPACE - Start next wave\n");
        printf("  ESC - Quit\n");
        printf("  Launch with --ghost to skip the menu into Ghost kit\n\n");
    }

    // Main loop
    game.run();

    // Cleanup
    music.stop();
    Mix_HookMusic(nullptr, nullptr);
    game.shutdown();
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
