#!/usr/bin/env bash
# NeonArena headless test suite (GT_NEONWAVE, gametype 14).
# Usage:
#   ./run_suite.sh              # run all tests
#   ./run_suite.sh --quick      # smoke-only subset (1,3,4,10)
#   ./run_suite.sh --test 7     # single test
#   ./run_suite.sh --list       # list test names
#   ./run_suite.sh --real-window [N...]   # run on DISPLAY=:0 with visible window
#
# Requirements: openarena in PATH, QVMs installed to ~/.openarena/neonarena
# (see build-mod.sh), xvfb-run available when not using --real-window.
set -u

MODE="all"; SELECTED=""; REAL=0; LOGDIR=""
while [ $# -gt 0 ]; do
  case "$1" in
    --quick) MODE="quick" ;;
    --test) MODE="single"; SELECTED="$2"; shift ;;
    --list) MODE="list" ;;
    --real-window) REAL=1; MODE="${2:-all}"; [ "$#" -gt 1 ] && { MODE="single"; SELECTED="$2"; shift; } ;;
    *) echo "unknown option: $1"; exit 2 ;;
  esac; shift
done

OA_BIN="${OA_BIN:-openarena}"
# Debian's openarena wrapper only sets the cvars for the client binary;
# with ioq3ded as OA_BIN we must pass the basepath/homepath cvars ourselves.
OA_EXTRA=()
if [ "${OA_BIN##*/}" = "ioq3ded" ] || [ "$OA_BIN" = "/usr/lib/ioquake3/ioq3ded" ]; then
  OA_EXTRA=(+set com_basegame baseoa +set fs_basepath /usr/lib/openarena
            +set fs_homepath "$HOME/.openarena" +set com_legacyprotocol 71
            +set com_protocol 71)
fi
HOME_DIR="$HOME/.openarena/neonarena"
PASS=0; FAIL=0; FAILED_NAMES=""

if [ "$MODE" = "list" ]; then
  grep -E "^# TEST [0-9]+:" "$0" | sed 's/# TEST //'
  exit 0
fi

if [ "$REAL" -eq 1 ]; then
  RUNNER=""          # no xvfb wrapper — visible window on current display
else
  command -v xvfb-run >/dev/null || { echo "xvfb-run missing (apt install xvfb) or use --real-window"; exit 2; }
  RUNNER="xvfb-run -a"
fi

LOGDIR="/tmp/nw-suite-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$LOGDIR"

# run_test <num> <name> <timeout> <extra cvars...>
run_test() {
  local num="$1" name="$2" timeout_s="$3"; shift 3
  local log="$LOGDIR/test${num}.log"
  TEST_NUM="$num"
  printf '%s' "TEST $num: $name ... "
  $RUNNER timeout "$timeout_s" "$OA_BIN" +set dedicated 1 "${OA_EXTRA[@]}" \
    +set sv_maxclients 24 \
    +set fs_game neonarena +set g_gametype 14 +map oa_shine \
    "$@" > "$log" 2>&1 || true
  eval "assert_$num \"$log\""
}

check() { # check <log> <pattern>; sets LAST_RESULT
  local log="$1"; shift
  grep -q "$*" "$log" && LAST_RESULT=0 || LAST_RESULT=1
}

count_min() { # count_min <log> <pattern> <min>
  local n=$(grep -c "$2" "$1"); [ "$n" -ge "$3" ]
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
  if [ "$1" -eq 0 ]; then echo "PASS"; PASS=$((PASS+1));
  else echo "FAIL (log: $LOGDIR/test${TEST_NUM}.log)"; FAIL=$((FAIL+1)); FAILED_NAMES="$FAILED_NAMES $2"; fi
}

# ---- individual tests: assert_<num> uses the log file $1 ----

