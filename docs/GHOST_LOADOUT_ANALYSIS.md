# Ghost Loadout Balance-Analyse

> Erstellt: 2026-09-08
> Stand: v0.82 Loadouts v1.2 (Infiltrator / Saboteur / Spectre)

---

## Energie-Budget pro Loadout

| Loadout | Start | Max | Regen | Cloak | EMP | Lockdown | Nuke | Multiscan |
|---------|-------|-----|-------|-------|-----|----------|------|-----------|
| **Infiltrator** | 80 | 100 | 4/s | 25 | 35 (25s) | 50 (20s) | — | 30 (3s) |
| **Saboteur** | 70 | 100 | 4/s | 25 | 25 (20s) | 35 (20s) | — | 30 (3s) |
| **Spectre** | 90 | 100 | 4/s | — | 35 (25s) | 50 (20s) | 80 (45s) | 30 (3s) |

---

## Kombinationsanalyse

### Infiltrator (Balanced)
- EMP + Lockdown = 85 Energy → **nicht gleichzeitig** möglich (80 Start)
- EMP + Multiscan = 65 → OK
- Lockdown + Multiscan = 80 → OK (genau)
- **Spielstil**: Taktisch, wähle zwischen CC (Lockdown) oder AoE (EMP)

### Saboteur (Energie-effizient)
- EMP + Lockdown = 60 Energy → **gleichzeitig** möglich (70 Start, 10 Rest)
- EMP + Lockdown + Multiscan = 90 → OK
- **Spielstil**: Aggressive CC-Kombo, häufigere Fähigkeiten

### Spectre (High Risk/High Reward)
- Nuke allein = 80 Energy → sofort nutzbar (90 Start)
- Nuke + EMP = 115 → über Max, warten nötig
- Kein Cloak = hohes Risiko (kein Invis-ESC)
- **Spielstil**: Alpha-Strike, Nuke-Painting, Boss-Meltdown

---

## Empfohlene Fein-Balancing (v0.90)

### Infiltrator
- Aktuell: Solide, ausgewogen
- Empfehlung: **Keine Änderungen**

### Saboteur
- Aktuell: Stärkste CC-Kombo
- Risiko: EMP (25) + Lockdown (35) + Multiscan (30) = 90 → 3-Fähigkeits-Kombo
- Empfehlung: EMP-CD von 20s auf **22s** erhöhen (verhindert Spam)

### Spectre
- Aktuell: Nuke sofort verfügbar (80 < 90)
- Risiko: Player spart 90 Energy, warten nur 1 Regen-Tick
- Empfehlung: Nuke-Cost von 80 auf **85** anheben

---

## Test-Szenarien

| # | Szenario | Erwartetes Verhalten |
|---|----------|---------------------|
| 1 | Infiltrator Wave 1 | EMP + Multiscan + 3 Schüsse Rail |
| 2 | Saboteur Wave 1 | EMP + Lockdown + 2 Schüsse Rail |
| 3 | Spectre Wave 1 | Nuke + 4 Schüsse Rail |
| 4 | Spectre Boss | Nuke-Paint → Rail-Phase → Lockdown |

---

## Playtest-Checkliste für v0.90

- [ ] Saboteur EMP+Lockdown-Kombo in Welle 1 zu stark?
- [ ] Spectre Nuke-DPS vs Boss zu hoch?
- [ ] Infiltrator ausgewogen?
- [ ] Rumble bei Nuke-Detonation wahrgenommen?
- [ ] Gamepad-Navi in Ghost-Menüs funktioniert?
- [ ] Loadout-Wechsel (`loadout 0/1/2`) funktioniert korrekt?
- [ ] Energie-Reset beim Loadout-Wechsel korrekt?
