# NeonArena Modding API

> Referenz für Modder: CVars, Commands, Config-Format, Map-Requirements.

---

## Server-CVars

### Wave-Survival

| CVar | Default | Beschreibung |
|------|---------|--------------|
| `g_neonwave_ghost` | 0 | Ghost-Kit aktivieren (1 = ja) |
| `g_neonwave_modifier` | 0 | Modifier erzwingen (1-14, 0 = auto) |
| `g_neonwave_modifier2` | 0 | Zweiter Modifier (Synergie) |
| `g_neonwave_bosstype` | 0 | Boss-Typ erzwingen (1-8, 0 = auto) |
| `g_neonwave_startwave` | 0 | Start-Welle (1-50) |
| `g_neonwave_maxwave` | 20 | Maximal-Welle (Endless-Modus) |
| `g_neonwave_autostart` | 0 | Wellen automatisch starten (headless) |
| `g_neonwave_autokill` | 0 | Bots automatisch töten (headless) |
| `g_neonwave_fastbreak` | 0 | Wellen-Pause überspringen |
| `g_neonwave_failrun` | 0 | Failed-Run erzwingen |
| `g_neonwave_best` | 0 | Best-Wave zurücksetzen |
| `g_neonwave_daily` | 0 | Daily-Challenge-Modus |
| `g_neonwave_dailyseed` | 0 | Daily-Challenge-Seed |
| `g_neonwave_coopmock` | 0 | Coop-Modus simulieren |
| `g_neonwave_coopdifficulty` | 0 | Coop-Schwierigkeit (0-2) |
| `g_neonwave_selfkill` | 0 | Selbst-Tötung erlauben |
| `g_neonwave_hardcore` | 0 | Hardcore-Modus (1.5x Boss-HP) |
| `g_neonwave_phaseforce` | 0 | Boss-Phase-2 erzwingen |
| `g_neonwave_rageforce` | 0 | Swarm-Mother-Rage erzwingen |
| `g_neonwave_dashforce` | 0 | Sniper-Dash erzwingen |
| `g_neonwave_wardenforce` | 0 | Warden erzwingen |
| `g_neonwave_perkforce` | 0 | Perk erzwingen (ID) |
| `g_neonwave_autopick` | 0 | Perk automatisch wählen |
| `g_neonwave_perkr` | 0 | Perk-Rank erzwingen |
| `g_neonwave_fakecombo` | 0 | Fake-Combo erzwingen |
| `g_neonwave_fakekills` | 0 | Fake-Kills erzwingen |
| `g_neonwave_codex` | 0 | Codex/Bestiary anzeigen |
| `g_neonwave_music` | 1 | Musik aktivieren |
| `g_neonwave_motd` | "" | Message of the Day |
| `g_neonwave_updateavail` | 0 | Update-verfügbar-Anzeige |

### Ghost-Kit

| CVar | Default | Beschreibung |
|------|---------|--------------|
| `g_ghost_energy_start` | 60 | Start-Energie (0-100) |
| `g_ghost_energy_max` | 100 | Max-Energie |
| `g_ghost_regen_amt` | 4 | Regeneration pro Sekunde |

### Replay

| CVar | Default | Beschreibung |
|------|---------|--------------|
| `g_neonwave_replaytest` | 0 | Replay-Test-Modus (1-80) |

### Momentum

| CVar | Default | Beschreibung |
|------|---------|--------------|
| `g_momentum` | 1 | Momentum-System aktivieren |
| `g_momentum_decay` | 5 | Momentum-Abfall pro Sekunde |
| `g_momentum_kill` | 15 | Momentum pro Kill |

### Legacy

| CVar | Default | Beschreibung |
|------|---------|--------------|
| `g_legacy_echo` | 1 | Legacy-Echo aktivieren |
| `g_legacy_boost_dmg` | 20 | Legacy-Boost-Schaden |
| `g_legacy_boost_dur` | 5 | Legacy-Boost-Dauer |

---

## Client-Commands

### Upgrade-System
```
upgrade hp       # +25 HP (max 200)
upgrade dmg      # +10% Schaden
upgrade speed    # +25 Speed (max 400)
```

### Ghost-Kit
```
ghost_cloak      # Cloak ein/aus (25 Energy + 5/s Drain)
ghost_emp        # EMP-Blase (35 Energy, 25s CD)
ghost_lockdown   # Lockdown (50 Energy, 20s CD, 4s Stun)
ghost_nuke       # Nuke (80 Energy, 45s CD)
```

### Replay
```
nw_replay start              # Aufnahme starten
nw_replay stop               # Aufnahme stoppen
nw_replay save [file]        # Aufnahme speichern
nw_replay load [file]        # Aufnahme laden
nw_replay play               # Wiedergabe starten
nw_replay status             # Status anzeigen
nw_replay bugreport [file]   # Bug-Replay speichern
```

