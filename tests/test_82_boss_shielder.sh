#!/bin/sh
# Test 82: Shielder boss
# CVars: autostart, startwave 18, bosstype 9
# Erwünschte Marker: `boss spawned: SHIELDER`, `SHIELDER deploys energy shield`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 18 \
    --timeout 90 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 18 +set g_neonwave_bosstype 9 +set g_neonwave_fastbreak 1 +set g_neonwave_autokill 1" \
    --expected 'boss spawned: SHIELDER' \
    --expected 'SHIELDER deploys energy shield' \
    "$@"
