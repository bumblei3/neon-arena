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

## Grafik-Engine – Quake3e mit Vulkan und Bloom

NeonArena profitiert stark von einer modernen Renderer-Pipeline: das gesamte Neon-Feeling (Additive-Elemente, cyan/magenta Glow, HUD-Vignette) kommt in voller Wirkung zum Tragen, wenn **Bloom** aktiv ist. Der empfohlene Weg dafür ist die [Quake3e](https://github.com/ec-/quake3e)-Engine – einer modernen Q3-Engine mit OpenGL2- und Vulkan-Renderer inkl. Bloom-Unterstützung.

Die klassische OpenArena-Engine bleibt ausdrücklich voll unterstützt. Quake3e ist die grafisch stärkste Variante, kein Ersatz für den Normal-Pfad.

### Installation

1. Engine-Binary besorgen:
   - **CI-Artifact** aus dem Release oder dem neusten `main`-Lauf (Artifact `neonarena-engine`, `dist/neonarena-engine-linux64.tar.gz`), entpackt nach `~/quake3e-engine/`.
   - **Selbst bauen:** `git clone --depth 1 https://github.com/ec-/quake3e && cd quake3e && make ARCH=x86_64` – das Binary landet in `build/release-linux-x86_64/`.
2. OpenArena-Basisdateien bereitstellen:
   - `baseoa/*.pk3` aus der OpenArena-Installation nach `~/quake3e-engine/baseq3/` kopieren **oder** das Engine-Patch verwenden: `patches/engine-quake3e-oa.patch` setzt das hardcoded `BASEGAME` von `"baseq3"` auf `"baseoa"` (im CI-Build automatisch angewendet), damit Quake3e OA-Content direkt aus `baseoa/` lädt.
3. NeonArena-Mod installiert (siehe Build-Abschnitt): die PK3(s) liegen in `~/.openarena/neonarena/`.

### Start mit Bloom

```sh
~/quake3e-engine/quake3e.x64 \
  +set cl_renderer vulkan \
  +set r_bloom 1 \
  +set fs_basepath ~/quake3e-engine \
  +set fs_homepath ~/.openarena \
  +set fs_game neonarena \
  +g_gametype 14 \
  +map oa_shine
```

- `cl_renderer vulkan` → Vulkan-Renderer (modernster Pfad).
- `cl_renderer opengl` → OpenGL2-Renderer, ebenfalls mit Bloom.
- `r_bloom 1` aktiviert Bloom. Ohne Bloom ist der Effekt deutlich blasser – Neon glüht nur mit Bloom richtig.

### Bloom-Kalibrierung (optional)

Quake3e bietet zusätzliche Bloom-Cvars, die eingeschossen werden können, wenn Neon zu stark / zu schwach überblitzt:

- `r_bloom` – Bloom an/aus (1/0).
- Je nach Quake3e-Version zusätzlich `r_bloomIntensity`, `r_bloomResolution`, `r_bloomQuality` – in der Konsole mit `seta r_bloom*` ersichtlich.

Empfehlung: erst mit `r_bloom 1` spielen, dann nur nachregulieren, wenn konkrete Elemente (z. B. Rail-Impact, Vignette, HUD) über- oder unterbelichtet wirken.

### Kompatibilitäts-Checkliste (Look-Pack + Bloom)

Bloom ist für NeonArena besonders sensitiv, weil das Look-Pack auf additive, leuchtende Elemente setzt. Beim Wechsel auf Quake3e + Bloom sollte visuell geprüft werden:

- **Skybox / Umgebungslicht:** `anoice1`-Skybox dunkel-genug, Neon-Punkte schlagen korrekt an.
- **Boden-Grid (`grid.tga` additive Schicht):** leuchtet, ohne ins Histo zu überlaufen.
- **Railgun / Lightning Gun:** `railCore`, `lightningBoltNew` etc. bleiben additiv und gut sichtbar.
- **Drohnen-Shell + Boss-Shell:** `neonarena/droneShell`, `neonarena/bossShell` (und optional `bossShellPulse`) lesen sich mit Bloom nicht „matschig".
- **HUD:** `neon_vignette`, `neon_bar`, `crosshaira`, Combo/Feedback-Overlays im cgame bleiben lesbar und wirken nicht durch Bloom verwaschen.
- **Energy-Blood / Impact-Puffs:** additive Spurt/Glow-Elemente wirken explosionsartig, nicht verbrannt.

Falls ein Element unter Bloom schlecht wirkt, liegt die Ursache meist in einem der Look-Pack-Blocks (z. B. zu starker additive `rgbGen const`, Kombination mit bereits hellem Basis-Textur) – das lässt sich in `assets/scripts/neon-look.shader` nachjustieren (Blend-Modus, Konstanten, `tcMod`-Skaling).

### CI: Engine-Build

`.github/workflows/engine-quake3e.yml` baut bei jedem Push auf `main` (und optional manuell) die Quake3e-Engine mit dem OpenArena-Kompatibilitäts-Patch und veröffentlicht das Binary als Artifact `neonarena-engine`. Der Pfad wird automatisch mitgepflegt, sobald der Upstream-Quake3e-Build kompatibel bleibt.

Der Patch `patches/engine-quake3e-oa.patch` ist **kein Mod-Patch** – er wird nur beim Engine-Build angewendet (nicht beim Mod-Bau `build-mod.sh`).

### FAQ

- **Ich habe nur OpenArena-0.8.8, kein Quake3e?** Dann läuft NeonArena normal auf der OpenArena-Engine – der Mod ist voll funktionstüchtig, nur ohne Bloom.
- **Bloom bringt nichts bei meinem OpenGL-Renderer?** Quake3e mit `cl_renderer opengl` liefert ebenfalls Bloom (OpenGL2). Der Vulkan-Pfad ist der mit Abstand stärkste, aber OpenGL2+Bloom ist besser als kein Bloom.
- **Kann ich Bloom nur für NeonArena aktivieren?** `r_bloom` ist eine Cvar der Quake3e-Engine, nicht des Mods. Sie gilt global pro Lauf.
- **Mod- und Engine-Updates gleichzeitig?** Mod-Updates (PK3, QVMs) kommen aus dem normalen Mod-Build. Engine-Updates kommen aus dem `engine-quake3e`-CI-Pfad oder eigenem Quake3e-Build. Beide Pfade sind unabhängig.

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

### v0.14-Highlights (Daily Challenge)

- **DAILY CHALLENGE:** `g_neonwave_daily 1` aktiviert den Tagesmodus — ein
  FNV-1a-Hash über das Datum (YYYY-MM-DD) leitet Boss-Rotation und
  Modifier-Reihenfolge ab. Gleicher Tag = gleiche Herausforderung für alle.
  Eigener Records-File (`neonwave_daily_records.dat`), normale Bestwerte
  bleiben unberührt. Test-Hook `g_neonwave_dailyseed N` erzwingt einen Seed
  (Suite-Test 15 verifiziert die Determinismus).
- Ohne `g_neonwave_daily` ändert sich nichts — Standardverhalten unverändert.

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
