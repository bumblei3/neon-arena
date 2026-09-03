# Architektur

Code-Struktur und Build-System von NeonArena.

> **Feedback willkommen!** Siehe [README](../README.md#feedback).

## Überblick

NeonArena ist ein Wave-Survival-Mod für OpenArena (Quake III Arena). Der Mod
lebt als eigenes Repository (`bumblei3/oa-gamecode`) mit Submodule-Einbindung
ins Haupt-Repo (`bumblei3/neon-arena`).

```
neon-arena/
├── .github/workflows/     # CI/CD (Build, Test, Release)
├── assets/                # Look-Pack (Shader, Texturen, Sounds)
├── docs/                  # Dokumentation
│   ├── ARCHITECTURE.md    # Dieses Dokument
│   ├── BOSS_REFERENCE.md  # Alle 7 Boss-Typen
│   ├── MODIFIER_REFERENCE.md  # Alle 14 Modifier
│   ├── PERK_REFERENCE.md  # Perk-System
│   └── CI-engine-quake3e.md   # Engine-Build-Dokumentation
├── oa-gamecode/           # Submodule: bumblei3/oa-gamecode
│   └── code/
│       ├── game/          # Server-seitige Logik (g_neonwave.c)
│       └── cgame/         # Client-seitige Logik (HUD, Rendering)
├── prototypes/            # Experimentelle Renderer (SDL2, Vulkan)
├── references/            # Historische Referenzen
├── scripts/               # Start-Skripte
├── tests/                 # Headless-Test-Suite
├── Makefile               # Build-Orchestrierung
└── README.md              # Projekt-README
```

## Modul-Grenzen

### Server (`code/game/`)

| Datei | Verantwortung |
|-------|---------------|
| `g_neonwave.c` | Wave-Survival-Hauptlogik (2612 Zeilen): Modifier, Boss, Perks, Records, Coop |
| `g_neonwave.h` | Defines (NW_MOD_*, NW_BOSS_*, NW_PERK_*, NW_MAX_*) |
| `g_cmds.c` | Upgrade-Kommando (`upgrade hp\|dmg\|speed`) |
| `g_main.c` | CVar-Registrierungen, Spielinitialisierung |
| `g_combat.c` | Damage-Hooks (Vampiric Heal, Mirror Reflektion) |
| `g_client.c` | Client-Persistenz (Upgrade-Punkte, Perk-Stände) |
| `g_bot.c` | Bot-Handling |

### Client (`code/cgame/`)

| Datei | Verantwortung |
|-------|---------------|
| `cg_draw.c` | HUD, Modifier-Anzeige, Codex/Bestiary, Upgrade-Shop |

## Datenfluss

```
NeonWave_Frame()
  ├── NW_PickModifier()      → Modifier für diese Welle
  ├── NW_ApplySynergy()      → Synergie-/Anti-Synergie-Prüfung
  ├── NeonWave_StartWave()   → Bot-Spawns, Skill-Berechnung
  ├── NW_SpawnBoss()         → Boss-Spawn (ab Welle 10)
  └── NW_GrantUpgradePoints() → Punkte nach Wave-Clear

NW_Cache()                   → Single-Pass-Aggregation aller Client-Stats
  ├── points                 → Upgrade-Punkte
  ├── kills                  → Run-Kills
  ├── bestCombo              → Beste Combo
  └── bossHp                 → Boss-HP
```

## CVar-System

### Registrierte CVars (g_main.c)

| CVar | Standard | Beschreibung |
|------|----------|-------------|
| `g_neonwave_best` | 0 | Beste Welle (persistent) |
| `g_neonwave_bossattr` | 0 | Boss-Attribute |
| `g_neonwave_music` | 1 | Hintergrundmusik |
| `g_neonwave_coopmock` | 0 | Coop-Mock (Test-Hook) |
| `g_neonwave_selfkill` | 0 | Self-Kill (Test-Hook) |
| `g_neonwave_coopdifficulty` | 0 | Coop-Schwierigkeit |
| `g_neonwave_phaseforce` | 0 | Phase-2-Trigger (Test-Hook) |
| `g_neonwave_modifier2` | 0 | Zweiter Modifier-Slot (Test-Hook) |
| `g_neonwave_daily` | 0 | Daily Challenge |
| `g_neonwave_dailyseed` | 0 | Daily Seed (Test-Hook) |

### Test-Hooks (Headless/CI)

| CVar | Beschreibung |
|------|-------------|
| `g_neonwave_autostart 1` | Wellen starten ohne menschlichen Spieler |
| `g_neonwave_startwave N` | Erzwingt Start bei Welle N |
| `g_neonwave_autokill 1` | Tötet alle Drones jeden Frame |
| `g_neonwave_fastbreak 1` | 500 ms statt 12 s Wellenpause |
| `g_neonwave_bosstype N` | Erzwingt Boss-Typ N |
| `g_neonwave_modifier N` | Erzwingt Modifier N |
| `g_neonwave_maxwave N` | Setzt Max-Welle |

## Build-System

### Abhängigkeiten

- `build-essential`, `bison`, `flex`, `zip`
- OpenArena (zum Testen)
- `xvfb` (für headless Tests)

### Build-Prozess

```bash
git clone --recurse-submodules https://github.com/bumblei3/neon-arena.git
cd neon-arena
./build-mod.sh
```

`build-mod.sh` orchestriert:
1. `cp Makefile.local oa-gamecode/`
2. `make -C oa-gamecode` — baut QVMs + native Module
3. Installiert nach `~/.openarena/neonarena/`
4. Packt Look-PK3 (`neon-look.pk3`)

### Makefile.local

- `NEONARENA_MOD` — Aktiviert NeonArena-spezifischen Code
- `GAMEVERSION` — Wird aus `g_local.h` gelesen
- `BASEGAME` — `baseoa` (OpenArena-Kompatibilität)

## CI/CD

### Workflows

| Workflow | Trigger | Beschreibung |
|----------|---------|-------------|
| `build-mod.yml` | Push, Tag, PR, Manual | Build + Test + Release |
| `build-mod-matrix-test.yml` | Manual | Matrix-Test (4 Chunks) |
| `engine-quake3e.yml` | Push, Manual | Engine-Build (Quake3e) |

### Test-Suite

- 56 Tests (1-56 + 9b)
- Verteilung auf 4 parallele Chunks
- Headless via `ioq3ded` + `xvfb-run`
- Assertions prüfen Log-Marker + Anti-Patterns

### Release

- Tag `v*` triggert automatisch
- GAMEVERSION muss mit Tag übereinstimmen
- Erstellt `neonarena.pk3` + `neonarena-qvm.pk3`
- Veröffentlicht als GitHub Release

## Performance

### Single-Pass-Cache (`NW_Cache()`)

Statt 5 separater O(maxclients) Loops (RunKills, BestCombo, CurrentCombo,
BossHealth, PointsBroadcast) wird eine einzige Aggregation verwendet und
bei State-Change invalidiert.

### Bot-Spawning

- Welle N spawnt N+1 Bots
- Skill: `1 + wave / 3` (1 → 5)
- SWARM-Modifier: doppelte Drone-Zahl

## Coop

- Bis 4 Spieler
- Upgrade-Punkte pro Client (per-client pool)
- Wave-Clear: Alle Drones tot + mindestens ein Human lebt
- Respawn: Tote Spieler respawnen am Wellenstart mit vollem HP/Armor
- Coop-Skalierung: Mehr Drones + Skill pro Human

## Historie

| Version | Meilenstein |
|---------|-------------|
| v0.5 | Boss-Railgun, Boss-Healthbar, Upgrades |
| v0.8 | Replay-Wert, Run-Statistik |
| v0.9 | Modi & Bosse (SNIPER, TANK, SWARM) |
| v0.13 | Submodule-Strategie (oa-gamecode) |
| v0.14 | Daily Challenge |
| v0.15 | WARDEN (Boss #5) |
| v0.16 | Dynamic Difficulty |
| v0.27 | Modifier-Reveal (VAMPIRE, FRENZY, OVERSHIELD) |
| v0.32 | Deck-Expansion (MIRROR, REGEN, SURGE) |
| v0.33 | Boss-Phase-2 |
| v0.35 | Synergie-/Anti-Synergie-System |
| v0.37 | Perk-System, AERIAL ASSAULT |
| v0.40 | BERSERKER (Boss #6) |
| v0.41 | TELEPORTER (Boss #7) |
| v0.43 | Steigende Upgrade-Kosten (1→2 ab Level 4), 60 Tests |
