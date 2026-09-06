#!/usr/bin/env python3
# tools/wave_editor.py — NeonArena Wave-Editor CLI
# Erstellt und bearbeitet Wellen-Configs für Custom Runs.
#
# Usage:
#   ./tools/wave_editor.py new <name>              # Neue Config erstellen
#   ./tools/wave_editor.py edit <file>             # Config bearbeiten
#   ./tools/wave_editor.py show <file>             # Config anzeigen
#   ./tools/wave_editor.py list                    # Alle Configs auflisten
#   ./tools/wave_editor.py validate <file>         # Config validieren

import sys
import os
import json
import argparse

# Default-Template für eine neue Config
DEFAULT_CONFIG = {
    "name": "Custom Run",
    "version": 1,
    "settings": {
        "g_neonwave_maxwave": 20,
        "g_neonwave_modifier": 0,
        "g_neonwave_modifier2": 0,
        "g_neonwave_bosstype": 0,
        "g_neonwave_startwave": 1,
        "g_neonwave_daily": 0,
        "g_neonwave_dailyseed": 0,
        "g_neonwave_hardcore": 0,
        "g_neonwave_ghost": 0,
        "g_momentum": 1,
        "g_momentum_decay": 5,
        "g_momentum_kill": 15
    },
    "description": "Custom NeonArena Run"
}

# Validierte CVar-Definitionen
VALID_CVARS = {
    "g_neonwave_maxwave": {"type": "int", "min": 1, "max": 100},
    "g_neonwave_modifier": {"type": "int", "min": 0, "max": 14},
    "g_neonwave_modifier2": {"type": "int", "min": 0, "max": 14},
    "g_neonwave_bosstype": {"type": "int", "min": 0, "max": 8},
    "g_neonwave_startwave": {"type": "int", "min": 1, "max": 50},
    "g_neonwave_daily": {"type": "int", "min": 0, "max": 1},
    "g_neonwave_dailyseed": {"type": "int", "min": 0, "max": 99999},
    "g_neonwave_hardcore": {"type": "int", "min": 0, "max": 1},
    "g_neonwave_ghost": {"type": "int", "min": 0, "max": 1},
    "g_momentum": {"type": "int", "min": 0, "max": 1},
    "g_momentum_decay": {"type": "int", "min": 0, "max": 20},
    "g_momentum_kill": {"type": "int", "min": 0, "max": 50}
}

CONFIGS_DIR = os.path.join(os.path.dirname(os.path.dirname(__file__)), "configs")


def cmd_new(args):
    """Neue Config erstellen."""
    name = args.name
    filepath = os.path.join(CONFIGS_DIR, f"{name}.json")

    if os.path.exists(filepath):
        print(f"Error: Config '{name}' already exists at {filepath}")
        return 1

    config = DEFAULT_CONFIG.copy()
    config["name"] = name

    os.makedirs(CONFIGS_DIR, exist_ok=True)
    with open(filepath, "w") as f:
        json.dump(config, f, indent=2)

    print(f"Created config: {filepath}")
    return 0


def cmd_edit(args):
    """Config bearbeiten (interaktiv)."""
    filepath = os.path.join(CONFIGS_DIR, f"{args.file}.json")

    if not os.path.exists(filepath):
        print(f"Error: Config '{args.file}' not found at {filepath}")
        return 1

    with open(filepath, "r") as f:
        config = json.load(f)

    print(f"\nEditing: {config.get('name', args.file)}")
    print("=" * 40)

    # Zeige aktuelle Einstellungen
    for i, (cvar, value) in enumerate(config["settings"].items()):
        print(f"  [{i+1}] {cvar} = {value}")

    print(f"  [0] Save and exit")

    while True:
        try:
            choice = int(input("\nSelect setting to edit (0 to save): "))
            if choice == 0:
                break
            if 1 <= choice <= len(config["settings"]):
                cvar = list(config["settings"].keys())[choice - 1]
                current = config["settings"][cvar]
                new_val = input(f"  {cvar} ({current}): ")

                if new_val.strip():
                    validated = validate_cvar(cvar, new_val)
                    if validated is not None:
                        config["settings"][cvar] = validated
                        print(f"  Set {cvar} = {validated}")
                    else:
                        print(f"  Invalid value for {cvar}")
            else:
                print("  Invalid choice")
        except ValueError:
            print("  Please enter a number")
        except KeyboardInterrupt:
            print("\nAborted.")
            return 1

    with open(filepath, "w") as f:
        json.dump(config, f, indent=2)

    print(f"\nSaved: {filepath}")
    return 0


