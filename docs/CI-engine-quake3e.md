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

- **Push auf `main`**: baut die Engine für alle drei Plattformen und veröffentlicht
  die Artifacts für die Dauer der Workflow-Run.
- **Manueller Dispatch** (`workflow_dispatch`): gleicher Build, manuell ausgelöst.

Bei Tags wird keine spezielle Tag-Build-Regel ausgelöst — der Workflow prüft in
Schritt 10 (`Release engine (tags only)`) nur, ob `github.ref` mit `refs/tags/`
beginnt, und erstellt dann eine GitHub-Release mit den Engine-Artifacts. Der
Workflow wird also auch bei einem Tag-push ausgelöst (weil `push: branches: [main]`
keine Tags ausschließt) und die Tag-Bedingung im letzten Schritt sorgt für die
Release-Erstellung.

## Plattformen und Builds

| Plattform | Runner | Build-Befehl | Ausgabeverzeichnis |
|-----------|--------|--------------|-------------------|
| Linux | `ubuntu-latest` | `make -C /tmp/quake3e ARCH=x86_64 -j4` | `build/release-linux-x86_64/` |
| Windows | `windows-latest` (MinGW) | `make ARCH=x86_64 PLATFORM=mingw32 CC=x86_64-w64-mingw32-gcc ... -j4` | `build/release-mingw32-x86_64/` |
| macOS | `macos-latest` (universal) | `make -C /tmp/quake3e ARCH=universal -j4` | `build/release-darwin-universal/` (Fallback: `release-darwin-x86_64/`) |

## Build-Schritte im Detail

1. **Checkout** des NeonArena-Repos (`actions/checkout@v4`).
2. **Build-Abhängigkeiten installieren** (plattform-spezifisch):
   - Linux: `build-essential`, `libsdl2-dev`, `libwayland-dev`, `zip`, `unzip`,
     `libcurl4-openssl-dev`, `libvulkan-dev`, `glslang-tools`.
   - Windows: MinGW 13.2.0 via Chocolatey, plus SDL2-2.30.5 Dev-Libs (heruntergeladen,
     entpackt, `SDL2_DIR` gesetzt).
   - macOS: `sdl2`, `vulkan-headers`, `molten-vk` via Homebrew; `ARCH=universal`
     für universal-binary Build.
3. **Quake3e-Clone** (shallow) von `https://github.com/ec-/quake3e` nach `/tmp/quake3e`.
4. **OpenArena-Kompatibilitäts-Patch anwenden**: `patches/engine-quake3e-oa.patch`
   wird mit `patch -p1` auf das Clone angewendet. Der Patch ändert das hardcoded
   `BASEGAME` von `"baseq3"` zu `"baseoa"`, damit die Engine OA-Inhalte direkt aus
   `baseoa/` lädt, ohne dass der Spieler die OA-pk3s umbenennen muss.
5. **Engine bauen** (plattform-spezifischer `make`-Aufruf).
6. **Package erstellen** (plattform-spezifisch):
   - Linux: `dist/neonarena-engine/` wird mit `quake3e.x64` + OpenGL- + Vulkan-.so
     befüllt, dann `tar -czf dist/neonarena-engine-linux64.tar.gz`.
   - Windows: `dist/neonarena-engine/` mit `quake3e.exe` + OpenGL- + Vulkan-.dll,
     plus `SDL2.dll` aus dem heruntergeladenen SDL2-Dev-Tarball. Dann
     `zip -r neonarena-engine-win64.zip`.
   - macOS: `dist/neonarena-engine/` mit `quake3e.x64` + OpenGL- + Vulkan-.dylib,
     dann `tar -czf neonarena-engine-macos-universal.tar.gz`. Fallback auf
     `release-darwin-x86_64/`, falls `release-darwin-universal/` nicht existiert.
7. **Artifacts hochladen** via `actions/upload-artifact@v4`:
   - `neonarena-engine-linux` → `dist/neonarena-engine-linux64.tar.gz`
   - `neonarena-engine-windows` → `dist/neonarena-engine-win64.zip`
   - `neonarena-engine-macos` → `dist/neonarena-engine-macos-universal.tar.gz`
8. **Release erstellen** (nur bei Tags): `softprops/action-gh-release@v2` veröffentlicht
   alle drei Archive als GitHub-Release-Assete.

## Was dieser Workflow NICHT tut

- **Kein Mod-Build**: Die QVMs (`vm/*.qvm`) und das `neon-look.pk3` werden nicht
  gebaut. Der Mod wird vom `build-mod.yml`-Workflow gebaut.
- **Keine Testsuite**: `tests/run_suite.sh` wird nicht ausgeführt. Die Test-Suite
  wird vom `build-mod.yml`-Workflow auf `main`-Pushes (Quick) und Tags (Full)
  ausgeführt.
- **Keine Release der Mod-Artifacts**: `dist/neonarena.pk3` und
  `dist/neonarena-qvm.pk3` werden nicht erstellt. Diese gehören zum Mod-Release
  (via `build-mod.yml` + Tag).

## Beziehung zu `build-mod.yml`

Die zwei Workflows sind unabhängig:

- `build-mod.yml`: baut den NeonArena-Mod (QVMs + pk3) und führt die Test-Suite aus.
  Tag-pushes erzeugen das Mod-Release.
- `engine-quake3e.yml`: baut die Quake3e-Engine und veröffentlicht sie als optionales
  Asset. Tag-pushes erstellen zusätzlich das Engine-Release.

Ein vollständiges NeonArena-Release (Mod + Engine) entsteht also durch einen einzelnen
Tag-push, der beide Workflows parallel ausführt. Die Getrenntheit ist gewollt: der
Mod kann auch ohne Quake3e (auf der klassischen OpenArena-Engine) spielbar bleiben.

## Offene Punkte / Anmerkungen

- Der Workflow installiert Vulkan-Headers und `glslang-tools` auf Linux, aber es ist
  nicht garantiert, dass der Vulkan-Renderer erfolgreich baut — falls nicht, wird
  nur der OpenGL-Renderer mitgeliefert (Schritt 6 해석서의 `if [ -f ... ]` prüft das
  pro Renderer). Die Artifacts sind also immer nutzbar, auch wenn Vulkan ausfällt.
- Windows-Build bündelt `SDL2.dll` aus dem manuell heruntergeladenen SDL2-2.30.5-Tarball.
  Wenn der SDL2-Downloaddownload fehlschlägt oder sich die Versionsnummer ändert,
  bricht der Windows-Build. Der Pfad ist hartkodiert im Download-Link.
- Der macOS-Universal-Build erwartet `build/release-darwin-universal/` — falls Quake3e
  diesen Pfad nicht erzeugt, fällt der Schritt auf `release-darwin-x86_64/` zurück.
  Das ist ein internes Quake3e-Verhalten, das bei Architekturänderungen brechen kann.
- Der Workflow verwendet `permissions: contents: write` für den Release-Schritt.
  Ohne diese Berechtigung kann `softprops/action-gh-release@v2` keinen Release erstellen.
- Der Patch `patches/engine-quake3e-oa.patch` ist **kein Mod-Patch** — er wird nur
  beim Engine-Build angewendet, nicht beim Mod-Build `build-mod.sh`. Siehe auch
  README, Abschnitt „Grafik-Engine".
