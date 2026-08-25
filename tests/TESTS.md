# NeonArena Test Suite — Test-Katalog

Headless-Suite `tests/run_suite.sh`, ausgeführt als ioq3ded (dedicated), mit
Test-Hooks via `g_neonwave_*` Cvars.

## Konventionen

- Jeder Test erzeugt ein Log `testN.log` in `/tmp/nw-suite-*/`.
- Assertions prüfen einerseits **erwünschte Marker**, andererseits
  **unerwünschte Marker** (Anti-Patterns).
- Neue Tests werden mit Nummer und Kurzname eingetragen; die Nummer
  darf nicht kollidieren.

## Test-Katalog

| # | Name | CVars | Erwünschte Marker (Auszug) | Anti-Patterns |
|---|------|-------|----------------------------|---------------|
| 1 | smoke+boss | autostart, startwave 10 | `gamename\\NeonArena-*`, `g_gametype\\14`, `starting wave 10`, `hc\\400` | keine Pattern |
| 2 | full-run-victory | autostart, autokill, fastbreak | `starting wave 20`, `All waves cleared`, `NEW BEST wave 20`, ≥19 `upgrade point granted` | keine `WARNING`, keine `Error`, keine `GameOver.*FAILED` |
| 3 | modifier-lowgrav | autostart, startwave 6, modifier 3 (LOWGRAV) | `starting wave 6.*\[LOW GRAVITY\]`, `gravity restored to 800` | keine anderen Modifier-Namen im selben Wave |
| 4 | failrun-stats | autostart, failrun | `RUN STATS`, `NeonWave over` | kein `All waves cleared` |
| 5 | boss-tank-hp | autostart, startwave 10, bosstype 2 | `hc\\600` | keine anderen Boss-Namen |
| 6 | endless-timeattack | autostart, autokill, fastbreak, startwave 20, maxwave 25 | `starting wave 25`, `All waves cleared`, `BEST TIME` | keine unerwarteten Wave-Skips |
| 7 | record-persistence | (intern: autostart, autokill, fastbreak, startwave 20) | `RECORDS SAVED`, `records loaded wave=[1-9]`, Datei `neonwave_records.dat` vorhanden | keine Lesefehler |
| 8 | combo-pipeline | autostart, startwave 2, fakecombo 7, botasplayer 1, failrun | `fake combo 7 registered`, `RUN STATS kills=7`, `bestCombo=7` | keine anderen Combo-Zahlen im Payload |
| 9 | swarm-minidrones | autostart, startwave 10, bosstype 3 | ≥1 `swarm mother spawns mini-drone` | keine RAGE-Tags (ohne rageforce) |
| 9b | swarm-rage | autostart, startwave 10, bosstype 3, rageforce 1 | `SWARM MOTHER ENRAGED`, ≥1 `mini-drone (RAGE)` | keine anderen Boss-Type-Marker |
| 10 | tank-shield-cycle | autostart, startwave 10, bosstype 2, fastbreak | je ≥1 `TANK raises SHIELD`, `TANK shield drops` | nur eine Seite des Zyklus |
| 11 | sniper-dash | autostart, startwave 10, bosstype 1, dashforce 30 | `starting wave 10.*BOSS`, `hc\\400`, ≥1 `SNIPER dashes to new position` | kein Dash ohne niedrigen HP (wenn Hook korrekt) |
| 12 | newrecord-flag | autostart, autokill, fastbreak, startwave 20 | `NEW RECORD WAVE\|NEW RECORD TIME`, `RECORDS SAVED` | keine alten Bestwerte unterdrücken |
| 13 | mega-combo-reward | autostart, startwave 2, fakecombo 8, botasplayer 1, autokill, fastbreak | `fake combo 8 registered`, `MEGA COMBO 8`, `item_quad` | kein Quad bei kleinerem Combo |
| 14 | boss-glass-cannon | autostart, startwave 10, bosstype 4 | `boss spawned: GLASS CANNON (hc 200)`, `hc\\200` | kein anderer Boss-Type |

## Ergänzung: Konfigurations-Pattern für Configstring-CsPayload

Der Configstring `CS_NEONWAVE` hat folgendes Format:

    <wave> <event> <bossHp> <bossMax> <breakMs> <pts> <best> <mod> <kills> <bestCombo> <runSec> <liveCombo>

Wir können Tests aufbauen, die die Payload parsen statt nur Einzel-Greps.

## Anti-Pattern-Asserts (global, nicht nur einzelne Tests)

Tests, die Victory/Keine-Warnungen erwarten, prüfen zusätzlich:

- `assert_no_pattern "$log" "WARNING"` (oder spezifischer: `WARNING cannot write`)
- `assert_no_pattern "$log" "ERROR"`
- `assert_no_pattern "$log" "GameOver.*FAILED"` (wenn eine Victory erwartet wird)

## Ergänzung: Test-Katalog pflegen

Jeder neue Test wird hier eingetragen (Nummer, Name, CVars, Erwünschte Marker,
Anti-Patterns). Bestehende Tests werden nur geändert, wenn ihre Assertions
sich diagnostisch ändern.
