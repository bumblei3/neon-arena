// main.cpp - SDL2 + OpenGL 3.3+ Core Profile Prototype
#include <SDL.h>
#include <GL/glew.h>
#include <cstdio>
#include "game.h"
#include "audio.h"

// Global audio system
AudioSystem* g_audio = nullptr;

// Legacy sound pointers (for backward compatibility)
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

    // Initialize Audio System
    g_audio = new AudioSystem();
    if (!g_audio->init()) {
        fprintf(stderr, "Audio system init failed\n");
        SDL_GL_DeleteContext(ctx);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create game
    Game game;
    if (!game.init(window)) {
        fprintf(stderr, "Failed to initialize game\n");
        delete g_audio;
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
    delete g_audio;
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
