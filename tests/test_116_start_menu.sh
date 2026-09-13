#!/bin/sh
# Test 116: Start menu is wired (PLAY/DAILY/GHOST/ARENA) + daily pool header
exec tests/helpers/autostart_test.sh \
    --autostart \
    --timeout 30 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_failrun 1" \
    --expected "NeonWave" \
    "$@"
