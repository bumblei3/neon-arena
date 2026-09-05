# Changelog

All notable changes to NeonArena will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.71] - 2026-09-05

### Added
- **StarCraft Ghost-Kit** (`g_neonwave_ghost 1`, Start: `scripts/start-quake3e.sh --ghost`) — optionaler Loadout für GT_NEONWAVE:
  - Spawn: Railgun (30), Energy 55/100, +3/s (kein Regen während Cloak), +15 pro Kill
  - **Cloak** (J, 25 Energy toggle) — Drain 8/s, kein Regen solange cloaked, J nochmal / 0 Energy = aus; bricht bei Schuss, Schaden, Detector; erster Rail in 2 s = Ambush 2×
  - **EMP** (H, 35 Energy, 25 s CD) — Plasma-Bolt, 400 u Armor-Strip + Stun 1.5 s
  - **Lockdown** (K, 50 Energy, 20 s CD) — Raketen-Bolt; nur Boss/Detector, Miss refundet Energy
  - **Tac Nuke** (N, 80 Energy, 45 s CD) — Calldown: 1.5 s stehen (Cancel bei Bewegung/Schaden), 4 s inbound mit Sky-Laser und Countdown, Bots fliehen, BFG-FX; Trash instakill, Boss 40 % maxHealth, Selbst 40
  - **Detector** ab Welle 8 (120 HP, 400 u / ~40° Cone); ab Welle 12 zwei; Skill +1 — Cloak-Break nach 800 ms Scan + 4 s Swarm; Boss Phase 2 sieht Cloak
  - HUD: Energy + J/H/K/N-CDs über `STAT_GHOST_*` (Coop-tauglich); Status `CLOAKED` / `AMBUSH` / `SCANNING` / `DETECTED` / `DESIGNATING` / `NUKE N`
  - Detector: 800 ms `SCANNING` (roter Rail-Tick) bevor Cloak bricht
  - Lockdown-Tell: cyan Light + Rail-Tick, Centerprint `LOCKED`
  - Binds in `assets/autoexec.cfg`: J/H/K/N, RMB `+zoom` (Ghost-Snipe: Cyan-Fadenkreuz + Vignette, `cg_zoomfov 28`)
  - Tests 73–75: Ghost kit W1 / Detector W8×1 / Detector W12×2
  - Feedback: Cloak-Vignette, eigene Sounds (Cloak/EMP/Lock/Nuke/Zoom), Ambush-Rail gold
  - Siehe [Ghost-Reference](docs/GHOST_REFERENCE.md)

### CVars Added
- `g_neonwave_ghost` — Ghost-Kit an/aus (ARCHIVE, SERVERINFO)
- `g_ghost_energy`, `g_ghost_cloakms`, `g_ghost_empcd`, `g_ghost_nukecd`, `g_ghost_lockcd`, `g_ghost_status` — HUD-Spiegel (ROM)

## [0.70] - 2026-09-05

### Added
- **M8 Replay-Recorder** — Input recording system for bug reports: records move/aim/fire/use/jump events, ring buffer mode, binary save/load, playback with reset. Console command: `nw_replay <start|stop|save|load|play|status>`
- **M9 Modding-Support** — CLI tools for community content creation:
  - `neon-tools wave-edit` — Interactive wave configuration editor (JSON)
  - `neon-tools wave-generate` — Generate default wave config
  - `neon-tools map-validate` — Validate arena maps
  - `neon-tools map-generate` — Generate default valid map
- **M11 Neue Inhalte** — HEALER boss (8th boss type) and SHIELD modifier (16th modifier)
  - **HEALER Boss** — Heals nearby bots, low HP (300 base), no direct attack
  - **SHIELD Modifier** — Temporary invulnerability at wave start (3 seconds)
- **Achievement System erweitert** — 17 neue Achievements (8→25 total), Save/Load-Integration, Punkte-System

### Test Suite
- **72 Haupt-Mod-Tests** (up from 58)
- Neue Tests: HEALER boss (71), SHIELD modifier (72)

