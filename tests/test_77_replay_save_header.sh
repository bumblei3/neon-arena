#!/bin/sh
# Test 77: replay save header metadata
# CVars: replaytest77 1, autostart, failrun
# Erwünschte Marker: `NeonWave: REPLAY SAVE magic=NRPY version=1 events=2`,
#   `map=`-Marker im Log, `durationMs=`-Marker
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --timeout 60 \
    --extra-args "+set g_neonwave_replaytest77 1 +set g_neonwave_failrun 1" \
    --expected 'NeonWave: REPLAY SAVE magic=NRPY version=1 events=2' \
    "$@"