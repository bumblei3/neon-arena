#!/bin/sh
# Test 103: Chronomancer Boss — time warp teleport
# CVars: autostart, bosstype 12 (CHRONOMANCER), startwave 12, autokill
# Erwünschte Marker: `CHRONOMANCER warps time and teleports` oder `boss spawned: CHRONOMANCER (hc 450`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 12 \
    --timeout 90 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 12 +set g_neonwave_autokill 1 +set g_neonwave_bosstype 12 +set g_neonwave_ghost 0 +set g_neonwave_drone_hp_scale 1.0 +set g_neonwave_drone_damage_scale 1.0 +set g_neonwave_drone_speed_scale 1.0 +set g_neonwave_drone_count_scale 1.0 +set g_neonwave_gravity_scale 1.0" \
    --expected 'CHRONOMANCER' \
    "$@"
