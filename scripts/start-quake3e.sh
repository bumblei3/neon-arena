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
EXTRA_CVARS=()

usage() {
  cat <<EOF
Start NeonArena mit Quake3e (Vulkan) + Bloom.

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
  $0 +set r_bloomIntensity 0.8 +set r_mapoverbrightbits 1
EOF
  exit 0
}

# tiny arg parser: stop at first non-+set / non-option
while [ $# -gt 0 ]; do
  case "$1" in
    --help|-h) usage ;;
    +set|-set) shift 2; EXTRA_CVARS+=("$1" "$2"); continue ;;
    *) break ;;
  esac
  shift
done

if [ ! -x "$ENGINE_BIN" ]; then
  echo "Engine-Binary nicht gefunden: $ENGINE_BIN" >&2
  echo "Setze QUAKE3E_DIR oder erstelle ~/quake3e-engine/ mit quake3e.x64." >&2
  exit 2
fi

exec "$ENGINE_BIN" \
  +set cl_renderer "$RENDERER" \
  +set r_bloom "$BLOOM" \
  +set fs_basepath "$BASE_PATH" \
  +set fs_homepath "$HOME_PATH" \
  +set fs_game "$GAME" \
  +set g_gametype "$GAME_TYPE" \
  +set map "$MAP" \
  "${EXTRA_CVARS[@]}"
