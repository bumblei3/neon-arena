#!/bin/sh
# Test 97: Shielder Bot — blue glow, gives shields to nearby bots
# CVars: autostart, forcebot 2 (Shielder), startwave 12 (Shielder ab Welle 10)
# Erwünschte Marker: `NeonWave: Drone W12-1 SHIELD`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 12 \
    --timeout 60 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 12 +set g_neonwave_autokill 1 +set g_neonwave_forcebot 2 +set g_neonwave_ghost 0 +set g_neonwave_drone_hp_scale 1.0 +set g_neonwave_drone_damage_scale 1.0 +set g_neonwave_drone_speed_scale 1.0 +set g_neonwave_drone_count_scale 1.0 +set g_neonwave_gravity_scale 1.0" \
    --expected 'SHIELD' \
    "$@"
