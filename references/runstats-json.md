# Run-stats JSON export + achievements (NeonArena)

The mod writes a per-run summary at game over (`NW_GameOver`, both VICTORY and
FAILED paths). This is the data contract the `tools/neon-stats.py` dashboard
consumes.

## `neonwave_runstats.json` (overwritten every game over)
Written by `NW_WriteRunStats(event)` to the server `fs_homepath`
(`~/.openarena/neonarena/` for the headless test server).

```json
{
  "version": 1,
  "result": "VICTORY" | "FAILED",
  "wave": <int>,
  "kills": <int>,
  "bestCombo": <int>,
  "timeSec": <int>,
  "difficulty": "NORMAL" | "EASY" | "HARD" | "RELAX",
  "modifiersSeen": <int bitmask>,
  "modifierNames": [ "<name>", ... ],
  "achievements": [ "<name>", ... ]
}
```

Schema notes:
- `modifiersSeen` — bitmask; bit `i` set if modifier `i` occurred this run.
  `modifierNames` is the human-readable expansion (order-independent).
- `achievements` — per-run badges earned this run (subset of the `NW_ACH_*`
  names). NOT the lifetime-unlocked set.
- The file is **regenerated on every game over**. A dedicated server restarts
  the map and re-runs `autostart` after game over, so the file on disk at the
  end of a headless harness run reflects the LAST (usually wave-1) run, not a
  forced high-wave run. **Do not assert `wave`/`result`/`achievements` values
  from the file for a forced-wave test** — assert via log markers instead
  (see below). For a real player session this is exactly "last completed run",
  which is the intended dashboard input.

## `neonwave_achievements.dat` (lifetime, persistent)
Raw array of `NW_ACH_COUNT` qbooleans written by `NW_SaveAchievements()`
(`trap_FS_Write` of the fixed-size `nw_achEver[]` array).

**Parsing from Python** (`tools/neon-stats.py` does this):
- `qboolean` in this codebase is `int` → **4 bytes, little-endian** per entry.
- Read `N*4` bytes; entry `i` is `int.from_bytes(buf[i*4:i*4+4], "little")`;
  `!= 0` ⇒ unlocked.
- Entry ORDER must match the `NW_ACH_*` `#define` block in `g_neonwave.c`:
  0 FIRST VICTORY, 1 SURVIVOR, 2 SHARPSHOOTER, 3 STREAKER, 4 FLAWLESS.
  If you add/renumber an achievement, update `ACHIEVEMENT_NAMES` in
  `tools/neon-stats.py` in lockstep.

## Log markers (the deterministic CI assertion path)
`NW_CheckAchievements(event)` emits at game over:
- `NeonWave: ACHIEVEMENT <NAME>` — every run the badge is earned (CI-assertable).
- `NeonWave: ACHIEVEMENT UNLOCKED <NAME>` — **only when `!nw_achEver[i]`**
  (first time ever). Suppressed on later runs once the dat records it.

Test 19 asserts `ACHIEVEMENT SURVIVOR` (wave≥15 forced) via the log, NOT the
JSON, precisely because of the map-restart clobber.

## Test 18 recipe (runstats-json)
- `assert_18()` in `tests/run_suite.sh`: greps `RUN STATS JSON written`, checks
  the file exists, then `python3 -c "import json; json.load(open(...))"` and
  asserts required keys + `result in (VICTORY, FAILED)` + `modifierNames` and
  `achievements` are lists.
- dispatch: `18) rm -f .../neonwave_runstats.json; run_test 18 "runstats-json" 90
  +set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_failrun 1 ;;`
- Added to `ALL_TESTS` and `QUICK_TESTS`.
- FULL suite must report `20 passed, 0 failed` (was 18 before 18/19 were added).

## C gotchas that bit this feature (keep in mind)
- `Com_sprintf` returns **void** here → build JSON with a fixed `char buf[1024]`
  + helper buffer (`mods[256]`, `achs[256]`) built by successive
  `Com_sprintf(buf+strlen(buf), ...)`, then one final embed. Never capture its
  return into an `int`.
- `#define NW_FOO\t"x"` — the patch tool doubles the backslash into a literal
  `\t` that breaks the preprocessor. Use a **space**: `#define NW_FOO "x"`.
- `NW_LoadAchievements` is called from `NeonWave_Reset` before its definition →
  needs a forward declaration near the other `static void NW_*()` prototypes,
  or `-Werror-implicit-function-declaration` fails the build.
