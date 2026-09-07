# NeonArena Roadmap

> Letztes Update: 2026-09-07
> Stand: v0.71 Ghost-Kit G1–G11 (Multiscan), 11 Bosse, 90+ Haupt-Mod-Tests
>
> Produkt ist der OpenArena-Mod (`fs_game neonarena`). `prototypes/sdl2-gl3` ist eine Skizze.

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

**G1–G11 ✅.** Nächster Hebel: Playtest-Daten sammeln für Fein-Balance. Keine neuen Verben.

Nicht geplant: extra Rail-Feuerverzögerung (Q3-Rail ist schon 1500 ms). Scanner Sweep ist Comsat, nicht Ghost.

---

## 🎯 Vision

NeonArena als **moderner, spielbarer Wave-Survival-Shooter** mit Fokus auf:
- **Satisfying Gameplay** — Waffen fühlen gut an, Feedback ist präzise
- **Replayability** — Modifier-System, Coop, verschiedene Strategien
- **Technische Sauberkeit** — Tests, CI, keine technische Schulden
- **Community** — Tools, Modding-Support, User-generated Content

---

## ✅ Erledigt (v0.71)

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
| Ghost-Kit G1–G11 | ✅ Fertig | 12 |
| **Gesamt** | | **310+** |

---

## 🗺️ Nächste Milestones

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
> *Ziel: Mehr Variety, frische Herausforderungen*

**Was:**
- 5 neue Arenas (Konzepte/Layouts fehlen noch)
- Neue Bot-Typen (Healer, Shielder, Sniper-Elite — Balance testen)
- Seasonal Challenges / Rotating Modifiers

**Aufwand:** ~5 Tage
**Tests:** +15 (Arena-Validation, Bot-Verhalten)
**Priorität:** Medium
**When:** Q4 2026

---

### M13: Controller-Support 🎮
> *Ziel: Gamepad-Steuerung mit Aim-Assist*

**Was:**
- ✅ CVar-Registrierung (`in_joystick`, `joy_sensitivity`, `joy_assist`, `joy_deadzone`)
- ✅ Standard-Binds in `autoexec.cfg`
- ✅ Aim-Assist-Logik in `g_active.c` (Magnetwirkung für Controller-Spieler)
- ⏳ Controller-Input-Verarbeitung (SDL2-Integration in Quake3e erforderlich)
- ⏳ UI-Navigation mit Gamepad (Quake3e UI-Layer)

**Aufwand:** ~2 Tage (Tag 1: CVars + Binds + Aim-Assist ✅, Tag 2: SDL2-Integration in Quake3e)
**Priorität:** Medium
**When:** Q4 2026

**Hinweis:** Die Analog-Stick-Steuerung und UI-Navigation erfordern SDL2-Integration im Quake3e-Engine. Die QVM-seitige Basis (CVars, Binds, Aim-Assist) ist implementiert.

---

## 📊 Priorisierung

| Milestone | Priority | Impact | Risk | When |
|-----------|----------|--------|------|------|
| **M12 Content-Erweiterung** | 🟡 Medium | Variety | Medium | Q4 2026 |
| **M13 Polish & Performance** | 🟠 Low | Quality | Low | Q4 2026 |
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
| Haupt-Mod Tests | 90+ | 100+ |
| Build Time | ~45s | <60s |
| Bot Types | 8 | 10 |
| Waffen | 5 | 6 |
| Arenas | 8 | 13 |
| Ghost Slices | G1–G11 | G11 + Balance |

---

## 🗓️ Timeline

```
Sep 2026  v0.71 released ✅ (Ghost-Kit G1–G11)
          M12 Content-Erweiterung Start
Okt 2026  M12 Abschluss
          M13 Polish & Performance Start
Nov 2026  M13 Abschluss
Dez 2026  Release v0.80 (Content-Complete)
Jan 2027  M10 Netzwerk-Coop Start
Feb 2027  M10 Abschluss
Mär 2027  Release v0.90 (Multiplayer-Ready)
```

---

## 🚧 Blocked / Warteliste

- **Ghost:** G1–G11 ✅. Nächster Hebel: Playtest-Daten für Fein-Balance. Plan: [docs/GHOST_ROADMAP.md](docs/GHOST_ROADMAP.md).
- **Netzwerk-Coop:** Erst nach lokalem Coop implementieren (M4 erledigt ✅)
- **Modding-Support:** M9 erledigt — Wave-Editor und Map-Validator nutzbar
- **Achievements:** Savegame-Erweiterung erledigt ✅
- **Linux/Mac Builds:** CI hat nur Windows/Linux — fehlt Mac M1+
- **Controller-Support:** Nicht priorisiert, Gamepad-Integration komplex
- **Mobile/Touch:** Nicht priorisiert
