# SoundForge – Synth-Plugin (VST3) für FL Studio

Ein polyphoner Synthesizer mit 18 Werks-Presets (Supersaw, Sub Bass, Reese, Acid, Pluck, Pads, Bells, FX …).

**Klangaufbau:** 2 Oszillatoren (Saw/Square/Sine/Triangle, PolyBLEP anti-aliased) + Sub + Noise,
Unison (bis 7 Stimmen, Detune, Stereo-Spread), Filter (LP/HP/BP) mit eigener Hüllkurve, Amp-Hüllkurve,
LFO (→ Cutoff / Pitch), Effekte: Drive, Chorus, Delay, Reverb. 16 Stimmen, Pitch-Bend.

## Variante A: Fertiges Plugin über GitHub bauen lassen (einfachste Option)

1. Auf github.com ein neues Repository anlegen und diesen Ordner hochladen (inkl. `.github`).
2. Reiter **Actions** → Workflow „Build SoundForge“ läuft automatisch (ca. 10–15 Min.).
3. Unter dem fertigen Lauf bei **Artifacts** `SoundForge-VST3-Windows` herunterladen und entpacken.

## Variante B: Selbst bauen (Windows)

Voraussetzungen: Visual Studio 2022 („Desktop-Entwicklung mit C++“), CMake, Git.

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target SoundForge_VST3
```

Ergebnis: `build\SoundForge_artefacts\Release\VST3\SoundForge.vst3`

## In FL Studio installieren

1. `SoundForge.vst3` nach `C:\Program Files\Common Files\VST3\` kopieren.
2. FL Studio: **Options → Manage plugins → Find installed plugins** (ggf. „Verify plugins“ aktivieren).
3. Im Channel Rack: **+ → SoundForge** (zu finden unter „Installed“ → „Generators“).
4. Presets: Dropdown oben im Plugin oder die Pfeile `<` `>`.

## Anpassen

- Neue Presets: `Source/Presets.h` (Parameter-Namen stehen in `Source/Params.h`).
- Neue Parameter: Eintrag in `P::Id` und `defs[]` in `Params.h`, Verwendung in `SynthVoice.h`.

## Hinweis

Der Code kompiliert (unter Linux geprüft), wurde aber nicht in FL Studio getestet.
Bei zu lautem/leisem Klang: Volume-Regler oder Level-Regler anpassen.
