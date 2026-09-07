#!/bin/sh
# Test 93: Arena Node Control — drone count scaling
# CVars: autostart, drone_count_scale 1.1
# Erwünschte Marker: `arena drone count scale 1.10`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 3 \
    --timeout 60 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 3 +set g_neonwave_drone_count_scale 1.1 +set g_neonwave_drone_hp_scale 1.0 +set g_neonwave_drone_damage_scale 1.0 +set g_neonwave_drone_speed_scale 1.0 +set g_neonwave_gravity_scale 1.0" \
    --expected 'arena drone count scale 1.10' \
    "$@"
