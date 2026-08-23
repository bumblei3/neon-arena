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
- **Benannte Drones:** Killfeed zeigt `Drone W3-1` statt `sarge`.
- **Belohnungen:** In der Wellen-Pause fallen Mega Health + Heavy Armor +
  Rail/LG-Ammo an deiner Position.
- **Highscore:** Beste Welle persistent (`g_neonwave_best`). Game-Over-Screen
  mit Welle/Best; FIRE startet neu.
- **Wellen-Jingles:** Sound-Signal bei Wellenstart und -clear.
- **Neon-Look:** Dunkle Skybox, Neon-Grid auf oa_shine, Cyan-Rail/LG mit D-Lights,
  Drohnen-Cyan-Shell (Boss magenta), Energy statt Blut, Vignette-HUD.

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
for p in ../patches/*.patch; do git apply --recount --whitespace=fix "$p"; done
cp ../patches/g_neonwave.c code/game/
cp ../Makefile.local .
make          # QVMs + native Module
cd ..
./build-mod.sh  # installiert nach ~/.openarena/neonarena/ + packt Look-PK3
```

### Patch-Serie

| Patch | Inhalt |
|---|---|
| `010-gamename.patch` | gamename → „NeonArena-0.1" (`#ifdef NEONARENA_MOD`) |
| `020-cyan-railtrail.patch` | Railtrail fix Cyan statt Spielerfarbe |
| `030-lcc-gnu89.patch` | lcc-Tools mit `-std=gnu89` (GCC ≥ 14: `constexpr`-Clash) |
| `040-neonwave-gametype.patch` | GT_NEONWAVE (Gametype 14) + CS_NEONWAVE |
| `050-neonwave-hooks.patch` | Wellen-Hooks (g_main.c/g_local.h/g_cmds.c) + Makefile |
| `060-neonwave-makefile.patch` | g_neonwave.o in beiden Objekt-Listen |
| `070-neonwave-bot-aggro.patch` | Bots jagen nur Menschen (kein Bot-vs-Bot) |
| `090-neonwave-rewards-skill.patch` | Reward-Drop-Deklaration |
| `100-neonwave-best-cvar.patch` | Highscore-Cvar `g_neonwave_best` |
| `110-neonwave-hud-best-sound.patch` | HUD (WAVE/BEST/CLEARED) + Jingles |
| `120-neonwave-weapon-lock-spawn.patch` | Spawn nur Rail+LG+Gauntlet |
| `130-neonwave-weapon-pickup-filter.patch` | Fremde Waffen-Pickups blockiert |
| `140-170-*.patch` | Boss-Drones (Flag, Struct, Health, Glow, Warmup) |
| `180-neonwave-v05-boss-upgrades.patch` | v0.5: Boss-Railgun, HUD-Bossbar, Upgrade-System |
| `190-neon-look-fx.patch` | Look v2: Drohnen-Glow, Rail/LG-Lights, Neon-HUD |
| `200-neon-wave-feel.patch` | Echte Wellen-Pause, F1/F2/F3-Upgrades, Game-Over, One-Life |

`patches/g_neonwave.c` (Wellen-Logik) wird zusätzlich nach `code/game/` kopiert.

### v0.5-Highlights

- **Boss-Railgun:** Der BOSS-W10+-Drone spawnt mit Railgun (999 Ammo) statt MG.
- **Boss-Healthbar:** Magenta Balken im HUD, live über den Configstring.
- **Upgrades:** Nur in der Wellen-Pause. **F1** +HP, **F2** +DMG, **F3** +SPEED
  (oder `upgrade hp|dmg|speed`). 1 Punkt pro Clear, 2 auf Boss-Wellen.
  +25 MaxHP (max 6), +10% Damage (max 5), +5% Speed (max 5).

## CI

`.github/workflows/build-mod.yml` baut bei jedem Push den Mod gegen aktuellen
Upstream-Gamecode, führt einen **Headless-Smoke-Test** aus (Server mit
Gametype 14 starten, Log auf `NeonArena-0.1` + `g_gametype 14` prüfen) und
packt die `neonarena.pk3` als Artifact. Tag-Pushes erzeugen Releases.

## SDL2-Prototyp (`main.cpp`)

Ein eigener Minimal-FPS (C++/SDL2/OpenGL, eingefroren – der OA-Mod ist der
aktive Entwicklungspfad):

```sh
make && ./neon-arena
```

WASD bewegen · Maus umsehen · Linksklick/Space schießen · ESC Ende.
