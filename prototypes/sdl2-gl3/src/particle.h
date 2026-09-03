// particle.h - Particle system for neon arena prototype
#pragma once
#include "math.h"

struct Particle {
    Vec3 pos;
    Vec3 vel;
    Vec3 color;
    float life;
    float maxLife;
    float size;

    Particle(Vec3 p, Vec3 v, Vec3 c, float l, float s)
        : pos(p), vel(v), color(c), life(l), maxLife(l), size(s) {}
};
