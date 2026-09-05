# Contributing to NeonArena

NeonArena ist ein Quake3/OpenArena-Gamecode-Mod (Gametype 14 "Neon Wave Survival").
Der Mod-Gamecode lebt als **Git-Submodul** unter `oa-gamecode/` — Änderungen daran
folgen einem anderen Workflow als Änderungen am Top-Level-Repo (PK3-Build, Scripts,
CI, Dokumentation).

## Projektstruktur (kurz)

```
neon-arena/
  README.md              # Übersicht, Startanleitung, Features
  docs/                  # BOSS/MODIFIER/PERK/GHOST_REFERENCE, ARCHITECTURE
  build-mod.sh           # Mod-Build + Installation nach ~/.openarena/neonarena/
  Makefile               # Top-Level (nur SDL2-Prototyp main.cpp)
  Makefile.local         # Lokale Overrides für oa-gamecode/ (BASEGAME=neonarena)
  oa-gamecode/           # Git-Submodul → bumblei3/oa-gamecode
    code/game/g_neonwave.c   # NeonWave-Logik (GT_NEONWAVE)
    code/game/g_ghost.c      # Ghost-Kit (g_neonwave_ghost)
    code/cgame/             # Client-Seite (HUD, Codex, Perks, Ghost-Leiste)
    Makefile.local          # Wird aus Top-Level kopiert
  assets/                # Look-Pack (shader, textures, gfx, sound, autoexec)
  tests/                 # Headless-Suite (run_suite.sh, TESTS.md, verify_catalog.py)
  references/            # Test-Harness, Build/Release, Parallel-Suite, Runstats-JSON, Musik
  scripts/               # start-quake3e.sh (Engine-Start mit Bloom, --ghost)
  dist/                  # CI-PK3-Artifacts (master-Releases)
  .github/workflows/     # build-mod.yml, engine-quake3e.yml
```

## Grundregeln

- Der modifizierte Gamecode wird **nicht per Patch** auf OpenArena-Upstream aufgebracht
  (seit v0.13). Das Gamecode lebt im Submodul `bumblei3/oa-gamecode`.
- Der Submodul-Commit **pinnt die exakte Gamecode-Version** — Patch-Drift gibt es
  nicht mehr. Alte Patch-Serie (bis v0.12) archiviert im Branch `archive/patches`.
- CI baut aus dem **getaggten Submodul-Stand** (Checkout mit `--recurse-submodules`).
  Ein Top-Level-Commit ohne Submodul-Update ändert nicht die getestete Version.

## Erste Schritte (lokales Setup)

### Voraussetzungen

- OpenArena 0.8.8 (oder kompatibel) installiert — zum Testen mit dem fertigen Mod.
- Build-Tools: `build-essential`, `bison`, `flex`, `zip` (für das Gamecode-Modul).
- Optional: Quake3e-Engine ( für Bloom ) — wird nur für den optischen Look benötigt,
  der Mod läuft auch auf der klassischen OpenArena-Engine.
- Für die Headless-Testsuite: `xvfb` (und `ioq3ded` oder `openarena` als Binär).

### Submodul-Klonen

```sh
git clone --recurse-submodules https://github.com/bumblei3/neon-arena.git
cd neon-arena
```

Ohne `--recurse-submodules` später nachholen:

```sh
git submodule update --init --recursive
```

### Gamecode-Modul bauen

```sh
cd oa-gamecode
for p in ../patches/0*.patch ../patches/1*.patch ../patches/2*.patch; do
  git apply --recount --whitespace=fix "$p"
done
cp ../Makefile.local .
make
cd ..
./build-mod.sh   # installiert nach ~/.openarena/neonarena/ + packt Look-PK3
```

`build-mod.sh` packt die QVM-Dateien (`cgame.qvm`, `qagame.qvm`, `ui.qvm`) und
das Look-PK3 (`neon-look.pk3`) in `~/.openarena/neonarena/`. Testen mit:

```sh
openarena +set fs_game neonarena +g_gametype 14 +map oa_shine
```

Nach dem ersten Start einmal `vid_restart` in der Konsole (`~`), damit
`r_mapoverbrightbits 1` die Map dunkler zieht und Neon besser knallt.

### Top-Level Makefile (nur SDL2-Prototyp)

```sh
make && ./neon-arena
```

Diese Datei gehört zum **eingefrorenen SDL2/OpenGL-Prototyp** und ist **nicht**
der Hauptentwicklungspfad. Der aktive Pfad ist der OA-Mod (`oa-gamecode/`).

## Gamecode-Änderungen (Submodul-Workflow)

Jede Änderung am Gameplay, an Kommandos, an Modifier-Logik, an Bossen, an der
Break-Ökonomie, an Records/Achievements oder an Test-Hooks gehört ins
**Submodul** `oa-gamecode/`.