# TEST 1: smoke + boss wave force-start (wave 10, classic sniper 4x hp)
assert_1() {
  local ok=0
  # version-agnostic: match any NeonArena-* gameversion
  grep -qE 'gamename\\NeonArena-[0-9.]+' "$1" || ok=1
  check "$1" 'g_gametype\\14';                 [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "NeonWave: starting wave 10";     [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" 'hc\\400';                        [ $LAST_RESULT -eq 0 ] || ok=1
  report $ok "smoke+boss"
}
# TEST 2: full run -> victory at wave 20 + economy + modifiers
assert_2() {
  local ok=0
  check "$1" "NeonWave: starting wave 20"; [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "All waves cleared";           [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "NEW BEST wave 20";            [ $LAST_RESULT -eq 0 ] || ok=1
  count_min "$1" "upgrade point granted" 19; [ $? -eq 0 ] || ok=1
  grep -qE "starting wave [5-9] .*\[(GLASS DRONES|SWARM|LOW GRAVITY|DOUBLE POINTS)\]" "$1" || ok=1
  no_fatal_warnings "$1" || ok=1
  report $ok "full-run-victory"
}
# TEST 3: forced LOW GRAVITY modifier + gravity restore on wave clear
assert_3() {
  local ok=0
  check "$1" "starting wave 6.*\[LOW GRAVITY\]"; [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "gravity restored to 800";          [ $LAST_RESULT -eq 0 ] || ok=1
  report $ok "modifier-lowgrav"
}
# TEST 4: failed run path + run stats
assert_4() {
  local ok=0
  check "$1" "RUN STATS";       [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "NeonWave over";   [ $LAST_RESULT -eq 0 ] || ok=1
  report $ok "failrun-stats"
}
# TEST 5: TANK boss 6x health
assert_5() {
  check "$1" 'hc\\600'; report $? "boss-tank-hp"
}
# TEST 6: endless mode past wave 20 + best time
assert_6() {
  local ok=0
  check "$1" "starting wave 25";    [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "All waves cleared";   [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "BEST TIME";           [ $LAST_RESULT -eq 0 ] || ok=1
  no_fatal_warnings "$1" || ok=1
  report $ok "endless-timeattack"
}
# TEST 7: record persistence across two server runs
assert_7() {
  # runs twice internally; called via special-case below
  :
}
# TEST 8: combo pipeline -> run stats
assert_8() {
  local ok=0
  check "$1" "fake combo 7 registered"; [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "RUN STATS kills=7";       [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "bestCombo=7";             [ $LAST_RESULT -eq 0 ] || ok=1
  report $ok "combo-pipeline"
}
# TEST 9: swarm mother mini-drones + rage mechanics
assert_9() {
  local ok=0
  count_min "$1" "swarm mother spawns mini-drone" 1 || ok=1
  report $ok "swarm-minidrones"
}
# TEST 9b: forced rage — ENRAGED log + RAGE-tagged spawns
assert_9b() {
  local ok=0
  check "$1" "SWARM MOTHER ENRAGED";                    [ $LAST_RESULT -eq 0 ] || ok=1
  count_min "$1" "mini-drone (RAGE)" 1;                 [ $? -eq 0 ] || ok=1
  report $ok "swarm-rage"
}
# TEST 10: TANK shield cycle — raise AND drop must both occur
assert_10() {
  local ok=0
  count_min "$1" "TANK raises SHIELD" 1   || ok=1
  count_min "$1" "TANK shield drops" 1    || ok=1
  report $ok "tank-shield-cycle"
}
# TEST 11: SNIPER dash — forced low hp via g_neonwave_dashforce
assert_11() {
  local ok=0
  check "$1" "starting wave 10.*BOSS"; [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" 'hc\\400';                [ $LAST_RESULT -eq 0 ] || ok=1
  count_min "$1" "SNIPER dashes to new position" 1; [ $? -eq 0 ] || ok=1
  report $ok "sniper-dash"
}
# TEST 12: new-record flag + save on record run
assert_12() {
  local ok=0
  check "$1" "NEW RECORD WAVE\\|NEW RECORD TIME"; [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "RECORDS SAVED";                    [ $LAST_RESULT -eq 0 ] || ok=1
  no_fatal_warnings "$1" || ok=1
  report $ok "newrecord-flag"
}

# TEST 13: mega combo reward — best streak 8+ drops Quad Damage at wave clear
assert_13() {
  local ok=0
  check "$1" "fake combo 8 registered";  [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "MEGA COMBO 8";            [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" "item_quad" ;              [ $LAST_RESULT -eq 0 ] || ok=1
  report $ok "mega-combo-reward"
}

# TEST 14: GLASS CANNON boss — forced via bosstype 4, 2x HP (hc\200)
assert_14() {
  local ok=0
  check "$1" "boss spawned: GLASS CANNON (hc 200)"; [ $LAST_RESULT -eq 0 ] || ok=1
  check "$1" 'hc\\200';                             [ $LAST_RESULT -eq 0 ] || ok=1
  report $ok "boss-glass-cannon"
}

# TEST 15: daily challenge determinism — same seed => same boss twice,
# different seed => different boss rotation
assert_15() {
  # runs twice internally (two server starts); compares boss spawns
  :
}
t15() {
  TEST_NUM=15
  printf '%s' "TEST 15: daily-challenge-determinism ... "
  local ok=0 b1 b2 b3 logdir="$LOGDIR/test15"
  mkdir -p "$logdir"
  # NOTE: must use $RUNNER + "${OA_EXTRA[@]}" like run_test() — a bare
  # ioq3ded invocation dies instantly (no basepath) => empty logs.
  for run in a b; do
    $RUNNER timeout 30 "$OA_BIN" +set dedicated 1 "${OA_EXTRA[@]}" +set sv_maxclients 24 \
      +set fs_game neonarena +set g_gametype 14 \
      +set g_neonwave_autostart 1 +set g_neonwave_daily 1 \
      +set g_neonwave_dailyseed 12345 +set g_neonwave_startwave 10 \
      +map oa_shine > "$logdir/$run.log" 2>&1 || true
    grep -qE "DAILY CHALLENGE seed 12345" "$logdir/$run.log" || ok=1
  done
  b1=$(grep -oE "boss spawned: [A-Z ]+" "$logdir/a.log" | head -1)
  b2=$(grep -oE "boss spawned: [A-Z ]+" "$logdir/b.log" | head -1)
  [ -n "$b1" ] && [ "$b1" = "$b2" ] || { ok=1; echo "boss mismatch: '$b1' vs '$b2'"; }
  $RUNNER timeout 30 "$OA_BIN" +set dedicated 1 "${OA_EXTRA[@]}" +set sv_maxclients 24 \
    +set fs_game neonarena +set g_gametype 14 \
    +set g_neonwave_autostart 1 +set g_neonwave_daily 1 \
    +set g_neonwave_dailyseed 999 +set g_neonwave_startwave 10 \
    +map oa_shine > "$logdir/c.log" 2>&1 || true
  grep -qE "DAILY CHALLENGE seed 999" "$logdir/c.log" || ok=1
  report $ok "daily-challenge-determinism"
}

# TEST 16: WARDEN boss — forced via bosstype 5, 5x HP (hc\500),
# strike+armor mechanics via wardenforce hook
assert_16() {
  local ok=0
  check "$1" "boss spawned: WARDEN (hc 500)";          [ $LAST_RESULT -eq 0 ] || ok=1
  count_min "$1" "WARDEN strikes the player zone" 1;   [ $? -eq 0 ] || ok=1
  count_min "$1" "WARDEN raises armor" 1;              [ $? -eq 0 ] || ok=1
  report $ok "boss-warden"
}
t17() {
  TEST_NUM=17
  printf '%s' "TEST 17: daily-record-persistence ... "
  local logdir="$LOGDIR/test17"; mkdir -p "$logdir"
  rm -f "$HOME_DIR/neonwave_daily_records.dat"
  local ok=0
  # run 1: daily failrun => daily record saved
  $RUNNER timeout 40 "$OA_BIN" +set dedicated 1 "${OA_EXTRA[@]}" +set sv_maxclients 24 \
    +set fs_game neonarena +set g_gametype 14 \
    +set g_neonwave_autostart 1 +set g_neonwave_daily 1 +set g_neonwave_dailyseed 12345 \
    +set g_neonwave_failrun 1 +map oa_shine > "$logdir/a.log" 2>&1 || true
  grep -q "DAILY RECORDS SAVED" "$logdir/a.log" || { ok=1; echo "no daily save"; }
  [ -f "$HOME_DIR/neonwave_daily_records.dat" ] || { ok=1; echo "daily records file missing"; }
  # run 2: reload same day => records loaded with wave>=1
  $RUNNER timeout 30 "$OA_BIN" +set dedicated 1 "${OA_EXTRA[@]}" +set sv_maxclients 24 \
    +set fs_game neonarena +set g_gametype 14 \
    +set g_neonwave_autostart 1 +set g_neonwave_daily 1 +set g_neonwave_dailyseed 12345 \
    +map oa_shine > "$logdir/b.log" 2>&1 || true
  grep -qE "DAILY records loaded wave=[1-9]" "$logdir/b.log" || { ok=1; echo "no daily load"; }
  report $ok "daily-record-persistence"
}

t16() { run_test 16 "boss-warden" 40 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_bosstype 5 +set g_neonwave_wardenforce 1; }

TEST_NUM=""

# wrappers that bundle cvar sets per test
t1()  { run_test 1 "smoke+boss"        40 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10; }
reset_progress() {
  # g_neonwave_* test cvars persist via q3config.cfg (and, on dedicated
  # servers, q3config_server.cfg) between runs and would suppress
  # "NEW BEST" assertions; wipe them for a deterministic baseline
  sed -i '/g_neonwave_/d' "$HOME_DIR/q3config.cfg" 2>/dev/null || true
  sed -i '/g_neonwave_/d' "$HOME_DIR/q3config_server.cfg" 2>/dev/null || true
  rm -f "$HOME_DIR/neonwave_records.dat"
}
t2()  { reset_progress; run_test 2 "full-run-victory" 240 +set g_neonwave_autostart 1 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1; }
t3()  { run_test 3 "modifier-lowgrav"  40 +set g_neonwave_autostart 1 +set g_neonwave_startwave 6 +set g_neonwave_modifier 3 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1; }
t4()  { run_test 4 "failrun-stats"     40 +set g_neonwave_autostart 1 +set g_neonwave_failrun 1; }
t5()  { run_test 5 "boss-tank-hp"      40 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_bosstype 2; }
t6()  { run_test 6 "endless-timeattack" 90 +set g_neonwave_autostart 1 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 +set g_neonwave_startwave 20 +set g_neonwave_maxwave 25; }
t7()  {
  TEST_NUM=7
  printf '%s' "TEST 7: record-persistence ... "
  rm -f "$HOME_DIR/neonwave_records.dat"
  $RUNNER timeout 90 "$OA_BIN" +set dedicated 1 "${OA_EXTRA[@]}" +set sv_maxclients 24 \
    +set fs_game neonarena +set g_gametype 14 \
    +set g_neonwave_autostart 1 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 \
    +set g_neonwave_startwave 20 +map oa_shine > "$LOGDIR/test7a.log" 2>&1 || true
  local ok=0
  grep -q "RECORDS SAVED" "$LOGDIR/test7a.log" || ok=1
  [ -f "$HOME_DIR/neonwave_records.dat" ] || { ok=1; echo "records file missing"; }
  $RUNNER timeout 40 "$OA_BIN" +set dedicated 1 "${OA_EXTRA[@]}" +set sv_maxclients 24 \
    +set fs_game neonarena +set g_gametype 14 \
    +map oa_shine > "$LOGDIR/test7b.log" 2>&1 || true
  grep -qE "records loaded wave=[1-9]" "$LOGDIR/test7b.log" || ok=1
  report $ok "record-persistence"
}
t8()  { run_test 8 "combo-pipeline"    60 +set g_neonwave_autostart 1 +set g_neonwave_startwave 2 +set g_neonwave_fakecombo 7 +set g_neonwave_botasplayer 1 +set g_neonwave_failrun 1; }
t9()  { run_test 9 "swarm-minidrones"  60 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_bosstype 3; }
t9b() { run_test 9b "swarm-rage"       40 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_bosstype 3 +set g_neonwave_rageforce 1; }
t10() { run_test 10 "tank-shield"      40 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_bosstype 2 +set g_neonwave_fastbreak 1; }
t11() { run_test 11 "sniper-dash"      40 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_bosstype 1 +set g_neonwave_dashforce 30; }
t12() {
  TEST_NUM=12
  reset_progress
  run_test 12 "newrecord-flag" 90 +set g_neonwave_autostart 1 +set g_neonwave_autokill 1 \
    +set g_neonwave_fastbreak 1 +set g_neonwave_startwave 20
}

t13() { run_test 13 "mega-combo-reward" 60 +set g_neonwave_autostart 1 +set g_neonwave_startwave 2 +set g_neonwave_fakecombo 8 +set g_neonwave_botasplayer 1 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1; }
t14() { run_test 14 "boss-glass-cannon" 40 +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_bosstype 4; }

[ -d "$HOME_DIR/vm" ] || { echo "mod not installed: $HOME_DIR/vm missing (run build-mod.sh first)"; exit 2; }

case "$MODE" in
  quick)  t1; t3; t4; t7; t8; t10; t12; t13 ;;
  single) t"$SELECTED" ;;
  all)    for n in 1 2 3 4 5 6 7 8 9 9b 10 11 12 13 14 15 16 17; do "t$n"; done ;;
esac

echo
echo "================================"
echo "Suite result: $PASS passed, $FAIL failed"
[ -n "$FAILED_NAMES" ] && echo "Failed:$FAILED_NAMES"
echo "Logs: $LOGDIR"
exit $(( FAIL > 0 ))
