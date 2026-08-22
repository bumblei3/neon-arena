# NEON ARENA

Ein Sci-Fi-Neon-Shooter-Projekt mit zwei Teilen:

1. **`main.cpp`** – eigener Minimal-FPS-Prototyp (C++ / SDL2 / OpenGL): WASD +
   Maus-Look, Hitscan-Schießen, verfolgende Neon-Würfel, Wave-Spawning, HUD.
2. **OpenArena-Mod `neonarena`** – voller OpenArena-Kompatibilitätsweg: der
   gebaute Gamecode läuft als Mod in OpenArena 0.8.8. Änderungen gegen
   [OpenArena/gamecode](https://github.com/OpenArena/gamecode) liegen als
   Patch-Serie in `patches/`.

## 1. SDL2-Prototyp

```sh
make
./neon-arena
```

Steuerung: WASD bewegen · Maus umsehen · Linksklick/Space schießen · ESC Ende.

## 2. OpenArena-Mod bauen

Voraussetzungen: `build-essential`, `bison`, `flex` (+ installiertes OpenArena).

```sh
git clone https://github.com/OpenArena/gamecode.git oa-gamecode
cd oa-gamecode
for p in ../patches/*.patch; do git apply "$p"; done
cp ../Makefile.local .
cd ..
./build-mod.sh
openarena +set fs_game neonarena
```

Der Mod heißt im Spiel `NeonArena-0.1` (statt `baseoa`) und rendert
Railgun-Trails immer in Neon-Cyan.

## Patch-Serie

| Patch | Inhalt |
|---|---|
| `010-gamename.patch` | gamename → "NeonArena-0.1" (`#ifdef NEONARENA_MOD`) |
| `020-cyan-railtrail.patch` | Railtrail fix Cyan statt Spielerfarbe |
| `030-lcc-gnu89.patch` | lcc-Tools mit `-std=gnu89` bauen (GCC ≥ 14: `constexpr`-Clash) |
| `040-neonwave-gametype.patch` | GT_NEONWAVE (Gametype 14) + CS_NEONWAVE Configstring |
| `050-neonwave-hooks.patch` | Wellen-Hooks in g_main.c/g_local.h/g_cmds.c + Makefile-Objekte |

Zusätzlich wird `patches/g_neonwave.c` nach `code/game/` kopiert – die
Wave-Survival-Logik (Gegnerwellen via Bots, Game Over wenn alle Menschen tot,
20 Wellen max). Spielen mit `g_gametype 14`.

## Look-Pack (assets/)

`assets/` wird zu `neon-look.pk3` gepackt (build-mod.sh macht das automatisch):
Shader-Overrides (Neon-Glow, Sky) + `autoexec.cfg` (längere Railtrails,
moderner Rail-Stil). Wird beim Start mit `fs_game=neonarena` geladen.

## CI

`.github/workflows/build-mod.yml` klont bei jedem Push den Upstream-Gamecode,
applied die Patch-Serie und baut den Mod – so bleibt die Serie gegen aktuelles
Upstream verifiziert. Artefakte (QVMs) werden als Artifact hochgeladen.

## Status

- Prototyp: lauffähig (verifiziert auf X11)
- Mod: baut (QVM + native .so), lädt verifiziert in OpenArena 0.8.8
