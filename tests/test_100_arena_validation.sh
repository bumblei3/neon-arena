#!/bin/sh
# Test 100: Arena Config Validation — arena scaling summary logged
# CVars: autostart, drone_hp_scale 1.1 (triggers arena scaling log)
# Erwünschte Marker: `arena drone scaling hp=1.10 dmg=1.00 spd=1.00`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 6 \
    --timeout 60 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 6 +set g_neonwave_autokill 1 +set g_neonwave_ghost 0 +set g_neonwave_drone_hp_scale 1.1 +set g_neonwave_drone_damage_scale 1.0 +set g_neonwave_drone_speed_scale 1.0 +set g_neonwave_drone_count_scale 1.0 +set g_neonwave_gravity_scale 1.0" \
    --expected 'arena drone scaling hp=1.10' \
    "$@"
