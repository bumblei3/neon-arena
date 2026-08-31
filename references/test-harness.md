# NeonArena test harness — full catalog, cvars, parse helpers

## Test catalog (from tests/TESTS.md, cvars as passed via `+set g_neonwave_*`)
| # | name | cvars | key markers |
|---|-------|-------|-------------|
| 1 | smoke+boss | autostart, startwave 10 | `starting wave 10`, `hc\400` |
| 2 | full-run-victory | autostart, autokill, fastbreak | `starting wave 20`, `All waves cleared`, `NEW BEST wave 20` |
| 3 | modifier-lowgrav | autostart, startwave 6, modifier 3 | `starting wave 6.*\[LOW GRAVITY\]`, `gravity restored to 800` |
| 4 | failrun-stats | autostart, failrun | `RUN STATS`, `NeonWave over` |
| 5 | boss-tank-hp | autostart, startwave 10, bosstype 2 | `hc\600` |
| 6 | endless-timeattack | autostart, autokill, fastbreak, startwave 20, maxwave 25 | `starting wave 25`, `All waves cleared`, `BEST TIME` |
| 7 | record-persistence | autostart, autokill, fastbreak, startwave 20 | `RECORDS SAVED`, `records loaded wave=[1-9]` |
| 8 | combo-pipeline | autostart, startwave 2, fakecombo 7, botasplayer 1, failrun | `fake combo 7 registered`, `RUN STATS kills=7`, `bestCombo=7` |
| 9 | swarm-minidrones | autostart, startwave 10, bosstype 3 | ≥1 `swarm mother spawns mini-drone` |
| 9b | swarm-rage | autostart, startwave 10, bosstype 3, rageforce 1 | `SWARM MOTHER ENRAGED`, ≥1 `mini-drone (RAGE)` |
| 10 | tank-shield-cycle | autostart, startwave 10, bosstype 2, fastbreak | ≥1 `TANK raises SHIELD`, `TANK shield drops` |
| 11 | sniper-dash | autostart, startwave 10, bosstype 1, dashforce 30 | `SNIPER dashes to new position` |
| 12 | newrecord-flag | autostart, autokill, fastbreak, startwave 20 | `NEW RECORD WAVE\|NEW RECORD TIME`, `RECORDS SAVED` |
| 13 | mega-combo-reward | autostart, startwave 2, fakecombo 8, botasplayer 1, autokill, fastbreak | `fake combo 8 registered`, `MEGA COMBO 8`, `item_quad` (FLAKY — retry) |
| 14 | boss-glass-cannon | autostart, startwave 10, bosstype 4 | `boss spawned: GLASS CANNON (hc 200)` |
| 15 | daily-challenge-determinism | daily 1, dailyseed 12345/999, startwave 10 | 2× `DAILY CHALLENGE seed N`, identical boss spawn per seed |
| 16 | boss-warden | autostart, startwave 10, bosstype 5, wardenforce 1 | `WARDEN strikes the player zone`, `WARDEN raises armor` |
| 17 | timewarp-modifier | autostart, startwave 6, modifier 5, fastbreak, autokill | `starting wave 6.*\[TIME WARP\]`, `g_speed changed to 520`, `g_speed changed to 320` |
| 18 | runstats-json | autostart, startwave 10, failrun | `RUN STATS JSON written`; file `neonwave_runstats.json` exists + valid JSON (version=1, result in VICTORY/FAILED, modifierNames[] AND achievements[] are lists) |
| 19 | achievements-json | autostart, startwave 15, failrun | `RUN STATS JSON written`; log marker `ACHIEVEMENT SURVIVOR` (wave>=15). NOTE: assert via LOG MARKER, not JSON contents — the dedi server's map-restart loop overwrites the JSON with the wave-1 restart. `ACHIEVEMENT UNLOCKED` fires only the first time ever (persistent dat). |

## Payload parse helpers (tests/cs_neonwave_parse.sh)
- `parse_cs_neonwave <log>` — parses the LAST `NEONWAVE_PAYLOAD` line (DANGER: last wave is boss, mod=0).
- `parse_cs_neonwave_at <log> <wave>` — parses the FIRST payload of that exact wave. USE THIS for
  forced-modifier / specific-wave assertions.
- After either, vars: `CS_WAVE CS_EVENT CS_BOSSHP CS_BOSSMX CS_BREAKTIME CS_PTS CS_BEST CS_MOD
  CS_KILLS CS_BESTCOMBO CS_RUNSEC CS_LIVECOMBO`.
- `modifier_name <n>` maps 0..5 → NONE/GLASS DRONES/SWARM/LOW GRAVITY/DOUBLE POINTS/TIME WARP.
  Extend both the `#define NW_MOD_*` list in g_neonwave.c AND this map when adding a modifier.

## Flaky / known issues
- Test 13 (mega-combo): spawn-fallback race under headless CI timing. `run_test` retries it once
  via `FLAKY_TESTS="13"`. Don't remove from `--quick`; the retry is the fix.
- Test 10 (tank-shield-cycle): TANK shield cycle is TIME-based (12s after spawn). `autokill` kills
  the boss before the cycle → no markers. Runs WITHOUT autokill, relies on the 90s timeout. No
  payload assertion (no clean wave-end payload on timeout-kill).
- `--test N` (single mode) had an EMPTY cvar map historically → ran with NO cvars, failing every
  test. The map is now complete (tests 1-44 incl 9b). Keep it in sync when adding tests.

## Automatische Katalogprüfung (verify_catalog.py)

Vor jedem Testlauf (lokal und in CI) wird `tests/verify_catalog.py` ausgeführt.
Das Skript prüft:
- Jede Nummer aus `ALL_TESTS` und `QUICK_TESTS` in `tests/run_suite.sh` hat eine
  entsprechende `dispatch_test()`-Case (Hard-Check, exit 1 if missing → FAIL).
- Jeder `dispatch_test()`-Case ist in `ALL_TESTS` (Warnung bei waisen Cases).
- Jeder Eintrag aus `ALL_TESTS` ist in `tests/TESTS.md` dokumentiert
  (INFO default, wird zu FAIL bei `--strict`).

Aufruf lokal: `python3 tests/verify_catalog.py`. Bei Inkonsistenz bricht das Skript
mit Fehlercode 1 ab und listet die Diskrepanzen auf. In CI läuft es als Pre-Flight-
Schritt vor der Testsuite (`.github/workflows/build-mod.yml`). Es verhindert, dass
ein inkonsistenter Katalog die Suite verfälscht (stille Auslassung von Tests).
