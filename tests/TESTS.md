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
| 15 | daily-challenge-determinism | daily 1, dailyseed 12345/999, startwave 10 | je 2× `DAILY CHALLENGE seed N`, `DAILY MAP oa_pulse` (seed 12345 → Index 5) | Boss-Mismatch zwischen Läufen |
| 16 | boss-warden | autostart, startwave 10, bosstype 5, wardenforce 1 | `boss spawned: WARDEN (hc 500)`, ≥1 `WARDEN strikes the player zone`, ≥1 `WARDEN raises armor` | kein anderer Boss-Type |
| 17 | timewarp-modifier | autostart, startwave 6, modifier 5 (TIME WARP), fastbreak, autokill | `starting wave 6.*\[TIME WARP\]`, payload mod=5 | keine Fatal-Warnung |
| 18 | runstats-json | autostart, startwave 10, failrun | `RUN STATS JSON written`, Datei `neonwave_runstats.json` existiert + valides JSON (version=1, result in VICTORY/FAILED, modifierNames=list, achievements=list) | keine Write-Warnung |
| 19 | achievements-json | autostart, startwave 15, failrun | `RUN STATS JSON written`, Log-Marker `ACHIEVEMENT SURVIVOR` (wave≥15) | keine doppelten Marker-Issues |
| 20 | hardcore-mode | autostart, startwave 10, bosstype 2, hardcore 1 | `HARDCORE mode enabled`, `HARDCORE` banner, `boss spawned: TANK (hc 900)` (600×1.5) | keine Write-/Fatal-Warnung |
| 21 | vampire-lifesteal | autostart, startwave 6, modifier 6 (VAMPIRE), botasplayer 1, autokill 1, fastbreak 1 | `starting wave 6.*\[VAMPIRE\]`, ≥1 `VAMPIRE lifesteal`, payload mod=6 | keine Fatal-Warnung |
| 22 | frenzy-quadfactor | autostart, startwave 6, modifier 7 (FRENZY), autokill 1, fastbreak 1 | `starting wave 6.*\[FRENZY\]`, ≥1 `FRENZY quadfactor set to 4` | keine Fatal-Warnung |
| 23 | overshield-armor | autostart, startwave 6, modifier 8 (OVERSHIELD), botasplayer 1, autokill 1, fastbreak 1 | `starting wave 6.*\[OVERSHIELD\]`, ≥1 `OVERSHIELD +50 armor granted` | keine Fatal-Warnung |
| 24 | speedrunner-ach | autostart, autokill, fastbreak, startwave 20 | `All waves cleared`, `ACHIEVEMENT SPEEDRUNNER` | keine Fatal-Warnung |
| 25 | hardcore-ach | autostart, autokill, fastbreak, startwave 20, hardcore 1 | `HARDCORE mode enabled`, `All waves cleared`, `ACHIEVEMENT HARDCORE` | keine Fatal-Warnung |
| 26 | combomaster-ach | autostart, startwave 2, fakecombo 12, botasplayer 1, failrun | `fake combo 12 registered`, `ACHIEVEMENT COMBOMASTER` | keine Fatal-Warnung |
| 27 | perk-offer | autostart, startwave 5, autokill, fastbreak | `PERK OFFER F1=` | keine Fatal-Warnung |
| 28 | perk-pierce-pick | startwave 5, perkforce 123, autopick 1 | `PERK OFFER F1=PIERCE`, `PERK TAKEN PIERCE` | keine Fatal-Warnung |
| 29 | perk-skip-mod | startwave 5, perkforce 612, autopick 1 | `PERK TAKEN SKIP`, `SKIP modifier`, `starting wave 6 (` | keine Fatal-Warnung |
| 30 | mirror-reflect | startwave 6, modifier 9 (MIRROR), botasplayer 1, autokill 1, fastbreak 1 | `starting wave 6.*[MIRROR]`, `g_neonwave_modifier_active.*9` | keine Fatal-Warnung |
| 31 | regen-health | startwave 6, modifier 10 (REGEN), botasplayer 1, autokill 1, fastbreak 1 | `starting wave 6.*[REGEN]`, `NeonWave: REGEN health topped up` | keine Fatal-Warnung |
| 32 | surge-points | startwave 6, modifier 11 (SURGE), botasplayer 1, autokill 1, fastbreak 1 | `starting wave 6.*[SURGE]`, `NeonWave: SURGE drones hardened`, `NeonWave: SURGE x3 upgrade points` | keine Fatal-Warnung |
| 33 | codex-bestiary | startwave 6, codex 1 | `g_neonwave_codex_rendered.*1` | keine Fatal-Warnung |
| 34 | tank-phase2 | startwave 10, bosstype 2 (TANK), phaseforce 1 | `boss spawned: TANK`, `NeonWave: TANK ENTERS PHASE 2`, `TANK raises SHIELD (PHASE 2)` | keine Fatal-Warnung |
| 35 | swarm-phase2 | startwave 10, bosstype 3 (SWARM MOTHER), phaseforce 1 | `boss spawned: SWARM MOTHER`, `NeonWave: SWARM MOTHER ENTERS PHASE 2`, `swarm mother spawns mini-drone.*PHASE 2` | keine Fatal-Warnung |
| 36 | warden-phase2 | startwave 10, bosstype 5 (WARDEN), phaseforce 1 | `boss spawned: WARDEN`, `NeonWave: WARDEN ENTERS PHASE 2`, `WARDEN strikes the player zone` | keine Fatal-Warnung |
| 37 | sniper-phase2 | startwave 10, bosstype 1 (SNIPER), phaseforce 1 | `boss spawned: SNIPER`, `NeonWave: SNIPER ENTERS PHASE 2`, `SNIPER dashes to new position` | keine Fatal-Warnung |
| 38 | glass-phase2 | startwave 10, bosstype 4 (GLASS CANNON), phaseforce 1 | `boss spawned: GLASS CANNON`, `NeonWave: GLASS CANNON ENTERS PHASE 2`, `GLASS CANNON summons support drone` | keine Fatal-Warnung |
| 39 | pierce-rank | autostart, startwave 6, perkr 1, fastbreak 1 | `PERK RANK PIERCE rank 1`, `PERK EFFECT PIERCE` (bzw. korrekter Marker für Rank-1-Pierce) | keine Fatal-Warnung |
| 40 | overcharge-rank | autostart, startwave 6, perkr 4, fastbreak 1 | `PERK RANK OVERCHARGE rank 2`, `PERK EFFECT OVERCHARGE` (bzw. korrekter Marker für Rank-2-Overcharge) | keine Fatal-Warnung |
| 41 | synergy-pair | autostart, startwave 6, modifier 3 (LOWGRAV), modifier2 4 (DOUBLE POINTS), fastbreak 1 | `NeonWave: SYNERGY AERIAL ASSAULT`, `SYNERGY EFFECT AERIAL ASSAULT: gravity 280, points x3` | keine Fatal-Warnung |
| 42 | anti-synergy-pair | autostart, startwave 6, modifier 8 (OVERSHIELD), modifier2 6 (VAMPIRE), fastbreak 1 | `NeonWave: ANTI-SYNERGY SHIELD BLEED`, `ANTI-SYNERGY EFFECT SHIELD BLEED: overshield 25, lifesteal 2`, `OVERSHIELD +25 armor granted` | keine Fatal-Warnung |
| 43 | mirror-slot2 | startwave 6, modifier2 9 (MIRROR in Slot 2), fastbreak 1 | `starting wave 6.*\\[MIRROR\\]`, `NeonWave: MIRROR active (mask .*, slot2=1)` | keine Fatal-Warnung |
| 44 | difficulty-lock-daily | autostart, daily 1, dailyseed 1, startwave 6, autokill, fastbreak | `dynamic difficulty locked (daily=1 hardcore=0)` | `dynamic difficulty ->` |
| 45 | coop-wave-clear | autostart, startwave 3, autokill, fastbreak, coopmock 1, botasplayer 1 | `starting wave 3`, `All waves cleared` | keine Fatal-Warnung |
| 46 | coop-respawn | autostart, startwave 3, autokill, fastbreak, selfkill 1, botasplayer 1 | `starting wave 3`, `COOP RESPAWN revived dead human` | keine Fatal-Warnung |
| 47 | coop-scaling | autostart, startwave 5, autokill, fastbreak, coopmock 1, coopdifficulty 2, botasplayer 1 | `starting wave 5`, `COOP scale 2 humans`, `All waves cleared` | keine Fatal-Warnung |
| 48 | frost-modifier | autostart, startwave 6, modifier 12, botasplayer 1, autokill, fastbreak | `starting wave 6.*\\[FROST\\]`, `FROST slowed to 220` | keine Fatal-Warnung |
| 49 | chaos-modifier | autostart, startwave 5, modifier 13, botasplayer 1, autokill, fastbreak | `starting wave 5.*\\[CHAOS\\]`, `CHAOS mode — random skill per drone`, `Drone W5-1 CHAOS` | keine Fatal-Warnung |
| 50 | berserker-boss | autostart, startwave 15, bosstype 6, rageforce 1, fastbreak | `starting wave 15.*BOSS`, `boss spawned: BERSERKER`, `BERSERKER ENTERS RAGE`, `hc\\700` | keine Fatal-Warnung |
| 51 | teleporter-boss | autostart, startwave 16, bosstype 7, fastbreak | `starting wave 16.*BOSS`, `boss spawned: TELEPORTER`, `TELEPORTER blinks to new position` | keine Fatal-Warnung |
| 52 | mimic-modifier | autostart, startwave 6, modifier 14, botasplayer 1, autokill, fastbreak | `starting wave 6.*\\[MIMIC\\]`, `MIMIC copied` | keine Fatal-Warnung |
| 53 | berserker-phase2 | autostart, startwave 15, bosstype 6, phaseforce 1, fastbreak, autokill | `boss spawned: BERSERKER`, `NeonWave: BERSERKER ENTERS PHASE 2` | keine Fatal-Warnung |
| 54 | teleporter-phase2 | autostart, startwave 16, bosstype 7, phaseforce 1, fastbreak, autokill | `boss spawned: TELEPORTER`, `NeonWave: TELEPORTER ENTERS PHASE 2` | keine Fatal-Warnung |
| 55 | modifier-interaction | autostart, startwave 6, modifier 10, modifier2 12, botasplayer 1, autokill, fastbreak | `starting wave 6.*\\[REGEN\\]`, `starting wave 6.*\\[FROST\\]`, `NeonWave: REGEN health topped up`, `FROST slowed to` | keine Fatal-Warnung |
| 56 | daily-records | daily 1, dailyseed 12345, autostart, autokill, fastbreak, startwave 10 | `DAILY CHALLENGE seed 12345`, `DAILY RECORDS SAVED` | keine Fatal-Warnung |
| 57 | upgrade-cost-escalation | autostart, autokill, fastbreak, startwave 10 | `UPGRADE:` | keine Fatal-Warnung |
| 58 | boss-hp-wave-scaling | autostart, startwave 15, bosstype 2, autokill | `boss spawned:.*hc` | keine Fatal-Warnung |
| 59 | wave-select | autostart, startwave 7, autokill, fastbreak | `starting wave 7` | keine Fatal-Warnung |
| 60 | coop-spectator | autostart, startwave 5, coopmock 1, botasplayer 1, autokill, fastbreak | `starting wave 5` | keine Fatal-Warnung |

