# CI-Workflow: engine-quake3e.yml

## Was dieser Workflow tut

`.github/workflows/engine-quake3e.yml` baut die **Quake3e-Engine** (OpenGL2 + Vulkan,
Bloom-Unterstützung) für Linux, Windows und macOS als optionale Binary-Artifacts.
Es wird **nicht** der NeonArena-Mod gebaut und es werden **keine** Tests ausgeführt.

Der Workflow ist separat vom Mod-Build-Workflow (`build-mod.yml`) und vom Testlauf
(`tests/run_suite.sh`). Er liefert ausschließlich die Engine-Binaries, die ein
Spieler zusätzlich zur OpenArena-Installation und zum NeonArena-Mod benötigt, um
NeonArena mit Bloom zu spielen.

## Auslösung

- **Push auf `main`**: baut die Engine für alle drei Plattformen. Die Archive liegen
  30 Tage als CI-Artefakte (`neonarena-engine-linux`, `neonarena-engine-windows`,
  `neonarena-engine-macos`).
- **Manueller Dispatch** (`workflow_dispatch`): gleicher Build.

Ein Tag-Push startet diesen Workflow nicht. `on.push.branches: [main]` gilt nur
für den Branch. Der Schritt „Release engine (tags only)“ prüft `github.ref` auf
`refs/tags/`, läuft aber nie. v0.91 (2026-10-13) enthält deshalb keine
Engine-Archive, nur `neonarena.pk3` und `neonarena-qvm.pk3`.

## Plattformen und Builds

| Plattform | Runner | Build-Befehl | Ausgabeverzeichnis |
|-----------|--------|--------------|-------------------|
| Linux | `ubuntu-latest` | `make -C /tmp/quake3e ARCH=x86_64 -j4` | `build/release-linux-x86_64/` |
| Windows | `ubuntu-22.04` (MinGW-Cross) | `make ARCH=x86_64 PLATFORM=mingw64 -j4` | `build/release-mingw64-x86_64/` |
| macOS | `macos-latest` (universal) | `make -C /tmp/quake3e ARCH=universal -j4` | `build/release-darwin-universal/` (Fallback: `release-darwin-arm64/`) |

## Build-Schritte im Detail

1. **Checkout** des NeonArena-Repos (`actions/checkout@v7`).
2. **Build-Abhängigkeiten installieren** (plattform-spezifisch):
   - Linux: `build-essential`, `libsdl2-dev`, `libwayland-dev`, `zip`, `unzip`,
     `libcurl4-openssl-dev`, `libvulkan-dev`, `glslang-tools` (apt, drei Versuche).
   - Windows: `gcc-mingw-w64-x86-64` und `zip` auf `ubuntu-22.04`. Kein Chocolatey,
     kein separater SDL2-Tarball.
   - macOS: `sdl2`, `vulkan-headers`, `molten-vk` via Homebrew; `ARCH=universal`.
3. **Quake3e-Clone** (shallow) von `https://github.com/ec-/quake3e` nach `/tmp/quake3e`.
4. **Patches**: `patches/quake3e-controller.patch` per `git apply`, danach
   `BASEGAME` in `code/qcommon/q_shared.h` per Perl auf `"baseoa"`. Unter Windows
   zusätzlich zwei MinGW-Workarounds in `sdl_gamma.c` und `sdl_glimp.c`.
5. **Engine bauen** (plattform-spezifischer `make`-Aufruf).
6. **Package erstellen** (plattform-spezifisch):
   - Linux: `dist/neonarena-engine/` wird mit `quake3e.x64` + OpenGL- + Vulkan-.so
     befüllt, dann `tar -czf dist/neonarena-engine-linux64.tar.gz`.
   - Windows: `dist/neonarena-engine/` mit `quake3e.exe` (aus `quake3e.x64.exe`),
     OpenGL- und, falls gebaut, Vulkan-DLLs. `SDL*.dll` nur, wenn der Build sie
     mitliefert. Dann `zip -r neonarena-engine-win64.zip`.
   - macOS: Binary aus `release-darwin-universal/`, sonst `release-darwin-arm64/`,
     plus vorhandene OpenGL-/Vulkan-dylibs. Archiv
     `neonarena-engine-macos-universal.tar.gz`.
7. **Artifacts hochladen** via `actions/upload-artifact@v7`, Aufbewahrung 30 Tage:
   - `neonarena-engine-linux` → `dist/neonarena-engine-linux64.tar.gz`
   - `neonarena-engine-windows` → `dist/neonarena-engine-win64.zip`
   - `neonarena-engine-macos` → `dist/neonarena-engine-macos-universal.tar.gz`
8. **Release-Schritt** `softprops/action-gh-release@v3` hängt die Archive nur an,
   wenn `github.ref` ein Tag ist. Der Workflow hat keinen Tag-Trigger, der Schritt
   läuft nicht. Die Archive bleiben CI-Artefakte.

## Was dieser Workflow NICHT tut

- **Kein Mod-Build**: Die QVMs (`vm/*.qvm`) und das `neon-look.pk3` werden nicht
  gebaut. Der Mod wird vom `build-mod.yml`-Workflow gebaut.
- **Keine Testsuite**: `tests/run_suite.sh` wird nicht ausgeführt. Die Test-Suite
  wird vom `build-mod.yml`-Workflow auf `main`, Tags und Pull Requests
  als volle Matrix (Chunks 0–7) ausgeführt.
- **Keine Release der Mod-Artifacts**: `dist/neonarena.pk3` und
  `dist/neonarena-qvm.pk3` werden nicht erstellt. Diese gehören zum Mod-Release
  (via `build-mod.yml` + Tag).

## Beziehung zu `build-mod.yml`

Die zwei Workflows sind unabhängig:

- `build-mod.yml`: baut den NeonArena-Mod (QVMs + pk3) und führt die Test-Suite aus.
  Tag-pushes erzeugen das Mod-Release.
- `engine-quake3e.yml`: baut die Quake3e-Engine bei Push auf `main`. Die Archive
  sind CI-Artefakte und hängen nicht am GitHub-Release.

Ein Tag-Release enthält nur die Mod-pk3s aus `build-mod.yml`. Portable-Zip und
Setup.exe kommen aus `build-installer` und `build-setup-exe` und bleiben ebenfalls
CI-Artefakte (`workflow_run` sieht den Default-Branch, keinen Tag). Der Mod bleibt
ohne diese Binaries auf der klassischen OpenArena-Engine spielbar.

## Offene Punkte / Anmerkungen

- Der Workflow installiert Vulkan-Headers und `glslang-tools` auf Linux, aber es ist
  nicht garantiert, dass der Vulkan-Renderer erfolgreich baut — falls nicht, wird
  nur der OpenGL-Renderer mitgeliefert (der Package-Schritt prüft das pro Renderer).
  Die Artefakte bleiben nutzbar, auch wenn Vulkan ausfällt.
- Windows läuft als MinGW-Cross auf `ubuntu-22.04` mit `-fgnu89-inline`. `SDL*.dll`
  wird nur kopiert, wenn der Engine-Build sie erzeugt.
- Der macOS-Universal-Build erwartet `build/release-darwin-universal/`. Fehlt der
  Pfad, fällt der Schritt auf `release-darwin-arm64/` zurück.
- Der Workflow setzt `permissions: contents: write`. Der Release-Schritt nutzt das
  nicht, solange der Workflow nur auf `main` läuft.
- `patches/quake3e-controller.patch` und die `BASEGAME`-Ersetzung gelten nur für
  diesen Engine-Build, nicht für `build-mod.sh`. Siehe auch README, Abschnitt
  „Grafik-Engine".
