#!/usr/bin/env python3
"""NeonArena run-stats dashboard.

Reads the run summary that the mod exports at game over
(~/.openarena/neonarena/neonwave_runstats.json) and keeps a local history
of every run you play, then prints a compact report:

  - last run (wave / kills / best combo / time / result)
  - all-time best wave / kills / combo / fastest victory
  - today's runs + today's best wave
  - modifier frequency (which modifiers you've survived/seen most)

The history file is neonwave_runs.jsonl in the same directory as the
run-stats json. Each invocation records the *current* json once (deduped
by a hash of its content, so re-running the tool without a new run does
not inflate the history).

Usage:
  python3 neon-stats.py            # full report
  python3 neon-stats.py --last     # only the most recent run
  python3 neon-stats.py --best     # only all-time bests
  python3 neon-stats.py --json     # dump the accumulated history as JSON
  python3 neon-stats.py --src PATH # override the run-stats json location
"""
import argparse
import hashlib
import json
import os
import sys
from datetime import datetime, date

DEFAULT_SRC = os.path.expanduser("~/.openarena/neonarena/neonwave_runstats.json")
HISTORY_NAME = "neonwave_runs.jsonl"  # lives next to the run-stats json

REQUIRED_KEYS = (
    "version", "result", "wave", "kills", "bestCombo",
    "timeSec", "difficulty", "modifiersSeen", "modifierNames",
)

# Must stay in sync with NW_ACH_* order in g_neonwave.c.
ACHIEVEMENT_NAMES = (
    "FIRST VICTORY",  # cleared wave 20
    "SURVIVOR",       # reached wave 15
    "SHARPSHOOTER",   # best combo >= 8
    "STREAKER",       # best combo >= 5
    "FLAWLESS",       # victory with 0 deaths
    "COMBOMASTER",    # best combo >= 12
    "SPEEDRUNNER",    # victory under 300s
    "HARDCORE",       # victory in hardcore mode
)
# qboolean is an int (4 bytes, little-endian) in this codebase; the mod writes
# N qbooleans as a raw struct, so we read N 4-byte ints.
ACH_FILE = "neonwave_achievements.dat"


