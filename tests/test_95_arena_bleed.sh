#!/bin/sh
# Test 95: Arena Bleed Chamber — ghost disabled
# CVars: autostart, ghost 0
# Erwünschte Marker: `g_neonwave_ghost=0`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 3 \
    --timeout 60 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 3 +set g_neonwave_ghost 0" \
    --expected 'g_neonwave_ghost=0' \
    "$@"
