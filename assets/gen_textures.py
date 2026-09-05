#!/usr/bin/env python3
"""Procedural NeonArena textures (TGA/JPG) for the look pack."""
import math
import os
import struct

ROOT = os.path.dirname(os.path.abspath(__file__))


def _clamp(v):
    return 0 if v < 0 else 255 if v > 255 else int(v)


def write_tga(path, w, h, pixels, alpha=True):
    """Uncompressed TGA, origin bottom-left. pixels is top-left RGB or RGBA."""
    os.makedirs(os.path.dirname(path), exist_ok=True)
    bpp = 32 if alpha else 24
    header = struct.pack(
        "<BBBHHBHHHHBB",
        0, 0, 2, 0, 0, 0, 0, 0, w, h, bpp, 8 if alpha else 0,
    )
    body = bytearray()
    for y in range(h - 1, -1, -1):
        row = pixels[y * w:(y + 1) * w]
        for p in row:
            r, g, b = p[0], p[1], p[2]
            if alpha:
                a = p[3] if len(p) > 3 else 255
                body.extend((b, g, r, a))
            else:
                body.extend((b, g, r))
    with open(path, "wb") as f:
        f.write(header)
        f.write(body)


def write_jpg(path, w, h, pixels):
    from PIL import Image
    os.makedirs(os.path.dirname(path), exist_ok=True)
    img = Image.new("RGB", (w, h))
    img.putdata([(p[0], p[1], p[2]) for p in pixels])
    img.save(path, "JPEG", quality=88, optimize=True)


def radial(w, h, color, power=2.2, inner=0.0):
    cx, cy = (w - 1) / 2.0, (h - 1) / 2.0
    maxd = math.sqrt(cx * cx + cy * cy)
    out = []
    cr, cg, cb = color
    for y in range(h):
        for x in range(w):
            nx = (x - cx) / maxd
            ny = (y - cy) / maxd
            d = math.sqrt(nx * nx + ny * ny)
            t = max(0.0, 1.0 - d)
            t = (t - inner) / (1.0 - inner) if inner < 1 else t
            if t < 0:
                t = 0
            t = t ** power
            out.append((_clamp(cr * t), _clamp(cg * t), _clamp(cb * t), _clamp(255 * t)))
    return out


def flare():
    # cyan core + white hot center, additive-friendly (black edges)
    w = h = 128
    cx = cy = (w - 1) / 2.0
    out = []
    for y in range(h):
        for x in range(w):
            dx = (x - cx) / cx
            dy = (y - cy) / cy
            d = math.sqrt(dx * dx + dy * dy)
            core = math.exp(-d * d * 9.0)
            halo = math.exp(-d * d * 2.2) * 0.55
            spike = 0.0
            if abs(dx) < 0.04:
                spike += math.exp(-abs(dy) * 3.5) * 0.45
            if abs(dy) < 0.04:
                spike += math.exp(-abs(dx) * 3.5) * 0.45
            t = min(1.0, core + halo + spike)
            r = 40 * t + 200 * core
            g = 230 * t + 25 * core
            b = 255 * t
            out.append((_clamp(r), _clamp(g), _clamp(b), 255))
    return w, h, out


def spark():
    w = h = 64
    cx = cy = (w - 1) / 2.0
    out = []
    for y in range(h):
        for x in range(w):
            dx = (x - cx) / cx
            dy = (y - cy) / cy
            d = math.sqrt(dx * dx * 0.35 + dy * dy)  # slightly tall
            t = math.exp(-d * d * 8.0)
            out.append((_clamp(80 * t), _clamp(255 * t), _clamp(255 * t), 255))
    return w, h, out


def railcore():
    w, h = 128, 16
    out = []
    for y in range(h):
        ny = abs((y - (h - 1) / 2.0) / ((h - 1) / 2.0))
        line = (1.0 - ny) ** 3
        glow = (1.0 - ny) ** 1.4 * 0.55
        for x in range(w):
            pulse = 0.75 + 0.25 * math.sin(x / w * math.pi * 4)
            t = min(1.0, (line + glow) * pulse)
            out.append((_clamp(30 * t), _clamp(255 * t), _clamp(255 * t), 255))
    return w, h, out


def grid(size=256, major=32, minor=8):
    out = []
    for y in range(size):
        for x in range(size):
            r = g = b = 0
            on_major = (x % major == 0) or (y % major == 0)
            on_minor = (x % minor == 0) or (y % minor == 0)
            # soft glow around major lines
            dxm = min(x % major, major - (x % major))
            dym = min(y % major, major - (y % major))
            dmin = min(dxm, dym)
            glow = math.exp(-dmin * dmin * 0.35) * 0.55
            if on_major:
                r, g, b = 20, 210, 230
            elif on_minor:
                r, g, b = 4, 40, 48
            r = _clamp(r + 18 * glow)
            g = _clamp(g + 90 * glow)
            b = _clamp(b + 110 * glow)
            out.append((r, g, b, 255))
    return size, size, out


