# NEON ARENA

**Wave-Survival-Shooter auf OpenArena-Basis** – Railgun & Lightning Gun gegen
steigende Bot-Wellen, Highscore-Jagd, kompletter Neon-Look.

> Spielen: `scripts/start-quake3e.sh` · Daily: `--daily` · Ghost: `--ghost`  
> oder: `openarena +set fs_game neonarena +g_gametype 14 +map oa_shine`

## Features (Mod `neonarena`, Gametype 14 „Neon Wave Survival")

- **Wave-Survival:** Bis zu 20 Wellen; Welle N spawnt N+1 Bots. Ein Leben —
  tot ist tot. 12 s Pause zwischen Wellen.
- **Waffen-Identität:** Spawn mit Railgun + Lightning Gun + Gauntlet. Andere
  Waffen-Pickups werden ignoriert – Rail/LG-Pickups dienen als Ammo-Nachschub.
- **Ghost-Kit** (optional, `g_neonwave_ghost 1` / `--ghost`): StarCraft-inspirierter
  Loadout — Rail-Snipe, Cloak-Drain, EMP-Round, Lockdown, Tac-Nuke-Calldown,
  Detector ab Welle 8. Siehe [Ghost-Reference](docs/GHOST_REFERENCE.md).
- **Skill-Kurve:** Bot-Skill steigt mit der Welle (1 → 5).
- **Wellen-Modifier** (ab Welle 5, auch in Boss-Wellen): 14 Modifier mit
  Synergie-/Anti-Synergie-System (ab Welle 8). Siehe [Modifier-Reference](docs/MODIFIER_REFERENCE.md).
- **Combo-System:** Kills innerhalb von 3 s ketten sich zu einer Serie.
  Ab Best-Serie 5 gibt es Bonus-Upgrade-Punkte (+1 pro weitere 5er-Stufe).
- **Upgrade-System:** Gesammelte Punkte in der Pause ausgeben —
  F1 = HP (bis 8), F2 = DMG (bis 7, +10 %/Level), F3 = SPD (bis 7).
  Kosten: 1 Punkt für Level 0-3, 2 Punkte für Level 4+.
  HUD zeigt Punkte + Level live.
- **Perk-System:** 3 Angebote pro Pause, F1/F2/F3 wählen. 6 Perks:
  PIERCE, OVERCHARGE, CHAIN, SKIP, SECOND WIND, MIRROR.
- **Run-Statistik & End-Screen:** Bei Victory/Game Over Overlay mit
  überlebten Wellen, Kills, bester Combo und Laufzeit.
- **Benannte Drones:** Killfeed zeigt `Drone W3-1` statt `sarge`.
- **Belohnungen:** In der Wellen-Pause fallen Mega Health + Heavy Armor +
  Rail/LG-Ammo an deiner Position.
- **Highscore:** Beste Welle persistent (`g_neonwave_best`). Game-Over-Screen
  mit Welle/Best; FIRE startet neu.
- **Daily Challenge:** Gleicher Tag = gleiche Herausforderung (FNV-1a-Hash
  über Datum bestimmt Boss-Rotation und Modifier-Reihenfolge).
- **Wellen-Jingles:** Sound-Signal bei Wellenstart und -clear.
- **Neon-Look:** Dunkle Skybox, Neon-Grid auf oa_shine, Cyan-Rail/LG mit D-Lights,
  Drohnen-Cyan-Shell (Boss magenta), Rail-Impact-Burst, LG-Sparks, Muzzle-Flare,
  pulsierende Boss-Gefahr-Vignette, Energy statt Blut, Vignette-HUD.
- **Coop:** Bis 4 Spieler, Wave-Clear erfordert alle Drones tot + mindestens
  ein Human lebt. Tote Spieler respawnen am Wellenstart oder können als
  Spectator zuschauen.
- **Wave-Select:** Wähle die Startwelle (1, 5, 10, 15, 20) per `waveselect` in der
  Konsole.
- **Achievements:** Erfolge wie FIRST VICTORY, SURVIVOR, SHARPSHOOTER, COMBOMASTER,
  SPEEDRUNNER, HARDCORE werden beim Freischalten im HUD angezeigt.
- **Daily Challenge:** Gleicher Tag = gleiche Herausforderung (FNV-1a-Hash über
  Datum bestimmt Boss-Rotation und Modifier-Reihenfolge).

## Dokumentation

- [Boss-Reference](docs/BOSS_REFERENCE.md) — Alle 7 Boss-Typen mit Phase-2-Verhalten
- [Modifier-Reference](docs/MODIFIER_REFERENCE.md) — Alle 14 Modifier + Synergien
- [Perk-Reference](docs/PERK_REFERENCE.md) — Perk-System mit allen 6 Perks
- [Ghost-Reference](docs/GHOST_REFERENCE.md) — StarCraft Ghost-Kit (Cloak, EMP, Lockdown, Nuke, Detector)
- [Ghost-Roadmap](docs/GHOST_ROADMAP.md) — nächste Slices (Balance, Lock-Round, Detector, Tests)
- [Architektur](docs/ARCHITECTURE.md) — Code-Struktur, Modul-Grenzen, Build-System
- [Test-Suite](tests/TESTS.md) — 56 Tests mit CVar-Hooks
- [Engine-Integration](docs/ENGINE_INTEGRATION.md) — Quake3e, Renderer, Bloom, Installation

## Schnellstart (Spieler)

