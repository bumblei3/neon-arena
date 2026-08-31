# NeonArena parallel test suite — canonical mechanics

This documents the WORKING `--parallel N` implementation (v0.25). It supersedes any
inline "recursive `./run_suite.sh --test <list>`" description. Reproduce from here.

## Component split
- `tests/run_suite.sh` — entry point. Modes: `all` (default), `quick`, `single` (`--test N`
  or `--test "1 3"`), `parallel` (`--parallel N`).
- `tests/run_parallel.sh` — helper invoked by the parallel mode. Takes test numbers as args
  and runs `run_suite.sh --test "$t"` per arg, with a unique `NW_LOGDIR_PREFIX` each time.

## Why the helper exists (the recursion trap)
`run_suite.sh` computes `LOGDIR="/tmp/nw-suite-$(date +%Y%m%d-%H%M%S)"`. If it recurses
into `run_suite.sh --test N`, two recursive calls in the same second share one LOGDIR → their
`testN.log` files clobber each other → vacuous pass or timeout. The helper exports a unique
prefix so each recursive instance writes elsewhere:
```bash
export NW_LOGDIR_PREFIX="/tmp/nw-parallel-$$/test-${t}"
"$TESTDIR/run_suite.sh" --test "$t"
```
And `run_suite.sh` honors it:
```bash
if [ -n "${NW_LOGDIR_PREFIX:-}" ]; then LOGDIR="${NW_LOGDIR_PREFIX}"; else LOGDIR="/tmp/nw-suite-$(date ...)"; fi
```

## Per-test isolation inside run_test()
```bash
local hp="$HOME/.openarena-nwtest/${num}"   # NOT /tmp — ioq3ded refuses write-protected homepaths
mkdir -p "$hp"
if [ ! -e "$hp/neonarena" ]; then ln -s "$HOME/.openarena/neonarena" "$hp/neonarena"; fi
local port=$(( 27970 + ( num % 80 ) ))
# in the ioq3ded invocation, add:  +set fs_homepath "$hp" +set net_port "$port"
# DO NOT add +set fs_basepath — OA_EXTRA already sets /usr/lib/openarena (baseoa assets)
```
Without the symlink the server loads `baseoa` and prints `g_gametype 14 is out of range,
defaulting to 0`. Without `$HOME` (i.e. `/tmp`) it prints `fs_homepath is write protected.`

## Parallel driver (in the `parallel)` case of run_suite.sh)
```bash
nprocs=${PARALLEL:-4}
slot_pids=""                       # init! set -u aborts on unset
# shard ALL_TESTS into N buckets (echo $t >> $bdir/bucket.$slot)
for slot in $(seq 0 $(( nprocs - 1 ))); do
  if [ -f "$bdir/bucket.$slot" ]; then
    blist=$(tr '\n' ' ' < "$bdir/bucket.$slot")
    ( "$TESTDIR/run_parallel.sh" $blist ) > "$bdir/slot.$slot.log" 2>&1 &
    slot_pids="$slot_pids $!"
  fi
done
for p in $slot_pids; do wait "$p"; done   # capture $! and wait per pid
# aggregate from per-test lines (NOT the slot's "Suite result" line)
for sl in "$bdir"/slot.*.log; do
  while IFS= read -r line; do
    case "$line" in
      *"... PASS") PASS=$(( ${PASS:-0} + 1 )) ;;
      *"... FAIL"*) FAIL=$(( ${FAIL:-0} + 1 )) ;;
    esac
  done < "$sl"
done
```
Do NOT use `local` inside the `parallel)` case — it runs at top level, not in a function
(`local: can only be used within a function`).

## Aggregation gotcha
A shard running 2 tests prints `Suite result: 1 passed` (last test only) TWICE. Summing the
slot `Suite result` lines UNDERCOUNTS (4 tests → reported 2). Always tally the per-test
`TEST N: ... PASS` / `TEST N: ... FAIL` lines instead.

## Smoke + full verification
- Smoke: `./tests/run_suite.sh --parallel 2 --test "1 3 17 21"`
- Full (background, notify): `./tests/run_suite.sh --parallel 4` → must end `N passed, 0 failed`
  where N = count of ALL_TESTS entries. Lower count = missing `dispatch_test()` case, not a
  parallel bug.
- Always `bash -n tests/run_suite.sh tests/run_parallel.sh` after edits.