---

## Map-Requirements

Eine NeonArena-kompatible Map benötigt:

1. **Spawn-Points**: Mindestens 1 `info_player_deathmatch`
2. **Arena-Größe**: 500-2000 Units (klein bis mittel)
3. **Deckel**: Keine offenen Skyboxen (Bot-Pathfinding)
4. **Items**: Mindestens 1 `item_armor_shard` oder `item_health`

### Map-Config-Format

Erstelle eine `neonarena.cfg` im Map-Ordner:

```cfg
// NeonArena Map Config
set g_neonwave_maxwave 20
set g_neonwave_modifier 0
set g_neonwave_bosstype 0
```

---

## Modifier-Referenz

| ID | Name | Effekt |
|----|------|--------|
| 1 | LOW GRAVITY | Gravity 300 (normal 800) |
| 2 | DOUBLE POINTS | 2x Upgrade-Punkte |
| 3 | AERIAL ASSAULT | Gravity 280 + 3x Punkte |
| 4 | FRENZY | Quad-Faktor 4 |
| 5 | TIME WARP | Speed 480 (normal 320) |
| 6 | VAMPIRE | Lifesteal bei Kills |
| 7 | FRENZY | Quad-Faktor 4 |
| 8 | OVERSHIELD | +50 Armor bei Wave-Start |
| 9 | MIRROR | 1/3 Schaden reflektiert |
| 10 | REGEN | HP bei Wave-Start auffüllen |
| 11 | SURGE | Drones +1 Skill, 3x Punkte |
| 12 | FROST | Speed 220 (langsam) |
| 13 | CHAOS | Zufällige Bot-Skills |
| 14 | MIMIC | Kopiert letzte Welle |

---

## Boss-Referenz

| ID | Name | HP | Spezial |
|----|------|----|---------|
| 1 | SNIPER | 400 | Dash-Angriffe |
| 2 | TANK | 600 | Shield-Cycle |
| 3 | SWARM MOTHER | 500 | Mini-Drones |
| 4 | GLASS CANNON | 200 | Support-Drones |
| 5 | WARDEN | 500 | Strike + Armor |
| 6 | BERSERKER | 700 | Rage bei 30% HP |
| 7 | TELEPORTER | 450 | Blink-Angriffe |
| 8 | HEALER | 300 | Heilt andere Bosse |

---

## Perk-Referenz

| ID | Name | Effekt |
|----|------|--------|
| 1 | PIERCE | Railgun durchdringt Ziele |
| 2 | OVERCHARGE | +50% Schaden für 5s |
| 3 | QUAD | Quad-Damage bei Combo |
| 4 | SPEED | +50 Speed permanent |
| 5 | VAMPIRE | Lifesteal permanent |
| 6 | SHIELD | +25 Armor permanent |

---

## Config-Beispiele

### Klassischer Run (20 Wellen)
```cfg
set g_neonwave_maxwave 20
set g_neonwave_modifier 0
set g_neonwave_bosstype 0
```

### Endless-Modus
```cfg
set g_neonwave_maxwave 50
set g_neonwave_modifier 0
set g_neonwave_bosstype 0
```

### Daily-Challenge
```cfg
set g_neonwave_daily 1
set g_neonwave_dailyseed 12345
```

### Hardcore
```cfg
set g_neonwave_hardcore 1
set g_neonwave_maxwave 20
```

### Ghost-Only
```cfg
set g_neonwave_ghost 1
set g_neonwave_maxwave 20
```

---

## Workshop-Integration

NeonArena unterstützt User-generated Content:

1. **Custom Maps**: Map + `neonarena.cfg` in `fs_game/neonarena/`
2. **Custom Configs**: `.cfg`-Dateien in `fs_game/neonarena/configs/`
3. **Custom Loadouts**: Ghost-Kit-CVars in der Map-Config

### Map-Validator

Prüfe ob eine Map NeonArena-kompatibel ist:

```bash
./tools/validate_map.sh maps/your_map.bsp
```

### Wave-Editor

Erstelle/Bearbeiten von Wellen-Configs:

```bash
./tools/wave_editor.sh configs/my_config.cfg
```

---

## Siehe auch

- [ARCHITECTURE.md](ARCHITECTURE.md) — Code-Struktur
- [BOSS_REFERENCE.md](BOSS_REFERENCE.md) — Alle Bosse
- [MODIFIER_REFERENCE.md](MODIFIER_REFERENCE.md) — Alle Modifier
- [PERK_REFERENCE.md](PERK_REFERENCE.md) — Perk-System
- [GHOST_REFERENCE.md](GHOST_REFERENCE.md) — Ghost-Kit
