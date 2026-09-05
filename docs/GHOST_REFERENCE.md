# Ghost-Reference

StarCraft-inspiriertes Ghost-Kit für NeonArena (OpenArena, Gametype 14).
Aktiv mit `g_neonwave_ghost 1`. Arena-Loadout (Rail + Lightning) bleibt der Default.

> **Produkt ist der OpenArena-Mod.** Der SDL2-GL3-Prototyp unter `prototypes/sdl2-gl3`
> ist nur eine Skizze — Zahlen und Loop dort nicht als Quelle nehmen.
>
> Nächste Slices: [GHOST_ROADMAP](GHOST_ROADMAP.md).
>
> **Feedback willkommen!** Siehe [README](../README.md#feedback).

## Start

```sh
scripts/start-quake3e.sh --ghost
# oder:
openarena +set fs_game neonarena +g_gametype 14 +set g_neonwave_ghost 1 +map oa_shine
```

Binds in `assets/autoexec.cfg`: **J** cloak · **H** emp · **K** lockdown · **N** nuke · **RMB** zoom.

Spawn: Railgun (30 Slugs) — das ist die Sniper. Keine Lightning Gun, kein Gauntlet.
Hip-Fire: normales Rail-Crosshair + Cyan/Gold Hit-Confirm. **RMB** (`+zoom`, `cg_zoomfov 28`): runde Cyan-Blende + Fadenkreuz, Hit-Confirm im Iris, Zoom-In/Out-Sound. Kein extra Feuer-Delay (Rail bleibt 1500 ms).

Sounds (OA-Stock): Cloak `protect3` / aus `wearoff` · EMP `hyprbf1a` · Lock `lg_hit` · Nuke-Paint `bfg_fire` · Ambush `hit`.

## Loop

Energy farmen (Start 55, Nuke braucht 80) → Cloak (Drain) → reposition →
RMB-Zoom → Rail (Ambush 2×) → Kill gibt Energy → EMP-Bolt in den Klumpen →
Lockdown auf Boss/Detector → Nuke als Calldown. Ab Welle 8 jagt ein Detector
den Cloak.

## Energy

| | Wert |
|---|---|
| Maximum | 100 |
| Spawn | 55 |
| Regen | +3 / s (nicht während Cloak) |
| Kill (Human) | +15 |

Kein Energy-Spend, wenn die Fähigkeit auf Cooldown ist. Cloak-Toggle-Aus kostet nichts.

## Fähigkeiten

| Taste | Command | Cost | Cooldown | Effekt |
|-------|---------|------|----------|--------|
| J | `cloak` | 25 | — (Drain 8/s) | Toggle `PW_INVIS`. Bots sehen dich nicht jenseits von 80 u, außer Detector / Swarm / Boss Phase 2. |
| H | `emp` | 35 | 25 s | Plasma-Bolt: 400 u Armor auf 0 + 1.5 s Stun. |
| K | `lockdown` | 50 | 20 s | Raketen-Bolt (900 u/s). Nur Boss/Detector; Miss refundet Energy, kein CD. |
| N | `nuke` | 80 | 45 s | Calldown: 1.5 s stehen + 4 s inbound. |

### Cloak

- J an (25 Energy), J nochmal aus. Drain 8 Energy/s, kein Regen solange cloaked. 0 Energy = auto-decloak.
- Bricht bei Schuss (`FireWeapon`), EMP, Lockdown, eingehendem Schaden und im Detector-Cone.
- Nach jedem Break: 2 s **Ambush** — nächster Rail 2× Schaden (`AMBUSH`), goldener Rail-Trail + Hit-Cue.
- Solange cloaked: kühle Cyan-Vignette auf dem eigenen Bildschirm; andere sehen eine Cyan-Shell (kein volles Stock-Invis).
- Bricht auch eine laufende Nuke-Designation (`NUKE CANCELLED`).
- HUD-Status `CLOAKED` / `AMBUSH`.

### EMP

- Plasma-Bolt (1600 u/s), kein Self-AoE. Explodiert am Treffer oder nach 3 s.
- Bots im 400 u Radius: Armor 0 + Stun 1500 ms.
- Decloakt den Ghost.

### Lockdown

- Raketen-Bolt (sichtbar, 900 u/s), kein Hitscan. Ein Bolt in der Luft.
- Trifft nur Boss (`neonwaveBoss`) oder Detector.
- 4 s: Velocity 0, kein `FireWeapon`, Detector scannt nicht.
- Tell: cyan `constantLight` + vertikaler Rail-Tick, Centerprint `LOCKED`.
- Miss / Trash / Wand: Energy zurück, kein CD (`Lockdown missed`).
- CD 20 s nur bei Treffer. Decloakt beim Abschuss.

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

Ab Welle 8 ein extra Sarge-Bot `Detector W<n>-1` (120 HP, rotes `constantLight`).
Ab Welle 12 ein zweiter (`W<n>-2`). Bot-Skill = Wellen-Skill + 1 (max 5).

| | Wert |
|---|---|
| Range | 400 u |
| Cone | Dot ≥ 0.76 (~40° Halbwinkel) |
| Warn | 800 ms im Cone: roter Rail-Tick Detector → Ghost, HUD `SCANNING` |
| On reveal | danach Cloak-Break + 4 s Swarm (`DETECTED`) |

Cone verlassen vor 800 ms setzt den Timer zurück. Während Swarm sehen **alle** Bots den Cloak. Ohne Swarm sieht nur der Detector selbst (plus Boss ab Phase 2).

Spawn-Pfad: `g_neonwave_nextdetector 1` → `addbot` setzt Userinfo `neonwave_detector` → `pers.neonwaveDetector`.

## Wer sieht Cloak?

`NW_GhostSeesInvis(viewer)` ist wahr wenn:

- Swarm aktiv ist, oder
- `viewer` ein Detector ist, oder
- `viewer` ein Boss in Phase ≥ 2 ist.

Gehookt in `BotEntityVisible` und `BotFindEnemy` (`ai_dmq3.c`). Unsichtbare Spieler unter 80 u bleiben für normale Bots sichtbar (Nahbereich).

## HUD

Pro-Client über `playerState.stats` (lokal und Coop). `g_ghost_*` CVars bleiben Debug-Spiegel.

| Stat | Inhalt |
|------|--------|
| `STAT_GHOST_ENERGY` | 0–100 |
| `STAT_GHOST_CDS` | empSec \| lockSec<<8 \| nukeSec<<16 \| cloakSec<<24 |
| `STAT_GHOST_ST` | Status 1 Cloak / 2 Ambush / 3 Scanning / 4 Detected / 5 Designating / 6 Nuke; Nuke-Countdown in Bits 8–15 |

Leiste unten links, Pips **J H K N** (cyan bereit, orange + Sekunden auf CD). Status zentriert.

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
| `g_client.c` | Ghost-Spawn (Rail only); Detector-HP 120 |
| `g_weapon.c` / `g_combat.c` | Cloak-Break bei Fire / Damage; Ambush-Rail; Lock blockt Fire |
| `g_missile.c` | EMP-Bolt Impact |
| `ai_dmq3.c` | Cloak vs Bot-Sicht |
| `g_cmds.c` | Commands `cloak` / `emp` / `lockdown` / `nuke` |
| `g_bot.c` | Userinfo `neonwave_detector` |
| `cgame/cg_draw.c` | Ghost-HUD aus `ps.stats` |
| `bg_public.h` | `STAT_GHOST_ENERGY` / `_CDS` / `_ST` |

`g_ghost.o` steht in beiden Makefile-Objektlisten (`BASEGAME` und `MISSIONPACK`). Alles hinter `NEONARENA_MOD`.

## Log-Marker

```
NeonWave: GHOST kit active (wave N)
NeonWave: DETECTOR spawned (wave N, C, skill S)
Ghost: detector revealed client N
Ghost: nuke detonated by <name>
```

Centerprints: `CLOAKED`, `DECLOAKED`, `AMBUSH`, `EMP`, `LOCKED`, `SCANNING`, `DESIGNATING — STAND STILL`, `NUKE INBOUND`, `NUKE N`, `NUCLEAR STRIKE`, `NUKE CANCELLED`, `DETECTED`.

Log: `Ghost: lockdown on <name>`

Weiter: [GHOST_ROADMAP](GHOST_ROADMAP.md). Nicht geplant: extra Rail-Feuerverzögerung, Scanner Sweep, Psionic Storm.
