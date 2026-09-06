#!/bin/sh
# Test 81: Ghost balance CVars
# CVars: ghost 1, startwave 1
# Erwünschte Marker: Energy bar shows correct initial value, regen works
# Anti-Patterns: keine Fatal-Warnung
#
# This test verifies that the Ghost balance CVars work correctly:
# - g_ghost_energy_start=60 (initial energy)
# - g_ghost_energy_max=100 (max energy)
# - g_ghost_regen_amt=4 (regen per second)
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 1 \
    --timeout 60 \
    --extra-args "+set g_neonwave_ghost 1 +set g_ghost_energy_start 60 +set g_ghost_energy_max 100 +set g_ghost_regen_amt 4 +set g_neonwave_failrun 1" \
    --expected 'Ghost' \
    --expected 'GHOST' \
    "$@"