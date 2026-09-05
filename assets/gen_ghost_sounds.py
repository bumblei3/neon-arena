#!/usr/bin/env python3
"""Short PCM cues for the Ghost kit. 44.1 kHz 16-bit mono, like nw_combo*.wav."""
import math
import os
import struct
import wave

ROOT = os.path.dirname(os.path.abspath(__file__))
SR = 44100


def write_wav(name, samples):
    path = os.path.join(ROOT, "sound", name)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with wave.open(path, "w") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SR)
        buf = b"".join(
            struct.pack("<h", max(-32767, min(32767, int(s * 32767)))) for s in samples
        )
        w.writeframes(buf)
    print("wrote", path, "n=", len(samples))


def env(i, n, atk=0.01, rel=0.12):
    t = i / float(n)
    a = min(1.0, t / atk) if atk > 0 else 1.0
    r = min(1.0, (1.0 - t) / rel) if rel > 0 else 1.0
    return a * r


def sine(n, f0, f1=None):
    out = []
    for i in range(n):
        t = i / float(SR)
        f = f0 if f1 is None else f0 + (f1 - f0) * (i / float(n))
        out.append(math.sin(2 * math.pi * f * t) * env(i, n))
    return out


def noise(n, amp=0.35):
    x = 1
    out = []
    for i in range(n):
        x = (1103515245 * x + 12345) & 0x7FFFFFFF
        out.append(((x / 0x7FFFFFFF) * 2 - 1) * amp * env(i, n, 0.002, 0.2))
    return out


def mix(*seqs):
    n = max(len(s) for s in seqs)
    out = [0.0] * n
    for s in seqs:
        for i, v in enumerate(s):
            out[i] += v
    m = max(0.001, max(abs(v) for v in out))
    if m > 0.95:
        out = [v * 0.95 / m for v in out]
    return out


def main():
    write_wav("ghost_cloak_on.wav", sine(int(SR * 0.28), 420, 1400))
    write_wav("ghost_cloak_off.wav", sine(int(SR * 0.22), 1100, 280))
    n = int(SR * 0.18)
    lo = sine(n, 90)
    nz = noise(n, 0.55)
    hi = sine(n, 1400, 400)
    write_wav("ghost_emp.wav", mix([lo[i] * 0.5 + nz[i] + hi[i] * 0.35 for i in range(n)]))
    a = sine(int(SR * 0.09), 880)
    b = [0.0] * int(SR * 0.04) + sine(int(SR * 0.11), 1320)
    write_wav("ghost_lock.wav", mix(a, b))
    write_wav("ghost_nuke.wav", mix(sine(int(SR * 0.36), 220, 55), noise(int(SR * 0.12), 0.25)))
    ping = sine(int(SR * 0.16), 1760)
    echo = [0.0] * int(SR * 0.07) + [v * 0.35 for v in sine(int(SR * 0.12), 1760)]
    write_wav("ghost_scan.wav", mix(ping, echo))


if __name__ == "__main__":
    main()
