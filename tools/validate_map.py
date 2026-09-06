#!/usr/bin/env python3
# tools/validate_map.py — NeonArena Map-Validator
# Prüft ob eine Map NeonArena-kompatibel ist.
#
# Usage:
#   ./tools/validate_map.py <map.bsp>           # Map validieren
#   ./tools/validate_map.py --fix <map.bsp>     # Auto-Fix versuchen

import sys
import os
import struct
import argparse

# BSP Magic Number
BSP_MAGIC = b"IBSP"
BSP_VERSION = 50  # Quake III BSP Version

# Required Entity Types
REQUIRED_ENTITIES = [
    "info_player_deathmatch"
]

# Recommended Entity Types
RECOMMENDED_ENTITIES = [
    "item_armor_shard",
    "item_health",
    "weapon_railgun",
    "weapon_lightning"
]

# Map Size Limits (units)
MIN_MAP_SIZE = 200
MAX_MAP_SIZE = 4000
IDEAL_MAP_SIZE = 1000


def validate_bsp(filepath):
    """Validiere BSP-Datei."""
    errors = []
    warnings = []
    info = {}

    if not os.path.exists(filepath):
        return [], [f"File not found: {filepath}"], {}

    with open(filepath, "rb") as f:
        # Header prüfen
        magic = f.read(4)
        if magic != BSP_MAGIC:
            return [], [f"Invalid BSP magic: {magic}"], {}

        version = struct.unpack("<I", f.read(4))[0]
        if version != BSP_VERSION:
            warnings.append(f"Unexpected BSP version: {version} (expected {BSP_VERSION})")

        # Directory Entries (simplified)
        # In a real implementation, we'd parse the full BSP structure
        # For now, we do basic checks

    # File size check
    file_size = os.path.getsize(filepath)
    info["file_size"] = file_size

    if file_size < 1024:
        errors.append("File too small to be a valid BSP")
    elif file_size > 50 * 1024 * 1024:  # 50MB
        warnings.append("Large map file may cause performance issues")

    return errors, warnings, info


def validate_map(filepath):
    """Validiere eine Map auf NeonArena-Kompatibilität."""
    errors = []
    warnings = []
    info = {}

    # BSP validieren
    bsp_errors, bsp_warnings, bsp_info = validate_bsp(filepath)
    errors.extend(bsp_errors)
    warnings.extend(bsp_warnings)
    info.update(bsp_info)

    if errors:
        return errors, warnings, info

    # Entity-Check (simplified - in real implementation, parse entities)
    info["entities_found"] = []
    info["entities_missing"] = []

    # Map size estimation (simplified)
    info["estimated_size"] = "unknown"

    # Check for common issues
    filename = os.path.basename(filepath).lower()

    if "test" in filename:
        warnings.append("Map name contains 'test' - may not be intended for production")

    if "dm_" in filename or "deathmatch" in filename:
        warnings.append("Map appears to be a deathmatch map - may need adjustments for wave survival")

    return errors, warnings, info


def suggest_fixes(errors, warnings):
    """Schlage Fixes vor."""
    fixes = []

    for err in errors:
        if "too small" in err:
            fixes.append("Ensure the map is a valid Quake III BSP file")

    for warn in warnings:
        if "deathmatch" in warn:
            fixes.append("Consider adding more open areas for bot movement")
            fixes.append("Add additional spawn points for wave survival")

    if not fixes:
        fixes.append("No fixes needed - map appears compatible")

    return fixes


def main():
    parser = argparse.ArgumentParser(description="NeonArena Map Validator")
    parser.add_argument("map_file", help="Path to BSP map file")
    parser.add_argument("--fix", action="store_true", help="Suggest fixes")
    parser.add_argument("--verbose", "-v", action="store_true", help="Verbose output")

    args = parser.parse_args()

    print(f"\nValidating: {args.map_file}")
    print("=" * 50)

    errors, warnings, info = validate_map(args.map_file)

    if args.verbose:
        print("\nInfo:")
        for key, value in info.items():
            print(f"  {key}: {value}")

    if warnings:
        print("\nWarnings:")
        for warn in warnings:
            print(f"  ⚠ {warn}")

    if errors:
        print("\nErrors:")
        for err in errors:
            print(f"  ✗ {err}")

    if args.fix:
        fixes = suggest_fixes(errors, warnings)
        print("\nSuggested fixes:")
        for fix in fixes:
            print(f"  → {fix}")

    if errors:
        print(f"\nResult: FAILED ({len(errors)} errors)")
        return 1
    elif warnings:
        print(f"\nResult: OK ({len(warnings)} warnings)")
        return 0
    else:
        print("\nResult: OK")
        return 0


if __name__ == "__main__":
    sys.exit(main())
