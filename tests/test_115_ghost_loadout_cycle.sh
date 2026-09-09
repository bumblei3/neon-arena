#!/bin/sh
# Test 115: Ghost loadout cycle (g_ghost_cycletest)
# Default loadout 0 + cycle → Saboteur (1), ENERGY=70
exec tests/helpers/autostart_test.sh \
    --autostart \
    --timeout 60 \
    --extra-args "+set g_neonwave_ghost 1 +set g_ghost_loadout 0 +set g_ghost_cycletest 1 +set g_neonwave_failrun 1" \
    --expected "Ghost: loadout set to 1" \
    "$@"
