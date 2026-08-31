# NeonArena build / install / dist / release commands

## Exact build sequence (order matters)
```bash
# 1. compile QVMs + .so (in submodule)
cd ~/neon-arena/oa-gamecode && make
# -> oa-gamecode/build/release-linux-x86_64/neonarena/{vm/*.qvm, *.so}

# 2. install fresh artifacts to the dir the server loads
cd ~/neon-arena && ./build-mod.sh
# -> copies vm/*.qvm + *.so to ~/.openarena/neonarena/
# -> zips neon-look.pk3 from assets/ (scripts textures gfx env sound autoexec.cfg neon-look.cfg)
#    into BOTH ~/.openarena/neonarena/ AND dist/
# -> installs QVMs to dist/neonarena-qvm.pk3
```
**PITFALL:** headless `ioq3ded` loads QVMs from `~/.openarena/neonarena/vm/`. If you `make` but skip
`build-mod.sh`, tests run against STALE bytecode and new features don't appear. Always `build-mod.sh`
after `make`.

## Clean dist/ packaging (release artifact, ~1.9 MB — NOT a dump of ~/.openarena)
```bash
cd ~/neon-arena
mkdir -p dist/neonarena/vm dist/neonarena-zip-tmp
OUT=oa-gamecode/build/release-linux-x86_64/neonarena
cp "$OUT/vm/"*.qvm dist/neonarena/vm/
cp "$OUT"/*.so dist/neonarena/ 2>/dev/null || true
( cd assets && zip -rq ../dist/neonarena-zip-tmp/neon-look.pk3 \
    scripts textures gfx env sound autoexec.cfg neon-look.cfg )
cp dist/neonarena-zip-tmp/neon-look.pk3 dist/neonarena/neon-look.pk3
cd dist && rm -f neonarena.pk3 && zip -rq neonarena.pk3 neonarena && cd ..
( cd "$OUT" && rm -f ~/neon-arena/dist/neonarena-qvm.pk3 && \
    zip -q ~/neon-arena/dist/neonarena-qvm.pk3 vm/cgame.qvm vm/qagame.qvm vm/ui.qvm )
```
- NEVER commit `games.log` (64 MB) or `q3config.cfg` into dist/. Past mistake: a dist/neonarena.pk3
  was 4.8 MB / 64 MB from bundling the live server dir.
- `dist/neon-arena.pk3` must contain `neon-look.pk3` (with sound) nested inside, or the release
  ships a silent mod (see CI divergence below).

## CI divergence (fixed v0.18 — keep both paths identical)
- `build-mod.yml` (GitHub Actions) assembled `neon-look.pk3` from ONLY `scripts + autoexec.cfg`,
  excluding `sound/` → the start-jingle (`neonwave_start.wav`) was DEAD in every release.
- Fix: both `build-mod.yml` (~line 74 and ~line 131) and `build-mod.sh` must zip the SAME set:
  `scripts textures gfx env sound autoexec.cfg neon-look.cfg`.
- Build-mod.yml also now copies `dist/neonarena-zip-tmp/neon-look.pk3` into the release artifact.

## Release gate
- Tag `v0.x` on parent `main` → CI builds + runs FULL suite on tag + creates GitHub release with
  `dist/neonarena.pk3` + `dist/neonarena-qvm.pk3` (via softprops/action-gh-release).
- Local full-suite gate: since v0.20 `./tests/run_suite.sh` (no args) runs ALL 18 tests via the
  `all` mode (`dispatch_test()` map). Use `--quick` (subset 1,3,4,7,8,10,12,13,17) for fast feedback,
  or a manual loop `for t in 1 2 3 4 5 6 7 8 9 9b 10 11 12 13 14 15 16 17; do ./tests/run_suite.sh --test "$t"; done`.
- Commit order before tag: (1) submodule feature commit → (2) `git add oa-gamecode` in parent →
  (3) parent commit (tests + parse map) → (4) dist/ rebuild commit → (5) `git tag v0.x` → push
  both repos + tag.
