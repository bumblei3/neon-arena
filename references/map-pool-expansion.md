# Map-Pool-Erweiterung für NeonArena

## Warum

Momentan drei Maps (`oa_shine`, `oa_minia`, `oa_rpg3dm2`) im Daily-Pool. Für regelmäßiges
Spielen (insbesondere Daily-Challenge) ist die Auswahl zu klein — spieler spüren nach
einigen Tagen Wiederholung. Für 5–8 Karten öffnet sich ein größeres Wiederspiel-Feld
und das tägliche Gefühl bleibt frisch.

## Ansatz

Zwei Schritte:
1. **Map-Pool erweitern** — neue `.map`-Dateien oder OA-kompatible Karten herunterladen,
   in pflegbare Sammlung übernehmen.
2. **Look-Config pro Karte** — jede Karte hat eigene Parameter für Skybox-Helligkeit,
   Grid-Alpha, Bloom-Intensität, damit Neon-Look auf jeder Karte gut funktioniert
   (nicht jede Karte trägt Additive-Elemente gleich gut).

## Map-Pool (Beispiel)

JSON-Schema (siehe `references/map-config-example.json`):

```jsonc
{
  "daily_indices": {
    "pool": [
      "oa_shine",
      "oa_minia",
      "oa_rpg3dm2",
      "oa_bleed",
      "oa_node",
      "oa_pulse",
      "oa_desert",
      "oa_vortex"
    ]
  },
  "maps": {
    "oa_shine": {
      "name": "Shine",
      "look": {
        "grid_alpha": 0.65,
        "bloom_intensity": 1.0
      }
    }
    // ... weitere
  }
}
```

Der Daily-Hash (`g_neonwave_dailyseed`-Logik in `g_neonwave.c`) wählt aus dem Pool
per `index % pool_size`. Mit mehr Karten bekommt jeder Tag eine andere Karte —
solange der Pool nicht genau 3 Karten hat.

## Look-Config (was wichtig ist)

Für jede Karte relevant:
- **Skybox-Helligkeit**: Wenn die Skybox zu hell ist, wirken additive Neon-Elemente
  (Railgun-Trails, Grid) weniger — dann `skybox_dark: true` + manuell dunkleres
  Skybox-TGA oder `r_skybox`-Override.
- **Grid-Alpha**: Die additive Grid-Schicht (`grid.tga` oder äquivalent) muss auf
  jeder Karte angepasst werden — zu stark auf dunklen Karten, zu schwach auf hellen.
- **Bloom-Intensität**: `r_bloom` ist global (Engine), aber pro Karte kann der
  Eindruck stark variieren. Eine Karte mit weniger additivem Hintergrund (z.B.
  `oa_desert`) braucht weniger Bloom, sonst wirken Neon-Elemente überstrahlt.

## Karte hinzufügen (schrittweise)

1. Map-Datei (Quell-zip mit `.map` + `.ent`) oder PK3 mit eingebauter `.map` besorgen.
   OpenArena-kompatibel — keine engine-spezifischen Bäume.
2. Testen: `openarena +map oa_<name>` — Karte lädt, keine Engine-Warnungen.
3. Look-Config in `references/map-config.json` (oder ähnlich) anlegen.
4. In `g_neonwave.c` oder Server-Start-Logik: Map-Config beim Map-Wechsel oder
   Daily-Auswahl anwenden (aktuell ist `ui_neonwave_dailymap` ein Cvar, das die
   ausgewählte Karte gibt — da kann parallel ein `ui_neonwave_dailymap_look`
   JSON-String oder Einzel-Cvars kommen).
5. Testen mit Bloom-Engine: Karte + Neon-Look + Bloom gut sichtbar, keine Überhellung.

## Scope (erste Version)

- 5–8 Karten im Pool.
- Pro Karte minimale Look-Config (Grid-Alpha, Bloom).
- Keine automatische Karten-Rekognition — keine AI, keine map-id-Erkennung außer dem
  Cvar `ui_neonwave_dailymap`.

## Risiken

- Maps aus fremden Quellen haben keine kompatible `.map`-Struktur → Engine-Warnung oder
  Absturz. Testen ist Pflicht.
- Einige Karten haben keine flachen Flächen für Grid-Schicht → Grid-Effekt wirkt
  verzerrt. Dann Grid-Alpha niedrig halten oder Karte aus Pool nehmen.
- Bloom-Überregulierung auf bestimmten Karten → manuelle Nachregulierung pro Karte
  (via Cvar) ist okay, aber sollte dokumentiert sein.
