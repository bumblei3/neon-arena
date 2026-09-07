#!/bin/sh
# Test 101: Performance Stress — high drone count (20 bots) with arena scaling
# CVars: autostart, drone_count_scale 2.0, startwave 15, autokill
# Erwünschte Marker: `arena drone count scale 2.00` (capped at 20)
# Anti-Patterns: keine Fatal-Warnung, kein Timeout
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 15 \
    --timeout 90 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 15 +set g_neonwave_autokill 1 +set g_neonwave_ghost 0 +set g_neonwave_drone_hp_scale 1.0 +set g_neonwave_drone_damage_scale 1.0 +set g_neonwave_drone_speed_scale 1.0 +set g_neonwave_drone_count_scale 2.0 +set g_neonwave_gravity_scale 1.0" \
    --expected 'arena drone count scale 2.00' \
    "$@"
