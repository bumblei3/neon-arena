#!/bin/sh
# Test 120: wave 6 SWARM doubles 7 to 14, then the live cap clamps to 8
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 6 \
    --timeout 30 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 6 +set g_neonwave_modifier 2 +set g_neonwave_ghost 0 +set g_neonwave_failrun 1" \
    --expected "drone cap 8: 14 -> 8" \
    "$@"
