# ---- CS_NEONWAVE strukturiert parsen ----
# Der Configstring CS_NEONWAVE hat Format:
#   <wave> <event> <bossHp> <bossMax> <breakMs> <pts> <best> <mod> <kills> <bestCombo> <runSec> <liveCombo>
# event: 0=running, 1=cleared/break, 2=failed, 3=victory
# Diese Funktion sucht im Log nach dem cvarlist-Eintrag für CS_NEONWAVE (im Log beim letzten Server-Zyklus
# oder in der Payload-Zeile mit der Configstring-Darstellung) und schreibt die Werte in global Variablen.
# Fehlende Felder werden als -1 markiert.
#
# Usage: parse_cs_neonwave <logfile>
#   setzt nach Aufruf: CS_WAVE CS_EVENT CS_BOSSHP CS_BOSSMX CS_BREAKTIME CS_PTS CS_BEST CS_MOD
#                     CS_KILLS CS_BESTCOMBO CS_RUNSEC CS_LIVECOMBO
#   Rückgabe: 0 wenn gefunden, 1 wenn nicht gefunden

parse_cs_neonwave() {
  local log="$1"
  local cs_line=""
  # ioq3ded gibt cvarlist-Ausgaben im Format aus:
  #   "        ? g_neonwave_cs_cs \"0 0 200 200 0 0 0 0 0 0 0 0\""
  # oder mit "A" für archivierte cvars:
  #   "        A  ? g_neonwave_dailyrecwave \"20\""
  # Die Payload steht in der Zeile nach dem Cvar-Namen, in Anführungszeichen.
  # Wir suchen nach Zeilen, die "g_neonwave_cs_cs" enthalten und die Payload in Anführungszeichen haben.
    cs_line=$(grep -E 'g_neonwave_cs_cs\s+"[^"]*"' "$log" 2>/dev/null | tail -1) || true
  if [ -z "$cs_line" ]; then
    # Fallback: Zeile mit Payload-Zahlen direkt suchen (manche Logs haben die Payload als Kommentar)
    cs_line=$(grep -E '^\s*[0-9]+[[:space:]]+[0-9]+[[:space:]]+[0-9]+' "$log" 2>/dev/null | tail -1) || true
  fi
  if [ -z "$cs_line" ]; then
    return 1
  fi
  # Extrahiere die Zahlen aus der Zeile
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