def night_sky(w=512, h=512, seed=7):
    # deterministic starfield
    rng = seed
    def rnd():
        nonlocal rng
        rng = (1103515245 * rng + 12345) & 0x7FFFFFFF
        return rng / 0x7FFFFFFF
    out = [(4, 6, 14)] * (w * h)
    # faint nebula
    for y in range(h):
        for x in range(w):
            n = 0.5 + 0.5 * math.sin(x * 0.01 + y * 0.007) * math.sin(x * 0.004 - y * 0.011)
            i = y * w + x
            r, g, b = out[i]
            out[i] = (
                _clamp(r + 8 * n),
                _clamp(g + 18 * n),
                _clamp(b + 28 * n),
            )
    # stars
    for _ in range(420):
        x = int(rnd() * w)
        y = int(rnd() * h)
        bright = 0.35 + rnd() * 0.65
        rad = 1 if rnd() > 0.15 else 2
        for dy in range(-rad, rad + 1):
            for dx in range(-rad, rad + 1):
                xx, yy = (x + dx) % w, (y + dy) % h
                fall = math.exp(-(dx * dx + dy * dy) * 0.7) * bright
                i = yy * w + xx
                r, g, b = out[i]
                out[i] = (
                    _clamp(r + 180 * fall),
                    _clamp(g + 230 * fall),
                    _clamp(b + 255 * fall),
                )
    return w, h, [(p[0], p[1], p[2], 255) for p in out]


def cubemap_face(kind, size=256):
    _, _, sky = night_sky(size, size, seed={"ft": 3, "bk": 11, "lf": 19, "rt": 29, "up": 41, "dn": 53}[kind])
    if kind == "dn":
        # looking down: darker, slight cyan wash
        return [( _clamp(p[0] * 0.4), _clamp(p[1] * 0.5 + 8), _clamp(p[2] * 0.55 + 12), 255) for p in sky]
    if kind == "up":
        return [(_clamp(p[0] * 0.7), _clamp(p[1] * 0.75), _clamp(p[2] * 0.9), 255) for p in sky]
    # horizon cyan band in the lower third
    out = []
    for y in range(size):
        band = math.exp(-((y / size) - 0.72) ** 2 * 40) * 0.35
        for x in range(size):
            p = sky[y * size + x]
            out.append((
                _clamp(p[0] + 10 * band),
                _clamp(p[1] + 80 * band),
                _clamp(p[2] + 110 * band),
                255,
            ))
    return out


def vignette(size=256):
    cx = cy = (size - 1) / 2.0
    out = []
    for y in range(size):
        for x in range(size):
            nx = abs(x - cx) / cx
            ny = abs(y - cy) / cy
            # square-ish vignette, stronger in corners
            e = max(nx, ny)
            t = 0.0
            if e > 0.45:
                t = ((e - 0.45) / 0.55) ** 1.4
            a = _clamp(210 * t)
            out.append((_clamp(0), _clamp(18 * t), _clamp(28 * t), a))
    return size, size, out


def crosshair(size=64):
    out = [(0, 0, 0, 0)] * (size * size)
    c = size // 2
    cyan = (40, 255, 255, 255)
    dim = (0, 140, 160, 200)
    gap, arm, thick = 4, 14, 1

    def put(x, y, col):
        if 0 <= x < size and 0 <= y < size:
            out[y * size + x] = col

    for d in range(gap, gap + arm):
        for t in range(-thick, thick + 1):
            put(c + d, c + t, cyan)
            put(c - d, c + t, cyan)
            put(c + t, c + d, cyan)
            put(c + t, c - d, cyan)
        put(c + d, c, dim)
        put(c - d, c, dim)
        put(c, c + d, dim)
        put(c, c - d, dim)
    # center dot
    put(c, c, cyan)
    put(c + 1, c, dim)
    put(c - 1, c, dim)
    put(c, c + 1, dim)
    put(c, c - 1, dim)
    return size, size, out


def energy_splat(size=128, seed=3):
    rng = seed
    def rnd():
        nonlocal rng
        rng = (1103515245 * rng + 12345) & 0x7FFFFFFF
        return rng / 0x7FFFFFFF
    cx = cy = (size - 1) / 2.0
    blobs = [(cx, cy, 0.55)]
    for _ in range(7):
        ang = rnd() * math.pi * 2
        dist = 12 + rnd() * 38
        blobs.append((cx + math.cos(ang) * dist, cy + math.sin(ang) * dist, 0.18 + rnd() * 0.28))
    out = []
    for y in range(size):
        for x in range(size):
            acc = 0.0
            for bx, by, br in blobs:
                d = math.sqrt((x - bx) ** 2 + (y - by) ** 2) / (size * br)
                acc += math.exp(-d * d * 4.5)
            t = min(1.0, acc)
            a = _clamp(255 * (t ** 0.8))
            out.append((_clamp(30 * t), _clamp(240 * t), _clamp(255 * t), a))
    return size, size, out


