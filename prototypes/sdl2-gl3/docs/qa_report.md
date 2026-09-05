# NeonArena Qualitätssicherung Report
> Erstellt: 2026-09-05
> Stand: v0.70

## Kritische Probleme (sofort fixen)

### 1. Memory Leaks im Prototyp
- **audio_manager.cpp:61,90**: `new short[]` und `new Mix_Chunk()` ohne `delete`
- **game.cpp:26,38,42,45**: `new Renderer()`, `new ParticleSystem()`, etc. ohne `delete`
- **renderer.cpp**: Mehrere `new Shader()` ohne `delete`
- **text.cpp:161**: `new unsigned char[]` ohne `delete`

### 2. Test-Abdeckung für neue Features
- **NW_BOSS_HEALER**: Kein direkter Test (nur indirekt über Test 71)
- **NW_MOD_SHIELD**: Kein direkter Test (nur indirekt über Test 72)

## Mittlere Probleme (beim nächsten Mal fixen)

### 3. Unsichere String-Funktionen
- `strcpy(name, "[world]")` in ai_chat.c — Puffergröße unbekannt
- `strcpy(bs->subteam, "")` in ai_cmd.c
- `strcpy(flagstatus, "  ")` in ai_main.c

### 4. FIXME-Kommentare aufräumen
- 20 FIXMEs in ai_cmd.c, ai_dmnet.c, ai_dmq3.c
- Einige sind seit Jahren offen

## Niedrige Probleme (optional)

### 5. Code-Duplikation
- Mehrmalige Boss-Logik in g_neonwave.c
- String-Konstanten nicht zentralisiert

## Nächste Schritte

- [x] Memory Leaks fixen (AudioManager)
- [ ] Tests für neue Features hinzufügen
- [ ] strcpy/strcat ernsetzen durch Q_strncpyz/Q_strncat
- [ ] FIXME-Kommentare aufräumen oder als Issues tracken
