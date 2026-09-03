// NEON ARENA - Vulkan + SDL2 Prototype
// Game state: arena, enemies, waves, player, audio
#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include <cstring>

class AudioSystem; // Forward declaration

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(float s) const { return {x*s, y*s, z*s}; }
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    Vec3 normalized() const { float l = length(); return l > 0 ? Vec3(x/l, y/l, z/l) : Vec3(); }
};

struct Vertex {
    Vec3 pos;
    Vec3 color;
};

struct UniformBufferObject {
    float view[16];
    float proj[16];
};

struct Enemy {
    Vec3 pos;
    float hp;
    float phase;
    bool alive;
    float hitFlash;
};

struct Tracer {
    Vec3 from, to;
    float life;
};

struct Spark {
    Vec3 pos, vel;
    float life;
};

class Game {
public:
    static const float ARENA;
    static const int MAX_ENEMIES = 12;

    // Player state
    float px = 0, pz = 0, pyaw = 0, ppitch = 0;
    float pvel_x = 0, pvel_z = 0;
    int hp = 100;
    int score = 0;
    uint32_t lastShot = 0;
    float recoil = 0;

    // Enemies / fx
    std::vector<Enemy> enemies;
    std::vector<Tracer> tracers;
    std::vector<Spark> sparks;
    uint32_t lastSpawn = 0;
    int wave = 1;
    double now_s = 0;

    // Input state
    bool keyW = false, keyA = false, keyS = false, keyD = false;
    bool keySpace = false;

    void setAudio(AudioSystem* a) { m_audio = a; }

    void update(float dt);
    void shoot();
    void spawnEnemy();
    void burst(Vec3 p, int n);

    // Get view matrix from player camera
    void getViewMatrix(float* out) const;
    void getProjMatrix(float* out, float aspect) const;

    // Generate arena geometry (floor grid + walls)
    std::vector<Vertex> getArenaGeometry() const;

    // Generate enemy geometry (cubes at enemy positions)
    std::vector<Vertex> getEnemyGeometry() const;

    // Generate tracer geometry (lines)
    std::vector<Vertex> getTracerGeometry() const;

    // Generate spark geometry (points as small quads)
    std::vector<Vertex> getSparkGeometry() const;

private:
    AudioSystem* m_audio = nullptr;

    static void mat4_identity(float* m);
    static void mat4_lookAt(float* m, const Vec3& eye, const Vec3& center, const Vec3& up);
    static void mat4_perspective(float* m, float fovy, float aspect, float zn, float zf);
    static void mat4_mul(float* out, const float* a, const float* b);
};
