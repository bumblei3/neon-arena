# NEON ARENA

**Wave-Survival-Shooter auf OpenArena-Basis** – Railgun & Lightning Gun gegen
steigende Bot-Wellen, Highscore-Jagd, kompletter Neon-Look.

> Spielen: `openarena +set fs_game neonarena +g_gametype 14 +map oa_shine`

## Features (Mod `neonarena`, Gametype 14 „Neon Wave Survival")

- **Wave-Survival:** Bis zu 20 Wellen; Welle N spawnt N+1 Bots. Ein Leben —
  tot ist tot. 8 s Pause zwischen Wellen.
- **Waffen-Identität:** Spawn mit Railgun + Lightning Gun + Gauntlet. Andere
  Waffen-Pickups werden ignoriert – Rail/LG-Pickups dienen als Ammo-Nachschub.
- **Skill-Kurve:** Bot-Skill steigt mit der Welle (1 → 5).
- **Wellen-Modifier** (ab Welle 5, nicht in Boss-Wellen): **GLASS DRONES**
  (1 Treffer tot, aber aggressiver), **SWARM** (doppelte Drone-Zahl),
  **LOW GRAVITY**, **DOUBLE POINTS**. Wird per Centerprint angesagt.
- **Combo-System:** Kills innerhalb von 3 s ketten sich zu einer Serie.
  Ab Best-Serie 5 gibt es Bonus-Upgrade-Punkte (+1 pro weitere 5er-Stufe).
- **Upgrade-System:** Gesammelte Punkte in der Pause ausgeben —
  F1 = HP (bis 6), F2 = DMG (bis 5, +10 %/Level), F3 = SPD (bis 5).
  HUD zeigt Punkte + Level live.
- **Run-Statistik & End-Screen:** Bei Victory/Game Over Overlay mit
  überlebten Wellen, Kills, bester Combo und Laufzeit.
- **Benannte Drones:** Killfeed zeigt `Drone W3-1` statt `sarge`.
- **Belohnungen:** In der Wellen-Pause fallen Mega Health + Heavy Armor +
  Rail/LG-Ammo an deiner Position.
- **Highscore:** Beste Welle persistent (`g_neonwave_best`). Game-Over-Screen
  mit Welle/Best; FIRE startet neu.
- **Wellen-Jingles:** Sound-Signal bei Wellenstart und -clear.
- **Neon-Look:** Dunkle Skybox, Neon-Grid auf oa_shine, Cyan-Rail/LG mit D-Lights,
  Drohnen-Cyan-Shell (Boss magenta), Rail-Impact-Burst, LG-Sparks, Muzzle-Flare,
  pulsierende Boss-Gefahr-Vignette, Energy statt Blut, Vignette-HUD.

## Test-Hooks (Headless/CI)

- `g_neonwave_autostart 1` — Wellen starten ohne menschlichen Spieler
- `g_neonwave_startwave N` — erzwingt Start bei Welle N (fire-once)
- `g_neonwave_autokill 1` — tötet alle Drones jeden Frame (Auto-Durchlauf)
- `g_neonwave_fastbreak 1` — 500 ms statt 8 s Wellenpause

Der CI-Workflow spielt damit eine komplette 20-Wellen-Partie headless durch
und prüft Victory, Highscore und Upgrade-Punkte-Ökonomie.

## Schnellstart (Spieler)

