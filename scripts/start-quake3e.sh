#!/usr/bin/env bash
set -euo pipefail

# --- defaults (override with args or edit here) ---
ENGINE_DIR="${QUAKE3E_DIR:-$HOME/quake3e-engine}"
ENGINE_BIN="$ENGINE_DIR/quake3e.x64"
BASE_PATH="${FS_BASEPATH:-$ENGINE_DIR}"
HOME_PATH="${FS_HOMEPATH:-$HOME/.openarena}"
GAME="${FS_GAME:-neonarena}"
GAME_TYPE="${GAME_TYPE:-14}"
MAP="${MAP:-oa_shine}"
MAP_FORCED=0
RENDERER="${RENDERER:-auto}"
BLOOM="${BLOOM:-auto}"
# Wayland: run SDL2's native Wayland backend instead of the X11 compat layer.
# Pass --wayland or set NW_WAYLAND=1. Falls back to X11 automatically if unset.
WAYLAND="${NW_WAYLAND:-0}"
DAILY=0
HARDCORE=0
GHOST=0
GFX_RESET=0
EXTRA_CVARS=()

usage() {
  cat <<EOF
Start NeonArena mit Quake3e (Vulkan) + Bloom.

Usage:
  $0 [--daily] [--hardcore] [+set cvar value ...]

Optionen:
  --daily         Daily Challenge (g_neonwave_daily 1); wählt die Tages-Map
  --hardcore      Hardcore-Lauf (g_neonwave_hardcore 1)
  --ghost         StarCraft Ghost kit (g_neonwave_ghost 1): Rail, Cloak, EMP, Lockdown, Nuke
  --wayland       SDL2 native Wayland backend (kein X11-Compat-Layer)
  --gfx-reset     GPU neu probe, gfx-auto.cfg überschreiben
  --map NAME      Map (Default: oa_shine; mit --daily überschreibt die Tages-Map)
  --help          diese Hilfe

Umgebungsvariablen (optional):
  QUAKE3E_DIR     Engine-Verzeichnis     (Default: $HOME/quake3e-engine)
  FS_BASEPATH     fs_basepath            (Default: $ENGINE_DIR)
  FS_HOMEPATH     fs_homepath            (Default: $HOME/.openarena)
  FS_GAME         fs_game                (Default: neonarena)
  GAME_TYPE       g_gametype             (Default: 14)
  MAP             map                    (Default: oa_shine)
  RENDERER        cl_renderer            (Default: auto → gfx-auto.cfg)
  BLOOM           r_bloom               (Default: auto → gfx-auto.cfg)
  NW_GFX_PRESET   low|med|high           (nur beim ersten Schreiben)

Extra Cvars als Argumente übergeben, z.B.:
  $0 --daily +set r_bloomIntensity 0.8
EOF
  exit 0
}

while [ $# -gt 0 ]; do
  case "$1" in
    --help|-h) usage ;;
    --daily) DAILY=1; shift ;;
    --hardcore) HARDCORE=1; shift ;;
    --ghost) GHOST=1; shift ;;
    --wayland) WAYLAND=1; shift ;;
    --gfx-reset) GFX_RESET=1; shift ;;
    --map)
      if [ $# -lt 2 ]; then
        echo "usage: --map <name>" >&2
        exit 2
      fi
      MAP="$2"
      MAP_FORCED=1
      shift 2
      ;;
    +set|-set)
      if [ $# -lt 3 ]; then
        echo "usage: +set <cvar> <value>" >&2
        exit 2
      fi
      EXTRA_CVARS+=("+set" "$2" "$3")
      shift 3
      ;;
    *) break ;;
  esac
done

if [ ! -x "$ENGINE_BIN" ]; then
  echo "Engine-Binary nicht gefunden: $ENGINE_BIN" >&2
  echo "Setze QUAKE3E_DIR oder erstelle ~/quake3e-engine/ mit quake3e.x64." >&2
  exit 2
fi

# Native Wayland backend: avoids the X11 compat layer on GNOME/Wayland.
# SDL2 auto-selects the backend, but forcing it prevents Xwayland fallback flicker.
if [ "$WAYLAND" -eq 1 ]; then
  export SDL_VIDEODRIVER=wayland
  echo "Wayland backend: SDL_VIDEODRIVER=wayland"
fi

daily_map_today() {
  # FNV-1a over YYYY-MM-DD, then (hash/40)%3 — must match g_neonwave.c
  python3 - <<'PY'
from datetime import date
h = 2166136261
for c in date.today().strftime("%Y-%m-%d").encode("ascii"):
    h ^= c
    h = (h * 16777619) & 0xffffffff
forced = h & 0x7fffffff
idx = (forced // 40) % 3
print(("oa_shine", "oa_minia", "oa_rpg3dm2")[idx])
PY
}

MODE_CVARS=()
if [ "$DAILY" -eq 1 ]; then
  MODE_CVARS+=(+set g_neonwave_daily 1)
  if [ "$MAP_FORCED" -eq 0 ]; then
    if MAP_TODAY=$(daily_map_today 2>/dev/null) && [ -n "$MAP_TODAY" ]; then
      MAP="$MAP_TODAY"
      echo "Daily map: $MAP"
    fi
  fi
fi
if [ "$HARDCORE" -eq 1 ]; then
  MODE_CVARS+=(+set g_neonwave_hardcore 1)
fi
if [ "$GHOST" -eq 1 ]; then
  MODE_CVARS+=(+set g_neonwave_ghost 1)
  echo "Ghost kit: cloak/emp/lockdown/nuke  (J/H/K/N)  RMB zoom/snipe"
fi

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DETECT="$ROOT/scripts/detect-gfx.sh"
GFX_AUTO="$HOME_PATH/$GAME/gfx-auto.cfg"
mkdir -p "$HOME_PATH/$GAME"
if [ "$GFX_RESET" -eq 1 ] && [ -f "$GFX_AUTO" ]; then
  rm -f "$GFX_AUTO"
  echo "gfx-auto.cfg reset"
fi
if [ -x "$DETECT" ]; then
  PRESET=$("$DETECT" --ensure "$GFX_AUTO")
  echo "gfx auto: $PRESET  ($GFX_AUTO)"
fi
cfg_cvar() {
  local key="$1" file="$2"
  [ -f "$file" ] || return 0
  awk -v k="$key" '$1=="seta" && $2==k { gsub(/"/, "", $3); print $3; exit }' "$file"
}
if [ "$RENDERER" = auto ]; then
  RENDERER=$(cfg_cvar cl_renderer "$GFX_AUTO")
  RENDERER="${RENDERER:-vulkan}"
fi
if [ "$BLOOM" = auto ]; then
  BLOOM=$(cfg_cvar r_bloom "$GFX_AUTO")
  BLOOM="${BLOOM:-1}"
fi

exec "$ENGINE_BIN" \
  +set cl_renderer "$RENDERER" \
  +set r_bloom "$BLOOM" \
  +set fs_basepath "$BASE_PATH" \
  +set fs_homepath "$HOME_PATH" \
  +set fs_game "$GAME" \
  +set g_gametype "$GAME_TYPE" \
  "${MODE_CVARS[@]}" \
  "${EXTRA_CVARS[@]}" \
  +map "$MAP"
