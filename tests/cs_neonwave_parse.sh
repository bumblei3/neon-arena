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
# Strategien (in Reihenfolge):
# 1. NEONWAVE_PAYLOAD: Zeile wird von g_neonwave.c via G_Printf ausgegeben
#    Format: "NEONWAVE_PAYLOAD: <wave> <event> ..."
# 2. cvarlist-Eintrag für g_neonwave_cs_cs (ioq3ded)
# 3. Payload als rohe Zahlenzeile (Fallback)

parse_cs_neonwave() {
  local log="$1"
  local cs_line=""
  local found=0

  # Strategie 1: NEONWAVE_PAYLOAD: Zeile (von g_neonwave.c ausgegeben)
  # Format: "NEONWAVE_PAYLOAD: <wave> <event> <bossHp> <bossMax> <breakMs> <pts> <best> <mod> <kills> <bestCombo> <runSec> <liveCombo>"
  cs_line=$(grep -E '^NEONWAVE_PAYLOAD:' "$log" 2>/dev/null | tail -1) || true
  if [ -n "$cs_line" ]; then
    cs_line=$(echo "$cs_line" | sed 's/^NEONWAVE_PAYLOAD: //')
    found=1
  fi

  # Strategie 2: cvarlist-Eintrag für g_neonwave_cs_cs (ioq3ded)
  # ioq3ded gibt cvarlist-Ausgaben im Format aus:
  #   "        ? g_neonwave_cs_cs \"0 0 200 200 0 0 0 0 0 0 0 0\""
  # oder mit "A" für archivierte cvars:
  #   "        A  ? g_neonwave_dailyrecwave \"20\""
  # Die Payload steht in der Zeile nach dem Cvar-Namen, in Anführungszeichen.
  if [ "$found" -eq 0 ]; then
    cs_line=$(grep -E 'g_neonwave_cs_cs\s+"[^"]*"' "$log" 2>/dev/null | tail -1) || true
    if [ -n "$cs_line" ]; then
      cs_line=$(echo "$cs_line" | grep -oE '"[^"]*"' | tr -d '"')
      found=1
    fi
  fi

  # Strategie 3: Payload als rohe Zahlenzeile (Fallback)
  if [ "$found" -eq 0 ]; then
    cs_line=$(grep -E '^\s*[0-9]+[[:space:]]+[0-9]+[[:space:]]+[0-9]+' "$log" 2>/dev/null | tail -1) || true
    if [ -n "$cs_line" ]; then
      found=1
    fi
  fi

  if [ "$found" -eq 0 ] || [ -z "$cs_line" ]; then
    return 1
  fi

  # Extrahiere die Zahlen aus der Zeile (erste 12 Zahlen)
  local vals=$(echo "$cs_line" | grep -oE '[0-9]+' | head -12)
  if [ -z "$vals" ]; then
    return 1
  fi
  local arr=($vals)
  if [ ${#arr[@]} -lt 12 ]; then
    return 1
  fi
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