def read_achievements(src):
    """Return list of unlocked achievement display names from the dat file.

    The mod persists lifetime unlock state in neonwave_achievements.dat as a
    raw array of NW_ACH_COUNT qbooleans (each 4 bytes, little-endian)."""
    path = os.path.join(os.path.dirname(os.path.abspath(src)), ACH_FILE)
    unlocked = []
    if not os.path.exists(path):
        return unlocked
    try:
        with open(path, "rb") as fh:
            data = fh.read()
    except OSError:
        return unlocked
    n = len(ACHIEVEMENT_NAMES)
    slots = min(n, len(data) // 4)
    for i in range(slots):
        val = int.from_bytes(data[i * 4:i * 4 + 4], "little")
        if val != 0:
            unlocked.append(ACHIEVEMENT_NAMES[i])
    return unlocked

def load_current(src):
    if not os.path.exists(src):
        return None
    try:
        with open(src, "r", encoding="utf-8") as fh:
            data = json.load(fh)
    except (json.JSONDecodeError, OSError) as exc:
        sys.stderr.write("neon-stats: cannot read %s: %s\n" % (src, exc))
        return None
    for key in REQUIRED_KEYS:
        if key not in data:
            sys.stderr.write("neon-stats: %s missing key %r\n" % (src, key))
            return None
    return data


def history_path(src):
    return os.path.join(os.path.dirname(os.path.abspath(src)), HISTORY_NAME)


def content_hash(run):
    blob = json.dumps(run, sort_keys=True, separators=(",", ":"))
    return hashlib.sha1(blob.encode("utf-8")).hexdigest()[:16]


def record_run(src, run):
    """Append run to the history jsonl (deduped by content hash)."""
    path = history_path(src)
    seen = set()
    if os.path.exists(path):
        with open(path, "r", encoding="utf-8") as fh:
            for line in fh:
                line = line.strip()
                if not line:
                    continue
                try:
                    seen.add(json.loads(line)["_hash"])
                except (json.JSONDecodeError, KeyError):
                    continue
    h = content_hash(run)
    if h in seen:
        return path  # already recorded
    entry = dict(run)
    entry["_hash"] = h
    entry["_ts"] = datetime.now().isoformat(timespec="seconds")
    with open(path, "a", encoding="utf-8") as fh:
        fh.write(json.dumps(entry) + "\n")
    return path


def load_history(src):
    path = history_path(src)
    runs = []
    if not os.path.exists(path):
        return runs
    with open(path, "r", encoding="utf-8") as fh:
        for line in fh:
            line = line.strip()
            if not line:
                continue
            try:
                runs.append(json.loads(line))
            except json.JSONDecodeError:
                continue
    return runs


def best_of(runs, key, predicate=lambda r: True):
    vals = [r[key] for r in runs if predicate(r)]
    return max(vals) if vals else 0


def print_report(runs, current, src):
    if not runs:
        print("No runs recorded yet. Play a NeonArena run, then re-run this tool.")
        return

    print("NeonArena — run stats")
    print("=" * 40)
    total = len(runs)
    victories = sum(1 for r in runs if r["result"] == "VICTORY")
    print("Runs played : %d  (victories: %d)" % (total, victories))

    best_wave = best_of(runs, "wave")
    best_kills = best_of(runs, "kills")
    best_combo = best_of(runs, "bestCombo")
    fastest = best_of(runs, "timeSec", lambda r: r["result"] == "VICTORY") or None
    print("Best wave   : %d" % best_wave)
    print("Best kills  : %d" % best_kills)
    print("Best combo  : %d" % best_combo)
    if fastest:
        print("Fastest win : %ds" % fastest)

    # today
    today = date.today().isoformat()
    today_runs = [r for r in runs if r.get("_ts", "").startswith(today)]
    if today_runs:
        print("-" * 40)
        print("Today (%s): %d run(s), best wave %d"
              % (today, len(today_runs),
                 best_of(today_runs, "wave")))

    # modifier frequency
    freq = {}
    for r in runs:
        for m in r.get("modifierNames", []):
            freq[m] = freq.get(m, 0) + 1
    if freq:
        print("-" * 40)
        print("Modifiers seen:")
        for m, n in sorted(freq.items(), key=lambda kv: -kv[1]):
            print("  %-16s %d" % (m, n))

    # lifetime achievements (persistent dat file from the mod)
    unlocked = read_achievements(src)
    total_ach = len(ACHIEVEMENT_NAMES)
    print("-" * 40)
    print("Achievements: %d/%d unlocked" % (len(unlocked), total_ach))
    if unlocked:
        for name in ACHIEVEMENT_NAMES:
            mark = "x" if name in unlocked else " "
            print("  [%s] %s" % (mark, name))

    if current:
        print("-" * 40)
        print("Last run    : wave %d, %d kills, combo %d, %ds [%s]"
              % (current["wave"], current["kills"],
                 current["bestCombo"], current["timeSec"],
                 current["result"]))


def main():
    ap = argparse.ArgumentParser(description="NeonArena run-stats dashboard")
    ap.add_argument("--src", default=DEFAULT_SRC, help="run-stats json path")
    ap.add_argument("--last", action="store_true", help="only newest run")
    ap.add_argument("--best", action="store_true", help="only all-time bests")
    ap.add_argument("--json", action="store_true", help="dump history as JSON")
    args = ap.parse_args()

    current = load_current(args.src)
    if current is None:
        sys.stderr.write(
            "neon-stats: no run-stats json at %s yet.\n"
            "Play a NeonArena run (it is written at game over).\n" % args.src)
        return 1

    history = load_history(args.src)
    # record only if this exact json was not already stored
    record_run(args.src, current)
    history = load_history(args.src)

    if args.json:
        print(json.dumps(history, indent=2))
        return 0
    if args.last:
        print("Last run: %s" % json.dumps(current, indent=2))
        return 0
    if args.best:
        if history:
            print("Best wave %d | Best kills %d | Best combo %d"
                  % (best_of(history, "wave"), best_of(history, "kills"),
                     best_of(history, "bestCombo")))
        return 0

    print_report(history, current, args.src)
    return 0


if __name__ == "__main__":
    sys.exit(main())
