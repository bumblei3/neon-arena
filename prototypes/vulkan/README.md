# NEON ARENA - Vulkan + SDL2 Prototype

Modern Vulkan renderer for NeonArena — triangle proof of concept, expandable to full game.

## Build

```sh
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./neon-arena
```

## Controls

- `ESC` — quit

## Architecture

```
main.cpp          — bootstrap, game loop, SDL2
renderer.h/cpp    — Vulkan instance, device, swapchain, pipeline
shaders/          — GLSL source → SPIR-V (compiled at build time)
```

The Vulkan renderer is split into initialization (`Renderer::init`) and a minimal
draw loop. Everything after the triangle proof-of-concept goes into `Game` state.
