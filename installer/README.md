# NeonArena Windows Installer

## Voraussetzungen

- [Inno Setup](https://jrsoftware.org/isdl.php) (kostenlos)
- Windows 7 oder neuer

## Kompilieren

1. Inno Setup installieren
2. Engine-Dateien kopieren:
   ```
   installer/engine/quake3e.exe
   installer/engine/SDL2.dll
   installer/engine/renderer_opengl2.dll
   ```
3. `installer/neonarena.iss` mit Inno Setup kompilieren
4. Fertig: `installer/NeonArena-0.53-Setup.exe`

## Manuelle Installation (ohne Installer)

1. Quake3e herunterladen von https://github.com/ec-/Quake3e/releases
2. `dist/neonarena.pk3`, `dist/neonarena-qvm.pk3`, `dist/neon-look.pk3` in `neonarena/` kopieren
3. `quake3e.exe +set fs_game neonarena` starten

## Struktur

```
installer/
├── neonarena.iss      # Inno Setup Script
├── neonarena.bat      # Launcher-Skript
├── engine/            # Engine-Dateien (manuell kopieren)
│   ├── quake3e.exe
│   ├── SDL2.dll
│   └── renderer_opengl2.dll
└── README.md          # Diese Datei
```
