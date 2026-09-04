# NeonArena Roadmap

> Letztes Update: 2026-09-04
> Stand: Prototyp 204 Tests, Haupt-Mod v0.53

---

## 🎯 Vision

NeonArena als **moderner, spielbarer Wave-Survival-Shooter** mit Fokus auf:
- **Satisfying Gameplay** — Waffen fühlen gut an, Feedback ist präzise
- **Replayability** — Modifier-System, Coop, verschiedene Strategien
- **Technische Sauberkeit** — Tests, CI, keine technischen Schulden

---

## ✅ Erledigt (Prototyp SDL2-GL3)

| Feature | Commit | Tests |
|---------|--------|-------|
| Architektur-Refactoring (7 Module) | `be28182` | 32 |
| Wave-Config + Modifier-System | `aeafee5` | 35 |
| CI-Integration (Build+Test) | `aeafee5` | — |
| Splitter-Mechanik | `6a29dc3` | 7 |
| Camera Shake | `7df15e0` | 5 |
| Audio-Manager (Spatial) | `c527504` | 15 |
| Spatial Hashing (Kollisionen) | `5b2642c` | 12 |
| HUD-Polish (Combo/Crosshair) | `024bb5c` | — |
| Test-Suite Expansion | `6906843` | +79 |
| Savegame-System | `8c6e167` | 24 |
| Musik-Synthesizer | `05cab6d` | 14 |
| Module-Integrationstests | `e0338c2` | +44 |

**Prototyp-Stand:** 204/204 Tests, Build grün, CI aktiv.

---

## 🗺️ Roadmap

### M1: Bot-AI Overhaul 🔜
> *Ziel: Bots fühlen sich weniger "dumm" an, mehr Variety*

**Was:**
- State-Machine basierte Bot-AI (Idle → Hunt → Attack → Retreat)
- Pathfinding mit A* auf Spatial Hash (bereits da!)
- Bot-Typen mit einzigartigem Verhalten:
  - **Melee**: Aggressiv, schnelle Vorstöße, Rückzug bei niedrigem HP
  - **Shooter**: Halten Distanz, präzise Schüsse
  - **Tank**: Langsam, zieht Aufmerksamkeit auf sich, hohes HP
  - **Flanker**: Umgeht Spieler, greift von der Seite an
  - **Boss**: Mehrphasen-Kampf (normal → enraged bei 30%)
- Bots kommunizieren: "Swarm-Intelligenz" — koordinieren Angriffe ab Welle 15

**Aufwand:** ~3 Tage
**Tests:** +20 (AI-Entscheidungen, State-Transitions, Pfad-Berechnung)

---

### M2: Partikel-ECS 🔥
> *Ziel: Performance + visuelle Explosionen ohne Framedrops*

**Was:**
- Entity-Component-System für Partikel (struct-of-arrays)
- GPU-basiert via Instanced Rendering (kein draw call pro Partikel)
- Pool-Allocator statt `new/delete` pro Frame
- Partikel-Typen: Spark, Smoke, Blood, Muzzle Flash, Explosion Shell, Trails
- Batch-Updates: Alle Partikel in einem `updateParticles(dt)` Call

**Warum jetzt:** 
- Prototyp hat ~200 Partikel gleichzeitig — bei Splittern explodiert das
- Ohne Instanced Rendering: Draw-Call-Overhead tötet FPS ab 15+ Bots

**Aufwand:** ~2 Tage
**Tests:** +10 (Pool-Allokation, Batch-Update, Memory-Limits)

---

### M3: Rendering-Polish 🌟
> *Ziel: Neon-Ästhetik, die runterfällt*

**Was:**
- **Bloom verbessern:** Multi-Pass Gaussian (3 Stufen: small/medium/large)
- **Tone-Mapping:** HDR → LDR mit Filmic-ACES statt linearem Clamp
- **Screen-Space Effekte:**
  - Chromatic Aberration an Lebensverlust-Screen-Edges
  - Vignette verstärkt bei Game Over
  - Hit-Flash (weißer Screen-Flash bei Treffer)
- **Bot-Visuals:**
  - Emissive-Color je nach Modifier (Rot=Shield, Grün=Regen, Blau=Speed)
  - Glow-Intensity skaliert mit HP
  - Death-Animation (explodiert in Partikel)

**Aufwand:** ~2 Tage
**Tests:** Visuell — keine Unit-Tests (Shader), aber Screenshot-Regression via CI optional

---

### M4: Coop-Modus 🤝
> *Ziel: Lokales 2-Spieler-Coop*

**Was:**
- Split-Screen oder Shared-Screen (Camera beide Spieler im Frame)
- Player-2 mit Gamepad (WASD+Mouse → Gamepad-Stick+Trigger)
- Geteilte Ressourcen? Upgrade-Points synchronisiert?
  - Variante A: Getrennte Punkte, gemeinsame Wellen
  - Variante B: Gemeinsamer Pool (Teamplay)
