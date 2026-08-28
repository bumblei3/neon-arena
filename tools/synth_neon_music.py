#!/usr/bin/env python3
"""Synthesize neonwave_music.wav: a seamless, royalty-free neon-synth loop.

Output: 16-bit PCM mono @ 44100 Hz, ~40s, matching the existing
assets/sound/*.wav format (so it drops straight into neon-look.pk3).

Design: a slow arpeggiated minor-key synth pad + sub bass + soft hat,
looped seamlessly by crossfading the tail into the head. No external
samples, no network, no accounts — fully local.
"""
import math
import struct
import wave

SR = 44100
BPM = 96
BEAT = 60.0 / BPM
BAR = BEAT * 4
# A minor pentatonic-ish loop (Hz): A2, C3, E3, A3, C4, E4
NOTES = [110.00, 130.81, 164.81, 220.00, 261.63, 329.63]
# 2-bar phrase (8 beats) of arpeggio indices
ARP = [0, 2, 4, 5, 4, 2, 0, 2]
LOOP_BARS = 5          # total loop length = 5 bars * 4 beats
TOTAL = int(SR * BAR * LOOP_BARS)
FADE = int(SR * 0.05)  # tiny crossfade to guarantee seamless loop

def env(t, dur, a=0.01, r=0.15):
    """simple ADSR-ish amplitude for a note of length dur starting at t."""
    if t < 0 or t > dur:
        return 0.0
    if t < a * dur:
        return t / (a * dur)
    if t > dur - r * dur:
        return max(0.0, (dur - t) / (r * dur))
    return 1.0

def note(t, freq, dur, amp, wave='saw'):
    pos = t % dur
    e = env(pos, dur)
    if e <= 0:
        return 0.0
    ph = 2 * math.pi * freq * pos
    if wave == 'saw':
        # band-limited-ish saw via a few harmonics
        s = (math.sin(ph) + 0.5 * math.sin(2 * ph) + 0.33 * math.sin(3 * ph))
        s /= 1.83
    elif wave == 'tri':
        s = abs((pos / dur) % 2 - 1) * 2 - 1
    else:
        s = math.sin(ph)
    return s * e * amp

buf = [0.0] * TOTAL
for bar in range(LOOP_BARS):
    for step, idx in enumerate(ARP):
        t0 = (bar * BAR + step * BEAT) * SR
        freq = NOTES[idx]
        # pad note (long, saw) + a higher octave shimmer (tri)
        for n in range(int(BEAT * SR)):
            i = int(t0) + n
            if i >= TOTAL:
                break
            s = (note(n / SR, freq, BEAT, 0.22, 'saw')
                 + note(n / SR, freq * 2, BEAT * 0.5, 0.06, 'tri'))
            buf[i] += s
        # sub bass on the beat
        for n in range(int(BEAT * SR)):
            i = int(t0) + n
            if i >= TOTAL:
                break
            buf[i] += note(n / SR, freq / 2, BEAT * 0.9, 0.18, 'sine')

# soft hat: very quiet noise burst on offbeats
for bar in range(LOOP_BARS):
    for step in range(8):
        if step % 2 == 1:
            t0 = (bar * BAR + step * BEAT) * SR
            for n in range(int(0.03 * SR)):
                i = int(t0) + n
                if i >= TOTAL:
                    break
                buf[i] += (0.04 * (0.5 - ((n * 9301 + 49297) % 233280) / 233280.0))

# normalize to ~0.8 peak
peak = max(1e-6, max(abs(x) for x in buf))
buf = [x / peak * 0.8 for x in buf]

# seamless loop: crossfade last FADE samples with first FADE samples
for n in range(FADE):
    a = n / FADE
    i_head = n
    i_tail = TOTAL - FADE + n
    buf[i_head] = buf[i_head] * (1 - a) + buf[i_tail] * a

with wave.open('neonwave_music.wav', 'w') as w:
    w.setnchannels(1)
    w.setsampwidth(2)
    w.setframerate(SR)
    frames = b''.join(struct.pack('<h', int(max(-1, min(1, x)) * 32767)) for x in buf)
    w.writeframes(frames)

print("wrote neonwave_music.wav: %d samples, %.1fs, %d Hz mono" %
      (len(buf), len(buf) / SR, SR))
