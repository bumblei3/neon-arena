#!/bin/sh
# Test 96: Healer Bot — green glow, heals nearby bots
# CVars: autostart, forcebot 1 (Healer), startwave 10 (Healer ab Welle 8)
# Erwünschte Marker: `NeonWave: Drone W10-1 HEALER`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 10 \
    --timeout 60 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 10 +set g_neonwave_autokill 1 +set g_neonwave_forcebot 1 +set g_neonwave_ghost 0 +set g_neonwave_drone_hp_scale 1.0 +set g_neonwave_drone_damage_scale 1.0 +set g_neonwave_drone_speed_scale 1.0 +set g_neonwave_drone_count_scale 1.0 +set g_neonwave_gravity_scale 1.0" \
    --expected 'HEALER' \
    "$@"
