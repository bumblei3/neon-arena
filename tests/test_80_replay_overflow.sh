#!/bin/sh
# Test 80: replay overflow
# CVars: replaytest80 1, autostart, failrun
# Erwünschte Marker: `NeonWave: REPLAY overflow recorded=32773 stored=32768`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --timeout 120 \
    --extra-args "+set g_neonwave_replaytest80 1 +set g_neonwave_failrun 1" \
    --expected 'NeonWave: REPLAY overflow recorded=32773 stored=32768' \
    "$@"