### Tabelle-Notizen

- **Test 9b** ist kein eigener Boss-Type-Test; er ist die RAGE-Variante von Test 9 (SWARM MOTHER). Der Hook `rageforce 1` erzwingt den Enrage-Zustand des Mutter-Drones. Es wird erwartet, dass der Boss nach dem Trigger `SWARM MOTHER ENRAGED` ausgibt und danach `mini-drone (RAGE)` spawnt. Test 9 (ohne rageforce) als Basis-Kontrolle; 9b zeigt den Enrage-Pfad.
- **Tests 39 und 40** prüfen jeweils einen bestimmten Perk-Rank (PIERCE = Perk-ID 1, OVERCHARGE = Perk-ID 4). Der Hook `perkr N` setzt am Run-Start den Rank des angegebenen Perks. Die korrekten Marker hängen von der Implementierung in `g_neonwave.c` ab; der Test verlangt mindestens eine Bestätigung, dass der Rank gesetzt wurde (z.B. `PERK RANK PIERCE rank 1`).
- **Tests 41 und 42** prüfen die Synergie-/Anti-Synergie-Paarungslogik (2. Slot ab Welle 8, erzwungen auch auf Welle 6). v0.37 verlangt neben dem Namen eine **mechanische** Wirkung: AERIAL ASSAULT setzt gravity 280 / points x3, SHIELD BLEED halbiert Overshield (25) und Lifesteal (2).
- **Test 44** stellt sicher, dass Daily (und Hardcore über denselben Helper) Adaptive Difficulty lockt — zwei Wave-Clears dürfen `dynamic difficulty ->` nicht ausgeben.
- **Test 43** zeigt den MIRROR-Modifier im 2. Slot (Slot 2). Der Hook `modifier2` setzt einen zweiten Modifier; bei MIRROR in Slot 2 erwartet der Test den Marker `slot2=1` im MIRROR-Status.

