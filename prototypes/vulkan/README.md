# NEON ARENA - Vulkan + SDL2 Prototype

Modern Vulkan renderer for NeonArena — arena, enemies, waves, shooting, particles.

## Build

```sh
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./neon-arena
```

Dependencies: `cmake`, `libsdl2-dev`, `libvulkan-dev`, `glslang-tools`

## Controls

- `WASD` — move
- `Mouse` — look
- `Click` / `Space` — shoot
- `ESC` — quit

## Architecture

```
main.cpp          — bootstrap, game loop, input
game.h/cpp        — game state, waves, enemies, fx, math
renderer.h/cpp    — Vulkan instance, device, swapchain, buffers
shaders/          — GLSL → SPIR-V (compiled at build time)
```

## Features

- Arena: floor grid, neon walls, boundary
- Enemies: chase player, touch damage, wave scaling
- Shooting: ray vs AABB, tracers, spark particles
- FX: hit flash, recoil, bloom-ready pipeline

## Next Steps

1. Separate line pipeline for grid + tracers
2. Bloom post-processing (render-to-texture + blur passes)
3. HUD overlay (text rendering)
4. Map loading from PK3
5. Multi-threaded command buffer recording
