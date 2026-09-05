#!/usr/bin/env bash
# NeonArena Installer — lädt Engine + Mod herunter und installiert beides
set -euo pipefail

INSTALL_DIR="${NEON_INSTALL_DIR:-$HOME/.openarena}"
ENGINE_DIR="${QUAKE3E_DIR:-$HOME/quake3e-engine}"
RELEASE_URL="https://github.com/bumblei3/neon-arena/releases"

usage() {
  cat <<EOF
NeonArena Installer

Usage:
  $0 [--update] [--engine-only] [--mod-only] [--help]

Optionen:
  --update        Aktualisiere auf die neueste Version
  --engine-only   Nur Engine herunterladen/installieren
  --mod-only      Nur Mod herunterladen/installieren
  --help          Diese Hilfe

Umgebungsvariablen:
  NEON_INSTALL_DIR  Installationsverzeichnis (Default: ~/.openarena)
  QUAKE3E_DIR       Engine-Verzeichnis (Default: ~/quake3e-engine)
EOF
  exit 0
}

log() { echo "[neon-install] $*"; }
err() { echo "[neon-install] ERROR: $*" >&2; exit 1; }

# Detect platform
detect_platform() {
  case "$(uname -s)" in
    Linux*)  PLATFORM="linux" ;;
    Darwin*) PLATFORM="macos" ;;
    MINGW*|MSYS*|CYGWIN*) PLATFORM="windows" ;;
    *) err "Unbekannte Plattform: $(uname -s)" ;;
  esac
  log "Plattform: $PLATFORM"
}

# Get latest release version
get_latest_version() {
  curl -sL "https://api.github.com/repos/bumblei3/neon-arena/releases/latest" \
    | grep -oP '"tag_name":\s*"\K[^"]+' || err "Konnte neueste Version nicht ermitteln"
}

# Download file with progress
download() {
  local url="$1" dest="$2"
  log "Lade herunter: $url"
  if command -v curl &>/dev/null; then
    curl -fL --progress-bar "$url" -o "$dest"
  elif command -v wget &>/dev/null; then
    wget --show-progress -q "$url" -O "$dest"
  else
    err "Weder curl noch wget gefunden"
  fi
}

# Install engine
install_engine() {
  local version="$1"
  local archive ext

  case "$PLATFORM" in
    linux)
      archive="neonarena-engine-linux64.tar.gz"
      ext="tar.gz"
      ;;
    windows)
      archive="neonarena-engine-win64.zip"
      ext="zip"
      ;;
    macos)
      archive="neonarena-engine-macos-universal.tar.gz"
      ext="tar.gz"
      ;;
  esac

  local url="$RELEASE_URL/download/$version/$archive"
  local tmp
  tmp=$(mktemp)

  log "Installiere Engine..."
  download "$url" "$tmp"

  mkdir -p "$ENGINE_DIR"
  case "$ext" in
    tar.gz) tar -xzf "$tmp" -C "$ENGINE_DIR" --strip-components=1 ;;
    zip) unzip -q "$tmp" -d "$ENGINE_DIR" ;;
  esac
  rm -f "$tmp"
  log "Engine installiert in: $ENGINE_DIR"
}

# Install mod
install_mod() {
  local version="$1"
  local url="$RELEASE_URL/download/$version/neonarena.pk3"
  local tmp
  tmp=$(mktemp)

  log "Installiere Mod..."
  download "$url" "$tmp"

  mkdir -p "$INSTALL_DIR/neonarena"
  cp "$tmp" "$INSTALL_DIR/neonarena/neonarena.pk3"
  rm -f "$tmp"
  log "Mod installiert in: $INSTALL_DIR/neonarena/"
}

# Create launcher script
create_launcher() {
  log "Erstelle Starter..."
  local launcher="$HOME/.local/bin/neonarena"
  local here gfx_auto detect

  here="$(cd "$(dirname "$0")" && pwd)"
  detect="$here/detect-gfx.sh"
  gfx_auto="$INSTALL_DIR/neonarena/gfx-auto.cfg"
  mkdir -p "$INSTALL_DIR/neonarena"
  if [ -x "$detect" ]; then
    "$detect" --ensure "$gfx_auto" >/dev/null || true
  fi

  mkdir -p "$(dirname "$launcher")"

  cat > "$launcher" <<LAUNCHER
#!/usr/bin/env bash
# NeonArena Launcher (automatisch generiert)
GFX_AUTO="$INSTALL_DIR/neonarena/gfx-auto.cfg"
RENDERER=vulkan
BLOOM=1
if [ -f "\$GFX_AUTO" ]; then
  r=\$(awk '\$1=="seta" && \$2=="cl_renderer" { gsub(/"/, "", \$3); print \$3; exit }' "\$GFX_AUTO")
  b=\$(awk '\$1=="seta" && \$2=="r_bloom" { gsub(/"/, "", \$3); print \$3; exit }' "\$GFX_AUTO")
  [ -n "\$r" ] && RENDERER="\$r"
  [ -n "\$b" ] && BLOOM="\$b"
fi
exec "$ENGINE_DIR/quake3e.x64" \\
  +set cl_renderer "\$RENDERER" \\
  +set r_bloom "\$BLOOM" \\
  +set fs_basepath "$ENGINE_DIR" \\
  +set fs_homepath "$INSTALL_DIR" \\
  +set fs_game neonarena \\
  +g_gametype 14 \\
  +map oa_shine \\
  "\$@"
LAUNCHER
  chmod +x "$launcher"
  log "Launcher erstellt: $launcher"
  log "Starte mit: neonarena"
}

# Update check
check_update() {
  local current=""
  if [ -f "$INSTALL_DIR/neonarena/version.txt" ]; then
    current=$(cat "$INSTALL_DIR/neonarena/version.txt")
  fi

  local latest
  latest=$(get_latest_version)

  if [ "$current" = "$latest" ]; then
    log "NeonArena ist aktuell ($current)"
    return 1
  fi

  log "Update verfügbar: $current -> $latest"
  return 0
}

# Main
UPDATE=0
ENGINE_ONLY=0
MOD_ONLY=0

while [ $# -gt 0 ]; do
  case "$1" in
    --help|-h) usage ;;
    --update) UPDATE=1; shift ;;
    --engine-only) ENGINE_ONLY=1; shift ;;
    --mod-only) MOD_ONLY=1; shift ;;
    *) usage ;;
  esac
done

detect_platform

if [ "$UPDATE" -eq 1 ]; then
  if ! check_update; then
    exit 0
  fi
fi

VERSION=$(get_latest_version)
log "Installiere NeonArena $VERSION"

if [ "$MOD_ONLY" -eq 0 ]; then
  install_engine "$VERSION"
fi

if [ "$ENGINE_ONLY" -eq 0 ]; then
  install_mod "$VERSION"
  echo "$VERSION" > "$INSTALL_DIR/neonarena/version.txt"
fi

create_launcher
log "Installation abgeschlossen!"
log ""
log "Starte mit: neonarena"
log "Oder: scripts/start-quake3e.sh --daily"
