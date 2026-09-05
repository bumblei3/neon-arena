# NeonArena SDL2-GL3 Prototype — ROADMAP

> **Last updated:** 2026-09-05
> **Status:** playable sketch only. The product is the OpenArena mod
> (`oa-gamecode`, `g_ghost.c`, `scripts/start-quake3e.sh --ghost`).
> See `docs/GHOST_REFERENCE.md` in the repo root for the live kit.
>
> Prototype Ghost numbers below are **not** the OA source of truth.

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
| Savegame | savegame.cpp/h | ✅ Active | Auto-save on pause/quit, delete on game over |
| Overclock | overclock.cpp/h | ✅ Active | Roguelite upgrades + bug effects |
| Echo | echo.cpp/h | ✅ Active | Ghost replay, boost, stun, chaos |
| BotAI | bot_ai.cpp/h | ✅ Active | State machine, personalities |
| Coop | coop.cpp/h | ✅ Active | Shared screen, player2, revive |
| WaveConfig | wave_config.h | ✅ Active | Data definitions |
| WaveEditor | wave_editor.cpp/h | ⚠️ Header-only | CLI tool, not integrated in-game |
| **Ghost Sniper** | **weapons.cpp/h** | ✅ **Active** | Hitscan 200 dmg, ADS, miss 4s / hit 1.5s / kill 0.8s, marked aim-assist |
| **Ghost Specials** | **specials.cpp/h** | ✅ **Active** | Scanner, EMP stun, **Tac Nuke**, Cloak; energy-guarded |
| **Detector Bots** | **bots.cpp/h** | ✅ **Active** | Type 6 from wave 8 — red cone reveals cloak, swarm call |
| **Stealth Bots** | **bots.cpp/h** | ✅ **Active** | Type 5 — hidden on minimap until scanned, erratic hover |
| **Ghost Kill Cloak** | **game.cpp/h** | ✅ **Active** | Kill grants 2s cloak; cloak hides from AI (last-known pos) |
| Weapons | weapons.cpp/h | ✅ Active | Railgun, Lightning Gun, Plasma, **Ghost Sniper** |
| Bots | bots.cpp/h | ✅ Active | Spawn, update, render + Stealth Bots |
| Score | score.cpp/h | ✅ Active | Combo, multiplier, kill feed |
| PowerUps | powerups.cpp/h | ✅ Active | Health, score, damage boost |
| Specials | specials.cpp/h | ✅ Active | Nuclear, time slow, shield, **Scanner/EMP/Nuke/Cloak** |
| HUD | hud.cpp/h | ✅ Active | Crosshair, minimap, wave announce |
| Menu | menu.cpp/h | ✅ Active | Basic menu system + Q: Quit to menu from pause |
| Balancing | balancing.cpp/h | ⚠️ Standalone | Playtest sim, not live-tuned |
| Achievements | achievements.cpp/h | ✅ Active | 25 achievements, hook into kills/waves/combat |
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
- [x] Weapons (Railgun, Lightning Gun, Plasma, **Ghost Sniper**)
- [x] Collision detection (spatial hash)
- [x] Particle effects (ECS)
- [x] HUD (health, ammo, wave counter)
- [x] Audio (SFX + dynamic music)

### Milestone 2: Juice & Feedback ✅ DONE
- [x] Camera shake on hit
- [x] Kill feed + damage numbers
- [x] Power-ups (health, score, damage)
- [x] Special abilities (nuclear, time slow, shield, **Scanner/EMP/Nuke**)
- [x] Overclock system (roguelite upgrades)
- [x] Score combos + multiplier
- [x] Audio polish (dynamic layers based on combat state)
- [x] Music scene system (menu, gameplay, boss, game over)
- [x] **Achievement popups** with sound feedback

### Milestone 3: Systems Integration 🔄 IN PROGRESS
- [x] Savegame: auto-save on pause/quit, delete on game over
- [x] Achievements: linked to kills/waves/combat, popup display
- [x] **Ghost-Modus**: Ghost Sniper, Cloak, Scanner, EMP, Tac Nuke + Detector bots
- [x] **Stealth Bots**: Invisible enemies (wave 6+), scanner-reveal mechanic
- [x] **Ghost Kill Cloaking**: 2s cloak after Ghost Sniper kill
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

## Ghost-Modus (StarCraft-style) — prototype sketch

This section describes the **SDL prototype** only. OpenArena Ghost is
documented in `../../docs/GHOST_REFERENCE.md` (Cloak 5s lump, EMP self-AoE,
Nuke calldown, Detector wave 8; no Scanner, no ambush 2× yet).

Pick **Start Ghost** in the prototype menu (separate loadout from Arena).

### Weapons
| Weapon | Damage | Cooldown | Notes |
|--------|--------|----------|-------|
| Ghost Sniper | 200 (400 ambush) | miss 4s / hit 1.5s / kill 0.8s | Hitscan, RMB ADS, marked targets have wider hit radius |

### Specials (Ghost kit)
| Special | Cost | Cooldown | Effect |
|---------|------|----------|--------|
| Scanner Sweep (G) | 25 | 15s | Ping radius 25m; stealth marked 5s, others 3s |
| EMP Blast (H) | 35 | 25s | Stun 1.5s + wipe enemy projectiles in 15m |
| Tac Nuke (N/I) | 80 | 45s | 1.5s stand-still paint + 4s inbound; trash dies, boss 400 dmg; bots flee |
| Cloak (J) | 40 | 20s | 5s invisible; bots hunt last-known pos; break on shot / damage / 2m / detector |

Arena kit keeps E/R/F (Nuke / Time Slow / Shield) and Rail/LG/Plasma.

### Stealth Bot (Bot Type 5)
- **Spawn**: Wave 6+, every 5th bot
- **Behavior**: Fast, erratic movement, semi-invisible (15% opacity without scanner)
- **Detection**: Scanner mark. Hidden on Ghost minimap until marked.
- **Health**: 60% of Flanker base, 1.4x speed multiplier

### Detector Bot (Bot Type 6)
- **Spawn**: Wave 8+, one per wave (plus on boss waves)
- **Behavior**: Shooter-like, red 12m detection cone
- **Counter**: Reveals cloak, 4s swarm call on the real player position
- Boss Phase 3+ ignores cloak even without a detector

### Ghost loop
- Start with 40 energy, regen 3/s — nuke needs 80, so farm kills first
- Cloak → reposition → sniper (ambush 2×) → kill grants 2s cloak + energy
- Mid-wave climax: stand still, paint laser, bots scatter, nuke lands
- Paint cancels if you move, take damage, or a detector breaks cloak
- Energy is not spent if the ability is on cooldown

---

## Architecture Notes

- All game logic is in `Game` class — modules are friend-accessed for free functions
- `g_audio` / `g_music` are globals defined in main.cpp, accessed via extern in game.h
- Particle ECS uses SoA layout (8192 fixed capacity, no realloc)
- SpatialHash is used for bot-player and projectile-bot collision broadphase
- Particle system and spatial hash are heap-allocated in Game::init()
- **New**: `AchievementSystem::AchievementProgress` stored in `Game`, saved with savegame
- **New**: `OverclockManager::useCount` tracks Overclock applications for achievements
- **New**: `EchoSystem::triggerCount` tracks Echo activations for achievements

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

Tests covering: audio, bot_ai, echo, game_state, map_validator, music, overclock, particle_ecs, perf_profiler, replay_recorder, savegame, spatial_hash, wave_config, wave_editor, achievements, game, integration, special abilities, **ghost rules**