### 1. Submodul-Repo checken

```sh
cd oa-gamecode
# falls nötig: Branch erstellen / aktualisieren
git checkout -b feat/mein-feature
```

### 2. Änderung vornehmen und committen

```sh
# Änderung...
git add code/game/g_neonwave.c  # oder andere betroffene Dateien
git commit -m "feat: mein Feature (short log)"
```

**Wichtig**: GAMEVERSION in `code/game/g_local.h` muss bei funktionalen Releases
angepasst werden. Der CI-Gate `Verify GAMEVERSION matches tag` prüft bei Tag-Pushes,
dass Tag und `GAMEVERSION` identisch sind.

### 3. Ins Submodul-Repo pushen

```sh
git push origin feat/mein-feature
```

Der Push landet im `bumblei3/oa-gamecode`-Repo, **nicht** im `bumblei3/neon-arena`-Repo.

### 4. Parent-Repo (neon-arena) auf den neuen Submodul-Stand setzen

```sh
cd ../                        # zurück nach neon-arena/
git add oa-gamecode           # Submodul als neuer Commit-Referenz aufnehmen
git commit -m "chore: pin oa-gamecode to <commit> for <feature>"
git push origin main
```

Der Parent-Commit merkt sich den **exakten Commit** des Submoduls. CI baut und testet
genau diesen Stand.

### Patch-Serie-Archiv

Die alten Patch-Dateien (`patches/0*.patch`, `1*.patch`, `2*.patch`) und
`patches/g_neonwave.c` sind **veraltet** seit dem Submodul-Ansatz (v0.13).
Sie sind aus historischer Dokumentation noch im Repo, aber **nicht anzuwenden** bei
einem frischen Submodul-Checkout. Bei Fragen: Issue oder PR mit Hinweis auf den
Submodul-Ansatz.

## Dokumentation

- Neue Features mit User-Facing-Änderung gehören in die README-Abschnitte
  "vX.X-Highlights".
- Neue Testfälle: `tests/TESTS.md` aktualisieren **und** `tests/run_suite.sh`
  (`ALL_TESTS`, `QUICK_TESTS`, `dispatch_test()`).
- Neuen Modifier / neuen Boss / neuen Perk: sowohl in `g_neonwave.c` als auch in
  `tests/cs_neonwave_parse.sh` (falls Payload-Parser relevant) dokumentieren.

## Testsuite

Die Headless-Suite läuft mit `ioq3ded` (dedicated) und simuliert Wave-Survival-Partien
ohne menschlichen Spieler. Hooks:

- `g_neonwave_autostart 1` — Wellen starten ohne Spieler
- `g_neonwave_startwave N` — erzwingt Start bei Welle N
- `g_neonwave_autokill 1` — tötet alle Drones jeden Frame (Auto-Durchlauf)
- `g_neonwave_fastbreak 1` — 500 ms Pause statt 12 s
- `g_neonwave_fakecombo N` — erzwingt Combo-Registration für Combo-Tests
- `g_neonwave_bosstype N` — erzwingt Boss-Typ (1..5)
- `g_neonwave_modifier N` / `g_neonwave_modifier2 N` — Modifier-Slots erzwingen
- `g_neonwave_perkforce NNN` — erzwingt Perk-Offer (3-stellige Kodierung)
- `g_neonwave_phaseforce 1` — erzwingt Boss-Phase-2-Trigger
- `g_neonwave_rageforce 1` — erzwingt SWARM-MOTHER-Enrage
- `g_neonwave_dashforce N` — erzwingt SNIPER-Dash-Zähler
- `g_neonwave_daily 1` + `g_neonwave_dailyseed N` — Daily Determinismus
- `g_neonwave_hardcore 1` — Hardcore-Modus
- `g_neonwave_codex 1` — Codex-Bestiary rendern (für Codex-Test)
- `g_neonwave_botasplayer 1` — Bots behandeln als Player (für Kills/Combos)
- `g_neonwave_failrun 1` — Run abbrechen (für Failrun-Tests)

### Suite lokal starten

```sh
cd tests
chmod +x run_suite.sh
./run_suite.sh                 # alle 45 Tests (1–44 inkl. 9b)
./run_suite.sh --quick         # Quick-Subset
./run_suite.sh --test 3        # einzelner Test
python3 verify_catalog.py       # Katalog-Konsistenz prüfen
```

Voraussetzung: QVMs in `~/.openarena/neonarena/vm/` (aus `build-mod.sh` oder CI-Artifact).

### Neue Tests hinzufügen

