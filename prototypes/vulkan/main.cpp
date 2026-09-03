// NEON ARENA - Vulkan + SDL2 Prototype
// Main: bootstrap, game loop, input
#include "renderer.h"
#include "game.h"
#include "hud.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    Renderer r;
    r.init();

    Game game;
    HudRenderer hud;
    bool running = true;

    Uint64 prev = SDL_GetPerformanceCounter();
    while (running) {
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)((double)(now - prev) / SDL_GetPerformanceFrequency());
        if (dt > 0.1f) dt = 0.1f;
        prev = now;
        game.now_s = (double)now / SDL_GetPerformanceFrequency();

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT: running = false; break;
            case SDL_KEYDOWN:
                if (ev.key.keysym.sym == SDLK_ESCAPE) running = false;
                if (ev.key.keysym.sym == SDLK_w) game.keyW = true;
                if (ev.key.keysym.sym == SDLK_a) game.keyA = true;
                if (ev.key.keysym.sym == SDLK_s) game.keyS = true;
                if (ev.key.keysym.sym == SDLK_d) game.keyD = true;
                if (ev.key.keysym.sym == SDLK_SPACE) game.keySpace = true;
                break;
            case SDL_KEYUP:
                if (ev.key.keysym.sym == SDLK_w) game.keyW = false;
                if (ev.key.keysym.sym == SDLK_a) game.keyA = false;
                if (ev.key.keysym.sym == SDLK_s) game.keyS = false;
                if (ev.key.keysym.sym == SDLK_d) game.keyD = false;
                if (ev.key.keysym.sym == SDLK_SPACE) game.keySpace = false;
                break;
            case SDL_MOUSEMOTION:
                game.pyaw += ev.motion.xrel * 0.0022f;
                game.ppitch -= ev.motion.yrel * 0.0022f;
                if (game.ppitch > 1.45f) game.ppitch = 1.45f;
                if (game.ppitch < -1.45f) game.ppitch = -1.45f;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (ev.button.button == SDL_BUTTON_LEFT) game.shoot();
                break;
            }
        }

        game.update(dt);

        // Build vertex buffers from game state
        auto arena = game.getArenaGeometry();
        auto enemies = game.getEnemyGeometry();
        auto tracers = game.getTracerGeometry();
        auto sparks = game.getSparkGeometry();

        // Lines: arena grid + tracers
        std::vector<Vertex> lines;
        lines.insert(lines.end(), arena.begin(), arena.end());
        lines.insert(lines.end(), tracers.begin(), tracers.end());
        r.updateLines(lines);

        // Triangles: enemies + sparks
        std::vector<Vertex> tris;
        tris.insert(tris.end(), enemies.begin(), enemies.end());
        tris.insert(tris.end(), sparks.begin(), sparks.end());
        r.updateTriangles(tris);

        // HUD
        hud.buildHud(game.hp, game.score, game.wave, Renderer::WIDTH, Renderer::HEIGHT);
        r.updateHud(hud.getVertices());

        // Update uniform buffer
        UniformBufferObject ubo{};
        game.getViewMatrix(ubo.view);
        game.getProjMatrix(ubo.proj, (float)Renderer::WIDTH / (float)Renderer::HEIGHT);
        r.updateUniform(ubo);

        r.drawFrame();
    }

    r.cleanup();
    printf("Exit clean. Final score: %d\n", game.score);
    return 0;
}