### CVars Added
- `g_neonwave_shieldactive` — Shield modifier active flag
- `g_neonwave_shieldtime` — Shield duration in ms

## [0.60] - 2026-09-05

### Added
- **M7 Tools & Editor** — Complete tooling suite for NeonArena:
  - **Wave-Editor** — Per-wave configuration editor with JSON load/save, interactive commands (`set`, `add`, `remove`, `generate`, `list`, `show`), modifier flag parsing, and clamping
  - **Map-Validator** — Arena validation system: checks size, spawn points, player/bot spawns, cover density, waypoints, connectivity; auto-generates default maps
  - **Replay-Recorder** — Input recording system for bug reports: records move/aim/fire/use/jump events, ring buffer mode, binary + JSON export/import, playback with reset
  - **Perf-Profiler** — Frame-time analysis: frame recording with draw call counting, summary (avg/min/max FPS, P99), histogram (20 buckets)
- **Achievement System** — 25 achievements across 5 categories (Combat, Wave, Collection, Skill, Secret), save/load integration, points system
- **M6 Audio-Polish** — Dynamic music layers, reverb, occlusion for spatial audio
- **M5 Balancing** — Playtest-simulation, balance database, spawn-radius fix, bot HP/damage tuning
- **M4 Coop-Modus** — Shared-screen coop, gamepad support, revive system, coop spectator
- **M3 Rendering-Polish** — Multi-pass bloom, hit-flash, ACES tonemapping
- **M1 Bot-AI Overhaul** — State machine AI, pathfinding, swarm intelligence, personality-driven behavior
- **Wave-Fusion System** — Modifier combinations create fusion effects (Eternal Winter, Bullet Hell, etc.)
- **Rival Ghost System** — Compete against rival player ghosts

### Performance
- **Status Dirty Flag** — Skip redundant configstring updates (~90% network overhead reduction)
- **Bot Batch Spawning** — Single console command for multiple bots (20x reduction)
- **Bot Cap** — Late-wave bot count capped at 15
- **Particle ECS** — Struct-of-arrays particle system (8192 max, instanced rendering)

### Changed
- Coop respawn now uses direct origin copy + link
- Swarm Mother mini-spawn cap reduced from 20 to 15

### CVars Added
- `ui_neonwave_achievement` — Achievement notification trigger
- `g_neonwave_updateavail` — Update available flag
- `g_neonwave_motd` — Message of the day

### Test Suite
- **506 tests** across 18 test files (up from 204)
- New tests: Wave-Editor (43), Map-Validator (33), Replay-Recorder (49), Perf-Profiler (23), Achievements (46)

## [0.54] - 2026-09-04

### Added
- **Bot-AI State Machine** — Bots now use personality-driven AI (AGGRESSIVE, DEFENSIVE, FLANKER, SWARM, BOSS) with state machine (IDLE/HUNT/ATTACK/RETREAT/FLANK/STUNNED)
- **Swarm Intelligence** — 3+ bots coordinate attacks and avoid clumping
- **Echo-System** — Ring-buffer replay system: Ghost playback, Player-Boost (2x speed on ghost collision), Bot-Stun (1s stun), Echo-Chaos (self-stun chance with 3+ echoes)
- **Overclock System** — Roguelite upgrade system with buff + negative side effect ("bugs")
- **Achievement System** — Client-side achievement popup display + server notification via CVars
- **Coop Spectator** — Dead humans in coop mode automatically follow living teammates
- **Update Notification** — Server can notify clients of new releases via `g_neonwave_updateavail` CVar
- **MOTD Support** — Configurable message of the day via `g_neonwave_motd` CVar

### Performance
- **Status Dirty Flag** — `NW_SendStatus()` skips redundant configstring updates (~90% reduction in network overhead)
- **Bot Batch Spawning** — `NW_SpawnBotsBatch()` batches multiple `addbot` commands into a single console call (20x reduction for large waves)
- **Bot Cap** — Late-wave bot count capped at 15 to prevent performance degradation
- **Particle ECS** — Struct-of-arrays particle system (8192 max, instanced rendering)

