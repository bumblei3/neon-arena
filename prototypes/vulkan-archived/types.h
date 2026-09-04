// NEON ARENA - Vulkan + SDL2 Prototype
// Shared types
#pragma once

#include <cmath>

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(float s) const { return {x*s, y*s, z*s}; }
    float length() const { return sqrtf(x*x + y*y + z*z); }
    Vec3 normalized() const { float l = length(); return l > 0 ? Vec3(x/l, y/l, z/l) : Vec3(); }
};

struct Vertex {
    Vec3 pos;
    Vec3 color;
};

struct HudVertex {
    float pos[2];
    float uv[2];
    float color[3];
};

struct UniformBufferObject {
    float view[16];
    float proj[16];
};
