#!/bin/sh
# Test 113: Ghost Loadout Balance - Spectre
# Spectre: 90 start energy, no cloak, has nuke
# Verifies: ENERGY=90, loadout 2
exec tests/helpers/autostart_test.sh \
    --autostart \
    --timeout 60 \
    --extra-args "+set g_neonwave_ghost 1 +set g_ghost_loadout 2 +set g_neonwave_failrun 1" \
    --expected "Ghost: .* ENERGY=90 (loadout 2)" \
    "$@"
