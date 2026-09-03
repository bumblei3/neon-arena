# Modifier-Reference

Alle 14 Modifier + Synergie-/Anti-Synergie-System in NeonArena.

> **Feedback willkommen!** Siehe [README](../README.md#feedback).

## Überblick

Modifier treten ab Welle 5 auf (auch in Boss-Wellen seit v0.37). Ein zweiter Modifier (Slot 2) wird ab Welle 8 hinzugefügt — wenn beide Slots belegt sind, wird geprüft, ob sie einen Synergie- oder Anti-Synergie-Paar bilden.

Rotation: `(wave - 5 + dailyOffset) % NW_MOD_POOL_SIZE`

## Modifier-Tabelle

| # | Name | ID | Effekt | Standardwert |
|---|------|----|--------|--------------|
| 0 | NONE | 0 | Kein Modifier | — |
| 1 | GLASS DRONES | 1 | Drones sterben mit 1 Treffer, aber +2 Skill-Aggression | — |
| 2 | SWARM | 2 | Doppelte Drone-Zahl, Skill gedeckelt | — |
| 3 | LOW GRAVITY | 3 | g_gravity halbiert | `g_gravity 400` |
| 4 | DOUBLE POINTS | 2 | Wave-Clear gibt x2 Upgrade-Punkte | — |
| 5 | TIME WARP | 5 | Player-Speed skaliert (g_speed) | `g_speed 280` |
| 6 | VAMPIRE | 6 | Jeder Kill heilt den Player | `vampheal 4` HP/Kill |
| 7 | FRENZY | 7 | g_quadfactor boosted (Schaden ×) | `g_quadfactor 4` |
| 8 | OVERSHIELD | 8 | Player erhält Bonus-Armor am Wellenstart | `+50 Armor` |
| 9 | MIRROR | 9 | Bot→Human-Schaden wird reflektiert | `1/3 Reflektion` |
| 10 | REGEN | 10 | Player regeneriert HP am Wellenstart | `Full HP` |
| 11 | SURGE | 11 | Zähmere Drones (+1 Skill), x3 Upgrade-Punkte | `ptsMul × 3` |
| 12 | FROST | 12 | Verlangsamter Player, frostige Drones | `g_speed 220` |
| 13 | CHAOS | 13 | Chaotische Spawns: Random Skill + Spawn-Delay | — |
| 14 | MIMIC | 14 | Drones kopieren zufälliges Upgrade von Human | — |

## Synergie-Paare

Wenn zwei bestimmte Modifier gleichzeitig aktiv sind, bilden sie ein Synergie- oder Anti-Synergie-Paar mit zusätzlichen Effekten.

### Positive Synergien

| # | Name | Kombination | Effekt |
|---|------|-------------|--------|
| 0 | AERIAL ASSAULT | LOW GRAVITY + DOUBLE POINTS | Gravity 280, Punkte ×3 |
| 1 | BLOOD WELL | VAMPIRE + REGEN | Lifesteal 8 HP/Kill |
| 2 | OVERDRIVE | FRENZY + SURGE | Quadfactor 6 (Schaden ×6) |
| 3 | HIVE MIRROR | SWARM + MIRROR | Reflektion gestärkt auf 1/2 |

### Anti-Synergien (Negativ)

| # | Name | Kombination | Effekt |
|---|------|-------------|--------|
| 4 | SHIELD BLEED | OVERSHIELD + VAMPIRE | Overshield nur 25, Lifesteal nur 2 |
| 5 | DRIFT LOCK | TIME WARP + LOW GRAVITY | Speed 400, Gravity 600 (geklammert) |

## Modifier-Pool-Reihenfolge

```
[GLASS, SWARM, LOWGRAV, DOUBLEPTS, TIMEWARP, VAMPIRE, FRENZY, OVERSHIELD,
 MIRROR, REGEN, SURGE, FROST, CHAOS, MIMIC, MIMIC]
```

MIMIC kommt doppelt vor (höhere Wahrscheinlichkeit).

## Test-Hooks

| CVar | Beschreibung |
|------|-------------|
| `g_neonwave_modifier N` | Erzwingt Modifier N in Slot 1 |
| `g_neonwave_modifier2 N` | Erzwingt Modifier N in Slot 2 |
| `g_neonwave_maxwave N` | Setzt Max-Welle (Standard: 20) |

## Log-Marker

```
NeonWave: SYNERGY <NAME> (<Mod1> + <Mod2>)
NeonWave: SYNERGY EFFECT <NAME>: <Effekt>
NeonWave: ANTI-SYNERGY <NAME> (<Mod1> + <Mod2>)
NeonWave: ANTI-SYNERGY EFFECT <NAME>: <Effekt>
NeonWave: GLASS DRONES active
NeonWave: SWARM active
NeonWave: LOW GRAVITY active
NeonWave: DOUBLE POINTS active
NeonWave: TIME WARP active
NeonWave: VAMPIRE lifesteal
NeonWave: FRENZY quadfactor set to <N>
NeonWave: OVERSHIELD +<N> armor granted
NeonWave: MIRROR active (mask <N>, slot2=<0|1>)
NeonWave: REGEN health topped up
NeonWave: SURGE drones hardened
NeonWave: SURGE x3 upgrade points
NeonWave: FROST slowed to <speed>
NeonWave: CHAOS mode — random skill per drone
NeonWave: MIMIC copies <Upgrade> level <N> from <Player>
```

## CVar-Liste

| CVar | Beschreibung |
|------|-------------|
| `g_neonwave_modifier_active` | Bitmask der aktiven Modifier |
| `g_neonwave_vampheal` | Lifesteal-Betrag pro Kill |
| `g_neonwave_mirrordiv` | Reflektions-Divisor (3 = 1/3) |
| `g_gravity` | Wird durch LOW GRAVITY gesetzt |
| `g_speed` | Wird durch TIME WARP/FROST gesetzt |
| `g_quadfactor` | Wird durch FRENZY gesetzt |
