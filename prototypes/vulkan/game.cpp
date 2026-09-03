// NEON ARENA - Vulkan + SDL2 Prototype
// Game logic: waves, enemies, shooting, fx
#include "game.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

const float Game::ARENA = 24.0f;

void Game::mat4_identity(float* m) {
    std::memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void Game::mat4_lookAt(float* m, const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 f = (center - eye).normalized();
    Vec3 s = Vec3(f.y * up.z - f.z * up.y, f.z * up.x - f.x * up.z, f.x * up.y - f.y * up.x).normalized();
    Vec3 u = Vec3(s.y * f.z - s.z * f.y, s.z * f.x - s.x * f.z, s.x * f.y - s.y * f.x);

    mat4_identity(m);
    m[0] = s.x; m[4] = s.y; m[8] = s.z;
    m[1] = u.x; m[5] = u.y; m[9] = u.z;
    m[2] = -f.x; m[6] = -f.y; m[10] = -f.z;
    m[12] = -(s.x * eye.x + s.y * eye.y + s.z * eye.z);
    m[13] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
    m[14] = f.x * eye.x + f.y * eye.y + f.z * eye.z;
}

void Game::mat4_perspective(float* m, float fovy, float aspect, float zn, float zf) {
    std::memset(m, 0, 16 * sizeof(float));
    float t = std::tan(fovy * 0.5f);
    m[0] = 1.0f / (aspect * t);
    m[5] = 1.0f / t;
    m[10] = -(zf + zn) / (zf - zn);
    m[11] = -1.0f;
    m[14] = -(2.0f * zf * zn) / (zf - zn);
}

void Game::mat4_mul(float* out, const float* a, const float* b) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            out[i * 4 + j] = 0;
            for (int k = 0; k < 4; k++) {
                out[i * 4 + j] += a[i * 4 + k] * b[k * 4 + j];
            }
        }
    }
}

void Game::getViewMatrix(float* out) const {
    // Camera at player position, looking forward
    float cy = 1.6f;
    
    // Camera position (eye)
    Vec3 eye(px, cy, pz);
    
    // Look direction from yaw and pitch
    // yaw=0 looks down -Z, yaw=90 looks down +X
    float yawRad = pyaw;
    float pitchRad = ppitch;
    
    Vec3 forward(
        std::sin(yawRad) * std::cos(pitchRad),
        -std::sin(pitchRad),
        -std::cos(yawRad) * std::cos(pitchRad)
    );
    
    Vec3 center = eye + forward;
    Vec3 up(0, 1, 0);
    
    mat4_lookAt(out, eye, center, up);
}

void Game::getProjMatrix(float* out, float aspect) const {
    mat4_perspective(out, 75.0f * 3.14159f / 180.0f, aspect, 0.1f, 200.0f);
}

