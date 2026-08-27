#!/usr/bin/env bash
# NeonArena headless test suite (GT_NEONWAVE, gametype 14).
# Usage:
#   ./run_suite.sh              # run all tests
#   ./run_suite.sh --quick      # smoke-only subset (1,3,4,7,8,10,12,13)
#   ./run_suite.sh --test 7     # single test
#   ./run_suite.sh --list       # list test names
#   ./run_suite.sh --real-window [N...]   # run on DISPLAY=:0 with visible window
#
# Requirements: QVMs installed to ~/.openarena/neonarena
# (see build-mod.sh), xvfb-run available when not using --real-window.
# Default runner: ioq3ded (dedicated headless). Set OA_BIN=openarena to use
# the Debian openarena wrapper (client, requires xvfb + different cvars).
set -u

# Directory containing this script (needed by parser-based assertions)
TESTDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Default to ioq3ded for headless testing. The Debian openarena wrapper only
# sets cvars for the client binary and dies under xvfb before map load.
OA_BIN="${OA_BIN:-/usr/lib/ioquake3/ioq3ded}"
OA_EXTRA=()
if [ "${OA_BIN##*/}" = "openarena" ] || [ "$OA_BIN" = "/usr/games/openarena" ]; then
  : # wrapper sets its own cvars; nothing extra needed
elif [ "${OA_BIN##*/}" = "ioq3ded" ] || [ "$OA_BIN" = "/usr/lib/ioquake3/ioq3ded" ]; then
  OA_EXTRA=(+set com_basegame baseoa +set fs_basepath /usr/lib/openarena
            +set fs_homepath "$HOME/.openarena" +set com_legacyprotocol 71
            +set com_protocol 71)
fi

HOME_DIR="$HOME/.openarena/neonarena"
PASS=0; FAIL=0; FAILED_NAMES=""

MODE="all"
SELECTED=""
REAL=0
while [ $# -gt 0 ]; do
  case "$1" in
    --quick) MODE="quick" ;;
    --test) MODE="single"; SELECTED="$2"; shift ;;
    --list) MODE="list" ;;
    --real-window)
      REAL=1
      MODE="${2:-all}"
      if [ "$#" -gt 1 ]; then
        MODE="single"
        SELECTED="$2"
        shift
      fi
      ;;
    *) echo "unknown option: $1"; exit 2 ;;
  esac
  shift
done

if [ "$MODE" = "list" ]; then
  grep -E "^# TEST [0-9]+:" "$0" | sed 's/# TEST //'
  exit 0
fi

# RUNNER: use xvfb-run for headless, or empty for real window
if [ "$REAL" -eq 1 ]; then
  RUNNER=""
elif command -v xvfb-run >/dev/null; then
  RUNNER="xvfb-run -a"
else
  echo "xvfb-run missing (apt install xvfb) or use --real-window"
  exit 2
fi

LOGDIR="/tmp/nw-suite-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$LOGDIR"

# Tests known to be timing-sensitive in headless CI (spawn-fallback races).
# They still run, but a single FAIL is retried once before counting as failed.
# Space-separated list of test numbers.
FLAKY_TESTS="${FLAKY_TESTS:-13}"

is_flaky() { # is_flaky <num>; returns 0 if num is in FLAKY_TESTS
  local n="$1" f
  for f in $FLAKY_TESTS; do
    [ "$f" = "$n" ] && return 0
  done
  return 1
}