### Changed
- Coop respawn now uses direct origin copy + link after spawn point selection
- Swarm Mother mini-spawn cap reduced from 20 to 15 bots

### CVars Added
- `ui_neonwave_achievement` — Achievement notification trigger
- `g_neonwave_updateavail` — Update available flag (0/1)
- `g_neonwave_motd` — Message of the day text

## [0.53] - 2026-08-XX

### Added
- SDL2-GL3 prototype with OpenGL 3.3+ renderer
- Bloom post-processing and Fresnel-Glow shader
- Lightning Gun with chain lightning
- 4 bot types (Melee, Shooter, Tank, Fast, Boss)
- Upgrade system (Railgun DMG, Lightning Range, Max HP, Speed)
- Procedural synthwave music generator
- Savegame system with binary format
- Spatial hashing for O(1) collision detection
- Full HUD with combo bar, dynamic crosshair, wave announcements

## [0.52] - 2026-08-XX

### Added
- Savegame system — binary save/load with versioning

## [0.51] - 2026-08-XX

### Added
- Procedural music synthesizer — synthwave streaming (4 scenes: Menu/Gameplay/Boss/GameOver)

## [0.50] - 2026-08-XX

### Added
- HUD polish — combo bar, dynamic crosshair, wave announcements

## [0.49] - 2026-08-XX

### Added
- Test suite expansion (67 → 146 tests)

## [0.48] - 2026-08-XX

### Added
- Module-level integration tests (160 → 204)

## [0.47] - 2026-08-XX

### Performance
- Status dirty flag — skip unchanged configstring updates

## [0.46] - 2026-08-XX

### Performance
- Bot batch spawning — single console command for multiple bots

## [0.45] - 2026-08-XX

### Performance
- Frame loop optimization — 100ms cache invalidation
- MIMIC effect conditional application

## [0.44] - 2026-08-XX

### Added
- Installer script (`scripts/install.sh`)
- Launcher creation

## [0.43] - 2026-08-XX

### Added
- Escalating upgrade costs: 1pt (L0-3), 2pt (L4+)

## [0.42] - 2026-08-XX

### Added
- Upgrade caps: HP 8, DMG 7, Speed 7
- Boss HP +20%/wave past W10

## [0.41.1] - 2026-08-XX

### Added
- Modifier system (15 modifiers, 2 slots with synergy)

[Unreleased]: https://github.com/bumblei3/neon-arena/compare/v0.71...HEAD
[0.71]: https://github.com/bumblei3/neon-arena/compare/v0.70...v0.71
[0.70]: https://github.com/bumblei3/neon-arena/compare/v0.60...v0.70
[0.60]: https://github.com/bumblei3/neon-arena/compare/v0.54...v0.60
[0.54]: https://github.com/bumblei3/neon-arena/compare/v0.53...v0.54
[0.53]: https://github.com/bumblei3/neon-arena/compare/v0.52...v0.53
[0.52]: https://github.com/bumblei3/neon-arena/compare/v0.51...v0.52
[0.51]: https://github.com/bumblei3/neon-arena/compare/v0.50...v0.51
[0.50]: https://github.com/bumblei3/neon-arena/compare/v0.49...v0.50
[0.49]: https://github.com/bumblei3/neon-arena/compare/v0.48...v0.49
[0.48]: https://github.com/bumblei3/neon-arena/compare/v0.47...v0.48
[0.47]: https://github.com/bumblei3/neon-arena/compare/v0.46...v0.47
[0.46]: https://github.com/bumblei3/neon-arena/compare/v0.45...v0.46
[0.45]: https://github.com/bumblei3/neon-arena/compare/v0.44...v0.45
[0.44]: https://github.com/bumblei3/neon-arena/compare/v0.43...v0.44
[0.43]: https://github.com/bumblei3/neon-arena/compare/v0.42...v0.43
[0.42]: https://github.com/bumblei3/neon-arena/compare/v0.41.1...v0.42
[0.41.1]: https://github.com/bumblei3/neon-arena/compare/v0.41...v0.41.1
