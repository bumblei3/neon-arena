#!/bin/sh
# Test 119: drones must not get the Ghost kit; wave 1 Ghost has 1 bot not 2
exec tests/helpers/autostart_test.sh \
    --autostart \
    --timeout 30 \
    --extra-args "+set g_neonwave_ghost 1 +set g_neonwave_autostart 1 +set g_neonwave_failrun 1" \
    --expected "starting wave 1 (1 bots, skill 1)" \
    "$@"