void Game::update(float dt) {
    // Movement
    float fwd = (keyW ? 1.0f : 0.0f) - (keyS ? 1.0f : 0.0f);
    float str = (keyD ? 1.0f : 0.0f) - (keyA ? 1.0f : 0.0f);
    float sp = 9.0f;
    float sx = std::sin(pyaw), cz = std::cos(pyaw);
    float ax = (-sx * fwd + cz * str) * sp;
    float az = (cz * fwd + sx * str) * sp;
    pvel_x += (ax - pvel_x) * std::fmin(1.0f, dt * 12.0f);
    pvel_z += (az - pvel_z) * std::fmin(1.0f, dt * 12.0f);
    px += pvel_x * dt;
    pz += pvel_z * dt;
    float lim = ARENA - 1.0f;
    px = px < -lim ? -lim : px > lim ? lim : px;
    pz = pz < -lim ? -lim : pz > lim ? lim : pz;

    if (keySpace) shoot();
    recoil *= std::pow(0.001f, dt);

    // Spawn waves
    uint32_t tms = (uint32_t)(now_s * 1000);
    int target = 3 + wave / 2;
    if (target > MAX_ENEMIES) target = MAX_ENEMIES;
    int alive = 0;
    for (auto& e : enemies) if (e.alive) alive++;
    if (alive == 0 && enemies.empty()) wave++;
    if ((tms - lastSpawn > 1500 && alive < target)) {
        spawnEnemy();
        lastSpawn = tms;
    }

    // Enemies chase player
    float speed = 2.2f + wave * 0.15f;
    for (auto& e : enemies) {
        if (!e.alive) continue;
        e.phase += dt * 4;
        e.hitFlash *= std::pow(0.01f, dt);
        float dx = px - e.pos.x, dz = pz - e.pos.z;
        float l = std::sqrt(dx * dx + dz * dz);
        if (l > 0.01f) { dx /= l; dz /= l; }
        e.pos.x += dx * speed * dt;
        e.pos.z += dz * speed * dt;
        e.pos.y = 0.9f + std::sin(e.phase) * 0.15f;
        if (l < 1.4f) {
            static uint32_t lastHurt = 0;
            if (tms - lastHurt > 600) {
                hp -= 10;
                lastHurt = tms;
                pvel_x -= dx * 8;
                pvel_z -= dz * 8;
            }
        }
    }

    // FX decay
    for (auto& t : tracers) t.life -= dt;
    for (auto& s : sparks) {
        s.life -= dt;
        s.vel.y -= 9 * dt;
        s.pos.x += s.vel.x * dt;
        s.pos.y += s.vel.y * dt;
        s.pos.z += s.vel.z * dt;
        if (s.pos.y < 0.05f) { s.pos.y = 0.05f; s.vel.y *= -0.4f; }
    }
    tracers.erase(std::remove_if(tracers.begin(), tracers.end(), [](const Tracer& t) { return t.life <= 0; }), tracers.end());
    sparks.erase(std::remove_if(sparks.begin(), sparks.end(), [](const Spark& s) { return s.life <= 0; }), sparks.end());

    if (hp <= 0) {
        printf("GAME OVER - Score: %d, Wave: %d\n", score, wave);
        hp = 100; score = 0; wave = 1;
        enemies.clear(); px = pz = 0;
    }
}

void Game::shoot() {
    uint32_t t = (uint32_t)(now_s * 1000);
    if (t - lastShot < 180) return;
    lastShot = t;
    recoil = 1.0f;

    Vec3 o(px, 1.5f, pz);
    Vec3 d(std::sin(pyaw) * std::cos(ppitch), -std::sin(ppitch), -std::cos(pyaw) * std::cos(ppitch));

    // Nearest hit enemy
    Enemy* best = nullptr;
    float bestT = 1e9f;
    for (auto& e : enemies) {
        if (!e.alive) continue;
        // Ray vs AABB
        float tmin = 0, tmax = 1e9f;
        float lo[3] = { e.pos.x - 0.8f, e.pos.y - 0.8f, e.pos.z - 0.8f };
        float hi[3] = { e.pos.x + 0.8f, e.pos.y + 0.8f, e.pos.z + 0.8f };
        float oo[3] = { o.x, o.y, o.z };
        float dd[3] = { d.x, d.y, d.z };
        bool hit = true;
        for (int i = 0; i < 3; i++) {
            if (std::fabs(dd[i]) < 1e-6f) {
                if (oo[i] < lo[i] || oo[i] > hi[i]) { hit = false; break; }
                continue;
            }
            float t1 = (lo[i] - oo[i]) / dd[i], t2 = (hi[i] - oo[i]) / dd[i];
            if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
            tmin = tmin > t1 ? tmin : t1;
            tmax = tmax < t2 ? tmax : t2;
            if (tmin > tmax) { hit = false; break; }
        }
        if (hit && tmin >= 0 && tmin < bestT) { bestT = tmin; best = &e; }
    }

    Vec3 end = d * bestT;
    Vec3 hit = o + end;
    if (bestT > 60) hit = o + d * 60;
    tracers.push_back({ o, hit, 0.12f });

    if (best) {
        best->hp--;
        best->hitFlash = 1;
        burst(hit, 10);
        if (best->hp <= 0) { best->alive = false; score += 10; burst(best->pos, 25); }
    } else if (bestT <= ARENA * 2) {
        burst(hit, 4);
    }
}

void Game::spawnEnemy() {
    float a = (rand() % 3600) * 0.017453f * 0.1f;
    float r = ARENA * 0.9f;
    Enemy e;
    e.pos = { std::cos(a) * r, 0.9f, std::sin(a) * r };
    e.hp = 3;
    e.phase = rand() % 628 * 0.01f;
    e.alive = true;
    e.hitFlash = 0;
    enemies.push_back(e);
}

