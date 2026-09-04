// Tests for Particle ECS
#include <cstdio>
#include <cassert>
#include <cmath>
#include "../src/particle_ecs.h"

static int partPassed = 0, partFailed = 0;

#define PART_TEST(name, expr) do { \
    printf("  Running %s... ", name); \
    fflush(stdout); \
    if (expr) { printf("PASSED\n"); partPassed++; } \
    else { printf("FAILED\n"); partFailed++; } \
} while(0)

void testParticleECS() {
    printf("\n[Particle ECS Tests]\n");

    // Construction
    {
        ParticleSystem ps;
        ps.init();
        PART_TEST("constructs", true);
        PART_TEST("starts_empty", ps.getActiveCount() == 0);
    }

    // Spawn single particle
    {
        ParticleSystem ps;
        ps.init();
        ps.spawnParticle(0, 0, 0, 1, 0, 0, 1.0f, 0.1f, 0.05f, 1, 1, 1, 1, ParticleType::SPARK);
        PART_TEST("spawn_creates_particle", ps.getActiveCount() == 1);
    }

    // Spawn burst
    {
        ParticleSystem ps;
        ps.init();
        ParticleBurst burst;
        burst.posX = 0; burst.posY = 0; burst.posZ = 0;
        burst.dirX = 0; burst.dirY = 1; burst.dirZ = 0;
        burst.speed = 10.0f;
        burst.spread = 1.0f;
        burst.life = 1.0f;
        burst.size = 0.1f;
        burst.sizeEnd = 0.05f;
        burst.r = 1; burst.g = 0.5f; burst.b = 0; burst.a = 1;
        burst.count = 50;
        burst.type = ParticleType::SPARK;
        ps.spawnBurst(burst);
        PART_TEST("burst_spawns_50", ps.getActiveCount() == 50);
    }

    // Update decays life
    {
        ParticleSystem ps;
        ps.init();
        ps.spawnParticle(0, 0, 0, 0, 0, 0, 1.0f, 0.1f, 0.05f, 1, 1, 1, 1, ParticleType::SPARK);
        ps.update(0.5f);
        PART_TEST("update_keeps_alive", ps.getActiveCount() == 1);
    }

    // Particle dies after life expires
    {
        ParticleSystem ps;
        ps.init();
        ps.spawnParticle(0, 0, 0, 0, 0, 0, 1.0f, 0.1f, 0.05f, 1, 1, 1, 1, ParticleType::SPARK);
        ps.update(1.5f);
        PART_TEST("particle_dies", ps.getActiveCount() == 0);
    }

    // Pool full returns -1
    {
        ParticleSystem ps;
        ps.init(10);
        for (int i = 0; i < 10; i++) {
            ps.spawnParticle(0, 0, 0, 0, 0, 0, 1.0f, 0.1f, 0.05f, 1, 1, 1, 1, ParticleType::SPARK);
        }
        // Pool full — next spawn should fail silently
        ps.spawnParticle(0, 0, 0, 0, 0, 0, 1.0f, 0.1f, 0.05f, 1, 1, 1, 1, ParticleType::SPARK);
        PART_TEST("pool_full_no_crash", ps.getActiveCount() == 10);
    }

    // Clear removes all
    {
        ParticleSystem ps;
        ps.init();
        for (int i = 0; i < 100; i++) {
            ps.spawnParticle(0, 0, 0, 0, 0, 0, 1.0f, 0.1f, 0.05f, 1, 1, 1, 1, ParticleType::SPARK);
        }
        ps.clear();
        PART_TEST("clear_removes_all", ps.getActiveCount() == 0);
    }

    // Render buffer
    {
        ParticleSystem ps;
        ps.init();
        ps.spawnParticle(1, 2, 3, 0, 0, 0, 1.0f, 0.1f, 0.05f, 1, 0.5f, 0, 1, ParticleType::SPARK);
        int count;
        const float* buf = ps.getRenderBuffer(count);
        PART_TEST("render_buffer_count", count == 1);
        PART_TEST("render_buffer_pos_x", std::abs(buf[0] - 1.0f) < 0.001f);
        PART_TEST("render_buffer_pos_y", std::abs(buf[1] - 2.0f) < 0.001f);
        PART_TEST("render_buffer_pos_z", std::abs(buf[2] - 3.0f) < 0.001f);
        PART_TEST("render_buffer_color_r", std::abs(buf[3] - 1.0f) < 0.001f);
        PART_TEST("render_buffer_color_g", std::abs(buf[4] - 0.5f) < 0.001f);
    }

    // Particle types
    {
        ParticleSystem ps;
        ps.init();
        ps.spawnParticle(0, 0, 0, 0, 0, 0, 1.0f, 0.1f, 0.05f, 1, 1, 1, 1, ParticleType::SPARK);
        ps.spawnParticle(0, 0, 0, 0, 0, 0, 1.0f, 0.1f, 0.05f, 1, 1, 1, 1, ParticleType::SMOKE);
        ps.spawnParticle(0, 0, 0, 0, 0, 0, 1.0f, 0.1f, 0.05f, 1, 1, 1, 1, ParticleType::BLOOD);
        ps.spawnParticle(0, 0, 0, 0, 0, 0, 1.0f, 0.1f, 0.05f, 1, 1, 1, 1, ParticleType::MUZZLE_FLASH);
        ps.spawnParticle(0, 0, 0, 0, 0, 0, 1.0f, 0.1f, 0.05f, 1, 1, 1, 1, ParticleType::EXPLOSION_SHELL);
        ps.spawnParticle(0, 0, 0, 0, 0, 0, 1.0f, 0.1f, 0.05f, 1, 1, 1, 1, ParticleType::TRAIL);
        ps.spawnParticle(0, 0, 0, 0, 0, 0, 1.0f, 0.1f, 0.05f, 1, 1, 1, 1, ParticleType::GLOW);
        PART_TEST("all_particle_types", ps.getActiveCount() == 7);
    }

    // Smoke rises
    {
        ParticleSystem ps;
        ps.init();
        ps.spawnParticle(0, 0, 0, 0, 0, 0, 2.0f, 0.1f, 0.05f, 1, 1, 1, 1, ParticleType::SMOKE);
        ps.update(0.1f);
        int count;
        const float* buf = ps.getRenderBuffer(count);
        PART_TEST("smoke_rises", buf[1] > 0.0f);
    }

    // Spark falls
    {
        ParticleSystem ps;
        ps.init();
        ps.spawnParticle(0, 10, 0, 0, 0, 0, 2.0f, 0.1f, 0.05f, 1, 1, 1, 1, ParticleType::SPARK);
        ps.update(0.1f);
        int count;
        const float* buf = ps.getRenderBuffer(count);
        PART_TEST("spark_falls", buf[1] < 10.0f);
    }

    // Performance: 1000 particles update
    {
        ParticleSystem ps;
        ps.init();
        for (int i = 0; i < 1000; i++) {
            ps.spawnParticle(i * 0.01f, 0, 0, 0, 1, 0, 1.0f, 0.1f, 0.05f, 1, 1, 1, 1, ParticleType::SPARK);
        }
        ps.update(0.016f); // ~60fps frame
        PART_TEST("perf_1000_particles", ps.getActiveCount() == 1000);
    }

    printf("\n[Particle ECS Results] Passed: %d, Failed: %d\n", partPassed, partFailed);
}

int main() {
    testParticleECS();
    return partFailed > 0 ? 1 : 0;
}
