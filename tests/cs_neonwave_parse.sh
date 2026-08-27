# ---- CS_NEONWAVE strukturiert parsen ----
# Der Configstring CS_NEONWAVE hat Format:
#   <wave> <event> <bossHp> <bossMax> <breakMs> <pts> <best> <mod> <kills> <bestCombo> <runSec> <liveCombo>
# event: 0=running, 1=cleared/break, 2=failed, 3=victory
# Diese Funktion sucht im Log nach der Payload (NEONWAVE_PAYLOAD-Zeile, cvarlist-Eintrag oder rohe Zahlen)
# und schreibt die Werte in global Variablen.
#
# Usage: parse_cs_neonwave <logfile>
#   setzt nach Aufruf: CS_WAVE CS_EVENT CS_BOSSHP CS_BOSSMX CS_BREAKTIME CS_PTS CS_BEST CS_MOD
#                     CS_KILLS CS_BESTCOMBO CS_RUNSEC CS_LIVECOMBO
#   Rückgabe: 0 wenn gefunden, 1 wenn nicht gefunden
#
# Usage: parse_cs_neonwave_at <logfile> <wave>
#   wie parse_cs_neonwave, aber sucht die ERSTE Payload mit exakt dieser Wave.
#   Nötig für tests, die eine bestimmte Wave (z.B. erzwungener Modifier) prüfen —
#   tail -1 würde die letzte Wave (oft ein Boss-Wave mit mod=0) treffen und
#   die Assertion fälschlich fehlschlagen lassen.
#
# Strategien (in Reihenfolge):
# 1. NEONWAVE_PAYLOAD: Zeile wird von g_neonwave.c via G_Printf ausgegeben
#    Format: "NEONWAVE_PAYLOAD: <wave> <event> ..."
# 2. cvarlist-Eintrag für g_neonwave_cs_cs (ioq3ded)
# 3. Payload als rohe Zahlenzeile (Fallback)

# interne Helfer: extrahiert die 12 Zahlen aus einer Payload-Zeile in die CS_*-Vars
_extract_cs_vars() {
  local cs_line="$1"
  local vals=$(echo "$cs_line" | grep -oE '[0-9]+' | head -12)
  [ -z "$vals" ] && return 1
  local arr=($vals)
  [ ${#arr[@]} -lt 12 ] && return 1
  CS_WAVE=${arr[0]}
  CS_EVENT=${arr[1]}
  CS_BOSSHP=${arr[2]}
  CS_BOSSMX=${arr[3]}
  CS_BREAKTIME=${arr[4]}
  CS_PTS=${arr[5]}
  CS_BEST=${arr[6]}
  CS_MOD=${arr[7]}
  CS_KILLS=${arr[8]}
  CS_BESTCOMBO=${arr[9]}
  CS_RUNSEC=${arr[10]}
  CS_LIVECOMBO=${arr[11]}
  return 0
}

# sucht die passende Payload-Zeile (inkl. "NEONWAVE_PAYLOAD: "-Prefix)
# $1=log $2=wave (leer=letzte) -> gibt die rohe Zeile auf stdout aus
_cs_find_payload() {
  local log="$1" wave="${2:-}" cs_line=""
  if [ -n "$wave" ]; then
    # erste Payload mit exakt dieser Wave
    cs_line=$(grep -E "^NEONWAVE_PAYLOAD: $wave " "$log" 2>/dev/null | head -1)
  else
    cs_line=$(grep -E '^NEONWAVE_PAYLOAD:' "$log" 2>/dev/null | tail -1)
  fi
  echo "$cs_line"
}

# Kernsuche: parst aus einer gefundenen Zeile (Strategien 1-3)
_parse_cs_from_line() {
  local log="$1" line="$2"
  if [ -n "$line" ]; then
    line=$(echo "$line" | sed 's/^NEONWAVE_PAYLOAD: //')
    _extract_cs_vars "$line" && return 0
  fi
  # Strategie 2: cvarlist-Eintrag für g_neonwave_cs_cs (ioq3ded)
  line=$(grep -E 'g_neonwave_cs_cs\s+"[^"]*"' "$log" 2>/dev/null | tail -1)
  if [ -n "$line" ]; then
    line=$(echo "$line" | grep -oE '"[^"]*"' | tr -d '"')
    _extract_cs_vars "$line" && return 0
  fi
  # Strategie 3: Payload als rohe Zahlenzeile (Fallback)
  line=$(grep -E '^\s*[0-9]+[[:space:]]+[0-9]+[[:space:]]+[0-9]+' "$log" 2>/dev/null | tail -1)
  if [ -n "$line" ]; then
    _extract_cs_vars "$line" && return 0
  fi
  return 1
}

parse_cs_neonwave() {
  local log="$1" line
  line=$(_cs_find_payload "$log" "")
  _parse_cs_from_line "$log" "$line" && return 0
  return 1
}

parse_cs_neonwave_at() {
  local log="$1" wave="$2" line
  line=$(_cs_find_payload "$log" "$wave")
  _parse_cs_from_line "$log" "$line" && return 0
  return 1
}

# map numeric modifier to name for test assertions
modifier_name() {
  local m="${1:-0}"
  case "$m" in
    0) echo "NONE" ;;
    1) echo "GLASS DRONES" ;;
    2) echo "SWARM" ;;
    3) echo "LOW GRAVITY" ;;
    4) echo "DOUBLE POINTS" ;;
    *) echo "UNKNOWN" ;;
  esac
}
