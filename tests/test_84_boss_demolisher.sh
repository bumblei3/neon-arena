#!/bin/sh
# Test 84: Demolisher boss
# CVars: autostart, startwave 20, bosstype 11
# Erwünschte Marker: `boss spawned: DEMOLISHER`, `DEMOLISHER fires rocket barrage`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 20 \
    --timeout 90 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 20 +set g_neonwave_bosstype 11 +set g_neonwave_fastbreak 1 +set g_neonwave_autokill 1" \
    --expected 'boss spawned: DEMOLISHER' \
    --expected 'DEMOLISHER fires rocket barrage' \
    "$@"
