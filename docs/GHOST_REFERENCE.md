# Ghost-Reference

StarCraft-inspiriertes Ghost-Kit für NeonArena (OpenArena, Gametype 14).
Aktiv mit `g_neonwave_ghost 1`. Arena-Loadout (Rail + Lightning) bleibt der Default.

> **Produkt ist der OpenArena-Mod.** Der SDL2-GL3-Prototyp unter `prototypes/sdl2-gl3`
> ist nur eine Skizze — Zahlen und Loop dort nicht als Quelle nehmen.
>
> **Feedback willkommen!** Siehe [README](../README.md#feedback).

## Start

```sh
scripts/start-quake3e.sh --ghost
# oder:
openarena +set fs_game neonarena +g_gametype 14 +set g_neonwave_ghost 1 +map oa_shine
```

Binds in `assets/autoexec.cfg`: **J** cloak · **H** emp · **N** nuke.

Spawn: Railgun (30 Slugs) + Gauntlet. Keine Lightning Gun.

## Loop

Energy farmen (Start 40, Nuke braucht 80) → Cloak → reposition → Rail →
Kill gibt Energy → EMP in den Klumpen → Nuke als Calldown (stehen bleiben,
Laser, 4-3-2-1, Bots fliehen). Ab Welle 8 jagt ein Detector den Cloak.

## Energy

| | Wert |
|---|---|
| Maximum | 100 |
| Spawn | 40 |
| Regen | +3 / s (auch während Cloak) |
| Kill (Human) | +15 |

Kein Energy-Spend, wenn die Fähigkeit auf Cooldown ist oder Cloak schon aktiv ist.

## Fähigkeiten

| Taste | Command | Cost | Cooldown | Effekt |
|-------|---------|------|----------|--------|
| J | `cloak` | 40 | — (5 s Duration) | `PW_INVIS`. Bots sehen dich nicht jenseits von 80 u, außer Detector / Swarm / Boss Phase 2. |
| H | `emp` | 35 | 25 s | Instant 400 u Radius: Bots 1.5 s Stun (`PMF_TIME_KNOCKBACK`). |
| N | `nuke` | 80 | 45 s | Calldown: 1.5 s stehen + 4 s inbound. |

### Cloak

- 5 s Unsichtbarkeit nach einmaligem 40-Energy-Kauf. Kein Toggle-Aus, kein Drain.
- Bricht bei Schuss (`FireWeapon`), bei eingehendem Schaden (`G_Damage`) und im Detector-Cone.
- Bricht auch eine laufende Nuke-Designation (`NUKE CANCELLED`).
- HUD-Status `CLOAKED`.

### EMP

- Self-AoE um den Spieler, kein Projektil.
- Trifft nur Bots. Stun 1500 ms, Velocity 0.
- Sound: `sound/weapons/plasma/plasmx1a.wav`.

### Tac Nuke (Calldown)

1. Trace vom View auf die Map → Epicenter.
2. **Designation 1.5 s:** stehen bleiben (Bewegung > 24 u, Tod, Schaden oder Cloak-Break = Cancel). Repeating Sky-Laser (`EV_RAILTRAIL`). Banner `DESIGNATING — STAND STILL`.
3. **Inbound 4 s:** Countdown `NUKE 4` … `NUKE 1` für alle Clients. Bots im Radius + 192 u fliehen mit 420 u/s vom Epicenter weg.
4. **Detonation:** BFG-Miss-FX (`EV_MISSILE_MISS` / `WP_BFG`), nicht Rocket.

| Ziel | Schaden |
|------|---------|
| Trash-Bot | 10000 (instakill) |
| Boss | 40 % von `pers.maxHealth` |
| Human (Selbst/Coop) | 40 |
| Means of death | `MOD_BFG` |
| Radius | 600 u |

## Detector

Ab Welle 8 ein extra Sarge-Bot `Detector W<n>` (120 HP, rotes `constantLight`).

| | Wert |
|---|---|
| Range | 400 u |
| Cone | Dot ≥ 0.76 (~40° Halbwinkel) |
| On reveal | Cloak-Break + 4 s Swarm (`DETECTED`) |

Während Swarm sehen **alle** Bots den Cloak. Ohne Swarm sieht nur der Detector selbst (plus Boss ab Phase 2).

Spawn-Pfad: `g_neonwave_nextdetector 1` → `addbot` setzt Userinfo `neonwave_detector` → `pers.neonwaveDetector`.

## Wer sieht Cloak?

`NW_GhostSeesInvis(viewer)` ist wahr wenn:

- Swarm aktiv ist, oder
- `viewer` ein Detector ist, oder
- `viewer` ein Boss in Phase ≥ 2 ist.

Gehookt in `BotEntityVisible` und `BotFindEnemy` (`ai_dmq3.c`). Unsichtbare Spieler unter 80 u bleiben für normale Bots sichtbar (Nahbereich).

## HUD

Client liest ROM-CVars (Server synct ~alle 200 ms):

| CVar | Inhalt |
|------|--------|
| `g_ghost_energy` | 0–100 |
| `g_ghost_cloakms` | Rest-Cloak in ms |
| `g_ghost_empcd` | EMP-Cooldown ms |
| `g_ghost_nukecd` | Nuke-Cooldown ms |
| `g_ghost_status` | `DESIGNATING` / `NUKE N` / `DETECTED` / `CLOAKED` / leer |

Leiste unten links + Zeile `GHOST <energy>  J cloak  H emp  N nuke`. Status zentriert.

## CVars

| CVar | Flags | Default | Rolle |
|------|-------|---------|-------|
| `g_neonwave_ghost` | ARCHIVE \| SERVERINFO | 0 | Kit an/aus |
| `g_ghost_*` | ROM | 0 / `""` | HUD-Spiegel (nicht setzen) |
| `g_neonwave_nextdetector` | intern | 0 | nächster `addbot` wird Detector |

## Code

| Datei | Verantwortung |
|-------|---------------|
| `oa-gamecode/code/game/g_ghost.c` | Energy, Fähigkeiten, Nuke-Calldown, Detector-Think, HUD-Sync |
| `g_neonwave.c` | `NW_GhostFrame` / `NW_GhostOnKill`; Detector-Spawn ab Welle 8 |
| `g_client.c` | Ghost-Spawn (Rail+Gauntlet); Detector-HP 120 |
| `g_weapon.c` / `g_combat.c` | Cloak-Break bei Fire / Damage |
| `ai_dmq3.c` | Cloak vs Bot-Sicht |
| `g_cmds.c` | Commands `cloak` / `emp` / `nuke` |
| `g_bot.c` | Userinfo `neonwave_detector` |
| `cgame/cg_draw.c` | Ghost-HUD in `CG_DrawNeonWave` |

`g_ghost.o` steht in beiden Makefile-Objektlisten (`BASEGAME` und `MISSIONPACK`). Alles hinter `NEONARENA_MOD`.

## Log-Marker

```
NeonWave: DETECTOR spawned (wave N)
Ghost: detector revealed client N
Ghost: nuke detonated by <name>
```

Centerprints: `CLOAKED`, `EMP`, `DESIGNATING — STAND STILL`, `NUKE INBOUND`, `NUKE N`, `NUCLEAR STRIKE`, `NUKE CANCELLED`, `DETECTED`.

## Geplant (nächste Slice)

Noch nicht im OA-Mod — nur die empfohlene Richtung:

1. **Cloak als Toggle + Drain** — 25 Energy an, ~8/s Drain, J nochmal aus, 0 = auto-decloak. Kein Regen während Cloak. Erster Rail nach Break = Ambush 2×.
2. **EMP als Projektil** — Plasma-Bolt, Armor auf 0 im Radius + kurzer Stun (kein Self-AoE).
3. **Lockdown (K)** — Trace auf Boss/Detector: 4 s kein Move/Fire. 50 Energy, 20 s CD.
4. Gauntlet aus dem Ghost-Spawn.

Nicht geplant: extra Rail-Feuerverzögerung (Q3-Rail hat schon 1500 ms). Scanner Sweep ist Comsat/Orbital, kein Ghost. Psionic Storm ist High Templar.
