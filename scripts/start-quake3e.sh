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
ARENA=""
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
  --map NAME      Map (ohne Flags: Startmenü; mit --daily/--ghost/--arena sofort starten)
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
    --arena)
      if [ $# -lt 2 ]; then
        echo "usage: --arena <name>" >&2
        exit 2
      fi
      ARENA="$2"
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

daily_pick_today() {
  # FNV-1a over YYYY-MM-DD — must match g_neonwave.c NW_DailyHash + pool.
  # Prints: pool_key <tab> bsp <tab> arena_json_stem (may be empty)
  # Optional first arg: forced seed (g_neonwave_dailyseed).
  python3 - "$1" <<'PY'
import sys
from datetime import date
MOD_POOL, BOSS_COUNT = 16, 13
# (pool key, loadable OA bsp, optional configs/arenas/*.json stem)
POOL = [
    ("oa_shine", "oa_shine", "neon_arena"),
    ("oa_minia", "oa_minia", ""),
    ("oa_rpg3dm2", "oa_rpg3dm2", "catacombs"),
    ("oa_bleed", "slimefac", "bleed_chamber"),
    ("oa_node", "oa_dm1", "node_control"),
    ("oa_pulse", "oa_dm3", ""),
    ("oa_desert", "islanddm", "desert_storm"),
    ("oa_vortex", "oa_dm6", "vortex_ring"),
    ("oa_frostbite", "oa_minia", "frostbite"),
    ("oa_skybridge", "suspended", "skybridge"),
    ("oa_underhive", "am_underworks", "underhive"),
    ("oa_reactor", "hydronex", "reactor"),
    ("oa_overgrowth", "am_galmevish", "overgrowth"),
    ("oa_thor", "oa_thor", ""),
]
forced = 0
arg = sys.argv[1] if len(sys.argv) > 1 else ""
if arg.isdigit() and int(arg) > 0:
    forced = int(arg)
else:
    h = 2166136261
    for c in date.today().strftime("%Y-%m-%d").encode("ascii"):
        h ^= c
        h = (h * 16777619) & 0xffffffff
    forced = h & 0x7fffffff
idx = (forced // (MOD_POOL * BOSS_COUNT)) % len(POOL)
name, bsp, arena = POOL[idx]
print(f"{name}\t{bsp}\t{arena}")
PY
}

MODE_CVARS=()
if [ "$DAILY" -eq 1 ]; then
  MODE_CVARS+=(+set g_neonwave_daily 1)
  DAILY_SEED=""
  i=0
  while [ $i -lt ${#EXTRA_CVARS[@]} ]; do
    if [ "${EXTRA_CVARS[$i]}" = "+set" ] && [ "${EXTRA_CVARS[$((i+1))]:-}" = "g_neonwave_dailyseed" ]; then
      DAILY_SEED="${EXTRA_CVARS[$((i+2))]:-}"
      break
    fi
    i=$((i+1))
  done
  if PICK=$(daily_pick_today "$DAILY_SEED" 2>/dev/null) && [ -n "$PICK" ]; then
    DAILY_KEY="${PICK%%$'\t'*}"
    REST="${PICK#*$'\t'}"
    DAILY_BSP="${REST%%$'\t'*}"
    DAILY_ARENA="${REST#*$'\t'}"
    echo "Daily: $DAILY_KEY  bsp=$DAILY_BSP${DAILY_ARENA:+  arena=$DAILY_ARENA}"
    if [ "$MAP_FORCED" -eq 0 ]; then
      MAP="$DAILY_BSP"
    fi
    if [ -z "$ARENA" ] && [ -n "$DAILY_ARENA" ]; then
      ARENA="$DAILY_ARENA"
    fi
  fi
fi
if [ "$HARDCORE" -eq 1 ]; then
  MODE_CVARS+=(+set g_neonwave_hardcore 1)
fi
AFTER_MAP=()
if [ "$GHOST" -eq 1 ]; then
  MODE_CVARS+=(+set g_neonwave_ghost 1)
  AFTER_MAP+=(+exec ghost-binds.cfg)
  echo "Ghost kit: J/LB cloak  H/RB emp  K/X lock  N/Y nuke  M/RS scan  L/Back kit  RMB/B zoom"
fi

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

# --- Arena config loading ---
if [ -n "$ARENA" ]; then
  ARENA_DIR="$ROOT/configs/arenas"
  ARENA_FILE=""
  # Try exact match first, then case-insensitive
  for f in "$ARENA_DIR/$ARENA.json" "$ARENA_DIR/${ARENA,,}.json"; do
    if [ -f "$f" ]; then
      ARENA_FILE="$f"
      break
    fi
  done
  if [ -z "$ARENA_FILE" ]; then
    echo "Arena nicht gefunden: $ARENA" >&2
    echo "Verfügbare Arenas:" >&2
    for f in "$ARENA_DIR"/*.json; do
      [ -f "$f" ] && echo "  - $(basename "$f" .json)" >&2
    done
    exit 2
  fi
  echo "Arena: $ARENA ($ARENA_FILE)"
  # Parse JSON and set CVars
  while IFS='=' read -r key value; do
    # Remove quotes and whitespace
    value=$(echo "$value" | sed 's/^ *"//;s/" *$//')
    case "$key" in
      map) MAP="$value"; MAP_FORCED=1 ;;
      g_neonwave_modifier|g_neonwave_bosstype|g_neonwave_startwave)
        # Daily owns modifier/boss rotation; don't let arena JSON pin them.
        if [ "$DAILY" -eq 1 ]; then
          continue
        fi
        MODE_CVARS+=(+set "$key" "$value")
        ;;
      g_neonwave_maxwave|g_momentum|g_momentum_decay|g_momentum_kill|g_neonwave_drone_hp_scale|g_neonwave_drone_damage_scale|g_neonwave_drone_count_scale|g_neonwave_drone_speed_scale|g_neonwave_gravity_scale|g_neonwave_ghost|g_ghost_energy_start|g_ghost_energy_max|g_ghost_regen_amt|g_neonwave_hardcore)
        MODE_CVARS+=(+set "$key" "$value")
        ;;
    esac
  done < <(python3 -c "
import json,sys
with open('$ARENA_FILE') as f:
    data = json.load(f)
settings = data.get('settings', {})
for k,v in settings.items():
    print(f'{k}={v}')
")
fi

DETECT="$ROOT/scripts/detect-gfx.sh"
GFX_AUTO="$HOME_PATH/$GAME/gfx-auto.cfg"
mkdir -p "$HOME_PATH/$GAME"
# Loose cfgs: quake3e does not exec autoexec.cfg out of a pk3.
for cfg in autoexec.cfg ghost-binds.cfg neon-look.cfg neon-gfx.cfg; do
  if [ -f "$ROOT/assets/$cfg" ]; then
    cp "$ROOT/assets/$cfg" "$HOME_PATH/$GAME/$cfg"
  fi
done
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

look_cvars_for_map() {
  # Must match oa-gamecode/code/game/neon_maplook.h
  python3 - "$1" <<'PY'
import sys
LOOK = {
    "oa_shine": (1, "1.40", "0.50", "0.60", "0.18"),
    "oa_minia": (1, "1.40", "0.48", "0.62", "0.16"),
    "oa_rpg3dm2": (1, "1.40", "0.50", "0.60", "0.16"),
    "slimefac": (1, "1.45", "0.55", "0.55", "0.32"),
    "oa_dm1": (1, "1.42", "0.52", "0.58", "0.30"),
    "oa_dm3": (0, "1.25", "0.40", "0.70", "0.35"),
    "islanddm": (0, "1.20", "0.35", "0.72", "0.38"),
    "oa_dm6": (1, "1.40", "0.50", "0.60", "0.28"),
    "suspended": (1, "1.38", "0.48", "0.62", "0.30"),
    "am_underworks": (1, "1.45", "0.55", "0.55", "0.30"),
    "hydronex": (1, "1.42", "0.52", "0.58", "0.28"),
    "am_galmevish": (1, "1.40", "0.48", "0.62", "0.26"),
    "oa_thor": (1, "1.40", "0.50", "0.60", "0.28"),
}
ob, gamma, bi, bt, grid = LOOK.get(sys.argv[1], LOOK["oa_shine"])
print(ob, gamma, bi, bt, grid)
PY
}

LOOK_CVARS=()
if MAP_LOOK=$(look_cvars_for_map "$MAP" 2>/dev/null); then
  # shellcheck disable=SC2086
  read -r LOOK_OB LOOK_GAMMA LOOK_BI LOOK_BT LOOK_GRID <<EOF
$MAP_LOOK
EOF
  if [ -n "$LOOK_OB" ] && [ -n "$LOOK_GRID" ]; then
    LOOK_CVARS=(+set r_mapoverbrightbits "$LOOK_OB" +set r_gamma "$LOOK_GAMMA" +set r_bloom_intensity "$LOOK_BI" +set r_bloom_threshold "$LOOK_BT" +set cg_neon_grid "$LOOK_GRID")
    echo "Look: $MAP  overbright=$LOOK_OB bloom=$LOOK_BI grid=$LOOK_GRID"
  fi
fi

LAUNCH_MAP=()
if [ "$MAP_FORCED" -eq 1 ] || [ "$DAILY" -eq 1 ] || [ "$GHOST" -eq 1 ] || [ "$HARDCORE" -eq 1 ] || [ -n "${ARENA:-}" ]; then
  LAUNCH_MAP=(+map "$MAP")
else
  echo "Startmenü: PLAY / DAILY / GHOST / ARENA"
fi

exec "$ENGINE_BIN" \
  +set cl_renderer "$RENDERER" \
  +set r_bloom "$BLOOM" \
  +set fs_basepath "$BASE_PATH" \
  +set fs_homepath "$HOME_PATH" \
  +set fs_game "$GAME" \
  +set g_gametype "$GAME_TYPE" \
  "${MODE_CVARS[@]}" \
  "${LOOK_CVARS[@]}" \
  "${EXTRA_CVARS[@]}" \
  "${LAUNCH_MAP[@]}" \
  "${AFTER_MAP[@]}"