def cmd_show(args):
    """Config anzeigen."""
    filepath = os.path.join(CONFIGS_DIR, f"{args.file}.json")

    if not os.path.exists(filepath):
        print(f"Error: Config '{args.file}' not found at {filepath}")
        return 1

    with open(filepath, "r") as f:
        config = json.load(f)

    print(f"\n{config.get('name', args.file)}")
    print("=" * 40)
    print(f"Description: {config.get('description', 'N/A')}")
    print()

    for cvar, value in config["settings"].items():
        print(f"  {cvar} = {value}")

    return 0


def cmd_list(args):
    """Alle Configs auflisten."""
    if not os.path.exists(CONFIGS_DIR):
        print("No configs directory found.")
        return 0

    configs = [f for f in os.listdir(CONFIGS_DIR) if f.endswith(".json")]

    if not configs:
        print("No configs found.")
        return 0

    print("\nAvailable configs:")
    print("=" * 40)
    for cfg in sorted(configs):
        filepath = os.path.join(CONFIGS_DIR, cfg)
        with open(filepath, "r") as f:
            data = json.load(f)
        name = data.get("name", cfg)
        desc = data.get("description", "")
        print(f"  {cfg:<30} {name}")
        if desc:
            print(f"    {desc}")

    return 0


def cmd_validate(args):
    """Config validieren."""
    filepath = os.path.join(CONFIGS_DIR, f"{args.file}.json")

    if not os.path.exists(filepath):
        print(f"Error: Config '{args.file}' not found at {filepath}")
        return 1

    with open(filepath, "r") as f:
        config = json.load(f)

    errors = []
    warnings = []

    # Prüfe Struktur
    if "settings" not in config:
        errors.append("Missing 'settings' section")
    else:
        for cvar, value in config["settings"].items():
            if cvar not in VALID_CVARS:
                warnings.append(f"Unknown CVar: {cvar}")
                continue

            cvar_def = VALID_CVARS[cvar]
            if not isinstance(value, int):
                errors.append(f"{cvar}: must be int, got {type(value).__name__}")
            elif value < cvar_def["min"] or value > cvar_def["max"]:
                errors.append(f"{cvar}: {value} out of range [{cvar_def['min']}, {cvar_def['max']}]")

    if errors:
        print(f"\nValidation FAILED for {args.file}:")
        for err in errors:
            print(f"  ERROR: {err}")
        return 1
    else:
        print(f"\nValidation OK for {args.file}")
        for warn in warnings:
            print(f"  WARNING: {warn}")
        return 0


def validate_cvar(cvar, value):
    """Validiere einen CVar-Wert."""
    if cvar not in VALID_CVARS:
        return None

    cvar_def = VALID_CVARS[cvar]
    try:
        val = int(value)
    except ValueError:
        return None

    if val < cvar_def["min"] or val > cvar_def["max"]:
        return None

    return val


def main():
    parser = argparse.ArgumentParser(description="NeonArena Wave-Editor")
    subparsers = parser.add_subparsers(dest="command")

    # new
    new_parser = subparsers.add_parser("new", help="Create new config")
    new_parser.add_argument("name", help="Config name")

    # edit
    edit_parser = subparsers.add_parser("edit", help="Edit config")
    edit_parser.add_argument("file", help="Config file name (without .json)")

    # show
    show_parser = subparsers.add_parser("show", help="Show config")
    show_parser.add_argument("file", help="Config file name (without .json)")

    # list
    subparsers.add_parser("list", help="List all configs")

    # validate
    validate_parser = subparsers.add_parser("validate", help="Validate config")
    validate_parser.add_argument("file", help="Config file name (without .json)")

    args = parser.parse_args()

    if args.command == "new":
        return cmd_new(args)
    elif args.command == "edit":
        return cmd_edit(args)
    elif args.command == "show":
        return cmd_show(args)
    elif args.command == "list":
        return cmd_list(args)
    elif args.command == "validate":
        return cmd_validate(args)
    else:
        parser.print_help()
        return 1


if __name__ == "__main__":
    sys.exit(main())
