# Perk-Reference

Das Perk-System in NeonArena: 3 Angebote pro Wellenpause, F1/F2/F3 wählen.

> **Feedback willkommen!** Siehe [README](../README.md#feedback).

## Perk-Tabelle

| ID | Name | Cap | Effekt |
|----|------|-----|--------|
| 1 | PIERCE | 3 | Railgun-Positionen ignorieren Wände (Bis zu 3 Durchdringungen) |
| 2 | CHAIN | 3 | Lightning Gun ricochet auf bis zu 3 additional targets |
| 3 | DASH | 2 | Doppel-Tap Sprint-Dash (Richtung + Speed) |
| 4 | OVERCHARGE | 2 | +50 % Schaden für 5 s nach jedem Kill (Stacking) |
| 5 | SECOND WIND | 1 | Ein Tod abfangen (Auto-Revive mit 50 % HP) |
| 6 | SKIP | 1 | Nächste Welle ohne Modifier (verbraucht sich) |

## Perk-Details

### PIERCE (Railgun Durchdringung)
- **Cap:** 3
- **Effekt:** Railgun-Bolt durchdringt N Enemies statt nach dem ersten aufzuhören
- **Level 1:** Durchdringt 1 Enemy (2 Treffer insgesamt)
- **Level 2:** Durchdringt 2 Enemies (3 Treffer)
- **Level 3:** Durchdringt 3 Enemies (4 Treffer)

### CHAIN (Lightning Gun Ricochet)
- **Cap:** 3
- **Effekt:** Lightning Gun springt zu N additional targets über
- **Level 1:** +1 Target (2 Treffer)
- **Level 2:** +2 Targets (3 Treffer)
- **Level 3:** +3 Targets (4 Treffer)

### DASH (Sprint-Dash)
- **Cap:** 2
- **Effekt:** Doppel-Tap Bewegungstaste = Dash in Richtung (500 ms Speed-Boost)
- **Cooldown:** 3 s

### OVERCHARGE (Kill-Buff)
- **Cap:** 2
- **Effekt:** Jeder Kill gibt +50 % Schaden für 5 s (stacking bis zu +100 %)
- **Level 1:** +50 % Schaden pro Stack
- **Level 2:** +50 % Schaden pro Stack, doppelte Stack-Dauer (10 s)

### SECOND WIND (Auto-Revive)
- **Cap:** 1
- **Effekt:** Beim Tod automatischer Revive mit 50 % HP (einmal pro Run)
- **Verbraucht sich nach Nutzung**

### SKIP (Modifier überspringen)
- **Cap:** 1
- **Effekt:** Nächste Welle hat keinen Modifier
- **Verbraucht sich nach Nutzung**

## Perk-Angebote

Pro Wellenpause werden 3 zufällige Perks als F1/F2/F3-Angebot angezeigt:
- Wiederholungen möglich (gleicher Perk mehrmals)
- Perks am Cap werden nicht angeboten
- SKIP und SECOND WIND nur wenn noch nicht am Cap

## Test-Hooks

| CVar | Beschreibung |
|------|-------------|
| `g_neonwave_perkforce NNN` | Erzwingt Perk-Angebote (3-stellige ID, z.B. `146` = PIERCE+OVERCHARGE+SKIP) |
| `g_neonwave_perkr N` | Setzt Rank für einen Perk (intern) |
| `g_neonwave_autopick 1` | Wählt automatisch F1-Angebot |

## Log-Marker

```
NeonWave: PERK OFFER F1=<Name> F2=<Name> F3=<Name>
NeonWave: PERK TAKEN <Name>
NeonWave: PERK RANK <Name> rank <N>
NeonWave: PERK EFFECT <Name>
NeonWave: SKIP modifier (client <ID>)
NeonWave: SECOND WIND consumed
```

## CVar-Liste

| CVar | Beschreibung |
|------|-------------|
| `g_neonwave_perkforce` | Erzwungene Perk-Angebote (3-stellige ID) |
| `g_neonwave_perkr` | Perk-Rank-Override für Tests |
| `g_neonwave_autopick` | Auto-F1-Pick (Test-Hook) |
