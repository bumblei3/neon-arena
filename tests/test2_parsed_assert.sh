# Test 2 mit strukturiertem CS_NEONWAVE-Parser
# Vorher: Einzel-Greps auf Marker wie "starting wave 20", "All waves cleared", "NEW BEST wave 20"
# Nachher: Der Parser extrahiert die Payload und setzt die Werte, dann prüfen wir die Werte strukturiert.
assert_2() {
  local ok=0
  . ./cs_neonwave_parse.sh 2>/dev/null || true

  # Nach dem Lauf sollte die Payload-Zeile im Log vorhanden sein
  parse_cs_neonwave "$1" || { ok=1; echo "keine CS_NEONWAVE-Payload im Log"; return; }

  # Erwartete Werte nach einem erfolgreichen Full-Run (wave 20, victory):
  # CS_EVENT sollte 1 (cleared) oder 3 (victory) sein — je nach Zeitpunkt des Log-Eintrags
  # Mindestens: CS_WAVE=20, CS_EVENT in {1,3}, CS_PTS>=19 (upgrade points), CS_BEST>=20
  [ "$CS_WAVE" -ge 20 ] || { ok=1; echo "CS_WAVE=$CS_WAVE < 20"; }
  [ "$CS_EVENT" -eq 1 ] || [ "$CS_EVENT" -eq 3 ] || { ok=1; echo "CS_EVENT=$CS_EVENT (erwartet 1 oder 3)"; }
  [ "$CS_BEST" -ge 20 ] || { ok=1; echo "CS_BEST=$CS_BEST < 20"; }
  [ "$CS_PTS" -ge 19 ] || { ok=1; echo "CS_PTS=$CS_PTS (erwartet >= 19)"; }

  # Optional: Boss-HP war bei Wave 10 SNIPER = 400 (oder mit Difficulty skaliert)
  # Wir prüfen nicht den exakten Wert, nur dass Boss HP > 0 ist
  [ "$CS_BOSSHP" -gt 0 ] || { ok=1; echo "CS_BOSSHP=$CS_BOSSHP (erwartet > 0)"; }

  # Neue strukturelle Assert: Wenn CS_EVENT=3 (Victory) dann muss CS_RUNSEC > 0 sein
  if [ "$CS_EVENT" -eq 3 ]; then
    [ "$CS_RUNSEC" -gt 0 ] || { ok=1; echo "CS_RUNSEC=$CS_RUNSEC (Victory ohne Zeit)"; }
  fi

  report $ok "full-run-victory"
}
