# Controller-Support Plan für NeonArena

> Stand v0.81/v0.82: **QVM-Seite fertig** (CVars, Binds, Aim-Assist, `[JOY]` HUD, Test 102).
> Offen: Analog-Stick-Input und UI-Navigation in Quake3e (`patches/quake3e-controller.patch`).

## Übersicht
OpenArena hat Joystick/Gamepad-Support nur für Menüs, nicht für die Spielsteuerung. Wir implementieren vollständigen Controller-Support mit Aim-Assist.

## Schritte

### 1. CVars registrieren (g_main.c)
- `in_joystick` — Joystick an/aus (default: 1)
- `joy_threshold` — Schwellenwert für Analog-Sticks (default: 0.15)
- `joy_sensitivity` — Empfindlichkeit (default: 2.0)
- `joy_deadzone` — Deadzone (default: 0.12)
- `joy_assist` — Aim-Assist-Stärke (default: 0.3, 0 = aus, 1 = max)

### 2. Controller-Input verarbeiten (g_active.c)
- SDL2 Joystick-Events lesen
- Analog-Stick → Bewegung + Zielen
- Trigger → Schießen
- Buttons → Jump, Use, Reload

### 3. Aim-Assist (g_combat.c oder g_active.c)
- Wenn Controller aktiv: leichte Magnetwirkung am nächsten Ziel
- Nur bei Treffer-Kandidaten (Crosshair-Nähe)
- Stärke über `joy_assist` CVar einstellbar

### 4. UI-Navigation (ui_shared.c)
- Gamepad-Navigation im Menü (bereits teilweise vorhanden)
- Cursor-Steuerung mit Stick
- A/B Buttons für Auswahl/Zurück

### 5. Binds (autoexec.cfg)
- Standard-Controller-Binds
- `set in_joystick 1`
- `bind JOY1 +attack`
- `bind JOY2 +jump`
- usw.

## Dateien
- `oa-gamecode/code/game/g_main.c` — CVar-Registrierung
- `oa-gamecode/code/game/g_active.c` — Controller-Input
- `oa-gamecode/code/game/g_client.c` — Aim-Assist
- `oa-gamecode/code/ui/ui_shared.c` — UI-Navigation
- `assets/autoexec.cfg` — Standard-Binds

## Aufwand
- Tag 1: CVars + Input + Binds
- Tag 2: Aim-Assist + UI + Testing