1. `dispatch_test()` in `tests/run_suite.sh` ergänzen (Nummer, Name, Timeout, CVars).
2. `assert_N()`-Funktion schreiben (Grep/Count/Min-Checks auf Logdatei).
3. `ALL_TESTS` und (optional) `QUICK_TESTS` aktualisieren.
4. `tests/TESTS.md` aktualisieren.
5. `python3 tests/verify_catalog.py` ausführen — das Skript prüft:
   - Jede `ALL_TESTS`-Nummer hat eine `dispatch_test()`-Case
   - Jeder `dispatch_test()`-Case ist in `ALL_TESTS` (Warnung bei waisen Cases)
   - Jeder `ALL_TESTS`-Eintrag ist in `TESTS.md` dokumentiert (INFO, Fail bei `--strict`)

`verify_catalog.py` läuft in CI als Pre-Flight-Schritt vor der Testsuite.

## CI

`.github/workflows/build-mod.yml`:
- Push auf `main` → Build + Quick-Suite (4 parallele Chunks).
- Tag `v*` oder `workflow_dispatch` → Build + **volle Suite** (alle Tests).
- `verify_catalog.py` als Pre-Flight vor der Suite.
- Bei Tag-Push: GAMEVERSION-Gate (`NeonArena-<tag>` muss mit `g_local.h` übereinstimmen).
- Release-Automatik: SOFTPROPS/ACTION-GH-RELEASE erstellt GitHub-Release mit PK3s.

`.github/workflows/engine-quake3e.yml`:
-_optionaler_ Quake3e-Engine-Build mit OA-Patch (kein Mod-Patch, nur Engine).
- Artefakt `neonarena-engine` (Binary) — optional, nicht erforderlich für Mod-Betrieb.

### CI-Matrix

| Chunk | Tests |
|-------|-------|
| 0 | 1, 3, 4, 7, 8, 9b |
| 1 | 10, 12, 13, 17, 18 |
| 2 | 19, 20, 21, 22, 23 |
| 3 | 26, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44 |

Test 9b (swarm-rage) ist im Katalog aktiv und wird ab vX.X in Chunk 0 mitgetestet.
Wenn ein Test als "flaky" markiert ist (`FLAKY_TESTS` in `run_suite.sh`), wird er
einmal retryt.

## Release-Workflow

### Versionierung

- `GAMEVERSION` in `oa-gamecode/code/game/g_local.h` ist die Quelle der Wahrheit.
- Format: `"NeonArena-<major>.<minor>.<patch>"`.
- Top-Level-README und Changelog zeigen dieselbe Version.

### Release-Tag setzen

```sh
# 1. GAMEVERSION in oa-gamecode/code/game/g_local.h setzen (z.B. NeonArena-0.38)
cd oa-gamecode
vim code/game/g_local.h
git add code/game/g_local.h
git commit -m "chore: bump GAMEVERSION to 0.38"
git push origin <branch>

# 2. Parent-Repo auf neuen Submodul-Stand
cd ../neon-arena
git add oa-gamecode
git commit -m "chore: pin oa-gamecode to <commit> for v0.38"
git push origin main

# 3. Tag setzen (Trigger für Full-Suite + Release)
git tag v0.38
git push origin v0.38
```

Der Tag-Push triggert:
- Build-Mod-Job mit FULL-Suite (alle 45 Tests).
- GAMEVERSION-Gate: Tag `v0.38` vs `g_local.h` → muss passen.
- GitHub-Release mit `dist/neonarena.pk3` + `dist/neonarena-qvm.pk3`.
- **Nicht** vergessen: Submodul-Repo (bumblei3/oa-gamecode) muss den Commit mit GAMEVERSION-Update bereits enthalten — der Parent-Tag triggert nur den Mod-Build aus dem gepinnten Submodul-Stand.

### Vor dem Tag

- `python3 tests/verify_catalog.py` lokal laufen lassen (Pre-Flight).
- Optional: Full-Suite lokal oder via `workflow_dispatch` ohne Tag-Lauf prüfen.
- Submodul-Commit mit GAMEVERSION-Update bereits gepusht haben.

## Verstöße / Translations

- Neue Sprachen für HUD/Chat: `assets/`-Ordner-Struktur für Lokalisierung prüfen.
- Neue Sounds: `assets/sound/` + `assets/scripts/neon-look.shader` (wenn Sound-Cues).
- Neue Karten: `assets/`-Look-Package für die Karte + Map-Config-Definition (siehe
  Issue/Plan für Map-Pool-Erweiterung).

## Sonstiges

- Der SDL2/OpenGL-Prototyp (`main.cpp`) ist eingefroren und nicht der Entwicklungspfad.
- Engine-Patch `patches/engine-quake3e-oa.patch` ist kein Mod-Patch — nur beim
  Quake3e-Engine-Build relevant.
- Patches in `patches/` (0*.patch, 1*.patch, 2*.patch, g_neonwave.c) sind historisch
  und **nicht anzuwenden** bei aktuellen Submodul-Checkouts.
