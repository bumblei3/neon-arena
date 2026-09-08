# Ghost-Roadmap

StarCraft-inspiriertes Ghost-Kit in **OpenArena** (`g_neonwave_ghost 1`).
Zahlen und Loop: [GHOST_REFERENCE](GHOST_REFERENCE.md). Produkt ist der OA-Mod, nicht `prototypes/sdl2-gl3`.

> Letztes Update: 2026-09-08 · Kit in **v0.82** · G12 Loadouts

## Stand

Der Kit-Loop ist **feature-complete** als Ghost-Analog, plus drei Loadouts:

Cloak (Toggle + Drain) → RMB-Snipe (Rail, Ambush 2×) → EMP-Round → Lockdown (Boss/Detector) → Tac-Nuke-Calldown (Spectre). Ab Welle 8 jagt ein Detector mit 800 ms Scan-Warnung.

Keine neuen Tasten. G12 splittet den bestehenden Verb-Satz in Infiltrator / Saboteur / Spectre. Nächster Hebel: **Playtest-Daten für Fein-Balance**.

Start: `scripts/start-quake3e.sh --ghost`  
J cloak · H emp · K lockdown · N nuke · RMB zoom · `loadout 0|1|2`.

## Erledigt

| Slice | Inhalt |
|-------|--------|
| v1 | Energy, Cloak-Potion, EMP-Blase, Nuke-Paint, Rail-Spawn, HUD-CVars |
| Calldown + Detector | Sky-Laser, 4-3-2-1, Bots fliehen, BFG-FX, custom Dmg; Detector W8; Swarm; Boss P2 sieht Cloak |
| Drain + EMP + Lock | Cloak Toggle/Drain + Ambush; EMP-Bolt Armor-Strip; Lockdown K; Gauntlet raus |
| HUD + Tells | `STAT_GHOST_*` (Coop); J/H/K/N-Pips; Lock-Cyan; Scan 800 ms |
| Snipe ADS | RMB `+zoom`, Cyan-Fadenkreuz; Rail bleibt 1500 ms |
| Feedback | Cloak-Vignette, OA-Sounds, goldener Ambush-Trail |
| G1 | Start-Energy 40 → **55** |
| G2 | Lockdown als Raketen-Bolt, Miss-Refund |
| G3 | Detector-Kurve: 1 ab W8, 2 ab W12, Skill +1 |
| G4 | Tests 73–75 |
| G5 | Runde Scope-Blende; Cyan-Cloak-Shell |
| G6 | Hit-Confirm im Scope; Tests 73–75 in CI (single) |
| G7 | Ortsfester Detector-Turm ab Welle 8; Lockdown zählt |
| G9 | Eigene Short-Sounds (Cloak/EMP/Lock/Nuke/Scan) |
| G8 | Drain 8 → 5/s nach Welle-1-Play |
| G10 | Cyan-Shell uncloaked (Modell + Waffe); `ghost-binds.cfg` |
| G11 | Multiscan: `multiscan` Command, 30 Energy, 3 s CD, 500 u Radius, Cloak-Reveal + 2 s Damage-Bonus |
| G12 | Loadouts: Infiltrator / Saboteur / Spectre (`loadout`, `g_ghost_loadout`) |

## Lücken vs StarCraft Ghost

| Thema | Jetzt | SC / Wunsch | Slice |
|-------|--------|-------------|-------|
| Treffer im Scope | Cyan/Gold im Iris | Snipe zeigt Treffer | G6 ✅ |
| Detector | Bot + Turm ab W8 (Rail-Ping, 800 ms) | SC Missile Turret analog | G7 ✅ |
| Balance | Drain 5/s; Loadout-Start 80/70/90 | eine Zahl nach einer Runde | G8 ✅ · G12 |
| Sounds | PK3 `ghost_*.wav` | eigene Short-Cues | G9 ✅ |
| Modell | Sarge + Cyan-Shell cloaked | Ghost-Look auch uncloaked | G10 ✅ |
| Tests in CI | Chunk 4: 73–75 single (echte Asserts) | 61–72 weiter nur lokal | G6 ✅ |
| Nuke-Inbound | 4 s | SC ~20 s | nicht — Arena |
| Scanner / Storm | — | Comsat / High Templar | nicht |

### G12 — Loadouts ✅

Drei Kits, keine neuen Tasten. Default Infiltrator (`g_ghost_loadout 0`).
Saboteur: EMP 25/20 s, Lockdown 35 Energy. Spectre: kein Cloak, Nuke + Multiscan, 90 Start.
Zahlen: [GHOST_REFERENCE](GHOST_REFERENCE.md#loadouts-v12).

## Nächste Slices (Reihenfolge)

Playtest, dann **eine** Balance-Zahl. Keine neuen Tasten.

### G6 — Snipe Hit-Confirm im Scope ✅

Cyan-Tick ~150 ms / Gold ~220 ms im runden Iris bei `PERS_HITS` / Kill. Hip-Fire behält den Arena-Marker. `build-mod` Chunk 4 läuft Tests 73–75 im Single-Modus (echte Asserts, nicht Parallel-Smoke).

### G7 — Detector-Jagd (Turm) ✅

Ab Welle 8 ein **ortsfester Scanner** (`ghost_detector_turret`, 120 HP) zusätzlich zum Bot-Detector. Dreht, 400 u / Cone 0.76, 800 ms `SCANNING` + Rail-Ping, dann Cloak-Break + Swarm. Lockdown freeze 4 s (Mechanical). Marker `DETECTOR turret (wave N)`. Bot-Kurve G3 unverändert (1 ab W8, 2 ab W12).

### G8 — Eine Balance-Zahl (nach Play) ✅

Play: Welle 1 tot, 1 Kill, 66 s. **Eine** Zahl: Drain **8 → 5/s**. Start 55 unverändert (Cloak 25 + EMP 35 = 60, nicht beides voll). Nach Cloak-Kosten 30 Energy → ~6 s Tarnung (vorher ~3.7 s).

Weitere Zahlen erst nach der nächsten Runde.

### G9 — Eigene Cues (PK3) ✅

`assets/sound/ghost_{cloak_on,cloak_off,emp,lock,nuke,scan}.wav`. Scan spielt einmal beim Cone-Eintritt (`SCANNING`). Ambush bleibt OA `hit`. Generator: `assets/gen_ghost_sounds.py`.

### G10 — Ghost-Look ✅

Uncloaked: Cyan-Shell (`neonarena/ghostShell`) auf Human-Modell und Waffe. Cloaked bleibt die schwache Cyan-Shell aus G5. Kein voller SC-Schimmer (Q3-Renderer). Binds: `ghost-binds.cfg` (cgame + `--ghost`), weil die Engine `autoexec.cfg` nicht aus dem PK3 exec't.

## Nicht geplant

- Extra Rail-Feuerverzögerung (1500 ms *ist* der Snipe-Takt)
- Scanner Sweep (Comsat / Orbital, nicht Ghost)
- Psionic Storm (High Templar)
- 20 s SC-Nuke-Inbound
- Neue Taste / sechstes Verb
- SDL2-GL3-Prototyp als Produkt
- Momentum-WIP in denselben Commits
- Netz-Coop als Ghost-Slice

## Definition of Done (jede Slice)

- C89, `g_ghost.o` in BASEGAME **und** MISSIONPACK
- `./build-mod.sh` exit 0
- [GHOST_REFERENCE](GHOST_REFERENCE.md) + dieses File + CHANGELOG Unreleased
- Tests 73–75 weiterhin PASS, plus Marker der Slice falls Spawn/Jagd
- Momentum/Legacy-CVars nicht mitcommitten
