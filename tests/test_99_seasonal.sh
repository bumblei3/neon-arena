#!/bin/sh
# Test 99: Seasonal Rotation — weekly modifier offset via g_neonwave_seasonal
# CVars: autostart, seasonal 1, modifier 0 (random), startwave 6
# Erwünschte Marker: `SEASONAL rotation week` (log output when seasonal is active)
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 6 \
    --timeout 60 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 6 +set g_neonwave_autokill 1 +set g_neonwave_seasonal 1 +set g_neonwave_ghost 0 +set g_neonwave_drone_hp_scale 1.0 +set g_neonwave_drone_damage_scale 1.0 +set g_neonwave_drone_speed_scale 1.0 +set g_neonwave_drone_count_scale 1.0 +set g_neonwave_gravity_scale 1.0" \
    --expected 'SEASONAL rotation' \
    "$@"
