---
title: "[Design] Coop Multiplayer / LAN-Coop Wave-Survival"
labels: [feature, design, coop]
---

## Intent

Einen kooperativen Multiplayer-Modus für NeonArena, in dem 2–4 Spieler gemeinsam
gegen Bot-Wellen kämpfen. Basiert auf dem Singleplayer-Headless-Code, mit
ergänzenden Mechanismen für team-basierte Wave-Clear, individuelles Upgrade,
Combo-Handling und Schwierigkeitsskalierung.

## Status

Design-Sketch — noch keine Implementierung. Erst wenn dieser Entwurf stabil ist,
beginnt die Codearbeit.

## Anforderungen (Behavior)

### Wave-Start

- Gleicher Start wie Singleplayer: `g_neonwave_autostart 1` + `g_gametype 14`.
- Bei `sv_maxclients > 1` und mindestens einem verbundenen Human-Client startet die
  erste Welle ohne `autostart` auch automatisch (Faustregel: sobald 1 Human online,
  läuft der Wave-Loop — verhindert, dass der Host auf einen zweiten Spieler warten
  muss).
- Wave-Loop läuft unabhängig von der Anzahl der Spieler — Drones werden gespawnt
  nach aktuellem `nw_wave` (Welle 1 = 2 Drones, Welle N = N+1 Drones).

### Wave-Clear-Bedingung (Gemeinsam)

- Ein Wave gilt als gemeistert, wenn **alle** verbundenen Human-Clients noch leben
  UND alle Drones des Waves ausgelöscht wurden.
- Wenn **irgendein** Human-Client stirbt, wird das Wave nicht abgebrochen, aber
  der Clear-Counter (Timer/Failrun) wird solange nicht ausgelöst, bis alle wieder
  am Leben sind oder der Wave durch andere Bedingungen endet.
- `g_neonwave_failrun 1` (Test-Hook) erzwingt Failrun auch in Coop, wenn die
  Bedingung eintritt (ein Human stirbt — die Entscheidung, ob das im
  Coop-Modus so gewünscht ist, ist designabhängig).

### Upgrade

- Upgrade-Punkte sind **pro Client**.
- Jeder Client hat eigenen `g_neonwave_upgradepoints`-Counter (oder ein
  Client-Index in `client->pers.neonwaveUpgradePts`).
- Break-Pause: jeder spielt F1/F2/F3 unabhängig — Perk-Karten werden pro Client
  angeboten (momentan ist `NW_RollOffers()` einmal pro Break für alle — das müsste
  in `NW_RollOffers` pro Client differenziert werden, oder die Offers werden pro
  Client an den jeweiligen Client gesendet mit individuellem Seed).
- Alternative (einfacherer Weg): globaler Punkte-Pool, aufgeteilt nach
  `NW_RunDeaths()` / Anzahl Spieler — weniger granular, aber leichter zu implementieren.
  Empfehlung: erstmal pro-Client-Pool, weil es fair ist und mit dem
  `client->pers.neonwaveUpHp/Dmg/Speed` bereits vorbereitet ist.

### Combo

- Jeder Client hat eigenen `nwCombo`-Counter (aktuell so).
- `nw_runBestCombo` (global für den Run) bleibt der beste Combo eines jeden Clients
  (aktuell so via `NW_RunBestCombo()`).
- Im HUD: jeder sieht seinen eigenen Combo, der globale Highscore zeigt den besten
  im Team.
- Team-Combo-Bonus (optional): wenn zwei Spieler innerhalb kurzer Zeit (< 2s)
  denselben Drone töten (zwei Kills auf einen Drone), gibt das einen
  Team-Combo-Bonus (+1 Punkte für jeden). Das ist eine spätere Erweiterung —
  nicht im ersten Design.

### Schwierigkeitsskalierung in Coop

- Bei mehr als einem Human-Spieler: Drone-Skill scaling?
  - Variante A: gleiche Drone-Anzahl, gleicher Skill — coop ist einfach
    spielerischer, weil man sich unterstützen kann.
  - Variante B: +1 Skill pro zusätzlichem Human-Client (max +2) — steigert
    Herausforderung, aber nicht überfordernd.
  - Variante C: mehr Drones pro Welle (+1 pro Human-Client) — erhöht
    Action-Dichte, aber nicht unbedingt Schwierigkeit.