## Ergänzung: v0.33 Boss-Phasenwechsel (PHASE 2)

Jeder Boss wechselt bei ≤50% HP in Phase 2 (sichtbarer Marker `NeonWave: <NAME> ENTERS PHASE 2`)
und eskaliert: TANK kürzere/öftere Shield-Cycles, SWARM schnellere Mini-Drone-Spawns,
WARDEN kürzere Strike-Intervalle, SNIPER öftere Reposition, GLASS CANNON ruft Support-Drones.
Test-Hook `g_neonwave_phaseforce 1` erzwingt den Trigger deterministisch für CI (Tests 34–38).
`nw_bossPhase` (1→2) wird in `NeonWave_Reset` zurückgesetzt.

## Ergänzung: v0.32 Deck-Expansion + Codex (MIRROR/REGEN/SURGE)

Modifier-Pool wuchs von 8 auf 11. Rotation in `NW_PickModifier` ist `(num-5+nw_dailyOffset) % 11`
(daily verschiebt den Startindex). MIRROR reflektiert 1/3 des Bot→Human-Schadens zurück
(gehookt in `G_Damage`, g_combat.c, liest `g_neonwave_modifier_active`). REGEN füllt HP bei
Wave-Start auf, SURGE härtet Drones (+1 skill) und vergibt x3 Upgrade-Points bei Wave-Clear.
Codex/Bestiary (`CG_DrawNeonCodex` in cg_draw.c): listet alle 11 Modifier + 5 Bosse + 6 Perks
mit Effekt, getriggert per `g_neonwave_codex 1` (später im Wave-Break auto-einblenden).

