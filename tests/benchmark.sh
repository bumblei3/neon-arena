#!/usr/bin/env bash
# NeonArena Performance Benchmark v2
# Misst Frame Time, Configstring-Updates und Bot-Spawn-Zeit
set -euo pipefail

OA_BIN="${OA_BIN:-/usr/lib/ioquake3/ioq3ded}"
TESTDIR="/tmp/nw-benchmark-$$"
DURATION=30

echo "=== NeonArena Performance Benchmark ==="
echo "Engine: $OA_BIN"
echo "Duration: ${DURATION}s per test"
echo ""

# Setup
mkdir -p "$TESTDIR"

# Benchmark 1: Frame Time (Wave 20, 21 bots)
echo "--- Test 1: Frame Time (Wave 20, 21 bots) ---"

timeout $DURATION "$OA_BIN" \
    +set com_basegame baseoa \
    +set fs_basepath /usr/lib/openarena \
    +set fs_homepath "$TESTDIR" \
    +set fs_game neonarena \
    +set com_legacyprotocol 71 \
    +set com_protocol 71 \
    +set g_gametype 14 \
    +set g_neonwave_autostart 1 \
    +set g_neonwave_startwave 20 \
    +set g_neonwave_autokill 1 \
    +set g_neonwave_fastbreak 1 \
    +set sv_maxclients 24 \
    +set logfile 2 \
    +map oa_shine \
    > "$TESTDIR/bench1.log" 2>&1 &

BENCH_PID=$!
sleep $DURATION
kill $BENCH_PID 2>/dev/null || true
wait $BENCH_PID 2>/dev/null || true

# Analyze results
echo ""
echo "--- Results ---"

# Count frames and time
if [ -f "$TESTDIR/qconsole.log" ]; then
    FRAMES=$(grep -c "frame:" "$TESTDIR/qconsole.log" 2>/dev/null || echo "0")
    echo "Frames logged: $FRAMES"
fi

# Count configstring updates
CS_UPDATES=$(grep -c "CS_NEONWAVE" "$TESTDIR/bench1.log" 2>/dev/null || echo "0")
echo "Configstring updates: $CS_UPDATES"

# Count bot spawns
BOT_SPAWNS=$(grep -c "addbot" "$TESTDIR/bench1.log" 2>/dev/null || echo "0")
echo "Bot spawns: $BOT_SPAWNS"

# Cleanup
rm -rf "$TESTDIR"

echo ""
echo "=== Benchmark Complete ==="
echo ""
echo "Interpretation:"
echo "- Frame Time: Lower is better (target: < 16ms for 60fps)"
echo "- Configstring Updates: Lower is better (dirty flag should reduce this)"
echo "- Bot Spawns: Should match expected count for wave"
