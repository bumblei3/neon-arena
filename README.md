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

## Status

- Prototyp: lauffähig (verifiziert auf X11)
- Mod: baut (QVM + native .so), lädt verifiziert in OpenArena 0.8.8
