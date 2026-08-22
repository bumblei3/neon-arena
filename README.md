# NEON ARENA

**Wave-Survival-Shooter auf OpenArena-Basis** – Railgun & Lightning Gun gegen
steigende Bot-Wellen, Highscore-Jagd, kompletter Neon-Look.

> Spielen: `openarena +set fs_game neonarena +g_gametype 14 +map oa_shine`

## Features (Mod `neonarena`, Gametype 14 „Neon Wave Survival")

- **Wave-Survival:** Bis zu 20 Wellen; Welle N spawnt N+1 Bots. Game Over, wenn
  alle Menschen tot sind.
- **Waffen-Identität:** Spawn mit Railgun + Lightning Gun + Gauntlet. Andere
  Waffen-Pickups werden ignoriert – Rail/LG-Pickups dienen als Ammo-Nachschub.
- **Skill-Kurve:** Bot-Skill steigt mit der Welle (1 → 5).
- **Benannte Drones:** Killfeed zeigt `Drone W3-1` statt `sarge`.
- **Belohnungen:** Nach jeder clearen Welle fallen Mega Health + Heavy Armor +
  Lightning-Amunition an deiner Position.
- **Highscore:** Beste Welle wird persistent gespeichert (`g_neonwave_best`),
  HUD zeigt `WAVE N` / `WAVE N CLEARED` / `BEST N` in Cyan.
- **Wellen-Jingles:** Sound-Signal bei Wellenstart und -clear.
- **Neon-Look:** Railtrails immer Cyan, Shader-Overrides im Look-Pack.

## Schnellstart (Spieler)

1. PK3 aus den [Releases](https://github.com/bumblei3/neon-arena/releases)
   herunterladen und nach `~/.openarena/` entpacken (es entsteht `neonarena/`).
2. `openarena +set fs_game neonarena +g_gametype 14 +map oa_shine`

OpenArena 0.8.8 (oder kompatibel) muss installiert sein.

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

`patches/g_neonwave.c` (Wellen-Logik) wird zusätzlich nach `code/game/` kopiert.

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
