#!/bin/sh
# Test 118: arena stem look override (Frostbite on oa_minia, not generic minia look)
exec tests/helpers/autostart_test.sh \
    --autostart \
    --timeout 30 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_failrun 1 +set g_neonwave_arena frostbite" \
    --expected "NeonArena: look frostbite overbright=1 bloom=0.38 grid=0" \
    "$@"
