#!/bin/sh
# Test 112: Ghost Loadout Balance - Saboteur
# Saboteur: 70 start energy, EMP cost 25 (35-10), Lockdown cost 35 (50-15)
# Verifies: ENERGY=70, loadout 1
exec tests/helpers/autostart_test.sh \
    --autostart \
    --timeout 60 \
    --extra-args "+set g_neonwave_ghost 1 +set g_ghost_loadout 1 +set g_neonwave_failrun 1" \
    --expected "Ghost: .* ENERGY=70 (loadout 1)" \
    "$@"
