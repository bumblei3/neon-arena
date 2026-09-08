# Neue Bot-Typen Plan für NeonArena

> Stand v0.80: **implementiert** — Healer / Shielder / Sniper-Elite, Tests 96–98, CVar `g_neonwave_forcebot`.

## Übersicht
3 neue Bot-Typen für mehr Variety:

### 1. Healer-Bot
- **Verlangsamt** andere Bots in der Nähe (20 HP alle 3s, Reichweite 200u)
- **Schwach** selbst (50 HP, kein Schaden)
- **Spawn**: Ab Welle 8, 1 Heiler pro 5 Drones
- **Visuell**: Grüner Glow

### 2. Shielder-Bot
- **Gibt** einem zufälligen Bot einen 50er Shield
- **Mittleres** HP (150), wenig Schaden
- **Spawn**: Ab Welle 10, 1 Shielder pro 6 Drones
- **Visuell**: Blauer Glow

### 3. Sniper-Elite-Bot
- **Schneller** Dash (alle 2s), höhere Präzision
- **Starker** Schaden (Railgun), wenig HP (75)
- **Spawn**: Ab Welle 12, 1 Sniper-Elite pro 7 Drones
- **Visuell**: Roter Glow

## Implementierung

### Dateien
- `oa-gamecode/code/game/g_neonwave.c` — Bot-Typen + Logik
- `oa-gamecode/code/game/g_local.h` — Neue Flags/Stati
- `configs/arenas/*.json` — Arena-Integration
- `tests/` — Tests 96-98

### Schritte
1. Neue Bot-Typen in `NW_SpawnBot` codieren
2. Healer-Logik: `NW_Frame` erweitern
3. Shielder-Logik: `NW_Frame` erweitieren
4. Sniper-Elite-Logik: Dash + Präzision
5. Tests 96-98 schreiben
6. Arena-Integration (optional: Healer-Arena)

## Aufwand
- Tag 1: Implementierung (Healer + Shielder + Sniper-Elite)
- Tag 2: Tests + Balancing
