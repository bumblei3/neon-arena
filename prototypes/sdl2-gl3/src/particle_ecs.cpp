#include "particle_ecs.h"
#include <cmath>
#include <algorithm>

ParticleSystem::ParticleSystem() {
    renderBuffer_.resize(ParticlePool::MAX_PARTICLES * 8);
}

void ParticleSystem::init(int maxParticles) {
    pool.capacity = std::min(maxParticles, ParticlePool::MAX_PARTICLES);
    pool.clear();
}

void ParticleSystem::update(float dt) {
    for (int i = 0; i < pool.count; i++) {
        if (!pool.alive[i]) continue;

        pool.life[i] -= dt;
        if (pool.life[i] <= 0.0f) {
            pool.kill(i);
            continue;
        }

        updateParticle(i, dt);
    }
    renderDirty = true;
}

void ParticleSystem::updateParticle(int i, float dt) {
    // Apply gravity based on type
    switch (static_cast<ParticleType>(pool.type[i])) {
        case ParticleType::SPARK:
            pool.velY[i] -= 15.0f * dt; // Strong gravity
            break;
        case ParticleType::SMOKE:
            pool.velY[i] += 2.0f * dt; // Rises
            pool.velX[i] *= 0.98f;     // Air resistance
            pool.velZ[i] *= 0.98f;
            break;
        case ParticleType::BLOOD:
            pool.velY[i] -= 20.0f * dt;
            break;
        case ParticleType::MUZZLE_FLASH:
            // No gravity, just fades
            break;
        case ParticleType::EXPLOSION_SHELL:
            pool.velY[i] -= 8.0f * dt;
            break;
        case ParticleType::TRAIL:
            pool.velX[i] *= 0.95f;
            pool.velY[i] *= 0.95f;
            pool.velZ[i] *= 0.95f;
            break;
        case ParticleType::GLOW:
            pool.velY[i] += 0.5f * dt;
            break;
        default:
            break;
    }

    // Integrate position
    pool.posX[i] += pool.velX[i] * dt;
    pool.posY[i] += pool.velY[i] * dt;
    pool.posZ[i] += pool.velZ[i] * dt;

    // Fade alpha based on life ratio
    float lifeRatio = pool.life[i] / pool.maxLife[i];
    if (lifeRatio < 0.3f) {
        pool.colorA[i] = lifeRatio / 0.3f;
    }
}

void ParticleSystem::spawnParticle(float x, float y, float z,
                                    float vx, float vy, float vz,
                                    float life, float size, float sizeEnd,
                                    float r, float g, float b, float a,
                                    ParticleType type) {
    int idx = pool.spawn();
    if (idx < 0) return; // Pool full

    pool.posX[idx] = x;
    pool.posY[idx] = y;
    pool.posZ[idx] = z;
    pool.velX[idx] = vx;
    pool.velY[idx] = vy;
    pool.velZ[idx] = vz;
    pool.life[idx] = life;
    pool.maxLife[idx] = life;
    pool.size[idx] = size;
    pool.sizeEnd[idx] = sizeEnd;
    pool.colorR[idx] = r;
    pool.colorG[idx] = g;
    pool.colorB[idx] = b;
    pool.colorA[idx] = a;
    pool.type[idx] = static_cast<uint8_t>(type);
    renderDirty = true;
}

void ParticleSystem::spawnBurst(const ParticleBurst& burst) {
    for (int i = 0; i < burst.count; i++) {
        // Random spread
        float rx = (rand() % 100 / 100.0f - 0.5f) * burst.spread;
        float ry = (rand() % 100 / 100.0f - 0.5f) * burst.spread;
        float rz = (rand() % 100 / 100.0f - 0.5f) * burst.speed;

        float vx = (burst.dirX + rx) * burst.speed;
        float vy = (burst.dirY + ry) * burst.speed;
        float vz = (burst.dirZ + rz) * burst.speed;

        spawnParticle(burst.posX, burst.posY, burst.posZ,
                      vx, vy, vz,
                      burst.life * (0.8f + rand() % 40 / 100.0f), // Life jitter
                      burst.size, burst.sizeEnd,
                      burst.r, burst.g, burst.b, burst.a,
                      burst.type);
    }
}

void ParticleSystem::clear() {
    pool.clear();
    renderDirty = true;
}

const float* ParticleSystem::getRenderBuffer(int& outCount) {
    if (renderDirty) {
        buildRenderBuffer();
        renderDirty = false;
    }
    outCount = pool.count;
    return renderBuffer_.data();
}

void ParticleSystem::buildRenderBuffer() {
    int writeIdx = 0;
    for (int i = 0; i < pool.count; i++) {
        if (!pool.alive[i]) continue;

        int off = writeIdx * 8;
        renderBuffer_[off + 0] = pool.posX[i];
        renderBuffer_[off + 1] = pool.posY[i];
        renderBuffer_[off + 2] = pool.posZ[i];
        renderBuffer_[off + 3] = pool.colorR[i];
        renderBuffer_[off + 4] = pool.colorG[i];
        renderBuffer_[off + 5] = pool.colorB[i];
        renderBuffer_[off + 6] = pool.colorA[i];

        // Interpolate size based on life ratio
        float lifeRatio = pool.life[i] / pool.maxLife[i];
        renderBuffer_[off + 7] = pool.size[i] + (pool.sizeEnd[i] - pool.size[i]) * (1.0f - lifeRatio);

        writeIdx++;
    }
    pool.count = writeIdx;
}
