#!/bin/sh
# Test 83: Sniper Elite boss
# CVars: autostart, startwave 19, bosstype 10
# Erwünschte Marker: `boss spawned: SNIPER ELITE`, `SNIPER ELITE rapid rail`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 19 \
    --timeout 90 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 19 +set g_neonwave_bosstype 10 +set g_neonwave_fastbreak 1 +set g_neonwave_autokill 1" \
    --expected 'boss spawned: SNIPER ELITE' \
    --expected 'SNIPER ELITE rapid rail' \
    "$@"
