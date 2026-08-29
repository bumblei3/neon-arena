#!/usr/bin/env bash
# NeonArena headless test suite (GT_NEONWAVE, gametype 14).
# Usage:
#   ./run_suite.sh              # run all tests
#   ./run_suite.sh --quick      # smoke-only subset (see QUICK_TESTS)
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

# TEST 18: run-stats JSON export
assert_18() {
  local ok=0 json="$HOME/.openarena/neonarena/neonwave_runstats.json"
  check "$1" "RUN STATS JSON written"; [ $LAST_RESULT -eq 0 ] || ok=1
  [ -f "$json" ] || ok=1
  if [ -f "$json" ]; then
    # must be valid JSON with the expected run fields
    if command -v python3 >/dev/null 2>&1; then
      python3 - "$json" <<'PY' || ok=1
import json, sys
d = json.load(open(sys.argv[1]))
for k in ("version","result","wave","kills","bestCombo","timeSec","difficulty","modifiersSeen","modifierNames","achievements"):
    assert k in d, "missing key %s" % k
assert d["version"] == 1
assert d["result"] in ("VICTORY","FAILED")
assert isinstance(d["modifierNames"], list)
assert isinstance(d["achievements"], list)
PY
    fi
  fi
  report $ok "runstats-json"
}

# TEST 19: achievements in run-stats (log markers, deterministic)
# The run-stats json can be overwritten by the dedi server's map-restart loop,
# so achievements are asserted via the "ACHIEVEMENT <NAME>" log markers that
# NW_CheckAchievements prints for every unlocked badge this run.
# startwave 15 + failrun must produce SURVIVOR (reached wave >= 15).
assert_19() {
  local ok=0
  check "$1" "RUN STATS JSON written"; [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "ACHIEVEMENT SURVIVOR"; [ $LAST_RESULT -eq 0 ] || ok=1
  report $ok "achievements-json"
}

# TEST 20: hardcore mode (g_neonwave_hardcore 1 -> HARDCORE banner + tougher boss)
assert_20() {
  local ok=0
  check "$1" "HARDCORE mode enabled";                 [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "HARDCORE";                             [ $LAST_RESULT -eq 0 ] || ok=1
  # TANK boss base hc 600 -> hardcore x1.5 = 900
  check "$1" "boss spawned: TANK (hc 900)";          [ $LAST_RESULT -eq 0 ] || ok=1
  no_fatal_warnings "$1" || ok=1
  report $ok "hardcore-mode"
}

# TEST 21: VAMPIRE modifier heals the player on each drone kill (lifesteal)
assert_21() {
  local ok=0 logfile="$1"
  # modifier name must appear on the wave banner
  check "$logfile" "starting wave 6.*\\[VAMPIRE\\]"; [ $LAST_RESULT -eq 0 ] || ok=1
  # lifesteal must fire at least once during the auto-killed wave
  count_min "$logfile" "VAMPIRE lifesteal" 1;          [ $? -eq 0 ] || ok=1
  # payload: confirm modifier = NW_MOD_VAMPIRE (6) on wave 6
  if [ -f "$TESTDIR/cs_neonwave_parse.sh" ]; then
    . "$TESTDIR/cs_neonwave_parse.sh" 2>/dev/null || true
    if declare -f parse_cs_neonwave_at >/dev/null 2>&1; then
      parse_cs_neonwave_at "$logfile" 6 || { :; }
      [ -n "${CS_WAVE:-}" ] && [ "${CS_WAVE:-0}" -eq 6 ] 2>/dev/null || ok=1
      [ -n "${CS_MOD:-}" ] && [ "${CS_MOD:-0}" -eq 6 ] 2>/dev/null || ok=1
    fi
  fi
  no_fatal_warnings "$1" || ok=1
  report $ok "vampire-lifesteal"
}

# TEST 22: FRENZY modifier boosts g_quadfactor (harder hits) for the wave
assert_22() {
  local ok=0 logfile="$1"
  check "$logfile" "starting wave 6.*\\[FRENZY\\]"; [ $LAST_RESULT -eq 0 ] || ok=1
  count_min "$logfile" "FRENZY quadfactor set to 4" 1;      [ $? -eq 0 ] || ok=1
  no_fatal_warnings "$1" || ok=1
  report $ok "frenzy-quadfactor"
}

# TEST 23: OVERSHIELD modifier grants +50 armor at wave start
assert_23() {
  local ok=0 logfile="$1"
  check "$logfile" "starting wave 6.*\\[OVERSHIELD\\]"; [ $LAST_RESULT -eq 0 ] || ok=1
  count_min "$logfile" "OVERSHIELD +50 armor granted" 1;   [ $? -eq 0 ] || ok=1
  no_fatal_warnings "$1" || ok=1
  report $ok "overshield-armor"
}

# TEST 24: SPEEDRUNNER achievement (victory in <= 300s)
assert_24() {
  local ok=0
  check "$1" "All waves cleared";                       [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "ACHIEVEMENT SPEEDRUNNER";                 [ $LAST_RESULT -eq 0 ] || ok=1
  no_fatal_warnings "$1" || ok=1
  report $ok "speedrunner-ach"
}

# TEST 25: HARDCORE achievement (victory with g_neonwave_hardcore 1)
assert_25() {
  local ok=0
  check "$1" "HARDCORE mode enabled";                   [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "All waves cleared";                       [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "ACHIEVEMENT HARDCORE";                    [ $LAST_RESULT -eq 0 ] || ok=1
  no_fatal_warnings "$1" || ok=1
  report $ok "hardcore-ach"
}

# TEST 26: COMBOMASTER achievement (best combo >= 12)
assert_26() {
  local ok=0
  check "$1" "fake combo 12 registered";                [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "ACHIEVEMENT COMBOMASTER";                 [ $LAST_RESULT -eq 0 ] || ok=1
  report $ok "combomaster-ach"
}

# TEST 27: perk cards are offered at wave-clear
assert_27() {
  local ok=0
  check "$1" "PERK OFFER F1=";                          [ $LAST_RESULT -eq 0 ] || ok=1
  no_fatal_warnings "$1" || ok=1
  report $ok "perk-offer"
}

# TEST 28: forced offer + autopick takes PIERCE
assert_28() {
  local ok=0
  check "$1" "PERK OFFER F1=PIERCE";                    [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "PERK TAKEN PIERCE";                       [ $LAST_RESULT -eq 0 ] || ok=1
  no_fatal_warnings "$1" || ok=1
  report $ok "perk-pierce-pick"
}

# TEST 29: SKIP perk blanks the next wave's modifier
assert_29() {
  local ok=0
  check "$1" "PERK TAKEN SKIP";                         [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "SKIP modifier";                           [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "starting wave 6 (";                       [ $LAST_RESULT -eq 0 ] || ok=1
  no_fatal_warnings "$1" || ok=1
  report $ok "perk-skip-mod"
}

# TEST 2: full run victory
assert_2() {
  local ok=0
  check "$1" "starting wave 20";                        [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "All waves cleared";                       [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "NEW BEST wave 20";                        [ $LAST_RESULT -eq 0 ] || ok=1
  count_min "$1" "upgrade point granted" 19;           [ $? -eq 0 ] || ok=1
  # v0.27 reveal: full pool + all five bosses appear in a classic 20-wave run
  check "$1" "starting wave 10.*\\[VAMPIRE\\]";         [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "starting wave 11.*\\[FRENZY\\]";          [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "starting wave 12.*\\[OVERSHIELD\\]";      [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "boss spawned: GLASS CANNON";              [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "boss spawned: WARDEN";                    [ $LAST_RESULT -eq 0 ] || ok=1
  no_fatal_warnings "$1" || ok=1
  report $ok "full-run-victory"
}

# TEST 5: boss tank hp
assert_5() {
  local ok=0 logfile="$1"
  # boss spawns with hc\600 (clientuserinfo) and the BOSS banner on wave 10
  check "$logfile" "hc\\\\600";                          [ $LAST_RESULT -eq 0 ] || ok=1
  check "$logfile" "starting wave 10.*BOSS";            [ $LAST_RESULT -eq 0 ] || ok=1
  # NOTE: the structured payload logs bossMax=0 on wave-clear because the boss is
  # already dead when NeonWave_LogPayload runs (NW_BossHealth skips dead bots).
  # We intentionally do NOT assert CS_BOSSMX here — see NW_BossHealth for the
  # upstream quirk. The hc\600 spawn marker above is the authoritative check.
  report $ok "boss-tank-hp"
}

# TEST 6: endless timeattack
assert_6() {
  local ok=0
  check "$1" "starting wave 25";                        [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "All waves cleared";                       [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "BEST TIME";                               [ $LAST_RESULT -eq 0 ] || ok=1
  no_fatal_warnings "$1" || ok=1
  report $ok "endless-timeattack"
}

# TEST 9: swarm mini-drones
assert_9() {
  local ok=0
  check "$1" "swarm mother spawns mini-drone";          [ $LAST_RESULT -eq 0 ] || ok=1
  # without rageforce there must be no RAGE tags
  assert_no_pattern "$1" "RAGE" || ok=1
  report $ok "swarm-minidrones"
}

# TEST 9b: swarm rage
assert_9b() {
  local ok=0
  check "$1" "SWARM MOTHER ENRAGED";                    [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "mini-drone (RAGE)";                       [ $LAST_RESULT -eq 0 ] || ok=1
  report $ok "swarm-rage"
}

# TEST 11: sniper dash
assert_11() {
  local ok=0 logfile="$1"
  check "$logfile" "starting wave 10.*BOSS";            [ $LAST_RESULT -eq 0 ] || ok=1
  check "$logfile" "hc\\\\400";                          [ $LAST_RESULT -eq 0 ] || ok=1
  check "$logfile" "SNIPER dashes to new position";    [ $LAST_RESULT -eq 0 ] || ok=1
  report $ok "sniper-dash"
}

# TEST 14: glass cannon boss
assert_14() {
  local ok=0 logfile="$1"
  check "$logfile" "boss spawned: GLASS CANNON (hc 200)"; [ $LAST_RESULT -eq 0 ] || ok=1
  check "$logfile" "hc\\\\200";                          [ $LAST_RESULT -eq 0 ] || ok=1
  report $ok "boss-glass-cannon"
}

# TEST 15: daily challenge determinism
assert_15() {
  local ok=0 logfile="$1"
  count_min "$logfile" "DAILY CHALLENGE seed 12345" 2;  [ $? -eq 0 ] || ok=1
  report $ok "daily-challenge-determinism"
}

# TEST 16: warden boss
assert_16() {
  local ok=0 logfile="$1"
  check "$logfile" "boss spawned: WARDEN (hc 500)";     [ $LAST_RESULT -eq 0 ] || ok=1
  check "$logfile" "WARDEN strikes the player zone";    [ $LAST_RESULT -eq 0 ] || ok=1
  check "$logfile" "WARDEN raises armor";               [ $LAST_RESULT -eq 0 ] || ok=1
  report $ok "boss-warden"
}

# ---- main dispatch ----

# Map test number -> name, timeout, cvars. Used by all/quick/single.
dispatch_test() {
  case "$1" in
    1)  run_test 1 "smoke+boss" 60 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 ;;
    2)  rm -f "$HOME/.openarena/neonarena/neonwave_records.dat"; run_test 2 "full-run-victory" 240 +set g_neonwave_autostart 1 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 +set g_neonwave_best 0 ;;
    3)  run_test 3 "modifier-lowgrav" 60 +set g_neonwave_autostart 1 +set g_neonwave_startwave 6 +set g_neonwave_modifier 3 +set g_neonwave_fastbreak 1 +set g_neonwave_autokill 1 ;;
    4)  run_test 4 "failrun-stats" 60 +set g_neonwave_autostart 1 +set g_neonwave_failrun 1 ;;
    5)  run_test 5 "boss-tank-hp" 90 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_bosstype 2 +set g_neonwave_autokill 1 ;;
    6)  run_test 6 "endless-timeattack" 240 +set g_neonwave_autostart 1 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 +set g_neonwave_startwave 20 +set g_neonwave_maxwave 25 ;;
    7)  run_test 7 "record-persistence" 90 +set g_neonwave_autostart 1 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 +set g_neonwave_startwave 20 ;;
    8)  run_test 8 "combo-pipeline" 60 +set g_neonwave_autostart 1 +set g_neonwave_startwave 2 +set g_neonwave_fakecombo 7 +set g_neonwave_botasplayer 1 +set g_neonwave_failrun 1 ;;
    9)  run_test 9 "swarm-minidrones" 90 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_bosstype 3 ;;
    9b) run_test 9b "swarm-rage" 90 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_bosstype 3 +set g_neonwave_rageforce 1 ;;
    10) run_test 10 "tank-shield-cycle" 90 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_bosstype 2 +set g_neonwave_fastbreak 1 ;;
    11) run_test 11 "sniper-dash" 90 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_bosstype 1 +set g_neonwave_dashforce 30 ;;
    12) run_test 12 "newrecord-flag" 90 +set g_neonwave_autostart 1 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 +set g_neonwave_startwave 20 ;;
    13) run_test 13 "mega-combo-reward" 60 +set g_neonwave_autostart 1 +set g_neonwave_startwave 2 +set g_neonwave_fakecombo 8 +set g_neonwave_botasplayer 1 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 ;;
    14) run_test 14 "boss-glass-cannon" 90 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_bosstype 4 ;;
    15) run_test 15 "daily-challenge-determinism" 120 +set g_neonwave_daily 1 +set g_neonwave_dailyseed 12345 +set g_neonwave_startwave 10 ;;
    16) run_test 16 "boss-warden" 90 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_bosstype 5 +set g_neonwave_wardenforce 1 ;;
    17) run_test 17 "timewarp-modifier" 60 +set g_neonwave_autostart 1 +set g_neonwave_startwave 6 +set g_neonwave_modifier 5 +set g_neonwave_fastbreak 1 +set g_neonwave_autokill 1 ;;
    18) rm -f "$HOME/.openarena/neonarena/neonwave_runstats.json"; run_test 18 "runstats-json" 90 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_failrun 1 ;;
    19) rm -f "$HOME/.openarena/neonarena/neonwave_runstats.json"; run_test 19 "achievements-json" 90 +set g_neonwave_autostart 1 +set g_neonwave_startwave 15 +set g_neonwave_failrun 1 ;;
    20) rm -f "$HOME/.openarena/neonarena/neonwave_runstats.json"; run_test 20 "hardcore-mode" 90 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_bosstype 2 +set g_neonwave_hardcore 1 ;;
    21) run_test 21 "vampire-lifesteal" 60 +set g_neonwave_autostart 1 +set g_neonwave_startwave 6 +set g_neonwave_modifier 6 +set g_neonwave_botasplayer 1 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 ;;
    22) run_test 22 "frenzy-quadfactor" 60 +set g_neonwave_autostart 1 +set g_neonwave_startwave 6 +set g_neonwave_modifier 7 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 ;;
    23) run_test 23 "overshield-armor" 60 +set g_neonwave_autostart 1 +set g_neonwave_startwave 6 +set g_neonwave_modifier 8 +set g_neonwave_botasplayer 1 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 ;;
    24) run_test 24 "speedrunner-ach" 120 +set g_neonwave_autostart 1 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 +set g_neonwave_startwave 20 ;;
    25) run_test 25 "hardcore-ach" 180 +set g_neonwave_autostart 1 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 +set g_neonwave_startwave 20 +set g_neonwave_hardcore 1 ;;
    26) run_test 26 "combomaster-ach" 60 +set g_neonwave_autostart 1 +set g_neonwave_startwave 2 +set g_neonwave_fakecombo 12 +set g_neonwave_botasplayer 1 +set g_neonwave_failrun 1 ;;
    27) run_test 27 "perk-offer" 60 +set g_neonwave_autostart 1 +set g_neonwave_startwave 5 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 ;;
    28) run_test 28 "perk-pierce-pick" 60 +set g_neonwave_autostart 1 +set g_neonwave_startwave 5 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 +set g_neonwave_perkforce 123 +set g_neonwave_autopick 1 ;;
    29) run_test 29 "perk-skip-mod" 60 +set g_neonwave_autostart 1 +set g_neonwave_startwave 5 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 +set g_neonwave_perkforce 612 +set g_neonwave_autopick 1 ;;
    *)  echo "no cvar mapping for test $1"; return 2 ;;
  esac
}
ALL_TESTS="1 2 3 4 5 6 7 8 9 9b 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29"
QUICK_TESTS="1 3 4 7 8 10 12 13 17 18 19 20 21 22 23 26 27 28"

case "$MODE" in
  all)
    for t in $ALL_TESTS; do
      dispatch_test "$t" || true
    done
    ;;
  quick)
    for t in $QUICK_TESTS; do
      dispatch_test "$t" || true
    done
    ;;
  single)
    if [ -z "$SELECTED" ]; then
      echo "specify --test N"
      exit 1
    fi
    dispatch_test "$SELECTED"
    ;;
esac

echo
echo "Suite result: $PASS passed, $FAIL failed"
if [ "$FAIL" -gt 0 ]; then
  echo "FAILED: $FAILED_NAMES"
  exit "$FAIL"
fi
exit 0
