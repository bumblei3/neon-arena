# NeonArena Roadmap

> Letztes Update: 2026-09-08
> Stand: v0.90 (released) Ghost-Kit G1–G12 (Loadouts), 13 Bosse, 14 Arenas, 110+ Tests
>
> Produkt ist der OpenArena-Mod (`fs_game neonarena`). `prototypes/sdl2-gl3` ist eine Skizze.
> Letztes GitHub-Release: **v0.90**.

---

## Ghost-Kit (StarCraft)

Optionaler Loadout: `scripts/start-quake3e.sh --ghost` / `g_neonwave_ghost 1`.
Referenz: [docs/GHOST_REFERENCE.md](docs/GHOST_REFERENCE.md) · Plan: [docs/GHOST_ROADMAP.md](docs/GHOST_ROADMAP.md).

| Slice | Inhalt | Status |
|-------|--------|--------|
| v1 | Energy, Cloak 5 s, EMP-Stun-Blase, Nuke-Paint, Rail-Spawn, HUD-CVars | ✅ |
| Calldown + Detector | Nuke Sky-Laser, 4-3-2-1, Bots fliehen, BFG-FX, custom Dmg; Detector Welle 8; Swarm; Boss Phase 2 sieht Cloak | ✅ |
| Drain + EMP-Round + Lockdown | Cloak Toggle/Drain + Ambush 2×; EMP als Plasma-Bolt (Armor-Strip); Lockdown vs Boss/Detector; Gauntlet raus | ✅ |
| HUD + Tells | Energy/CDs in `ps.stats` (Coop); J/H/K/N-Pips; Lockdown-Cyan; Detector-Scan 800 ms vor Swarm | ✅ |
| Ghost-Snipe ADS | RMB `+zoom`, Cyan-Fadenkreuz + Vignette im Scope, kein extra Rail-Delay | ✅ |
| Ghost-Feedback | Cloak-Sicht, OA-Sounds pro Verb, goldener Ambush-Trail | ✅ |
| G5 Cosmetic | Runde Scope-Blende; Cyan-Cloak-Shell | ✅ |
| G6 Scope Hit-Confirm | Cyan/Gold im Iris; Tests 73–75 in CI (single) | ✅ |
| G7 Detector-Turm | Ortsfester Scanner, sichtbarer Hunt | ✅ |
| G8 eine Balance-Zahl | Drain 8 → 5/s (Play Welle 1) | ✅ |
| G9 eigene Cues | Short-Sounds statt OA-Stock | ✅ |
| G10 Ghost-Look | Cyan-Shell uncloaked (Waffe + Modell) | ✅ |
| G11 Multiscan | `multiscan` Command, 30 Energy, 3 s CD, 500 u Radius, Cloak-Reveal + 2 s Damage-Bonus | ✅ |
| G12 Loadouts | Infiltrator / Saboteur / Spectre (`loadout 0\|1\|2`) | ✅ |

**G1–G12 ✅.** Nächster Hebel: Playtest-Daten sammeln für Fein-Balance. Keine neuen Tasten.

Nicht geplant: extra Rail-Feuerverzögerung (Q3-Rail ist schon 1500 ms). Scanner Sweep ist Comsat, nicht Ghost.

---

## 🎯 Vision

NeonArena als **moderner, spielbarer Wave-Survival-Shooter** mit Fokus auf:
- **Satisfying Gameplay** — Waffen fühlen gut an, Feedback ist präzise
- **Replayability** — Modifier-System, Coop, verschiedene Strategien
- **Technische Sauberkeit** — Tests, CI, keine technische Schulden
- **Community** — Tools, Modding-Support, User-generated Content

---

## ✅ Erledigt (v0.82)

| Feature | Status | Tests |
|---------|--------|-------|
| M1 Bot-AI Overhaul | ✅ Fertig | 36 |
| M2 Partikel-ECS | ✅ Fertig | 18 |
| M3 Rendering-Polish | ✅ Fertig | — |
| M4 Coop-Modus | ✅ Fertig | 15 |
| M5 Balancing | ✅ Fertig | 15 |
| M6 Audio-Polish | ✅ Fertig | 5 |
| M7 Tools & Editor | ✅ Fertig | 148 |
| Achievement System | ✅ Fertig | 46 |
| M8 Replay-Recorder | ✅ Fertig | 5 |
| M9 Modding-Support | ✅ Fertig | — |
| M11 Neue Waffen & Bot-Typen | ✅ Fertig | 10 |
| M12 Content-Erweiterung | ✅ Fertig (v0.80 + Seasonal v0.82) | 15 |
| M13 Controller-Support | ✅ Fertig (SDL2 + UI-Nav + Rumble) | 1 |
| Ghost-Kit G1–G12 | ✅ Fertig | 12 |
| **Gesamt** | | **320+** |

---

## 🗺️ Nächste Milestones

### M14: v0.90 — Gamepad-Ready Release 🎯
> *Ziel: Stabiles, gamepad-fähiges Release mit Ghost-Balance* — **abgeschlossen** ✅

**Was:**
- ✅ M13 Controller-Support komplett (SDL2 + UI-Nav + Rumble)
- ✅ Gamepad-Navigation im Menü (A=Enter, B=Escape, DPad)
- ✅ Analog-Stick Deadzone/Sensitivity CVars
- ✅ Rumble-Support (`joy_rumble_low/high/duration`)
- ✅ Ghost Playtest-Balance (Saboteur EMP-CD, Spectre Nuke-Cost)
- ✅ Bugfixes (nach Balance-Analyse)
- ✅ Installer + Release-Notes
- ✅ v0.90 GitHub Release mit Engine-Artifacts

