# NeonArena: Prototyp → Haupt-Mod Gap-Analyse

## Status
- **Prototyp:** 27 Module (C++), 506 Tests
- **Haupt-Mod:** ioq3e Submodule (C), g_neonwave.c + g_bot.c + g_spawn.c + g_client.c + g_combat.c + cg_draw.c

## Features: Prototyp vs Haupt-Mod

| Feature | Prototyp | Haupt-Mod | Rückportierbar? |
|---------|----------|-----------|-----------------|
| Bot-AI State Machine | ✅ bot_ai.cpp | ✅ g_bot.c (teilweise) | Erweitern |
| Partikel-ECS | ✅ particle_ecs.cpp | ✅ (cg_draw.c) | Nein (Renderer) |
| Rendering-Polish | ✅ renderer.cpp | ✅ (cg_draw.c) | Nein (Engine) |
| Coop-Modus | ✅ coop.cpp | ✅ (g_client.c teilweise) | Erweitern |
| Balancing-DB | ✅ balancing.cpp | ✅ (g_neonwave.c) | Erweitern |
| Audio-Polish | ✅ audio_polish.cpp | ❌ Nein | Ja (CVar-basiert) |
| Spatial Hashing | ✅ spatial_hash.cpp | ✅ (g_neonwave.c) | Nein (Performance) |
| Savegame | ✅ savegame.cpp | ❌ Nein | Optional |
| Music Generator | ✅ music_generator.cpp | ❌ Nein | Optional |
| Wave-Fusion System | ✅ wave_config.h | ✅ (g_neonwave.c teilweise) | Erweitern |
| Wave-Editor | ✅ wave_editor.cpp | ❌ Nein | Ja (Tool) |
| Map-Validator | ✅ map_validator.cpp | ❌ Nein | Ja (Tool) |
| Replay-Recorder | ✅ replay_recorder.cpp | ❌ Nein | Ja (Feature) |
| Perf-Profiler | ✅ perf_profiler.cpp | ❌ Nein | Ja (Feature) |
| Achievement System | ✅ achievements.cpp | ✅ (CVar-basiert, minimal) | **Ja (Priorität)** |
| Overclock System | ✅ overclock.cpp | ✅ (g_neonwave.c) | Erweitern |
| Echo System | ✅ echo.cpp | ✅ (g_neonwave.c) | Erweitern |

## Rückportierungs-Priorisierung

### Priorität 1 (hoch, hoher Impact)
- **Achievement System** — 25 Achievements, Save/Load, CVar-Integration
- **Replay-Recorder** — Input-Aufzeichnung für Bug-Reports

### Priorität 2 (mittel, niedrig Risiko)
- **Audio-Polish** — Dynamic Layers, Reverb, Occlusion (CVar-basiert)
- **Wave-Editor** — Externes Tool (JSON-basiert)

### Priorität 3 (niedrig, hoher Aufwand)
- **Netzwerk-Coop** — Online-Multiplayer
- **Modding-Support** — API-Dokumentation
- **Neue Waffen/Bot-Typen** — Content

## Architektonische Entscheidungen

1. **Sprache:** Haupt-Mod ist C, Prototyp ist C++. Rückportierung muss in C erfolgen.
2. **Speicherung:** Savegame im Haupt-Mod via `trap_FS_Open/Close/Read/Write`.
3. **CVars:** Alle Features über CVars steuerbar (bestehendes Pattern).
4. **Tests:** ioq3e hat Test-Suite (`tests/run_suite.sh`). Neue Tests dort hinzufügen.

## Nächste Schritte

1. [ ] Achievement System zurückportieren (P1)
2. [ ] Replay-Recorder zurückportieren (P1)
3. [ ] Audio-Polish CVar-Integration (P2)
4. [ ] Wave-Editor als Tool (P2)
5. [ ] CI-Matrix aktualisieren für neue Tests
