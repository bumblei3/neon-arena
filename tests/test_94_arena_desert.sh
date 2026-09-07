#!/bin/sh
# Test 94: Arena Desert Storm — drone speed scaling
# CVars: autostart, drone_speed_scale 1.1
# Erwünschte Marker: `g_neonwave_drone_speed_scale=1.1`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 3 \
    --timeout 60 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 3 +set g_neonwave_drone_speed_scale 1.1" \
    --expected 'g_neonwave_drone_speed_scale=1.1' \
    "$@"
