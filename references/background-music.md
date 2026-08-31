# NeonArena background music / sound recipe (v0.23)

## Where sounds live
- Source: `~/neon-arena/assets/sound/*.wav` are zipped into `neon-look.pk3` by `build-mod.sh`
  (the `sound` dir IS in the zip list: `scripts textures gfx env sound autoexec.cfg neon-look.cfg`).
- Verify after install: `unzip -l ~/.openarena/neonarena/neon-look.pk3 | grep neonwave_music`
- Existing format (match exactly): PCM **s16le, 44100 Hz, mono**. Check with:
  `ffmpeg -hide_banner -i file.wav 2>&1 | grep Stream`

## Synthesize the loop locally (no external assets / accounts)
`tools/synth_neon_music.py` uses only `numpy` + stdlib `wave` (no `soundfile` needed):
```
cd ~/neon-arena
python3 tools/synth_neon_music.py          # writes neonwave_music.wav at cwd
cp neonwave_music.wav assets/sound/
rm -f neonwave_music.wav
```
Design notes from the generator: A-minor pentatonic arpeggio + sub bass + soft hat, 5-bar phrase,
12.5 s, seamless crossfade of the last ~50 ms into the head so it loops without a click. Tune
`LOOP_BARS`, `BPM`, `ARP` to taste — keep it mono 44100 so it matches the other wavs.

## Wire it into the mod (3 edits)
1. `code/cgame/cg_local.h` — add field next to `neonWaveStartSound`:
   `sfxHandle_t   neonMusicSound;`
2. `code/cgame/cg_main.c` in `CG_RegisterSounds()` (near the `neonWaveStartSound` line):
   `cgs.media.neonMusicSound = trap_S_RegisterSound("sound/neonwave_music.wav", qfalse);`
3. `code/cgame/cg_draw.c` — in the `CS_NEONWAVE` event block, on `ev == 0` (wave start), after the
   start jingle, re-trigger the loop, gated by a cvar:
   ```c
   char mbBuf[8];
   trap_Cvar_VariableStringBuffer("g_neonwave_music", mbBuf, sizeof(mbBuf));
   if (atoi(mbBuf) != 0)
       trap_S_StartLocalSound(cgs.media.neonMusicSound, CHAN_ANNOUNCER);
   ```
   Q3 local sounds are ONE-SHOT, so re-triggering on each wave start keeps it continuous.

## Gotchas
- `trap_S_RegisterSound` / `StartLocalSound` are SAFE in headless ioq3ded (no audio device) — no-op,
  no new test required. Verify the wav is in the pk3, not that it "plays".
- Don't add a test that greps for audio output — there is none in headless.
- After editing cgame, run `make` in `oa-gamecode`, then `./build-mod.sh` in the parent, then check the
  pk3 contains the wav.
