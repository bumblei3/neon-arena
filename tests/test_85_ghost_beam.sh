#!/bin/sh
# Test 85: Ghost beam weapon
# CVars: ghost 1, autostart, failrun
# Erwünschte Marker: `BEAM`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --timeout 60 \
    --extra-args "+set g_neonwave_ghost 1 +set g_neonwave_autostart 1 +set g_neonwave_failrun 1" \
    --expected 'BEAM' \
    "$@"
