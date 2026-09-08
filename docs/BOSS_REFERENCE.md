# Boss-Reference

Alle 13 Boss-Typen in NeonArena. Bosse spawnen ab Welle 10, einer pro Welle.
Rotation: `(wave - 10 + dailyOffset) % NW_BOSS_COUNT` (Typen 1–13).
In einem 20-Wellen-Lauf erscheinen Chronomancer/Void Walker nur mit Daily-Offset,
Endless oder `g_neonwave_bosstype`.

> **Feedback willkommen!** Siehe [README](../README.md#feedback).

## Boss-Tabelle

| # | Name | Basis-HP | Waffe | Besonderheit | Phase-2-Eskalation |
|---|------|----------|-------|--------------|-------------------|
| 1 | SNIPER | 400 (4×) | Railgun | Fernkampf, präzise | Häufigere Dashes |
| 2 | TANK | 600 (6×) | MG | Langsam, 40 % Speed-Malus | Kürzere/öftere Shield-Cycles |
| 3 | SWARM MOTHER | 400 (4×) | Railgun | Spawnt alle 10 s Mini-Drones | Schnellere Mini-Drone-Spawns |
| 4 | GLASS CANNON | 200 (2×) | Railgun | Schnell (140 % Speed), fragil | Ruft Support-Drones |
| 5 | WARDEN | 500 (5×) | MG | Teleportiert in Spielerzone + Armor-Phase (3 s) | Kürzere Strike-Intervalle |
| 6 | BERSERKER | 700 (7×) | MG | Langsam, massiv — enrages below 30 % HP | RAGE-Modus (schnellere Angriffe) |
| 7 | TELEPORTER | 350 (3.5×) | MG | Teleportiert weg bei Treffer (evasiv) | Häufigere Blinks |
| 8 | HEALER | 300 (3×) | none | Heilt nahe Bots, fragil | Heilt öfter und mehr HP |
| 9 | SHIELDER | 400 (4×) | energy shield | Deploys shield vs projectiles | Shield drops bei HP < 200 |
| 10 | SNIPER ELITE | 400 (4×) | Railgun | Schneller Rail-Schuss (1.5 s) | Schnellere Feuerrate |
| 11 | DEMOLISHER | 400 (4×) | rockets | Rocket-Barrage alle 3 s | Mehr Rockets pro Salve |
| 12 | CHRONOMANCER | 450 (4.5×) | MG | Teleport 6 s + Slow-Puls 12 s | Teleport 4 s, Slow 8 s |
| 13 | VOID WALKER | 550 (5.5×) | MG | Phaseshift-Invuln 2 s / 8 s, Void-Aura | Phaseshift 5 s; Enrage < 30 % HP |

## HP-Skalierung

Boss-HP skaliert mit der Welle (seit v0.42):

```
HP = base_HP × (100 + (wave - 10) × 20) / 100
```

| Welle | Multiplikator |
|-------|---------------|
| 10 | 1.0× |
| 11 | 1.2× |
| 12 | 1.4× |
| 15 | 2.0× |
| 20 | 3.0× |

Zusätzlich:
- **Dynamic Difficulty:** `HP × (100 + difficulty × 15) / 100` (difficulty: -2..1)
- **Hardcore:** `HP × 1.5`

## Phase 2

Jeder Boss wechselt bei ≤50 % HP in Phase 2. Trigger:
- Natürlich: HP ≤ 50 %
- Test-Hook: `g_neonwave_phaseforce 1` (erzwingt sofort)

Phase-2-Marker im Log: `NeonWave: <NAME> ENTERS PHASE 2`

### Verhalten pro Typ in Phase 2

- **SNIPER:** Häufigere Reposition (Dash-Cooldown halbiert)
- **TANK:** Shield-Cycle von 5 s auf 2.5 s, Shield-Dauer 3 s statt 5 s
- **SWARM MOTHER:** Mini-Drone-Spawn-Rate von 10 s auf 5 s
- **GLASS CANNON:** Ruft alle 8 s einen Support-Drone (Railgun, 100 HP)
- **WARDEN:** Strike-Intervall von 4 s auf 2 s
- **BERSERKER:** RAGE-Modus — Angriffsgeschwindigkeit +50 %, Bewegung +30 %
- **TELEPORTER:** Blink-Cooldown von 3 s auf 1.5 s
- **HEALER:** Heilt öfter (Cooldown halbiert) und mehr HP pro Heilung (+50%)
- **SHIELDER:** Shield-Cycle kürzer, aber Shield-Dauer sinkt — bei <25 % HP fällt Shield
- **SNIPER ELITE:** Feuerrate +50 %, Tarn-Cooldown halbiert, schneller Rail-Schuss
- **DEMOLISHER:** Mehr Rockets pro Salve (+33 %), kürzere Salven-Intervalle
- **CHRONOMANCER:** Teleport-CD 6 s → 4 s, Slow-Puls 12 s → 8 s
- **VOID WALKER:** Phaseshift-CD 8 s → 5 s; unter 30 % HP `VOID WALKER ENRAGES`

| CVar | Beschreibung |
|------|-------------|
| `g_neonwave_bosstype N` | Erzwingt Boss-Typ N (1-13) |
| `g_neonwave_phaseforce 1` | Erzwingt Phase-2-Trigger sofort |
| `g_neonwave_rageforce 1` | Erzwingt Berserker-Rage |
| `g_neonwave_wardenforce 1` | Erzwingt Warden-Strike |
| `g_neonwave_dashforce N` | Erzwingt Sniper-Dash alle N ms |
| `g_neonwave_healforce 1` | Erzwingt Healer-Heilung |
| `g_neonwave_shieldforce 1` | Erzwingt Shielder-Shield |
| `g_neonwave_cloakforce 1` | Erzwingt Sniper-Elite-Tarnung |
| `g_neonwave_barrageforce 1` | Erzwingt Demolisher-Rocket-Barrage |

## Log-Marker

```
NeonWave: boss spawned: <NAME> (hc <HP>)
NeonWave: <NAME> ENTERS PHASE 2
NeonWave: TANK raises SHIELD
NeonWave: TANK shield drops
NeonWave: SWARM MOTHER ENRAGED
NeonWave: BERSERKER ENTERS RAGE
NeonWave: TELEPORTER blinks to new position
NeonWave: WARDEN strikes the player zone
NeonWave: WARDEN raises armor
NeonWave: SNIPER dashes to new position
NeonWave: GLASS CANNON summons support drone
NeonWave: HEALER heals nearby bots for N HP
NeonWave: SHIELDER deploys energy shield
NeonWave: SHIELDER shield drops
NeonWave: SNIPER ELITE rapid rail
NeonWave: DEMOLISHER fires rocket barrage
NeonWave: CHRONOMANCER warps time and teleports
NeonWave: CHRONOMANCER slows nearby bots
NeonWave: VOID WALKER phaseshift
NeonWave: VOID WALKER materializes
NeonWave: VOID WALKER void aura pulses
NeonWave: VOID WALKER ENRAGES
```
