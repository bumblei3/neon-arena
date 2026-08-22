#!/bin/bash
# Build the NeonArena OpenArena mod and install it to ~/.openarena/neonarena
set -e
cd "$(dirname "$0")/oa-gamecode"

make

OUT=build/release-linux-x86_64/neonarena
DEST="$HOME/.openarena/neonarena"
mkdir -p "$DEST/vm"
cp "$OUT/vm/cgame.qvm" "$OUT/vm/qagame.qvm" "$OUT/vm/ui.qvm" "$DEST/vm/"
cp "$OUT/cgamex86_64.so" "$OUT/qagamex86_64.so" "$OUT/uix86_64.so" "$DEST/" 2>/dev/null || true

echo "Installed NeonArena mod to $DEST:"
ls -la "$DEST" "$DEST/vm"
echo
echo "Play:  openarena +set fs_game neonarena"