**Aufwand:** 3 Tage (Tag 1: Playtest-Balance, Tag 2: Bugfixes, Tag 3: Release)
**Priorität:** Hoch ✅
**When:** Q4 2026 ✅

**Definition of Done:**
- [ ] Controller-Support in CI getestet (Engine-Build mit Patch)
- [ ] Ghost Loadouts balanced (Drain, Cooldowns, Energy per Playtest-Daten)
- [ ] 110+ Tests (Basis 103 + Balance-Regression-Tests)
- [ ] CHANGELOG.md aktualisiert
- [ ] GitHub Release v0.90 mit Engine-Artifacts
- [ ] Installer getestet (Windows/Linux)

---

### M10: Netzwerk-Coop 🌐
> *Ziel: Online-Multiplayer*

**Was:**
- Client-Server Netzwerk-Code
- Tickrate-Interpolation
- Lag Compensation
- Lobby-System

**Aufwand:** ~6 Tage
**Tests:** +20 (Netcode, Sync, Latenz)
**Priorität:** Niedrig (technisch anspruchsvoll, lokales Coop läuft)
**When:** Q1 2027

---

### M12: Content-Erweiterung 🎮
> *Ziel: Mehr Variety, frische Herausforderungen* — **erledigt**

**Was:**
- ✅ 5 neue Arenen (Frostbite, Skybridge, Underhive, Reactor, Overgrowth) — v0.80
- ✅ Neue Bot-Typen (Healer, Shielder, Sniper-Elite) — v0.80
- ✅ Seasonal Rotation + wöchentliche Challenges + lokale Rangliste — v0.82
- ✅ Chronomancer + Void Walker (Bosse 12–13) — v0.82

**Tests:** 91–105 (Arenas, Bots, Seasonal, neue Bosse)
**When:** Q4 2026 ✅

---

### M13: Controller-Support 🎮
> *Ziel: Gamepad-Steuerung mit Aim-Assist*

**Was:**
- ✅ CVar-Registrierung (`in_joystick`, `joy_sensitivity`, `joy_assist`, `joy_deadzone`, `ui_controller_active`)
- ✅ Standard-Binds in `autoexec.cfg`
- ✅ Aim-Assist-Logik in `g_active.c` (Magnetwirkung für Controller-Spieler)
- ✅ `[JOY]` HUD-Indikator im Ghost-Kit
- ✅ Controller-Input-Verarbeitung (SDL2-Integration in Quake3e)
- ✅ UI-Navigation mit Gamepad (Quake3e UI-Layer)
- ✅ Rumble-Support (`joy_rumble_low`, `joy_rumble_high`, `joy_rumble_duration`)

**Aufwand:** ~2 Tage (Tag 1: CVars + Binds + Aim-Assist + HUD ✅, Tag 2: SDL2-Integration in Quake3e ✅)
**Priorität:** Medium
**When:** Q4 2026 ✅ (M13 abgeschlossen)

---

## 📊 Priorisierung

| Milestone | Priority | Impact | Risk | When |
|-----------|----------|--------|------|------|
| **M14 v0.90 Gamepad-Ready** | ✅ Fertig | Release | Low | Q4 2026 ✅ |
| **M13 Engine-Controller (SDL2)** | ✅ Fertig | Gamepad | — | Q4 2026 ✅ |
| **Ghost Playtest-Balance** | 🟡 Medium | Feel | Low | Q4 2026 |
| **M10 Netzwerk-Coop** | 🟠 Niedrig | Multiplayer | High | Q1 2027 |

---

## 🎯 Definition of Done

Jedes Milestone ist fertig wenn:
- [ ] Feature implemented
- [ ] Tests geschrieben (mindestens 10 neue Tests)
- [ ] CI grün (Build + alle Tests)
- [ ] `CHANGELOG.md` aktualisiert
- [ ] Code-Review durch (Community/PR)

---

## 📈 Metrics

| Metric | Current | Next Target |
|--------|---------|-------------|
| Haupt-Mod Tests | 113 | 113 ✅ |
| Build Time | ~45s | <60s |
| Bosse | 13 | 13 |
| Bot-Typen (Trash) | Healer / Shielder / Elite | — |
| Waffen | 5 | 5 |
| Arenas | 14 | 14 |
| Ghost Slices | G1–G12 | Playtest-Balance |

---

## 🗓️ Timeline

```
Sep 2026  v0.71 Ghost-Kit G1–G11 ✅
          v0.80 Content-Complete (14 Arenas, Bot-Typen) ✅
          v0.81 Controller QVM ✅
          v0.82 Ghost Loadouts + Seasonal + Bosse 12–13 ✅
          v0.90 Gamepad-Ready Release ✅
Okt 2026  Playtest-Balance, M13 Engine-SDL2
Nov 2026  M13 Abschluss
Dez 2026  M14 v0.90 Gamepad-Ready Release — Ziel
Jan 2027  M10 Netzwerk-Coop Start
Mär 2027  Release v1.0 (Multiplayer-Ready) — Ziel
```

---

## 🚧 Blocked / Warteliste

- **Ghost:** G1–G12 ✅. Nächster Hebel: Playtest-Daten für Fein-Balance. Plan: [docs/GHOST_ROADMAP.md](docs/GHOST_ROADMAP.md).
- **Netzwerk-Coop:** Erst nach lokalem Coop implementieren (M4 erledigt ✅)
- **Modding-Support:** M9 erledigt — Wave-Editor und Map-Validator nutzbar
- **Achievements:** Savegame-Erweiterung erledigt ✅
- **Linux/Mac Builds:** CI hat Windows/Linux — macOS-Engine-Artifact existiert; M1-Playtest offen
- **Controller-Support:** ✅ M13 abgeschlossen (SDL2 + UI-Nav + Rumble)
- **Mobile/Touch:** Nicht priorisiert
