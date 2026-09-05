# Changelog

All notable changes to NeonArena will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **Wave-Editor (M7)** — Per-wave configuration editor with JSON load/save, interactive commands (`set`, `add`, `remove`, `generate`, `list`, `show`), modifier flag parsing, and clamping. 43 tests.

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

[Unreleased]: https://github.com/bumblei3/neon-arena/compare/v0.54...HEAD
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
