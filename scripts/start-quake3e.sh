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
RENDERER="${RENDERER:-vulkan}"
BLOOM="${BLOOM:-1}"
DAILY=0
HARDCORE=0
EXTRA_CVARS=()

usage() {
  cat <<EOF
Start NeonArena mit Quake3e (Vulkan) + Bloom.

Usage:
  $0 [--daily] [--hardcore] [+set cvar value ...]

Optionen:
  --daily         Daily Challenge (g_neonwave_daily 1)
  --hardcore      Hardcore-Lauf (g_neonwave_hardcore 1)
  --help          diese Hilfe

Umgebungsvariablen (optional):
  QUAKE3E_DIR     Engine-Verzeichnis     (Default: $HOME/quake3e-engine)
  FS_BASEPATH     fs_basepath            (Default: $ENGINE_DIR)
  FS_HOMEPATH     fs_homepath            (Default: $HOME/.openarena)
  FS_GAME         fs_game                (Default: neonarena)
  GAME_TYPE       g_gametype             (Default: 14)
  MAP             map                    (Default: oa_shine)
  RENDERER        cl_renderer            (Default: vulkan, alternativen: opengl)
  BLOOM           r_bloom               (Default: 1)

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

MODE_CVARS=()
if [ "$DAILY" -eq 1 ]; then
  MODE_CVARS+=(+set g_neonwave_daily 1)
fi
if [ "$HARDCORE" -eq 1 ]; then
  MODE_CVARS+=(+set g_neonwave_hardcore 1)
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
