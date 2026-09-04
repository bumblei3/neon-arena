// NEON ARENA - Vulkan + SDL2 Prototype
// Main: bootstrap, game loop, input, audio, map loading
#include "renderer.h"
#include "game.h"
#include "hud.h"
#include "particle_system.h"
#include "audio.h"
#include "map_loader.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    Renderer r;
    AudioSystem audio;
    ParticleSystem particles;

    r.init();
    audio.init();
    particles.init(r.device, r.physicalDevice);

    Game game;
    HudRenderer hud;
    
    // Load map from JSON file (try multiple paths)
    MapData map;
    const char* mapPaths[] = {
        "maps/neon_arena_v2.json",
        "../maps/neon_arena_v2.json",
        "../../maps/neon_arena_v2.json",
        "../../../maps/neon_arena_v2.json",
        "/home/tobber/neon-arena/prototypes/vulkan/maps/neon_arena_v2.json"
    };
    bool mapLoaded = false;
    for (const char* path : mapPaths) {
        printf("Trying map path: %s\n", path);
        if (MapLoader::loadFromJSON(path, map)) {
            mapLoaded = true;
            printf("Map loaded from: %s\n", path);
            break;
        }
    }
    if (!mapLoaded) {
        printf("Failed to load map, using default arena\n");
        MapLoader::generateDefaultArena(map);
    }
    auto mapGeometry = MapLoader::extractGeometry(map);
    
    // Load highscore
    game.loadHighScore();

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
                // Weapon switching
                if (ev.key.keysym.sym == SDLK_1) game.currentWeapon = WeaponType::RAILGUN;
                if (ev.key.keysym.sym == SDLK_2) game.currentWeapon = WeaponType::LIGHTNING;
                if (ev.key.keysym.sym == SDLK_3) game.currentWeapon = WeaponType::PLASMA;
                if (ev.key.keysym.sym == SDLK_q) {
                    if (game.currentWeapon == WeaponType::RAILGUN) {
                        game.currentWeapon = WeaponType::LIGHTNING;
                    } else if (game.currentWeapon == WeaponType::LIGHTNING) {
                        game.currentWeapon = WeaponType::PLASMA;
                    } else {
                        game.currentWeapon = WeaponType::RAILGUN;
                    }
                }
                // Specials
                if (ev.key.keysym.sym == SDLK_e) game.activateNuclearBlast();
                if (ev.key.keysym.sym == SDLK_r) game.activateTimeSlow();
                if (ev.key.keysym.sym == SDLK_f) game.activateShield();
                // Upgrade menu
                if (game.showUpgradeMenu) {
                    if (ev.key.keysym.sym == SDLK_UP) {
                        game.upgradeSelection--;
                        if (game.upgradeSelection < 0) game.upgradeSelection = 3;
                    }
                    if (ev.key.keysym.sym == SDLK_DOWN) {
                        game.upgradeSelection++;
                        if (game.upgradeSelection > 3) game.upgradeSelection = 0;
                    }
                    if (ev.key.keysym.sym == SDLK_RETURN) {
                        game.applyUpgrade(game.upgradeSelection);
                    }
                    if (ev.key.keysym.sym == SDLK_TAB) {
                        game.showUpgradeMenu = false;
                        game.wave++;
                    }
                }
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
                if (ev.button.button == SDL_BUTTON_LEFT) {
                    game.shootRailgun();
                    audio.playShoot();
                }
                if (ev.button.button == SDL_BUTTON_RIGHT) {
                    game.shootLightning();
                    audio.playShoot();
                }
                break;
            }
        }

        game.update(dt);

        // Build vertex buffers from game state
        auto arena = game.getArenaGeometry();
        auto enemies = game.getEnemyGeometry();
        auto tracers = game.getTracerGeometry();
        auto sparks = game.getSparkGeometry();
        auto lightning = game.getLightningGeometry();
        auto powerUps = game.getPowerUpGeometry();
        auto hudGeom = game.getHUDGeometry();
        auto minimap = game.getMinimapGeometry();
        auto killFeed = game.getKillFeedGeometry();
        auto damageNumbers = game.getDamageNumbersGeometry();

        // Lines: arena grid + tracers + lightning + power-ups
        std::vector<Vertex> lines;
        lines.insert(lines.end(), arena.begin(), arena.end());
        lines.insert(lines.end(), tracers.begin(), tracers.end());
        lines.insert(lines.end(), lightning.begin(), lightning.end());
        lines.insert(lines.end(), powerUps.begin(), powerUps.end());
        lines.insert(lines.end(), hudGeom.begin(), hudGeom.end());
        lines.insert(lines.end(), minimap.begin(), minimap.end());
        lines.insert(lines.end(), killFeed.begin(), killFeed.end());
        r.updateLines(lines);

        // Triangles: map + enemies + sparks + damage numbers
        std::vector<Vertex> tris = mapGeometry;
        tris.insert(tris.end(), enemies.begin(), enemies.end());
        tris.insert(tris.end(), sparks.begin(), sparks.end());
        tris.insert(tris.end(), damageNumbers.begin(), damageNumbers.end());
        r.updateTriangles(tris);

        // HUD text
        char hudText[256];
        snprintf(hudText, sizeof(hudText), "WAVE %d  SCORE %d  HIGH %d  HP %d/%d", 
                 game.wave, game.score, game.highScore, game.hp, game.maxHp);
        hud.buildHud(game.hp, game.score, game.wave, Renderer::WIDTH, Renderer::HEIGHT);
        r.updateHud(hud.getVertices());

        // Update particles
        particles.update(dt);
        
        // Emit particles on shoot
        if (game.keySpace) {
            float pos[3] = {game.px, 1.5f, game.pz};
            float color[3] = {0.2f, 1.0f, 1.0f};
            particles.emit(5, pos, color, 0.5f, 5.0f);
        }

        // Update particle buffer in renderer
        r.updateParticles(particles.getVertices(), particles.getActiveCount());

        // Update uniform buffer
        UniformBufferObject ubo{};
        game.getViewMatrix(ubo.view);
        game.getProjMatrix(ubo.proj, (float)Renderer::WIDTH / (float)Renderer::HEIGHT);
        r.updateUniform(ubo);

        r.drawFrame();
    }

    audio.cleanup();
    r.cleanup();
    printf("Exit clean. Final score: %d\n", game.score);
    return 0;
}
