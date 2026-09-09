#!/bin/sh
# Test 111: Ghost Loadout Balance - Infiltrator (default)
# Infiltrator: 80 start energy, standard costs, has cloak
# Verifies: ENERGY=80, loadout 0, cloak available
exec tests/helpers/autostart_test.sh \
    --autostart \
    --timeout 60 \
    --extra-args "+set g_neonwave_ghost 1 +set g_neonwave_failrun 1" \
    --expected "Ghost: .* ENERGY=80 (loadout 0)" \
    "$@"
