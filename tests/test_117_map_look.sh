#!/bin/sh
# Test 117: per-BSP neon look applied on map start
exec tests/helpers/autostart_test.sh \
    --autostart \
    --timeout 30 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_failrun 1" \
    --expected "NeonArena: look oa_shine" \
    "$@"
