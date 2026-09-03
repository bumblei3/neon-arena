# Engine-Integration

NeonArena verwendet die [Quake3e](https://github.com/ec-/quake3e)-Engine als modernen Renderer
mit Bloom, HDR und Vulkan-Unterstützung. Dieses Dokument beschreibt die Integration.

## Architektur

```
┌─────────────────────────────────────────────────────────┐
│                    NeonArena Mod                         │
│  (QVMs: cgame.qvm, qagame.qvm, ui.qvm)                 │
├─────────────────────────────────────────────────────────┤
│                    Quake3e Engine                        │
│  (quake3e.x64 + Renderer: OpenGL2/Vulkan)               │
├─────────────────────────────────────────────────────────┤
│                    OpenArena Assets                      │
│  (baseoa/*.pk3 — vom Spieler bereitgestellt)            │
└─────────────────────────────────────────────────────────┘
```

## Engine vs. Mod

- **Engine:** Rendering, Input, Netzwerk, Sound (binär, pro Plattform)
- **Mod:** Spiellogik, HUD, Assets (QVMs + PK3s, plattformunabhängig)

## Releases

### Mod-Release (Haupt-Release)

Jedes `v*` Tag triggert:
- QVM-Build + PK3-Packaging
- Test-Suite (56 Tests)
- GitHub Release mit `neonarena.pk3` + `neonarena-qvm.pk3`

### Engine-Release (optional)

Engine-Builds werden bei jedem Push auf `main` erstellt und als
CI-Artifacts veröffentlicht. Bei Tags werden sie dem Release angehängt.

## Installation

### Linux

```bash
# Engine herunterladen (aus Release oder CI)
tar -xzf neonarena-engine-linux64.tar.gz
mv neonarena-engine ~/quake3e-engine/

# Start
~/quake3e-engine/quake3e.x64 \
  +set cl_renderer vulkan \
  +set r_bloom 1 \
  +set fs_basepath ~/quake3e-engine \
  +set fs_homepath ~/.openarena \
  +set fs_game neonarena \
  +g_gametype 14 +map oa_shine
```

### Windows

```powershell
# Engine herunterladen und entpacken
# quake3e.exe + quake3e_vulkan.dll + SDL2.dll in Ordner mit baseoa pk3s
quake3e.exe +set cl_renderer vulkan +set r_bloom 1 +set fs_game neonarena +g_gametype 14 +map oa_shine
```

### macOS

```bash
# Engine herunterladen und entpacken
./quake3e.universal \
  +set cl_renderer vulkan \
  +set r_bloom 1 \
  +set fs_basepath . \
  +set fs_homepath ~/.openarena \
  +set fs_game neonarena \
  +g_gametype 14 +map oa_shine
```

## OpenArena-Basisdateien

Quake3e erfordert `baseoa/*.pk3` (OpenArena-Content). Diese können kommen von:

- OpenArena-Installation (`/usr/share/games/openarena/` oder `~/.openarena/baseoa/`)
- Steam-Version (Pfad anpassen)
- Eigenständiger OpenArena-Install

Die Engine wird mit `BASEGAME=baseoa` gebaut, sodass sie direkt aus `baseoa/` lädt.

## Renderer-Features

### Vulkan-Renderer (empfohlen)

- Moderne Pipeline, bessere Performance
- Vollständige Bloom-Unterstützung
- HDR-Framebuffer

### OpenGL2-Renderer (Fallback)

- Ältere Hardware
- ebenfalls Bloom-Unterstützung
- Ähnliche Features, geringere Performance

### CVars

| CVar | Standard | Beschreibung |
|------|----------|-------------|
| `cl_renderer` | opengl | Renderer: `opengl` oder `vulkan` |
| `r_bloom` | 0 | Bloom an/aus (1/0) |
| `r_bloom_intensity` | 0.5 | Bloom-Stärke |
| `r_bloom_threshold` | 0.6 | Bloom-Extraktions-Schwelle |
| `r_bloom_modulate` | 0 | Farbmodulation (0/1/2) |
| `r_hdr` | 0 | HDR-Framebuffer (0/1) |
| `r_ext_multisample` | 0 | MSAA (0/2/4/6/8) |

## Bloom-Kalibrierung für NeonArena

NeonArena nutzt additive, leuchtende Elemente. Bloom-Werte:

```
r_bloom 1
r_bloom_intensity 0.5      # Standard (0.3-0.7)
r_bloom_threshold 0.6      # Standard (0.4-0.8)
r_bloom_modulate 0         # Aus (1=selbstmoduliert, 2=Intensität)
r_bloom_threshold_mode 0   # Beliebiger Kanal
```

Bei Überbelichtung: `r_bloom_intensity` senken oder `r_bloom_threshold` erhöhen.
Bei Unterbelichtung: `r_bloom_intensity` erhöhen oder `r_bloom_threshold` senken.

## CI/CD

### engine-quake3e.yml

- Trigger: Push auf `main`, manuell
- Matrix: Linux, Windows (Cross), macOS
- Artifacts: Engine-Pakete pro Plattform
- Release: Anhängen an Tag-Release

### build-mod.yml

- Trigger: Push, Tag, PR, manuell
- Build: QVMs + PK3s
- Tests: 56 Tests
- Release: `neonarena.pk3` + `neonarena-qvm.pk3`

## Troubleshooting

### Engine startet nicht
- `fs_basepath` muss auf Engine-Ordner zeigen
- `fs_homepath` muss `baseoa/` enthalten
- `BASEGAME=baseoa` muss im Engine-Build aktiv sein

### Vulkan nicht verfügbar
- Fallback auf OpenGL: `cl_renderer opengl`
- Treiber aktualisieren (NVIDIA/AMD)

### Bloom zu stark/zu schwach
- `r_bloom_intensity` anpassen
- `r_bloom_threshold` anpassen

### Schwarzschrim nach Map-Load
- `vid_restart` in Konsole
- `r_mapoverbrightbits` prüfen (1=dunkler, 2=heller)