def blood_screen(size=256):
    # edge-only cyan energy so the center stays readable
    rng = 99
    def rnd():
        nonlocal rng
        rng = (1103515245 * rng + 12345) & 0x7FFFFFFF
        return rng / 0x7FFFFFFF
    out = [(0, 0, 0, 0)] * (size * size)
    for _ in range(18):
        side = int(rnd() * 4)
        if side == 0:
            x0, y0 = rnd() * size, rnd() * 28
        elif side == 1:
            x0, y0 = rnd() * size, size - rnd() * 28
        elif side == 2:
            x0, y0 = rnd() * 28, rnd() * size
        else:
            x0, y0 = size - rnd() * 28, rnd() * size
        rad = 18 + rnd() * 40
        for y in range(size):
            for x in range(size):
                d = math.sqrt((x - x0) ** 2 + (y - y0) ** 2) / rad
                t = math.exp(-d * d * 3.2)
                if t < 0.04:
                    continue
                i = y * size + x
                r, g, b, a = out[i]
                nt = t * (0.7 + 0.3 * rnd())
                out[i] = (
                    _clamp(r + 20 * nt),
                    _clamp(g + 220 * nt),
                    _clamp(b + 255 * nt),
                    _clamp(a + 180 * nt),
                )
    return size, size, out


def ghost_scope(size=512):
    """Circular sniper iris: transparent hole, opaque black outside, cyan ring."""
    cx = cy = (size - 1) / 2.0
    r_clear = size * 0.40
    r_ring = size * 0.42
    r_dark = size * 0.46
    out = []
    for y in range(size):
        for x in range(size):
            d = math.sqrt((x - cx) ** 2 + (y - cy) ** 2)
            if d < r_clear:
                a = 0.0
            elif d < r_dark:
                t = (d - r_clear) / (r_dark - r_clear)
                a = t * t
            else:
                a = 1.0
            ring = math.exp(-((d - r_ring) ** 2) / ((size * 0.012) ** 2))
            r = 12 * a + 40 * ring
            g = 8 * a + 220 * ring
            b = 10 * a + 255 * ring
            alpha = max(a * 235, ring * 200)
            out.append((_clamp(r), _clamp(g), _clamp(b), _clamp(alpha)))
    return size, size, out


def hud_bar(w=256, h=32):
    out = []
    for y in range(h):
        for x in range(w):
            edge = x < 2 or x >= w - 2 or y < 2 or y >= h - 2
            inner = 2 <= x < w - 2 and 2 <= y < h - 2
            if edge:
                out.append((40, 255, 255, 230))
            elif inner:
                ny = abs(y - h / 2) / (h / 2)
                t = 0.35 + 0.45 * (1 - ny)
                out.append((_clamp(10 * t), _clamp(80 * t), _clamp(90 * t), 160))
            else:
                out.append((0, 0, 0, 0))
    return w, h, out


def main():
    jobs = []

    w, h, p = flare()
    write_tga(os.path.join(ROOT, "textures/neonarena/flare.tga"), w, h, p, True)
    jobs.append("flare")

    w, h, p = spark()
    write_tga(os.path.join(ROOT, "textures/neonarena/spark.tga"), w, h, p, True)
    jobs.append("spark")

    w, h, p = railcore()
    write_tga(os.path.join(ROOT, "textures/neonarena/railcore.tga"), w, h, p, True)
    jobs.append("railcore")

    w, h, p = grid()
    write_tga(os.path.join(ROOT, "textures/neonarena/grid.tga"), w, h, p, True)
    jobs.append("grid")

    w, h, p = night_sky()
    write_jpg(os.path.join(ROOT, "textures/neonarena/night.jpg"), w, h, p)
    jobs.append("night")

    for face in ("ft", "bk", "lf", "rt", "up", "dn"):
        pix = cubemap_face(face)
        write_jpg(os.path.join(ROOT, f"env/anoice1/anoice1_{face}.jpg"), 256, 256, pix)
    jobs.append("skybox")

    w, h, p = vignette()
    write_tga(os.path.join(ROOT, "gfx/2d/neon_vignette.tga"), w, h, p, True)
    jobs.append("vignette")

    w, h, p = crosshair()
    write_tga(os.path.join(ROOT, "gfx/2d/crosshaira.tga"), w, h, p, True)
    jobs.append("crosshair")

    w, h, p = energy_splat(128, 3)
    write_tga(os.path.join(ROOT, "gfx/damage/blood_stain.tga"), w, h, p, True)
    w, h, p = energy_splat(64, 11)
    write_tga(os.path.join(ROOT, "gfx/damage/blood_spurt.tga"), w, h, p, True)
    w, h, p = blood_screen()
    write_tga(os.path.join(ROOT, "gfx/damage/blood_screen.tga"), w, h, p, True)
    jobs.append("energy-blood")

    w, h, p = hud_bar()
    write_tga(os.path.join(ROOT, "gfx/2d/neon_bar.tga"), w, h, p, True)
    jobs.append("hud-bar")

    w, h, p = ghost_scope()
    write_tga(os.path.join(ROOT, "gfx/2d/ghost_scope.tga"), w, h, p, True)
    jobs.append("ghost-scope")

    print("wrote:", ", ".join(jobs))


if __name__ == "__main__":
    main()