### Option A: Installer (empfohlen)

```bash
curl -sL https://raw.githubusercontent.com/bumblei3/neon-arena/main/scripts/install.sh | bash
```

Oder manuell:
```bash
git clone https://github.com/bumblei3/neon-arena.git
cd neon-arena
./scripts/install.sh
```

Starte mit: `neonarena`

### Option B: Manuell

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

### Wayland (native, kein X11-Layer)

Auf GNOME/Wayland läuft Quake3e sauberer über SDL2s native Wayland-Backend statt dem
Xwayland-Compat-Layer (vermeidet Flicker/Resolution-Fallback nach Standby). Das
`scripts/start-quake3e.sh` erzwingt es per `--wayland` (oder `NW_WAYLAND=1`):

```sh
scripts/start-quake3e.sh --wayland --daily
# bzw. direkt:
SDL_VIDEODRIVER=wayland ~/quake3e-engine/quake3e.x64 +set cl_renderer vulkan +set r_bloom 1 \
  +set fs_basepath ~/quake3e-engine +set fs_homepath ~/.openarena +set fs_game neonarena \
  +g_gametype 14 +map oa_shine
```

Voraussetzung: die Engine wurde mit SDL2+Wayland-Unterstützung gebaut (CI-Engine-Artifact
enthält `libwayland-dev`, also Wayland-tauglich). Falls Wayland nicht startet, fällt SDL2
automatisch auf X11 zurück.

### Bloom-Kalibrierung (optional)

Quake3e bietet zusätzliche Bloom-CVars, die eingestellt werden können, wenn Neon zu stark / zu schwach überblitzt:

- `r_bloom` – Bloom an/aus (1/0).
- `r_bloom_intensity` – Endgültige Bloom-Blend-Faktor, Standard 0.5. Höher = stärkerer Bloom, niedriger = dezenterer Bloom.
- `r_bloom_threshold` – Farb-Schwellenwert für die Bloom-Extraktion, Standard 0.6. Höher = weniger Bloom, niedriger = stärkerer Bloom.
- `r_bloom_modulate` – Farbmodulation: 0 = aus (Standard), 1 = self-modulate (color × color), 2 = Intensity-modulate (color × luma).
- `r_bloom_threshold_mode` – Extraktionsmodus: 0 = beliebiger Kanal ≥ Schwellenwert, 1 = Durchschnitt ≥ Schwellenwert, 2 = Luminanz ≥ Schwellenwert.

Weitere Grafik-CVars:

- `r_ext_multisample` – MSAA Anti-Aliasing (0/2/4/6/8). Bessert Kanten in geometrischen Details (Grid-Linien, Wände).
- `r_hdr` – HDR-Framebuffer (0/1). 16-bit Farbtiefe, bessere Bloom-Precision, aber schlechtere Performance.

Empfehlung: erst mit `r_bloom 1` spielen, dann nur nachregulieren, wenn konkrete Elemente (z.B. Rail-Impact, Vignette, HUD) über- oder unterbelichtet wirken.

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
git clone --recurse-submodules https://github.com/bumblei3/neon-arena.git
cd neon-arena && ./build-mod.sh   # baut + installiert nach ~/.openarena/neonarena
```

Gamecode-Änderungen werden **im Submodule** committed und gepusht
(`cd oa-gamecode && git push`), danach der neue Stand im Parent-Repo
festgehalten (`git add oa-gamecode && git commit`).

### Build-System

- `build-mod.sh` — baut QVMs + native Module, installiert nach `~/.openarena/neonarena/`
- `Makefile.local` — Build-Konfiguration (NEONARENA_MOD, lcc-Toolchain)
- `oa-gamecode/Makefile` — OpenArena-Makefile mit NeonArena-Erweiterungen

### Code-Struktur

```
oa-gamecode/
├── code/game/
│   ├── g_neonwave.c      # Hauptlogik (Waves, Modifier, Boss, Perks, Records)
│   ├── g_neonwave.h      # Defines (NW_MOD_*, NW_BOSS_*, NW_PERK_*)
│   ├── g_ghost.c         # Ghost-Kit (Cloak, EMP, Nuke-Calldown, Detector)
│   ├── g_cmds.c          # Upgrade-Kommando (upgrade hp|dmg|speed)
│   └── g_main.c          # CVar-Registrierungen
└── code/cgame/
    └── cg_draw.c         # HUD, Modifier-Anzeige, Codex, Ghost-Leiste
```

Siehe [Architektur](docs/ARCHITECTURE.md) für Details.

### Releases

Ein Tag `v*` triggert automatisch:
1. Build der QVMs und PK3-Dateien
2. Vollständige Test-Suite (56 Tests)
3. Erstellung eines GitHub Releases mit `neonarena.pk3` und `neonarena-qvm.pk3`

GAMEVERSION in `code/game/g_local.h` muss mit dem Tag übereinstimmen.

## Lizenz

GNU GPL v2 (kompatibel mit OpenArena).

## Feedback

Feedback willkommen! Probleme, Ideen und Feature-Requests:

- **Issues:** https://github.com/bumblei3/neon-arena/issues
- **Discord:** *(Link folgt)*
- **Email:** *(optional)*

Jede Rückmeldung hilft, NeonArena besser zu machen. Egal ob Bug-Bericht, Balancing-Vorschlag oder Lob — ich höre zu.

---

**Version:** v0.70 | **Letzte Aktualisierung:** 2026-09-05