Test 2 (full-run) prüft zusätzlich v0.27-Reveal: `[VAMPIRE]` auf Welle 10, `[FRENZY]` auf 11, `[OVERSHIELD]` auf 12, plus `GLASS CANNON` und `WARDEN` im Boss-Zyklus.

Hinweis zu Test 15: die Server-Aufrufe in `t15()` müssen `$RUNNER` +
`${OA_EXTRA[@]}` wie `run_test()` verwenden — ein nackter ioq3ded-Aufruf
stirbt instant (kein basepath) → leere Logs → "boss mismatch: '' vs ''"
(CI-Fail 2026-08-25/26).

## Ergänzung: Konfigurations-Pattern für Configstring-CsPayload

Der Configstring `CS_NEONWAVE` hat folgendes Format:

    <wave> <event> <bossHp> <bossMax> <breakMs> <pts> <best> <mod> <kills> <bestCombo> <runSec> <liveCombo> [<bossType>]

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

## Gesamtüberblick

Die Suite umfasst **45 Tests** (1–44 inkl. 9b). Der vollständige Katalog
steht in `tests/run_suite.sh` in der Variable `ALL_TESTS` sowie in der
`dispatch_test()`-Funktion. Die Tabelle oben ist die menschlich-lesbare
Dokumentation; jeweils eine Änderung an einem Test erfordert:
1. Anpassung der Assertions in `run_suite.sh` (oder neue Assert-Funktion).
2. ggf. Anpassung der `dispatch_test()`-Case (CVars setzen).
3. ggf. Ergänzung/Eintrag hier in TESTS.md.

## Weiterführende Dokumentation

- `references/test-harness.md` — Test-Harness-Details, Payload-Parsing, Flaky-Tests
- `references/build-test-release.md` — Build, Install, Dist, Release-Gate
- `references/parallel-suite.md` — Parallelisierung (`--parallel N`)
- `references/runstats-json.md` — Run-Statistiken JSON-Schema und Tooling
- `references/background-music.md` — Sound-Assets und lokale Synthese
