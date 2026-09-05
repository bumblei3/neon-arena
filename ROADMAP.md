# NeonArena Roadmap

> Letztes Update: 2026-09-05
> Stand: v0.60 released, 506 Prototyp-Tests, 70 Haupt-Mod-Tests

---

## 🎯 Vision

NeonArena als **moderner, spielbarer Wave-Survival-Shooter** mit Fokus auf:
- **Satisfying Gameplay** — Waffen fühlen gut an, Feedback ist präzise
- **Replayability** — Modifier-System, Coop, verschiedene Strategien
- **Technische Sauberkeit** — Tests, CI, keine technische Schulden
- **Community** — Tools, Modding-Support, User-generated Content

---

## ✅ Erledigt (v0.60)

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
| **Gesamt** | | **506** |

---

## 🗺️ Roadmap v0.70

### M8: Replay-Recorder & Bug-Reporting 🔜
> *Ziel: Spielbarkeit verbessern durch Community-Feedback*

**Was:**
- ✅ Replay-Recorder: Inputs aufzeichnen → Wiedergabe für Bug-Reports
- Automatische Crash-Reports mit Replay-Anhang
- Community-Bug-Tracker Integration

**Status:** Replay-Recorder implementiert, Tests ausstehend
**Aufwand:** ~2 Tage
**Tests:** +10 (Roundtrip, Export, Import)

---

### M9: Modding-Support 🛠️
> *Ziel: Community kann eigene Inhalte erstellen*

**Was:**
- API-Dokumentation für Module
- Wave-Editor als externes Tool (CLI)
- Map-Validator als externes Tool
- Workshop-Support für User-generated Content

**Aufwand:** ~4 Tage
**Tests:** +15 (API-Stabilität, Tool-Integration)

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

---

### M11: Neue Waffen & Bot-Typen 🎮
> *Ziel: Mehr Content, mehr Variety*

**Was:**
- **Neue Waffen:** Rocket Launcher, Beam Laser, EMP
- **Neue Bot-Typen:** Healer, Shielder, Sniper-Elite
- **Neue Arenas:** 5 neue Maps mit unterschiedlichen Themen

**Aufwand:** ~5 Tage
**Tests:** +25 (Waffen-Balance, Bot-Verhalten)

---

## 📊 Priorisierung

| Milestone | Priority | Impact | Risk | When |
|-----------|----------|--------|------|------|
| **M8 Replay-Recorder** | 🔴 Hoch | Bug-Fixing | Low | **Q4 2026** |
| **M9 Modding-Support** | 🟡 Mittel | Community | Medium | Q4 2026 |
| **M10 Netzwerk-Coop** | 🟠 Niedrig | Multiplayer | High | Q1 2027 |
| **M11 Neue Content** | 🟠 Niedrig | Variety | Medium | Q1 2027 |

---

## 🎯 Definition of Done

Jedes Milestone ist fertig wenn:
- [ ] Feature implemented
- [ ] Tests geschrieben (mindestens 10 neue Tests)
- [ ] CI grün (Build + alle Tests)
- `CHANGELOG.md` aktualisiert
- Code-Review durch (Community/PR)

---

## 📈 Metrics

| Metric | Current | v0.70 Target |
|--------|---------|--------------|
| Prototyp Tests | 506 | 550+ |
| Haupt-Mod Tests | 70 | 100+ |
| Build Time | ~45s | <60s |
| Bot Types | 5 | 8 |
| Waffen | 3 | 5 |
| Arenas | 8 | 13 |

---

## 🗓️ Timeline

```
Sep 2026  v0.60 released ✅
          M8 Replay-Recorder Start
Okt 2026  M8 Abschluss
          M9 Modding-Support Start
Nov 2026  M9 Abschluss
Dez 2026  Release v0.70 (Community-Ready)
Jan 2027  M10 Netzwerk-Coop Start
          M11 Neue Waffen & Bot-Typen
Feb 2027  M10 Abschluss
Mär 2027  Release v0.80 (Multiplayer-Ready)
```

---

## 🚧 Blocked / Warteliste

- **Netzwerk-Coop:** Erst nach lokalem Coop implementieren (M4 erledigt ✅)
- **Modding-Support:** Benötigt stabile API (erst nach M7 ✅)
- **Achievements:** Benötigt Savegame-Erweiterung (erledigt ✅)
- **Linux/Mac Builds:** CI hat nur Windows/Linux — fehlt Mac M1+
- **Controller-Support:** Nicht priorisiert, Gamepad-Integration komplex
- **Mobile/Touch:** Nicht priorisiert