- Sync: Beide Spieler müssen Wave abschließen
- Revive-System: Spieler 2 kann Spieler 1 wiederbeleben (5 Sekunden)

**Aufwand:** ~4 Tage
**Tests:** +15 (Input-Handling, Sync, Revive, Disconnect)

---

### M5: Haup-Mod Balancing ⚖️
> *Ziel: ioq3e-Mod spielbar machen — nicht nur Code-Stabilität*

**Was:**
- Balancing-Datenbank als JSON:
  ```json
  {
    "wave_5": { "bot_count": 6, "health_mult": 1.5, "modifier": ["shield"] },
    "wave_10": { "boss": true, "boss_type": "overlord", "minions": 4 }
  }
  ```
- Playtest-Simulation (Headless, 100 Wellen, Bot-KI gegen "Ideal-Spieler")
- Daraus: Upgrade-Kosten-Kurve anpassen, Waffen-DPS-Balance, Boss-HP-Tuning
- Neue Waffen für Haupt-Mod: Railgun, Plasma, Lightning (Prototyp-Waffen portieren)

**Aufwand:** ~5 Tage (inkl. Playtests)
**Tests:** +15 (Balancing-Regression, Simulation)

---

### M6: Audio-Polish 🎵
> *Ziel: Alle Sounds prozedural, kein Asset-Import nötig*

**Was:**
- **Mehr Musik-Szenzen:** "Victory" nach Boss-Kill, "Tension" bei 1-Bot-übrig
- **Dynamische Musik:** Layer ein/aus basierend auf Wave-Gesundheit
  - Wenige Bots → Drums aus
  - Boss → Alle Layer an + Pitch-Shift
- **Sound-Design pro Waffe:**
  - Railgun: Punchy, sub-bass hit
  - Lightning: Arcing, elektskrackeln
  - Plasma: Deep, resonant thud
- **Spatial Audio verbessern:** Reverb für große Arenen, Occlusion für Wände

**Aufwand:** ~2 Tage
**Tests:** +5 (Layer-Übergänge, Crossfade-Dauer)

---

### M7: Tools & Editor 🛠️
> *Ziel: Externe Tooling für Community*

**Was:**
- **Wave-Editor (Terminal-basiert):** JSON-Editor mit Vorschau
- **Map-Validator:** Prüft ob Arena-Größe, Spawn-Points gültig sind
- **Replay-Recorder:** Inputs aufzeichnen → Wiedergabe für Bug-Reports
- **Perf-Profiler:** Frame-Time Heatmap, Draw-Call Counter

**Aufwand:** ~3 Tage
**Tests:** +10 (Replay-Roundtrip, Map-Validierung)

---

## 📊 Priorisierung

| Milestone | Priority | Impact | Risk | When |
|-----------|----------|--------|------|------|
| **M2 Partikel-ECS** | 🔴 Hoch | Performance | Low | **M1.5** (vor M1!) |
| **M1 Bot-AI** | 🔴 Hoch | Gameplay | Medium | Next Sprint |
| **M4 Coop** | 🟡 Mittel | Replayability | Medium | Q4 2026 |
| **M3 Rendering** | 🟡 Mittel | Visuell | Low | Parallel zu M1 |
| **M5 Balancing** | 🟠 Niedrig | Polish | High (Playtest) | Q1 2027 |
| **M6 Audio** | 🟠 Niedrig | Atmosphäre | Low | Q1 2027 |
| **M7 Tools** | 🟠 Niedrig | Community | Low | Q2 2027 |

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

| Metric | Current | M1 Target | M4 Target |
|--------|---------|-----------|-----------|
| Tests | 204 | 230 | 260 |
| Build Time | ~45s | <60s | <90s |
| FPS (30 Bots) | ~60 | >55 | >50 |
| Bot Types | 5 (dumm) | 5 (smart) | 5 (smart) |
| Coop Players | 1 | 1 | 2 |
| Lines of Code | ~5300 | ~6000 | ~7500 |

---

## 🗓️ Timeline

```
Sep 2026  M2 Partikel-ECS (vor AI, wegen Performance)
          M1 Bot-AI Overhaul
Okt 2026  M3 Rendering-Polish (parallel)
          M4 Coop-Modus Start
Nov 2026  M4 Coop Abschluss
Dez 2026  Release v0.60 (Prototyp spielbar)
Jan 2027  M5 Balancing (Haupt-Mod)
          M6 Audio-Polish
Feb 2027  M7 Tools & Editor
Mär 2027  Release v0.70 (Feature-Complete)
```

---

## 🚧 Blocked / Warteliste

- **Netzwerk-Coop:** Erst nach lokalem Coop implementieren
- **Modding-Support:** Benötigt stabile API (erst nach M1-M4)
- **Achievements:** Benötigt Savegame-Erweiterung
- **Linux/Mac Builds:** CI hat nur Windows/Linux — fehlt Mac M1+
- **Controller-Support:** Nicht priorisiert, Gamepad-Integration komplex