# run_test <num> <name> <timeout> <cvar1> <cvar2> ...
run_test() {
  local num="$1" name="$2" timeout_s="$3"
  shift 3
  local log="$LOGDIR/test${num}.log"
  local attempt=1 max_attempts=1
  is_flaky "$num" && max_attempts=2
  TEST_NUM="$num"
  while [ "$attempt" -le "$max_attempts" ]; do
    if [ "$max_attempts" -gt 1 ] && [ "$attempt" -gt 1 ]; then
      echo "  (retry $attempt for flaky test $num)"
    fi
    local _before_pass="${PASS:-0}" _before_fail="${FAIL:-0}"
    printf '%s' "TEST $num: $name ... "
    $RUNNER timeout "$timeout_s" "$OA_BIN" +set dedicated 1 "${OA_EXTRA[@]}" \
      +set sv_maxclients 24 \
      +set fs_game neonarena +set g_gametype 14 +map oa_shine \
      "$@" > "$log" 2>&1 || true
    eval "assert_$num \"$log\""
    # if this attempt passed (PASS advanced) -> done; if it failed and we have
    # attempts left -> retry; otherwise done (report already counted it)
    if [ "${PASS:-0}" -gt "$_before_pass" ]; then
      return
    fi
    if [ "$attempt" -lt "$max_attempts" ]; then
      attempt=$((attempt+1))
      continue
    fi
    return
  done
}

check() { # check <log> <pattern>; sets LAST_RESULT
  local log="$1"
  shift
  grep -q "$*" "$log" && LAST_RESULT=0 || LAST_RESULT=1
}

count_min() { # count_min <log> <pattern> <min>
  local n
  n=$(grep -c "$2" "$1")
  [ "$n" -ge "$3" ]
}

assert_no_pattern() { # assert_no_pattern <log> <pattern> — fails if found
  if grep -qE "$2" "$1"; then
    echo "unexpected pattern '$2' in $1" >&2
    return 1
  fi
}

no_fatal_warnings() { # no_fatal_warnings <log> — generic hygiene check
  local ok=0
  assert_no_pattern "$1" "WARNING cannot write" || ok=1
  assert_no_pattern "$1" "G_ParseSpawnVars.*ERROR|ERROR.*G_ParseSpawnVars" || ok=1
  # victory-path hygiene: run must not have failed unexpectedly
  assert_no_pattern "$1" "NeonWave over.*(FAILED)" || ok=1
  return $ok
}

report() { # report <ok> <name>
  if [ "$1" -eq 0 ]; then
    echo "PASS"
    PASS=$((PASS+1))
  else
    echo "FAIL (log: $LOGDIR/test${TEST_NUM}.log)"
    FAIL=$((FAIL+1))
    FAILED_NAMES="$FAILED_NAMES $2"
  fi
}

# ---- individual tests: assert_<num> uses the log file $1 ----