1. PK3 aus den [Releases](https://github.com/bumblei3/neon-arena/releases)
   herunterladen und nach `~/.openarena/` entpacken (es entsteht `neonarena/`).
2. `openarena +set fs_game neonarena +g_gametype 14 +map oa_shine`

OpenArena 0.8.8 (oder kompatibel) muss installiert sein.

Nach dem ersten Start einmal `vid_restart` in der Konsole (`~`), damit `r_mapoverbrightbits 1` die Map dunkler zieht und Neon besser knallt. Zurück zur hellen Map: `seta r_mapoverbrightbits 2` + `vid_restart`.

## Optional: Quake3e-Engine (Bloom + Vulkan)

NeonArena läuft auch auf [Quake3e](https://github.com/ec-/quake3e) — einer modernen
Q3-Engine mit OpenGL2- und Vulkan-Renderer inkl. **Bloom**. Damit glühen alle
Neon-Elemente (Rails, Drohnen-Shells, HUD) richtig.

1. Engine-Binary aus dem CI-Artifact `neonarena-engine`
   (oder selbst bauen: `git clone https://github.com/ec-/quake3e && make`)
   nach `~/quake3e-engine/` legen.
2. Die **OpenArena-Basisdateien** (`baseoa/*.pk3`) nach `~/quake3e-engine/baseq3/`
   kopieren — der Engine-Patch `patches/engine-quake3e-oa.patch` überspringt die
   Q3-CDROM-Checksummenprüfung, damit OA-Content akzeptiert wird.
3. Starten:
   ```sh
   ~/quake3e-engine/quake3e.x64 +set cl_renderer vulkan +set r_bloom 1 \
     +set fs_basepath ~/quake3e-engine +set fs_homepath ~/.openarena \
     +set fs_game neonarena +g_gametype 14 +map oa_shine
   ```
   (`cl_renderer opengl` statt `vulkan` für den GL2-Renderer mit Bloom.)

Der klassische OpenArena-Client bleibt voll unterstützt — Quake3e ist rein optional.

## Bauen (Entwickler)

Voraussetzungen: `build-essential`, `bison`, `flex`, `zip` (+ installiertes
OpenArena zum Testen).

```sh
git clone https://github.com/OpenArena/gamecode.git oa-gamecode
cd oa-gamecode
for p in ../patches/0*.patch ../patches/1*.patch ../patches/2*.patch; do
  git apply --recount --whitespace=fix "$p"
done
cp ../patches/g_neonwave.c code/game/
cp ../Makefile.local .
make          # QVMs + native Module
cd ..
./build-mod.sh  # installiert nach ~/.openarena/neonarena/ + packt Look-PK3
```

(Hinweis: `engine-quake3e-oa.patch` ist kein Mod-Patch — er wird nur beim
Quake3e-Engine-Build angewendet, siehe Workflow `engine-quake3e.yml`.)

### Gamecode-Submodule (seit v0.13)

Der Mod-Gamecode lebt im eigenen Repo **bumblei3/oa-gamecode** und ist als
Git-Submodule unter `oa-gamecode/` eingebunden. Der Submodule-Commit PINNT
die exakte Gamecode-Version — kein Patch-Drift mehr.

```sh
git clone --recurse-submodules https://github.com/bumblei3/neon-arena.git
cd neon-arena && ./build-mod.sh   # baut + installiert nach ~/.openarena/neonarena
```

Gamecode-Änderungen werden **im Submodule** committed und gepusht
(`cd oa-gamecode && git push`), danach der neue Stand im Parent-Repo
festgehalten (`git add oa-gamecode && git commit`).

Historie: Bis v0.12 wurde der Gamecode per Patch-Serie auf den
OpenArena-Upstream angewandt. Diese Serie (inkorrekter Hunk-Zähler,
kumulativer 280er-Patch) ist seit dem Submodule-Ansatz obsolet und liegt
archiviert im Branch `archive/patches`.

### v0.5-Highlights

- **Boss-Railgun:** Der BOSS-W10+-Drone spawnt mit Railgun (999 Ammo) statt MG.
- **Boss-Healthbar:** Magenta Balken im HUD, live über den Configstring.
- **Upgrades:** Nur in der Wellen-Pause. **F1** +HP, **F2** +DMG, **F3** +SPEED
  (oder `upgrade hp|dmg|speed`). 1 Punkt pro Clear, 2 auf Boss-Wellen.
  +25 MaxHP (max 6), +10% Damage (max 5), +5% Speed (max 5).

### v0.8-Highlights (Replay-Wert)

- **Wellen-Modifier:** ab Welle 5 rotieren GLASS DRONES / SWARM / LOW GRAVITY /
  DOUBLE POINTS durch die Normalwellen (Boss-Wellen ausgenommen).
- **Combos:** Kill-Serien innerhalb von 3 s; ab Best-Serie 5 Bonus-Upgrade-Punkte.
- **End-Screen:** Waves / Kills / Best Combo / Zeit bei Victory und Game Over.

### v0.9-Highlights (Modi & Bosse)

- **Endless Mode:** `g_neonwave_maxwave 0` (Standard) = klassische 20 Wellen;
  höherer Wert = mehr Wellen mit weiter wachsender Bot-Zahl. Victory bei der
  gesetzten Zielwelle.
- **Time Attack:** Bei jedem Sieg wird die Laufzeit gegen `g_neonwave_besttime`
  getauscht — `NEW BEST TIME` im Log/HUD.
- **Boss-Vielfalt:** SNIPER (Rail, 4× HP), TANK (MG-Spam, 6× HP, 40 % langsamer),
  SWARM MOTHER (Rail + spawnt alle 10 s Mini-Drones) — rotieren pro Boss-Welle,
  Test-Hook `g_neonwave_bosstype 1..3`.
- **Boss-Kill-Bonus:** +3 Upgrade-Punkte pro bezwungenem Boss.

### v0.13-Highlights

- **GLASS CANNON (Boss #4):** schnell (140 % Speed), nur 2× HP, Railgun —
  fragile Gefahr. Rotiert ab der 4. Boss-Welle in den Zyklus;
  Test-Hook `g_neonwave_bosstype 4`.
- **Mega-Combo-Reward:** Best-Serie ≥ 8 droppt einen Quad-Damage-Pickup
  beim Wellen-Abschluss.
- **Combo-Bonus-Fix:** Run-Best-Serie überlebt Bot-Disconnects (globaler
  Tracker) — der ≥5-Bonus greift jetzt zuverlässig auch headless.
- **Gamecode als Submodule** (`bumblei3/oa-gamecode`): gepinnte Versionen,
  CI baut direkt aus dem getaggten Stand; Patch-Serie archiviert.
- **Suite 15/15 grün** unter ioq3ded.

### v0.12-Highlights (Juice)

- **Combo-Sounds:** Pentatonik-Synth-Blips (C5→E6) pro Combo-Stufe 1–8 —
  jede höhere Stufe klingt höher, nur beim Aufsteigen der Serie.
- **Boss-Hit-Shake:** Screenshake (180 ms, decaying) bei Boss-Treffern,
  serverseitig rate-limitet (400 ms).
- **NEW-RECORD-Fanfare:** Arpeggio-Jingle einmalig pro Lauf, wenn neue
  Bestwerte (Welle/Zeit) erreicht werden.
- **Test-Suite 13/13 grün unter ioq3ded** (dedicated-Headless): FFA-Fix
  für GT_NEONWAVE, entkoppelter failrun-Hook, Mini-Drone-Cap angehoben,
  Cvar-Reset deckt `q3config_server.cfg` ab.

## CI

`.github/workflows/build-mod.yml` baut bei jedem Push den Mod gegen aktuellen
Upstream-Gamecode und führt drei Headless-Tests aus:

1. **Smoke:** Mod lädt mit Gametype 14 (`NeonArena-0.1`, `g_gametype\14`).
2. **Boss-Wave:** Force-Start Welle 10 → Boss-Spawn mit 4× HP verifiziert.
3. **Full Run:** Kompletter 20-Wellen-Durchlauf (autokill+fastbreak) →
   Victory, Highscore `NEW BEST wave 20`, ≥19 Upgrade-Punkte vergeben.

Zusätzlich baut `.github/workflows/engine-quake3e.yml` die Quake3e-Engine
(OpenGL2+Vulkan, Bloom) als optionales Binary-Artifact.
Tag-Pushes erzeugen Releases.

## SDL2-Prototyp (`main.cpp`)

Ein eigener Minimal-FPS (C++/SDL2/OpenGL, eingefroren – der OA-Mod ist der
aktive Entwicklungspfad):

```sh
make && ./neon-arena
```

WASD bewegen · Maus umsehen · Linksklick/Space schießen · ESC Ende.
