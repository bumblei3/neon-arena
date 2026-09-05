#!/bin/bash
# Build the NeonArena OpenArena mod and install it to ~/.openarena/neonarena
set -e
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT/oa-gamecode"

make

OUT=build/release-linux-x86_64/neonarena
DEST="$HOME/.openarena/neonarena"
mkdir -p "$DEST/vm"
cp "$OUT/vm/cgame.qvm" "$OUT/vm/qagame.qvm" "$OUT/vm/ui.qvm" "$DEST/vm/"
cp "$OUT/cgamex86_64.so" "$OUT/qagamex86_64.so" "$OUT/uix86_64.so" "$DEST/" 2>/dev/null || true

# QVMs must ship inside a pk3: with sv_pure the engine only accepts VMs from
# paks, otherwise it falls back to the vanilla stubs in baseoa (crash source).
cd "$OUT"
rm -f "$DEST/neonarena-qvm.pk3"
zip -q "$DEST/neonarena-qvm.pk3" vm/cgame.qvm vm/qagame.qvm vm/ui.qvm
cd - >/dev/null

# look pack (shaders + textures + cfg) from assets/
cd "$ROOT/assets"
if [ ! -f textures/neonarena/flare.tga ]; then
	python3 gen_textures.py
fi
rm -f "$DEST/neon-look.pk3"
zip -rq "$DEST/neon-look.pk3" scripts textures gfx env sound autoexec.cfg neon-look.cfg neon-gfx.cfg gfx-auto.cfg

echo "Installed NeonArena mod to $DEST:"
ls -la "$DEST" "$DEST/vm"
echo
echo "Play:  openarena +set fs_game neonarena"
