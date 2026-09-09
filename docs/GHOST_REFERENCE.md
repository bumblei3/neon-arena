# Ghost-Reference

StarCraft-inspiriertes Ghost-Kit für NeonArena (OpenArena, Gametype 14).
Aktiv mit `g_neonwave_ghost 1`. Arena-Loadout (Rail + Lightning) bleibt der Default.
Drei Ghost-Kits per `loadout 0|1|2` / `g_ghost_loadout` (Default: Infiltrator).

> **Produkt ist der OpenArena-Mod.** Der SDL2-GL3-Prototyp unter `prototypes/sdl2-gl3`
> ist nur eine Skizze — Zahlen und Loop dort nicht als Quelle nehmen.
>
> Kit G1–G12 in **v0.90**. Nächster Hebel: [GHOST_ROADMAP](GHOST_ROADMAP.md) (Playtest-Balance).
>
> **Feedback willkommen!** Siehe [README](../README.md#feedback).

## Start

```sh
scripts/start-quake3e.sh --ghost
# oder:
openarena +set fs_game neonarena +g_gametype 14 +set g_neonwave_ghost 1 +map oa_shine
```

Binds in `assets/ghost-binds.cfg` (cgame exec't das File wenn das Kit an ist; `--ghost` kopiert es nach `~/.openarena/neonarena/`): **J** cloak · **H** emp · **K** lockdown · **N** nuke · **RMB** zoom. Spectre-Multiscan hat **keinen Default-Bind** — `bind <taste> multiscan`.

Spawn: Railgun (30 Slugs) — das ist die Sniper. Keine Lightning Gun, kein Gauntlet.
Hip-Fire: normales Rail-Crosshair + Cyan/Gold Hit-Confirm. **RMB** (`+zoom`, `cg_zoomfov 28`): runde Cyan-Blende + Fadenkreuz, Hit-Confirm im Iris, Zoom-In/Out-Sound. Kein extra Feuer-Delay (Rail bleibt 1500 ms).

Sounds (PK3): Cloak `sound/ghost_cloak_on.wav` / aus `_off` · EMP `ghost_emp` · Lock-Treffer `ghost_lock` · Nuke-Paint `ghost_nuke` · Scan-Warn `ghost_scan`. Ambush bleibt `sound/feedback/hit.wav`.

## Loadouts (v1.2)

Konsole: `loadout` zeigt das aktuelle Kit, `loadout 0|1|2` wechselt (setzt Energy auf den Kit-Start, wenn lebend). Persistenz: `g_ghost_loadout`.

| ID | Name | Start-Energy | Cloak | EMP | Lockdown | Nuke | Multiscan |
|----|------|--------------|-------|-----|----------|------|-----------|
| 0 | **Infiltrator** (Default) | 80 | ja | 35 / 25 s | 50 / 20 s | — | — |
| 1 | **Saboteur** | 70 | ja | **25 / 20 s** | **35** / 20 s | — | — |
| 2 | **Spectre** | 90 | — | 35 / 25 s | 50 / 20 s | ja | ja |

Spectre: `cloak` → `CLOAK NOT AVAILABLE`. Infiltrator/Saboteur: `nuke` / `multiscan` → `NOT AVAILABLE`. Lockdown-Dauer bleibt 4 s für alle Kits (Saboteur ist nur günstiger, nicht länger).

## Loop

Infiltrator/Saboteur: Energy farmen → Cloak (Drain) → reposition → RMB-Zoom →
Rail (Ambush 2×) → Kill gibt Energy → EMP-Bolt in den Klumpen → Lockdown auf
Boss/Detector. Spectre: ohne Cloak, dafür Nuke-Calldown und Multiscan.
Ab Welle 8 jagt ein Detector den Cloak.

## Energy

| | Wert |
|---|---|
| Maximum | 100 (`g_ghost_energy_max`) |
| Spawn | Loadout-Start (sonst Fallback `g_ghost_energy_start`, Default 60) |
| Regen | +4 / s (`g_ghost_regen_amt`; nicht während Cloak) |
| Kill (Human) | +15 |

Kein Energy-Spend, wenn die Fähigkeit auf Cooldown ist. Cloak-Toggle-Aus kostet nichts.

## Fähigkeiten

Kosten/CDs gelten für Infiltrator; Saboteur/Spectre siehe Loadout-Tabelle.

| Taste | Command | Cost | Cooldown | Effekt |
|-------|---------|------|----------|--------|
| J | `cloak` | 25 | — (Drain 5/s) | Toggle `PW_INVIS`. Bots sehen dich nicht jenseits von 80 u, außer Detector / Swarm / Boss Phase 2. Nicht Spectre. |
| H | `emp` | 35 | 25 s | Plasma-Bolt: 400 u Armor auf 0 + 1.5 s Stun. |
| K | `lockdown` | 50 | 20 s | Raketen-Bolt (900 u/s). Nur Boss/Detector; Miss refundet Energy, kein CD. |
| N | `nuke` | 80 | 45 s | Calldown: 1.5 s stehen + 4 s inbound. Nur Spectre. |
| — | `multiscan` | 30 | 3 s | 500 u Radius: Cloak-Reveal + 2 s Damage-Bonus. Nur Spectre. |

### Cloak

- J an (25 Energy), J nochmal aus. Drain 5 Energy/s, kein Regen solange cloaked. 0 Energy = auto-decloak.
- Bricht bei Schuss (`FireWeapon`), EMP, Lockdown, eingehendem Schaden und im Detector-Cone.
- Nach jedem Break: 2 s **Ambush** — nächster Rail 2× Schaden (`AMBUSH`), goldener Rail-Trail + Hit-Cue.
- Solange cloaked: kühle Cyan-Vignette auf dem eigenen Bildschirm; andere sehen eine schwache Cyan-Shell (G5, kein volles Stock-Invis).
- Uncloaked (G10): stärkere Cyan-Shell auf Körper und Rail — kein Sarge-Look.
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

### Multiscan (Spectre)

- Command `multiscan`, 30 Energy, 3 s CD, 500 u Radius.
- Enttarnt `PW_INVIS` im Radius (`<name> DETECTED`) und gibt 2 s Damage-Bonus.
- Kein Default-Bind.

## Detector

Ab Welle 8 ein extra Sarge-Bot `Detector W<n>-1` (120 HP, rotes `constantLight`)
plus ein **ortsfester Turm** (`ghost_detector_turret`, 120 HP, dreht, Rail-Ping).
Ab Welle 12 ein zweiter Bot (`W<n>-2`). Bot-Skill = Wellen-Skill + 1 (max 5).
Lockdown trifft Bot **und** Turm. Marker: `DETECTOR turret (wave N)`.

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
| `g_ghost_loadout` | ARCHIVE | 0 | 0 Infiltrator / 1 Saboteur / 2 Spectre |
| `g_ghost_energy_start` | ARCHIVE | 60 | Fallback-Start, wenn Loadout unbekannt |
| `g_ghost_energy_max` | ARCHIVE | 100 | Energy-Cap |
| `g_ghost_regen_amt` | ARCHIVE | 4 | Regen / s (nicht während Cloak) |
| `g_ghost_*` (HUD) | ROM | 0 / `""` | HUD-Spiegel (nicht setzen) |
| `g_neonwave_nextdetector` | intern | 0 | nächster `addbot` wird Detector |

## Code

| Datei | Verantwortung |
|-------|---------------|
| `oa-gamecode/code/game/g_ghost.c` | Loadouts, Energy, Fähigkeiten, Nuke-Calldown, Detector-Think, HUD-Sync |
| `g_neonwave.c` | `NW_GhostFrame` / `NW_GhostOnKill`; Detector-Spawn ab Welle 8 |
| `g_client.c` | Ghost-Spawn (Rail only); Detector-HP 120 |
| `g_weapon.c` / `g_combat.c` | Cloak-Break bei Fire / Damage; Ambush-Rail; Lock blockt Fire |
| `g_missile.c` | EMP-Bolt Impact |
| `ai_dmq3.c` | Cloak vs Bot-Sicht |
| `g_cmds.c` | `cloak` / `emp` / `lockdown` / `nuke` / `multiscan` / `loadout` |
| `g_bot.c` | Userinfo `neonwave_detector` |
| `cgame/cg_draw.c` | Ghost-HUD aus `ps.stats` |
| `bg_public.h` | `STAT_GHOST_ENERGY` / `_CDS` / `_ST` |

`g_ghost.o` steht in beiden Makefile-Objektlisten (`BASEGAME` und `MISSIONPACK`). Alles hinter `NEONARENA_MOD`.

## Log-Marker

```
NeonWave: GHOST kit active (wave N)
NeonWave: DETECTOR spawned (wave N, C, skill S)
Ghost: <name> joined the Ghost team (loadout N)
Ghost: detector revealed client N
Ghost: nuke detonated by <name>
```

Centerprints: `CLOAKED`, `DECLOAKED`, `AMBUSH`, `EMP`, `LOCKED`, `SCANNING`, `DESIGNATING — STAND STILL`, `NUKE INBOUND`, `NUKE N`, `NUCLEAR STRIKE`, `NUKE CANCELLED`, `DETECTED`, `LOADOUT: INFILTRATOR|SABOTEUR|SPECTRE`.

Log: `Ghost: lockdown on <name>`

Weiter: [GHOST_ROADMAP](GHOST_ROADMAP.md). Nicht geplant: extra Rail-Feuerverzögerung, Scanner Sweep, Psionic Storm.