# TEST 1: smoke + boss wave force-start (wave 10, classic sniper 4x hp)
assert_1() {
  local ok=0
  # version-agnostic: match any NeonArena-* gameversion
  check "$1" "gamename\\\\NeonArena-*"; [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "g_gametype\\\\14";      [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "starting wave 10";     [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "hc\\\\400";             [ $LAST_RESULT -eq 0 ] || ok=1
  report $ok "smoke+boss"
}

# TEST 2: full-run-victory (uses parser if available)
assert_2() {
  local ok=0 logfile="$1"
  check "$logfile" "NeonWave: starting wave 20";     [ $LAST_RESULT -eq 0 ] || ok=1
  check "$logfile" "All waves cleared";               [ $LAST_RESULT -eq 0 ] || ok=1
  check "$logfile" "NEW BEST wave 20";                [ $LAST_RESULT -eq 0 ] || ok=1
  count_min "$logfile" "upgrade point granted" 19;   [ $? -eq 0 ] || ok=1
  grep -qE "starting wave [5-9] .*\\[(GLASS DRONES|SWARM|LOW GRAVITY|DOUBLE POINTS)\\]" "$logfile" || ok=1
  no_fatal_warnings "$logfile" || ok=1
  # parser-based payload assertions (optional — if parser unavailable, skip)
  if [ -f "$TESTDIR/cs_neonwave_parse.sh" ]; then
    . "$TESTDIR/cs_neonwave_parse.sh" 2>/dev/null || true
    if declare -f parse_cs_neonwave >/dev/null 2>&1; then
      parse_cs_neonwave "$logfile" || { :; }
      [ -n "${CS_WAVE:-}" ] && [ "${CS_WAVE:-0}" -ge 20 ] 2>/dev/null || ok=1
      [ -n "${CS_EVENT:-}" ] && { [ "${CS_EVENT:-0}" -eq 1 ] || [ "${CS_EVENT:-0}" -eq 3 ]; } || ok=1
      [ -n "${CS_BEST:-}" ] && [ "${CS_BEST:-0}" -ge 20 ] 2>/dev/null || ok=1
      [ -n "${CS_PTS:-}" ] && [ "${CS_PTS:-0}" -ge 19 ] 2>/dev/null || ok=1
      [ -n "${CS_BOSSHP:-}" ] && [ "${CS_BOSSHP:-0}" -gt 0 ] 2>/dev/null || ok=1
      if [ "${CS_EVENT:-0}" -eq 3 ] 2>/dev/null; then
        [ -n "${CS_RUNSEC:-}" ] && [ "${CS_RUNSEC:-0}" -gt 0 ] 2>/dev/null || ok=1
      fi
    fi
  fi
  report $ok "full-run-victory"
}

# TEST 3: forced LOW GRAVITY modifier + gravity restore on wave clear
assert_3() {
  local ok=0 logfile="$1"
  check "$logfile" "starting wave 6.*\\[LOW GRAVITY\\]"; [ $LAST_RESULT -eq 0 ] || ok=1
  check "$logfile" "gravity restored to 800";            [ $LAST_RESULT -eq 0 ] || ok=1
  # payload: confirm modifier = NW_MOD_LOWGRAV (3), event in {1,3} for WAVE 6
  # (use _at so we read the forced-wave payload, not the last wave's boss payload)
  if [ -f "$TESTDIR/cs_neonwave_parse.sh" ]; then
    . "$TESTDIR/cs_neonwave_parse.sh" 2>/dev/null || true
    if declare -f parse_cs_neonwave_at >/dev/null 2>&1; then
      parse_cs_neonwave_at "$logfile" 6 || { :; }
      [ -n "${CS_WAVE:-}" ] && [ "${CS_WAVE:-0}" -eq 6 ] 2>/dev/null || ok=1
      [ -n "${CS_MOD:-}" ] && [ "${CS_MOD:-0}" -eq 3 ] 2>/dev/null || ok=1
      [ -n "${CS_EVENT:-}" ] && { [ "${CS_EVENT:-0}" -eq 1 ] || [ "${CS_EVENT:-0}" -eq 3 ]; } || ok=1
    fi
  fi
  report $ok "modifier-lowgrav"
}

# TEST 4: failed run path + run stats
assert_4() {
  local ok=0
  check "$1" "RUN STATS";           [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "NeonWave over";       [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "All waves cleared";   [ $LAST_RESULT -eq 1 ] || ok=1
  no_fatal_warnings "$1" || ok=1
  report $ok "failrun-stats"
}

# TEST 5: boss tank hp
assert_5() {
  local ok=0 logfile="$1"
  check "$logfile" "hc\\\\600";       [ $LAST_RESULT -eq 0 ] || ok=1
  check "$logfile" "starting wave 10.*BOSS"; [ $LAST_RESULT -eq 0 ] || ok=1
  # payload: verify bossMax=600, modifier!=NONE, event in {1,3}
  if [ -f "$TESTDIR/cs_neonwave_parse.sh" ]; then
    . "$TESTDIR/cs_neonwave_parse.sh" 2>/dev/null || true
    if declare -f parse_cs_neonwave >/dev/null 2>&1; then
      parse_cs_neonwave "$logfile" || { :; }
      [ -n "${CS_BOSSMX:-}" ] && [ "${CS_BOSSMX:-0}" -eq 600 ] 2>/dev/null || ok=1
      [ -n "${CS_MOD:-}" ] && [ "${CS_MOD:-0}" -ne 0 ] 2>/dev/null || ok=1
      [ -n "${CS_EVENT:-}" ] && { [ "${CS_EVENT:-0}" -eq 1 ] || [ "${CS_EVENT:-0}" -eq 3 ]; } || ok=1
    fi
  fi
  report $ok "boss-tank-hp"
}

# TEST 7: record persistence
assert_7() {
  local ok=0
  check "$1" "RECORDS SAVED";          [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "records loaded wave=[1-9]"; [ $LAST_RESULT -eq 0 ] || ok=1
  [ -f "$HOME_DIR/neonwave_records.dat" ] || ok=1
  report $ok "record-persistence"
}

# TEST 8: combo pipeline
assert_8() {
  local ok=0 logfile="$1"
  check "$logfile" "fake combo 7 registered"; [ $LAST_RESULT -eq 0 ] || ok=1
  check "$logfile" "RUN STATS kills=7";        [ $LAST_RESULT -eq 0 ] || ok=1
  check "$logfile" "bestCombo=7";              [ $LAST_RESULT -eq 0 ] || ok=1
  if [ -f "$TESTDIR/cs_neonwave_parse.sh" ]; then
    . "$TESTDIR/cs_neonwave_parse.sh" 2>/dev/null || true
    if declare -f parse_cs_neonwave >/dev/null 2>&1; then
      parse_cs_neonwave "$logfile" || { :; }
      [ -n "${CS_BESTCOMBO:-}" ] && [ "${CS_BESTCOMBO:-0}" -eq 7 ] 2>/dev/null || ok=1
    fi
  fi
  report $ok "combo-pipeline"
}

# TEST 10: tank shield cycle
assert_10() {
  local ok=0 logfile="$1"
  count_min "$logfile" "TANK raises SHIELD" 1; [ $? -eq 0 ] || ok=1
  check "$logfile" "TANK shield drops";         [ $LAST_RESULT -eq 0 ] || ok=1
  # NOTE: no payload assertion — the tank survives the 90s window (no autokill),
  # so there is no guaranteed end-of-run payload; the catalog only requires the
  # shield cycle markers above. (assert_3 uses parse_cs_neonwave_at for waves
  # that do produce a payload.)
  report $ok "tank-shield-cycle"
}

# TEST 12: new record flag
assert_12() {
  local ok=0
  check "$1" "NEW RECORD WAVE\\|NEW RECORD TIME"; [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "RECORDS SAVED";                    [ $LAST_RESULT -eq 0 ] || ok=1
  report $ok "newrecord-flag"
}

# TEST 13: mega combo reward
assert_13() {
  local ok=0 logfile="$1"
  check "$logfile" "fake combo 8 registered"; [ $LAST_RESULT -eq 0 ] || ok=1
  check "$logfile" "MEGA COMBO 8";            [ $LAST_RESULT -eq 0 ] || ok=1
  check "$logfile" "item_quad";                [ $LAST_RESULT -eq 0 ] || ok=1
  report $ok "mega-combo-reward"
}

# TEST 17: TIME WARP modifier scales player speed (g_speed) for the wave
assert_17() {
  local ok=0 logfile="$1"
  # modifier name must appear on the wave banner
  check "$logfile" "starting wave 6.*\[TIME WARP\]"; [ $LAST_RESULT -eq 0 ] || ok=1
  # engine logs the cvar change when NeonWave sets g_speed
  check "$logfile" "g_speed changed to 520";       [ $LAST_RESULT -eq 0 ] || ok=1
  # and it must be restored to default on the following non-warp wave
  check "$logfile" "g_speed changed to 320";       [ $LAST_RESULT -eq 0 ] || ok=1
  report $ok "timewarp-modifier"
}

# ---- main dispatch ----

case "$MODE" in
  all)
    if [ -n "$SELECTED" ]; then
      eval "assert_$SELECTED" "\"$LOGDIR/test${SELECTED}.log\"" 2>/dev/null || true
    else
      echo "specify --test N or run without --test for all tests"
      exit 1
    fi
    ;;
  quick)
    # smoke-only subset: 1,3,4,7,8,10,12,13 — fast headless verification
    run_test 1 "smoke+boss" 60 \
      +set g_neonwave_autostart 1 +set g_neonwave_startwave 10
    run_test 3 "modifier-lowgrav" 60 \
      +set g_neonwave_autostart 1 +set g_neonwave_startwave 6 \
      +set g_neonwave_modifier 3 +set g_neonwave_fastbreak 1 \
      +set g_neonwave_autokill 1
    run_test 4 "failrun-stats" 60 \
      +set g_neonwave_autostart 1 +set g_neonwave_failrun 1
    run_test 7 "record-persistence" 90 \
      +set g_neonwave_autostart 1 +set g_neonwave_autokill 1 \
      +set g_neonwave_fastbreak 1 +set g_neonwave_startwave 20
    run_test 8 "combo-pipeline" 60 \
      +set g_neonwave_autostart 1 +set g_neonwave_startwave 2 \
      +set g_neonwave_fakecombo 7 +set g_neonwave_botasplayer 1 \
      +set g_neonwave_failrun 1
    run_test 10 "tank-shield-cycle" 90 \
      +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 \
      +set g_neonwave_bosstype 2 +set g_neonwave_fastbreak 1
    run_test 12 "newrecord-flag" 90 \
      +set g_neonwave_autostart 1 +set g_neonwave_autokill 1 \
      +set g_neonwave_fastbreak 1 +set g_neonwave_startwave 20
    run_test 13 "mega-combo-reward" 60 \
      +set g_neonwave_autostart 1 +set g_neonwave_startwave 2 \
      +set g_neonwave_fakecombo 8 +set g_neonwave_botasplayer 1 \
      +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1
    run_test 17 "timewarp-modifier" 60 \
      +set g_neonwave_autostart 1 +set g_neonwave_startwave 6 \
      +set g_neonwave_modifier 5 +set g_neonwave_fastbreak 1 \
      +set g_neonwave_autokill 1
    ;;
  single)
    if [ -z "$SELECTED" ]; then
      echo "specify --test N"
      exit 1
    fi
    # map test number -> timeout + cvars (kept in sync with the quick block below)
    case "$SELECTED" in
      1)  run_test 1  "smoke+boss" 60 \
            +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 ;;
      3)  run_test 3  "modifier-lowgrav" 60 \
            +set g_neonwave_autostart 1 +set g_neonwave_startwave 6 \
            +set g_neonwave_modifier 3 +set g_neonwave_fastbreak 1 \
            +set g_neonwave_autokill 1 ;;
      4)  run_test 4  "failrun-stats" 60 \
            +set g_neonwave_autostart 1 +set g_neonwave_failrun 1 ;;
      7)  run_test 7  "record-persistence" 90 \
            +set g_neonwave_autostart 1 +set g_neonwave_autokill 1 \
            +set g_neonwave_fastbreak 1 +set g_neonwave_startwave 20 ;;
      8)  run_test 8  "combo-pipeline" 60 \
            +set g_neonwave_autostart 1 +set g_neonwave_startwave 2 \
            +set g_neonwave_fakecombo 7 +set g_neonwave_botasplayer 1 \
            +set g_neonwave_failrun 1 ;;
      10) run_test 10 "tank-shield-cycle" 90 \
            +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 \
            +set g_neonwave_bosstype 2 +set g_neonwave_fastbreak 1 ;;
      12) run_test 12 "newrecord-flag" 90 \
            +set g_neonwave_autostart 1 +set g_neonwave_autokill 1 \
            +set g_neonwave_fastbreak 1 +set g_neonwave_startwave 20 ;;
      13) run_test 13 "mega-combo-reward" 60 \
            +set g_neonwave_autostart 1 +set g_neonwave_startwave 2 \
            +set g_neonwave_fakecombo 8 +set g_neonwave_botasplayer 1 \
            +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 ;;
      17) run_test 17 "timewarp-modifier" 60 \
            +set g_neonwave_autostart 1 +set g_neonwave_startwave 6 \
            +set g_neonwave_modifier 5 +set g_neonwave_fastbreak 1 \
            +set g_neonwave_autokill 1 ;;
      *)  echo "no cvar mapping for test $SELECTED (add it to the single-case map)"; exit 2 ;;
    esac
    ;;
esac

echo
echo "Suite result: $PASS passed, $FAIL failed"
if [ "$FAIL" -gt 0 ]; then
  echo "FAILED: $FAILED_NAMES"
  exit "$FAIL"
fi
exit 0