- Empfehlung: Variante A (kein automatischer Bonus) + optionalen
  `g_neonwave_coopdifficulty 1..3` (1 = einfach, 2 = normal, 3 = viel
  Drones/Skill).

### Boss

- Boss-Wellen (ab Welle 10) bleiben gleich — ein Boss pro Wave, auch in Coop.
- Boss-HP scaling? Aktuell skaliert Boss-HP mit `nw_difficulty` (nicht mit
  Anzahl Spieler). Für Coop sinnvoll: Boss-HP unverändert (leichter als
  mehrere Spieler gemeinsam), oder leichter Skalierung (Boss HP +25% pro
  zusätzlichem Spieler).
- Empfehlung: keinen Bonus, da 5 Bosse bereits stark genug — Coop-Spieler
  können sich über gemeinsame Rail/LG-Kombinationen den Boss nehmen.

### Records / Highscore

- Globaler Highscore (`g_neonwave_best`) bleibt singleplayer-orientiert (Wave 20).
- Coop-Ergebnis: Run-Statistik zeigt Team-Ergebnis (Wave, Kills gesamt,
  bestes Team-Combo, Zeit).
- Coop-spezifischer Highscore-Bereich (optional): `g_neonwave_coop_best_wave`
  als separater Cvar/Record — nicht zwingend für erste Version.

### Dedizierter Server / LAN

- `ioq3ded` Headless läuft ohne Renderung — für LAN-Hosts braucht man einen
  spielbaren Server (OpenArena-Client als Host, oder ein separater Dedicated mit
  `sv_maxclients` und `r_mode`/`vid_fullscreen 0`).
- Für lokalen LAN-Gamefall reicht ein gehosteter OpenArena mit `+set fs_game
  neonarena +set g_gametype 14`.

## Technische Anmerkungen

- `g_neonwave.c`: Änderungen an `NeonWave_CheckWaveClear()`, `NW_SpawnBot()`,
  `NW_SpawnBoss()`, `NW_SendStatus()`, `NW_UpdateRecords()`,
  `NW_RunKills()`, `NW_RunBestCombo()`.
- `cgame`: `CG_DrawNeonWaveHUD` (oder äquivalent) muss Upgrade-Punkte pro Client
  anzeigen — wenn der Spieler selbst wählen kann, was er kaufen will.
- `cg_draw.c` / `cg_hud.c`: Break-Pause-Shopping-Bildschirm — aktuell global,
  muss pro Client differenzieren.
- Tests: neue Testfälle für Coop-Wave-Clear (2 Spieler, beide leben, Clear),
  Coop-Failrun (ein Spieler stirbt, kein Clear), Coop-Upgrade (pro Client),
  Coop-Combo (Kills beider Spieler zählen individuell).

## Prioritäten

- [ ] Design-Review (dieses File) — stabilisieren
- [ ] Coop-Wave-Clear (Gemeinsam) — Wave endet erst wenn alle leben + Drones tot
- [ ] Coop-Upgrade (pro Client) — jeder hat eigenen Punkte-Pool
- [ ] Break-Pause-Shopping pro Client (Perk-Karten für jeden)
- [ ] Coop-Skala (optional: `g_neonwave_coopdifficulty`)
- [ ] Neue Testfälle (Coop-Suite)
- [ ] LAN-Hosting-Anleitung (README oder SEPARATE Doc)

## Offene Fragen

- Soll ein gestorbener Spieler das Wave nicht abbrechen? Ja — sonst wäre
  Coop zu hart (ein Todesfall = Partie vorbei für alle).
- Soll der Wave weiterlaufen wenn ein Spieler disconnectet? Ja — verbleibende
  Spieler spielen weiter (der Wave-Loop wartet nicht auf reconnects).
- Perk-Karten: jeder Spieler wählt individuell — das erfordert eine
  Client-spezifische Offer-Logik (momentan global `NW_RollOffers()`).
- Boss-HP Scaling in Coop: nein (erste Version), da 5 Bosse ohnehin stark.
