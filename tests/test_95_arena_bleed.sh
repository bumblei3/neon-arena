#!/bin/sh
# Test 95: Arena Bleed Chamber — ghost disabled
# CVars: autostart, ghost 0, autokill (to clear wave)
# Erwünschte Marker: `NeonWave over` (normal gameplay, no ghost)
# Anti-Patterns: keine Fatal-Warnung, kein `Ghost kit active`
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 3 \
    --timeout 60 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 3 +set g_neonwave_autokill 1 +set g_neonwave_ghost 0 +set g_neonwave_drone_hp_scale 1.0 +set g_neonwave_drone_damage_scale 1.0 +set g_neonwave_drone_speed_scale 1.0 +set g_neonwave_drone_count_scale 1.0 +set g_neonwave_gravity_scale 1.0" \
    --expected 'NeonWave over' \
    "$@"
