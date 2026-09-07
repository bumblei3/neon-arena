#!/bin/sh
# Test 104: Void Walker Boss — phaseshift + invulnerability
# CVars: autostart, bosstype 13 (VOIDWALKER), startwave 12, autokill
# Erwünschte Marker: `VOID WALKER phaseshift` oder `boss spawned: VOID WALKER (hc 550`
# Anti-Patterns: keine Fatal-Warnung
exec tests/helpers/autostart_test.sh \
    --autostart \
    --startwave 12 \
    --timeout 90 \
    --extra-args "+set g_neonwave_autostart 1 +set g_neonwave_startwave 12 +set g_neonwave_autokill 1 +set g_neonwave_bosstype 13 +set g_neonwave_ghost 0 +set g_neonwave_drone_hp_scale 1.0 +set g_neonwave_drone_damage_scale 1.0 +set g_neonwave_drone_speed_scale 1.0 +set g_neonwave_drone_count_scale 1.0 +set g_neonwave_gravity_scale 1.0" \
    --expected 'VOID WALKER' \
    "$@"
