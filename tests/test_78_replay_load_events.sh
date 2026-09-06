#!/bin/sh
# Test 78: replay load and verify events
# CVars: replaytest78 1, autostart, failrun
# Erwünschte Marker: `NeonWave: REPLAY SAVE saved`, `NeonWave: REPLAY LOAD loaded`,
#   `NeonWave: REPLAY LOAD verify match=1`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --timeout 60 \
    --extra-args "+set g_neonwave_replaytest 78 +set g_neonwave_failrun 1" \
    --expected 'NeonWave: REPLAY SAVE saved' \
    --expected 'NeonWave: REPLAY LOAD loaded' \
    --expected 'NeonWave: REPLAY LOAD verify match=1' \
    "$@"