#!/bin/sh
# Test 105: Seasonal Challenge — weekly challenge load + progress tracking
# CVars: autostart, seasonal 1, startwave 6, autokill 1, fastbreak 1
# Erwünschte Marker: `SEASONAL challenge` (log output when seasonal is active),
#   `ui_neonwave_seasonal_title` set (non-empty challenge title)
# Anti-Patterns: keine Fatal-Warnung, kein leeres `ui_neonwave_seasonal_title`
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 6 \
    --timeout 60 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 6 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1 +set g_neonwave_seasonal 1 +set g_neonwave_ghost 0 +set g_neonwave_drone_hp_scale 1.0 +set g_neonwave_drone_damage_scale 1.0 +set g_neonwave_drone_speed_scale 1.0 +set g_neonwave_drone_count_scale 1.0 +set g_neonwave_gravity_scale 1.0 +set g_neonwave_modifier 1" \
    --expected 'SEASONAL challenge' \
    "$@"
