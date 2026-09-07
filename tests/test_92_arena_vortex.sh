#!/bin/sh
# Test 92: Arena Vortex Ring — gravity scaling
# CVars: autostart, gravity_scale 0.7
# Erwünschte Marker: `g_gravity` changed (to ~560)
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 3 \
    --timeout 60 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 3 +set g_neonwave_gravity_scale 0.7" \
    --expected 'g_gravity' \
    "$@"