void Game::burst(Vec3 p, int n) {
    for (int i = 0; i < n; i++) {
        Spark s;
        s.pos = p;
        s.vel = { (rand() % 2000 - 1000) * 0.004f, (rand() % 1500 + 200) * 0.004f, (rand() % 2000 - 1000) * 0.004f };
        s.life = 0.5f + (rand() % 30) * 0.01f;
        sparks.push_back(s);
    }
}

std::vector<Vertex> Game::getArenaGeometry() const {
    std::vector<Vertex> verts;
    // Floor grid lines
    for (float i = -ARENA; i <= ARENA; i += 2.0f) {
        float fade = 1.0f - std::fabs(i) / ARENA;
        Vec3 c1(0.0f, 0.7f * fade + 0.1f, 0.9f * fade + 0.15f);
        Vec3 c2(0.0f, 0.7f * fade + 0.1f, 0.9f * fade + 0.15f);
        // Line along X
        verts.push_back({ {i, 0, -ARENA}, c1 });
        verts.push_back({ {i, 0, ARENA}, c2 });
        // Line along Z
        verts.push_back({ {-ARENA, 0, i}, c1 });
        verts.push_back({ {ARENA, 0, i}, c2 });
    }
    // Arena walls (boundary)
    Vec3 wallColor(1.0f, 0.2f, 0.6f);
    verts.push_back({ {-ARENA, 0, -ARENA}, wallColor });
    verts.push_back({ {ARENA, 0, -ARENA}, wallColor });
    verts.push_back({ {ARENA, 0, -ARENA}, wallColor });
    verts.push_back({ {ARENA, 0, ARENA}, wallColor });
    verts.push_back({ {ARENA, 0, ARENA}, wallColor });
    verts.push_back({ {-ARENA, 0, ARENA}, wallColor });
    verts.push_back({ {-ARENA, 0, ARENA}, wallColor });
    verts.push_back({ {-ARENA, 0, -ARENA}, wallColor });
    return verts;
}

std::vector<Vertex> Game::getEnemyGeometry() const {
    std::vector<Vertex> verts;
    for (auto& e : enemies) {
        if (!e.alive) continue;
        float fl = e.hitFlash;
        Vec3 c(0.25f + fl * 0.75f, 0.02f + fl * 0.6f, 0.35f + fl * 0.6f);
        
        // Simple quad for each enemy (billboard-style)
        float s = 0.8f;
        Vec3 pos = e.pos;
        
        // Two triangles for a quad
        verts.push_back({{pos.x - s, pos.y - s, pos.z}, c});
        verts.push_back({{pos.x + s, pos.y - s, pos.z}, c});
        verts.push_back({{pos.x + s, pos.y + s, pos.z}, c});
        
        verts.push_back({{pos.x - s, pos.y - s, pos.z}, c});
        verts.push_back({{pos.x + s, pos.y + s, pos.z}, c});
        verts.push_back({{pos.x - s, pos.y + s, pos.z}, c});
    }
    return verts;
}

std::vector<Vertex> Game::getTracerGeometry() const {
    std::vector<Vertex> verts;
    for (auto& t : tracers) {
        Vec3 c(0.2f, 1.0f, 1.0f);
        verts.push_back({ t.from, c });
        verts.push_back({ t.to, c });
    }
    return verts;
}

std::vector<Vertex> Game::getSparkGeometry() const {
    std::vector<Vertex> verts;
    for (auto& s : sparks) {
        Vec3 c(1.0f, 0.8f, 0.2f);
        float size = 0.05f;
        // Small quad for each spark
        verts.push_back({ {s.pos.x - size, s.pos.y - size, s.pos.z}, c });
        verts.push_back({ {s.pos.x + size, s.pos.y - size, s.pos.z}, c });
        verts.push_back({ {s.pos.x + size, s.pos.y + size, s.pos.z}, c });
        verts.push_back({ {s.pos.x - size, s.pos.y - size, s.pos.z}, c });
        verts.push_back({ {s.pos.x + size, s.pos.y + size, s.pos.z}, c });
        verts.push_back({ {s.pos.x - size, s.pos.y + size, s.pos.z}, c });
    }
    return verts;
}
