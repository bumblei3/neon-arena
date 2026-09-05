# NeonArena SDL2-GL3 Prototype — ROADMAP

> **Last updated:** 2026-09-05
> **Status:** ~60% complete — core loop functional, several modules wired but not fully utilized

---

## Module Status

| Module | File(s) | Status | Notes |
|--------|---------|--------|-------|
| Renderer | renderer.cpp/h | ✅ Active | GL3.3 Core, shaders, bloom, VAO/VBO |
| Game Core | game.cpp/h | ✅ Active | Wave survival, player, collisions |
| Particle ECS | particle_ecs.cpp/h | ✅ Active | SoA, 8192 max, instanced |
| AudioManager | audio_manager.cpp/h | ✅ Active | SFX, spatial audio |
| AudioPolish | audio_polish.cpp/h | ✅ Active | Dynamic layers, reverb, occlusion |
| MusicGenerator | music_generator.cpp/h | ✅ Active | Procedural synthwave, scene-based |
| SpatialHash | spatial_hash.cpp/h | ✅ Active | Collision broadphase |
| Savegame | savegame.cpp/h | ⚠️ Partial | Code complete, save/load not called in game loop |
| Overclock | overclock.cpp/h | ✅ Active | Roguelite upgrades + bug effects |
| Echo | echo.cpp/h | ✅ Active | Ghost replay, boost, stun, chaos |
| BotAI | bot_ai.cpp/h | ✅ Active | State machine, personalities |
| Coop | coop.cpp/h | ✅ Active | Shared screen, player2, revive |
| WaveConfig | wave_config.h | ✅ Active | Data definitions |
| WaveEditor | wave_editor.cpp/h | ⚠️ Header-only | CLI tool, not integrated in-game |
| Weapons | weapons.cpp/h | ✅ Active | Railgun, Lightning, Plasma |
| Bots | bots.cpp/h | ✅ Active | Spawn, update, render |
| Score | score.cpp/h | ✅ Active | Combo, multiplier, kill feed |
| PowerUps | powerups.cpp/h | ✅ Active | Health, score, damage boost |
| Specials | specials.cpp/h | ✅ Active | Nuclear, time slow, shield |
| HUD | hud.cpp/h | ✅ Active | Crosshair, minimap, wave announce |
| Menu | menu.cpp/h | ✅ Active | Basic menu system |
| Balancing | balancing.cpp/h | ⚠️ Standalone | Playtest sim, not live-tuned |
| Achievements | achievements.cpp/h | ⚠️ Not linked | Code exists, not in game.h |
| MapValidator | map_validator.cpp/h | ⚠️ Standalone | CLI tool, not integrated |
| PerfProfiler | perf_profiler.cpp/h | ⚠️ Not linked | Frame analysis, not in game loop |
| ReplayRecorder | replay_recorder.cpp/h | ⚠️ Not linked | Input recording, not in game |
| Texture | texture.cpp/h | ✅ Active | stb_image loading |
| Shader | shader.cpp/h | ✅ Active | GL shader compilation |
| Text | text.cpp/h | ✅ Active | Bitmap font rendering |
| Playtest | playtest.cpp/h | ✅ Active | Balance DB + simulation tests |

---

## Milestones

### Milestone 1: Playable Loop ✅ DONE
- [x] Window + GL context (SDL2 + GLEW)
- [x] Basic renderer (shaders, VBO, VAO)
- [x] Player movement + mouse look
- [x] Wave spawning system
- [x] Bot AI (state machine, pathfinding)
- [x] Weapons (Railgun, Lightning Gun, Plasma)
- [x] Collision detection (spatial hash)
- [x] Particle effects (ECS)
- [x] HUD (health, ammo, wave counter)
- [x] Audio (SFX + dynamic music)

### Milestone 2: Juice & Feedback ✅ DONE
- [x] Camera shake on hit
- [x] Kill feed + damage numbers
- [x] Power-ups (health, score, damage)
- [x] Special abilities (nuclear, time slow, shield)
- [x] Overclock system (roguelite upgrades)
- [x] Score combos + multiplier
- [x] Audio polish (dynamic layers based on combat state)
- [x] Music scene system (menu, gameplay, boss, game over)

### Milestone 3: Systems Integration 🔄 IN PROGRESS
- [ ] Savegame: wire save/load into game loop (pause menu, auto-save)
- [ ] Achievements: link to game events, popup display
- [ ] PerfProfiler: integrate frame timing, in-game overlay toggle
- [ ] ReplayRecorder: in-game recording trigger, playback mode
- [ ] WaveEditor: in-game overlay for live wave editing
- [ ] MapValidator: validate generated arena at startup
- [ ] Balancing: live-tune based on playtest data

### Milestone 4: Input & Controls 📋 PLANNED
- [ ] Gamepad support (SDL2 GameController API)
- [ ] Input remapping (keyboard + mouse)
- [ ] Aim assist for controllers
- [ ] Haptic feedback (rumble on hit/fire)
- [ ] Touch input (future: mobile port consideration)

### Milestone 5: Content & Progression 📋 PLANNED
- [ ] Full weapon unlock system
- [ ] Perk/upgrade persistence across sessions
- [ ] Multiple arena maps (procedural generation)
- [ ] Boss variety (5th-8th boss types from oa-gamecode)
- [ ] Wave modifier system (fusion effects)
- [ ] Rival ghost integration (compete vs recorded runs)
- [ ] Full coop features (revive, shared upgrades, spectator)

### Milestone 6: Polish & Release 📋 PLANNED
- [ ] Proper menu system (settings, credits, level select)
- [ ] Options: graphics, audio, controls, accessibility
- [ ] Localization (DE, EN)
- [ ] Main menu background (animated arena preview)
- [ ] Splash screen + intro
- [ ] Icon + desktop integration
- [ ] Packaging: AppImage, Windows installer

---

## Immediate Next Steps (this week)

1. **Savegame wiring** — Call SavegameManager::save() on pause/exit, ::load() on startup
2. **Achievements** — Include achievements.h, hook into game events (kills, waves, combos)
3. **PerfProfiler** — Add to game loop, toggle with F3, draw overlay

## Architecture Notes

- All game logic is in `Game` class — modules are friend-accessed for free functions
- `g_audio` / `g_music` are globals defined in main.cpp, accessed via extern in game.h
- Particle ECS uses SoA layout (8192 fixed capacity, no realloc)
- SpatialHash is used for bot-player and projectile-bot collision broadphase
- Particle system and spatial hash are heap-allocated in Game::init()

---

## Build

```bash
cd prototypes/sdl2-gl3
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./neon-arena
```

**Dependencies:** SDL2, SDL2_mixer, GLEW, OpenGL 3.3+

## Test Suite

Run: `cd prototypes/sdl2-gl3/tests && ./run_tests.sh`

222 tests covering: audio, bot_ai, echo, game_state, map_validator, music, overclock, particle_ecs, perf_profiler, replay_recorder, savegame, spatial_hash, wave_config, wave_editor, achievements, game, integration
