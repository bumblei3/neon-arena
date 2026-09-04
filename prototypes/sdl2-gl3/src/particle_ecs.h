#pragma once
// particle_ecs.h - Entity-Component-System for particles
// Struct-of-arrays layout, pool allocator, zero per-frame allocation
#include <vector>
#include <cstdint>
#include <cstring>

// Particle data layout: SoA (struct of arrays) for cache efficiency
struct ParticlePool {
    static constexpr int MAX_PARTICLES = 8192;

    // Position
    float posX[MAX_PARTICLES];
    float posY[MAX_PARTICLES];
    float posZ[MAX_PARTICLES];

    // Velocity
    float velX[MAX_PARTICLES];
    float velY[MAX_PARTICLES];
    float velZ[MAX_PARTICLES];

    // Life
    float life[MAX_PARTICLES];
    float maxLife[MAX_PARTICLES];

    // Color (premultiplied alpha)
    float colorR[MAX_PARTICLES];
    float colorG[MAX_PARTICLES];
    float colorB[MAX_PARTICLES];
    float colorA[MAX_PARTICLES];

    // Size
    float size[MAX_PARTICLES];
    float sizeEnd[MAX_PARTICLES];

    // Type
    uint8_t type[MAX_PARTICLES];
    uint8_t alive[MAX_PARTICLES]; // 1 = active, 0 = dead

    int count = 0;        // Active particles
    int capacity = MAX_PARTICLES;

    ParticlePool() { clear(); }

    void clear() {
        count = 0;
        std::memset(alive, 0, sizeof(alive));
    }

    // Spawn a particle, returns index or -1 if full
    int spawn() {
        // Find dead slot (linear scan — predictable cache behavior)
        for (int i = 0; i < capacity; i++) {
            if (!alive[i]) {
                alive[i] = 1;
                if (i >= count) count = i + 1;
                life[i] = 1.0f;
                return i;
            }
        }
        return -1; // Pool full
    }

    void kill(int index) {
        if (index >= 0 && index < capacity) {
            alive[index] = 0;
            // Update count if we killed the last active particle
            while (count > 0 && !alive[count - 1]) count--;
        }
    }
};

// Particle types
enum class ParticleType : uint8_t {
    SPARK = 0,
    SMOKE,
    BLOOD,
    MUZZLE_FLASH,
    EXPLOSION_SHELL,
    TRAIL,
    GLOW,
    COUNT
};

// Spawn configuration for a burst
struct ParticleBurst {
    float posX, posY, posZ;
    float dirX, dirY, dirZ;
    float speed;
    float spread;
    float life;
    float size;
    float sizeEnd;
    float r, g, b, a;
    int count;
    ParticleType type;
};

class ParticleSystem {
public:
    ParticleSystem();

    // Initialize particle pool
    void init(int maxParticles = ParticlePool::MAX_PARTICLES);

    // Update all particles (single pass, no allocation)
    void update(float dt);

    // Spawn a burst of particles
    void spawnBurst(const ParticleBurst& burst);

    // Spawn a single particle
    void spawnParticle(float x, float y, float z,
                       float vx, float vy, float vz,
                       float life, float size, float sizeEnd,
                       float r, float g, float b, float a,
                       ParticleType type);

    // Clear all particles
    void clear();

    // Get active particle data for rendering
    const ParticlePool& getPool() const { return pool; }
    int getActiveCount() const { return pool.count; }

    // Pre-allocated render buffer (positions + colors + sizes for instancing)
    // Format: [x,y,z, r,g,b, a, size] per particle = 8 floats
    const float* getRenderBuffer(int& outCount);
    int getRenderStride() const { return 8; } // floats per particle

private:
    ParticlePool pool;
    std::vector<float> renderBuffer_; // Pre-allocated render data

    // Apply per-type update behavior
    void updateParticle(int i, float dt);

    // Build render buffer from active particles
    void buildRenderBuffer();
    bool renderDirty = true;
    mutable bool renderDirty_ = true; // For const-correctness
};
