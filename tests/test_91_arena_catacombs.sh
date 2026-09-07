#!/bin/sh
# Test 91: Arena Catacombs — drone HP/damage scaling CVars
# CVars: autostart, drone_hp_scale 0.85, drone_damage_scale 1.15
# Erwünschte Marker: `arena drone scaling hp=0.85 dmg=1.15`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 3 \
    --timeout 60 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 3 +set g_neonwave_drone_hp_scale 0.85 +set g_neonwave_drone_damage_scale 1.15 +set g_neonwave_drone_speed_scale 1.0 +set g_neonwave_drone_count_scale 1.0 +set g_neonwave_gravity_scale 1.0" \
    --expected 'arena drone scaling hp=0.85 dmg=1.15' \
    "$@"
