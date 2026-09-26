#!/bin/sh
# Test 121: Ghost wave-1 roll skips CHAIN (rail-only). Unfiltered wave 1 is CHAIN/PIERCE/OVERCHARGE.
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 1 \
    --timeout 45 \
    --extra-args "+set g_neonwave_ghost 1 +set g_ghost_loadout 0 +set g_neonwave_autostart 1 +set g_neonwave_autokill 1 +set g_neonwave_fastbreak 1" \
    --expected "PERK OFFER F1=SECOND WIND F2=PIERCE F3=SKIP" \
    "$@"
