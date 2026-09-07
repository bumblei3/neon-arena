#!/bin/sh
# Test 102: Controller Active — ui_controller_active published when joystick enabled
# CVars: autostart, in_joystick 1, joy_assist 0.5, ghost 1, startwave 6, autokill
# Erwünschte Marker: `GHOST kit active` (proves controller aim assist ran)
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 6 \
    --timeout 60 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 6 +set g_neonwave_autokill 1 +set g_neonwave_ghost 1 +set in_joystick 1 +set joy_assist 0.5 +set g_neonwave_drone_hp_scale 1.0 +set g_neonwave_drone_damage_scale 1.0 +set g_neonwave_drone_speed_scale 1.0 +set g_neonwave_drone_count_scale 1.0 +set g_neonwave_gravity_scale 1.0" \
    --expected 'GHOST kit active' \
    "$@"
