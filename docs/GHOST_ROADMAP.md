# Ghost-Roadmap

StarCraft-inspiriertes Ghost-Kit in **OpenArena** (`g_neonwave_ghost 1`).
Zahlen und Loop: [GHOST_REFERENCE](GHOST_REFERENCE.md). Produkt ist der OA-Mod, nicht `prototypes/sdl2-gl3`.

> Letztes Update: 2026-09-05

## Stand

Der Kit-Loop ist **feature-complete** als Ghost-Analog:

Cloak (Toggle + Drain) → RMB-Snipe (Rail, Ambush 2×) → EMP-Round → Lockdown (Boss/Detector) → Tac-Nuke-Calldown. Ab Welle 8 jagt ein Detector mit 800 ms Scan-Warnung.

Keine neuen Verben, bis eine echte Runde sagt, dass eines fehlt. Nächster Hebel ist **Balance und Jagd**, nicht Ability-Spam.

## Erledigt

| Slice | Inhalt |
|-------|--------|
| v1 | Energy, Cloak-Potion, EMP-Blase, Nuke-Paint, Rail-Spawn, HUD-CVars |
| Calldown + Detector | Sky-Laser, 4-3-2-1, Bots fliehen, BFG-FX, custom Dmg; Detector W8; Swarm; Boss P2 sieht Cloak |
| Drain + EMP + Lock | Cloak Toggle/Drain + Ambush; EMP-Bolt Armor-Strip; Lockdown K; Gauntlet raus |
| HUD + Tells | `STAT_GHOST_*` (Coop); J/H/K/N-Pips; Lock-Cyan; Scan 800 ms |
| Snipe ADS | RMB `+zoom`, Cyan-Fadenkreuz, Vignette; Rail bleibt 1500 ms |
| Feedback | Cloak-Sicht, OA-Sounds, goldener Ambush-Trail |

## Lücken vs StarCraft Ghost

| Thema | Jetzt | SC / Wunsch | Prio |
|-------|--------|-------------|------|
| Cloak-Länge am Start | 55 Energy, Cloak 25, Drain 8/s → ~3.7 s | G1 ✅; weiter nur nach Playtest | — |
| Lockdown | Raketen-Bolt, Miss-Refund | G2 ✅ | — |
| Detector | einer ab Welle 8 | später mehr Jagd (Turm / zweiter Detector) | 2 |
| Tests | keine OA-Suite für Ghost | Log-Marker + CVar-Hooks wie Boss/Perk | 2 |
| Cloak für andere | `PW_INVIS` = voll unsichtbar | SC-Schimmer | 3 (Q3-Renderer) |
| Sounds | OA-Stock, funktional | eigene Cues | 3 |
| Modell | Sarge + Rail | Ghost-Skin | 3 |
| Nuke-Inbound | 4 s | SC ~20 s | nicht — Arena |

## Nächste Slices (Reihenfolge)

### G1 — Start-Energy 55 ✅

Eine Zahl: `GH_ENERGY_START` 40 → **55**. Drain 8/s und Kosten unverändert.

Welle 1: Cloak (25) + EMP (35) = 60, also nicht beides voll — aber Cloak mit ~3.7 s Drain, oder EMP sofort, oder Cloak und nach einem Kill EMP. Vorher: Cloak und fast leer.

Weitere Zahlen erst nach der nächsten Runde, nicht stapeln.

### G2 — Lockdown als Round ✅

Raketen-Bolt 900 u/s. Energy wird beim Abschuss reserviert, bei Miss refundet. CD nur bei Treffer auf Boss/Detector.

### G3 — Detector-Kurve ✅

Welle 8–11: 1 Detector. Ab Welle 12: 2. Skill = Wellen-Skill + 1 (cap 5). Namen `Detector W<n>-1` / `-2`. Scan-Warn 800 ms unverändert.

### G4 — Headless-Tests ✅

Tests 73–75 in `tests/run_suite.sh`: Ghost an ohne Detector (W1), ein Detector (W8), zwei Detectoren (W12). Marker `GHOST kit active` / `DETECTOR spawned (wave N, C`.

### G5 — Cosmetic (nur wenn G1–G4 sitzen)

- Eigene Short-Sounds statt Stock (PK3).
- Scope: runde Blende statt Balken.
- Ghost-Player-Model / Cyan-Shell solange cloaked (lokal schon Vignette).

## Nicht geplant

- Extra Rail-Feuerverzögerung (1500 ms *ist* der Snipe-Takt)
- Scanner Sweep (Comsat / Orbital, nicht Ghost)
- Psionic Storm (High Templar)
- 20 s SC-Nuke-Inbound
- SDL2-GL3-Prototyp als Produkt
- Momentum-WIP in denselben Commits

## Definition of Done (jede Slice)

- C89, `g_ghost.o` in BASEGAME **und** MISSIONPACK
- `./build-mod.sh` exit 0
- [GHOST_REFERENCE](GHOST_REFERENCE.md) + dieses File + CHANGELOG Unreleased
- Momentum/Legacy-CVars nicht mitcommitten

## Start zum Playtest (G1)

```sh
scripts/start-quake3e.sh --ghost
```

J cloak · H emp · K lockdown · N nuke · RMB zoom.
