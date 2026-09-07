#!/bin/sh
# Test 91: Arena Catacombs — drone HP/damage scaling CVars
# CVars: autostart, drone_hp_scale 0.85, drone_damage_scale 1.15
# Erwünschte Marker: `g_neonwave_drone_hp_scale=0.85`, `g_neonwave_drone_damage_scale=1.15`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 3 \
    --timeout 60 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 3 +set g_neonwave_drone_hp_scale 0.85 +set g_neonwave_drone_damage_scale 1.15" \
    --expected 'g_neonwave_drone_hp_scale=0.85' \
    --expected 'g_neonwave_drone_damage_scale=1.15' \
    "$@"
