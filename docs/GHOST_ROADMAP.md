# Ghost-Roadmap

StarCraft-inspiriertes Ghost-Kit in **OpenArena** (`g_neonwave_ghost 1`).
Zahlen und Loop: [GHOST_REFERENCE](GHOST_REFERENCE.md). Produkt ist der OA-Mod, nicht `prototypes/sdl2-gl3`.

> Letztes Update: 2026-09-05 · Kit in **v0.71** · G7 Detector-Turm

## Stand

Der Kit-Loop ist **feature-complete** als Ghost-Analog:

Cloak (Toggle + Drain) → RMB-Snipe (Rail, Ambush 2×) → EMP-Round → Lockdown (Boss/Detector) → Tac-Nuke-Calldown. Ab Welle 8 jagt ein Detector mit 800 ms Scan-Warnung.

Keine neuen Verben, bis eine echte Runde sagt, dass eines fehlt. Weiter geht’s mit **Jagd, Lesbarkeit, einer Balance-Zahl**.

Start: `scripts/start-quake3e.sh --ghost`  
J cloak · H emp · K lockdown · N nuke · RMB zoom.

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

## Lücken vs StarCraft Ghost

| Thema | Jetzt | SC / Wunsch | Slice |
|-------|--------|-------------|-------|
| Treffer im Scope | Cyan/Gold im Iris | Snipe zeigt Treffer | G6 ✅ |
| Detector | Bot + Turm ab W8 (Rail-Ping, 800 ms) | SC Missile Turret analog | G7 ✅ |
| Balance | 55 Start, Drain 8/s, ~3.7 s Cloak | eine Zahl nach einer Runde | **G8** |
| Sounds | OA-Stock | eigene Short-Cues | **G9** |
| Modell | Sarge + Cyan-Shell cloaked | Ghost-Look auch uncloaked | **G10** |
| Tests in CI | Chunk 4: 73–75 single (echte Asserts) | 61–72 weiter nur lokal | G6 ✅ |
| Nuke-Inbound | 4 s | SC ~20 s | nicht — Arena |
| Scanner / Storm | — | Comsat / High Templar | nicht |

## Nächste Slices (Reihenfolge)

Eine Slice nach der anderen. Keine neuen Tasten.

### G6 — Snipe Hit-Confirm im Scope ✅

Cyan-Tick ~150 ms / Gold ~220 ms im runden Iris bei `PERS_HITS` / Kill. Hip-Fire behält den Arena-Marker. `build-mod` Chunk 4 läuft Tests 73–75 im Single-Modus (echte Asserts, nicht Parallel-Smoke).

### G7 — Detector-Jagd (Turm) ✅

Ab Welle 8 ein **ortsfester Scanner** (`ghost_detector_turret`, 120 HP) zusätzlich zum Bot-Detector. Dreht, 400 u / Cone 0.76, 800 ms `SCANNING` + Rail-Ping, dann Cloak-Break + Swarm. Lockdown freeze 4 s (Mechanical). Marker `DETECTOR turret (wave N)`. Bot-Kurve G3 unverändert (1 ab W8, 2 ab W12).

### G8 — Eine Balance-Zahl (nach Play)

Erst eine Runde `--ghost` mindestens bis Welle 8, dann **eine** Änderung:

| Wenn sich so anfühlt | Änderung |
|---|---|
| Cloak in Welle 1 unbrauchbar | Drain 8 → 5/s **oder** Start 55 → 70 |
| Detector unfair | Scan-Warn 800 → 1200 ms |
| Lockdown zu kurz am Boss | 4 s → 6 s |
| Rail zu spammy | 30 Slugs → 20 |
| Nuke zu oft | Cost 80 → 90 oder CD 45 → 60 s |

Keine Zahlen stapeln. Ohne Runde bleibt G8 liegen.

### G9 — Eigene Cues (PK3)

Kurze eigene Sounds statt Stock: Cloak an/aus, EMP-Abschuss, Lock-Treffer, Nuke-Paint, Scan-Warn. Liegen in `assets/sound/`, nicht SDL.

### G10 — Ghost-Look

Uncloaked: Cyan-Shell oder eigenes Player-Model, damit es kein Sarge mit Rail ist. Cloaked bleibt die schwache Cyan-Shell aus G5. Kein voller SC-Schimmer (Q3-Renderer).

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
