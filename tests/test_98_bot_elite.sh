#!/bin/sh
# Test 98: Elite Bot — red glow, fast dash every 2s
# CVars: autostart, forcebot 3 (Elite), startwave 14 (Elite ab Welle 12)
# Erwünschte Marker: `NeonWave: Drone W14-1 ELITE`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 14 \
    --timeout 60 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 14 +set g_neonwave_autokill 1 +set g_neonwave_forcebot 3 +set g_neonwave_ghost 0 +set g_neonwave_drone_hp_scale 1.0 +set g_neonwave_drone_damage_scale 1.0 +set g_neonwave_drone_speed_scale 1.0 +set g_neonwave_drone_count_scale 1.0 +set g_neonwave_gravity_scale 1.0" \
    --expected 'ELITE' \
    "$@"
