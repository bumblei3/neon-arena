// NEON ARENA - Vulkan + SDL2 Prototype
// Game logic: waves, enemies, shooting, fx, upgrades, specials, minimap
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
    float cy = 1.6f;
    Vec3 eye(px, cy, pz);
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
    float sp = 9.0f + (speedLevel - 1) * 1.5f;
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

    // Shooting
    if (keySpace) {
        if (currentWeapon == WeaponType::RAILGUN) {
            shootRailgun();
        } else if (currentWeapon == WeaponType::LIGHTNING) {
            shootLightning();
        } else {
            shootPlasma();
        }
    }
    recoil *= std::pow(0.001f, dt);

    // Weapon cooldowns
    if (railgunCooldown > 0.0f) railgunCooldown -= dt;
    if (lightningCooldown > 0.0f) lightningCooldown -= dt;
    if (plasmaCooldown > 0.0f) plasmaCooldown -= dt;

    // Score multiplier decay
    if (scoreMultiplier > 1) {
        multiplierTimer -= dt;
        if (multiplierTimer <= 0.0f) {
            scoreMultiplier = 1;
            comboCount = 0;
        }
    }

    // Damage boost decay
    if (damageBoostTimer > 0.0f) damageBoostTimer -= dt;

    // Spawn waves
    uint32_t tms = (uint32_t)(now_s * 1000);
    int target = 3 + wave / 2;
    if (target > MAX_ENEMIES) target = MAX_ENEMIES;
    int alive = 0;
    for (auto& e : enemies) if (e.alive) alive++;
    if (alive == 0 && enemies.empty()) wave++;
    if ((tms - lastSpawn > 1500 && alive < target) && !showUpgradeMenu) {
        spawnEnemy();
        lastSpawn = tms;
    }

    // Enemies chase player
    float speed = 2.2f + wave * 0.15f;
    for (auto& e : enemies) {
        if (!e.alive) continue;
        e.phase += dt * 4;
        e.hitFlash *= std::pow(0.01f, dt);
        e.attackCooldown -= dt;
        float dx = px - e.pos.x, dz = pz - e.pos.z;
        float l = std::sqrt(dx * dx + dz * dz);
        if (l > 0.01f) { dx /= l; dz /= l; }

        float preferredDist = 3.0f;
        if (e.type == EnemyType::SHOOTER) preferredDist = 12.0f;
        if (e.type == EnemyType::FAST) preferredDist = 2.0f;
        if (e.type == EnemyType::BOSS) preferredDist = 5.0f;

        if (l > preferredDist) {
            e.pos.x += dx * e.moveSpeed * dt;
            e.pos.z += dz * e.moveSpeed * dt;
        } else if (e.type == EnemyType::SHOOTER && l < preferredDist - 2.0f) {
            e.pos.x -= dx * e.moveSpeed * 0.5f * dt;
            e.pos.z -= dz * e.moveSpeed * 0.5f * dt;
        }

        e.pos.y = 0.9f + std::sin(e.phase) * 0.15f;
        if (e.type == EnemyType::BOSS) e.pos.y *= 2.0f;

        // Attacks
        if (l < 4.0f && e.attackCooldown <= 0.0f) {
            if (e.type == EnemyType::MELEE || e.type == EnemyType::TANK || e.type == EnemyType::FAST) {
                hp -= 10;
                e.attackCooldown = 1.0f;
                pvel_x -= dx * 8;
                pvel_z -= dz * 8;
            }
        }
        if (e.type == EnemyType::SHOOTER && l < 20.0f && e.attackCooldown <= 0.0f) {
            Tracer t;
            t.from = e.pos;
            t.to = Vec3(px, 1.5f, pz);
            t.life = 0.15f;
            tracers.push_back(t);
            hp -= 5;
            e.attackCooldown = 2.0f;
        }
        if (e.type == EnemyType::BOSS && l < 25.0f && e.attackCooldown <= 0.0f) {
            for (int i = 0; i < 5; i++) {
                float angle = (i - 2) * 0.3f;
                Tracer t;
                t.from = e.pos;
                t.to = Vec3(px + std::sin(angle) * 5, 1.5f, pz + std::cos(angle) * 5);
                t.life = 0.2f;
                tracers.push_back(t);
            }
            hp -= 15;
            e.attackCooldown = 3.0f;
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
    for (auto& la : lightningArcs) la.life -= dt;
    for (auto& pu : powerUps) {
        pu.life -= dt;
        pu.rotation += dt * 2.0f;
    }
    tracers.erase(std::remove_if(tracers.begin(), tracers.end(), [](const Tracer& t) { return t.life <= 0; }), tracers.end());
    sparks.erase(std::remove_if(sparks.begin(), sparks.end(), [](const Spark& s) { return s.life <= 0; }), sparks.end());
    lightningArcs.erase(std::remove_if(lightningArcs.begin(), lightningArcs.end(), [](const LightningArc& a) { return a.life <= 0; }), lightningArcs.end());
    powerUps.erase(std::remove_if(powerUps.begin(), powerUps.end(), [](const PowerUp& p) { return p.life <= 0; }), powerUps.end());

    // Power-up collection
    for (int i = (int)powerUps.size() - 1; i >= 0; i--) {
        float dx = px - powerUps[i].pos.x;
        float dz = pz - powerUps[i].pos.z;
        if (std::sqrt(dx * dx + dz * dz) < 2.0f) {
            switch (powerUps[i].type) {
                case 0: hp = maxHp; break;
                case 1: addScore(500); break;
                case 2: damageBoostTimer = 10.0f; break;
            }
            powerUps.erase(powerUps.begin() + i);
        }
    }

    // Update specials
    updateSpecials(dt);
    updateKillFeed(dt);
    updateDamageNumbers(dt);

    if (hp <= 0) {
        printf("GAME OVER - Score: %d, Wave: %d\n", score, wave);
        saveHighScore();
        hp = maxHp; score = 0; wave = 1;
        enemies.clear(); px = pz = 0;
        upgradePoints = 0;
        railgunLevel = 1; lightningLevel = 1; healthLevel = 1; speedLevel = 1;
        showUpgradeMenu = false;
    }
}

void Game::shootRailgun() {
    uint32_t t = (uint32_t)(now_s * 1000);
    if (t - lastShot < 180) return;
    lastShot = t;
    recoil = 1.0f;

    Vec3 o(px, 1.5f, pz);
    Vec3 d(std::sin(pyaw) * std::cos(ppitch), -std::sin(ppitch), -std::cos(pyaw) * std::cos(ppitch));

    Enemy* best = nullptr;
    float bestT = 1e9f;
    for (auto& e : enemies) {
        if (!e.alive) continue;
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
        float damage = 50.0f + (railgunLevel - 1) * 15.0f;
        if (damageBoostTimer > 0.0f) damage *= 2.0f;
        best->hp -= damage;
        best->hitFlash = 1;
        burst(hit, 10);
        if (best->hp <= 0) {
            best->alive = false;
            addScore(10);
            if (best->type == EnemyType::BOSS) {
                addKillFeed("BOSS KILLED!", Vec3(1.0f, 0.8f, 0.0f));
            } else {
                addKillFeed("KILL", Vec3(0.0f, 1.0f, 0.5f));
            }
            addDamageNumber(best->pos, 50);
            burst(best->pos, 25);
            if (rand() % 100 < 30) {
                PowerUp pu;
                pu.pos = best->pos;
                pu.type = rand() % 3;
                pu.life = 15.0f;
                pu.rotation = 0;
                powerUps.push_back(pu);
            }
        }
    } else if (bestT <= ARENA * 2) {
        burst(hit, 4);
    }
}

void Game::shootLightning() {
    if (lightningCooldown > 0.0f) return;
    lightningCooldown = lightningFireRate;

    Vec3 forward(std::sin(pyaw) * std::cos(ppitch), -std::sin(ppitch), -std::cos(pyaw) * std::cos(ppitch));
    Vec3 muzzle(px + forward.x * 0.5f, 1.5f, pz + forward.z * 0.5f);

    std::vector<std::pair<float, Enemy*>> sorted;
    for (auto& e : enemies) {
        if (!e.alive) continue;
        float dx = px - e.pos.x, dz = pz - e.pos.z;
        float d = std::sqrt(dx * dx + dz * dz);
        if (d < lightningRange) {
            sorted.push_back({d, &e});
        }
    }
    std::sort(sorted.begin(), sorted.end());

    int count = 0;
    for (auto& pair : sorted) {
        if (count >= lightningChainCount) break;
        Enemy* e = pair.second;
        LightningArc la;
        la.start = muzzle;
        la.end = e->pos;
        la.life = 0.15f;
        la.segments = 10;
        lightningArcs.push_back(la);

        float damage = lightningDamage + (lightningLevel - 1) * 5.0f;
        if (damageBoostTimer > 0.0f) damage *= 2.0f;
        e->hp -= damage;
        if (e->hp <= 0 && e->alive) {
            e->alive = false;
            addScore(10);
            burst(e->pos, 25);
            if (rand() % 100 < 30) {
                PowerUp pu;
                pu.pos = e->pos;
                pu.type = rand() % 3;
                pu.life = 15.0f;
                pu.rotation = 0;
                powerUps.push_back(pu);
            }
        }
        count++;
    }

    if (count == 0) {
        LightningArc la;
        la.start = muzzle;
        la.end = Vec3(px + forward.x * lightningRange, 1.5f, pz + forward.z * lightningRange);
        la.life = 0.1f;
        la.segments = 6;
        lightningArcs.push_back(la);
    }
}

void Game::shootPlasma() {
    if (plasmaCooldown > 0.0f) return;
    plasmaCooldown = plasmaFireRate;

    Vec3 forward(std::sin(pyaw) * std::cos(ppitch), -std::sin(ppitch), -std::cos(pyaw) * std::cos(ppitch));
    Vec3 muzzle(px + forward.x * 0.5f, 1.5f, pz + forward.z * 0.5f);

    // Plasma ist ein langsamer Energieball mit Flächenschaden
    Tracer plasma;
    plasma.from = muzzle;
    plasma.to = Vec3(px + forward.x * 30, 1.5f, pz + forward.z * 30);
    plasma.life = 0.3f;
    tracers.push_back(plasma);

    // Flächenschaden
    for (auto& e : enemies) {
        if (!e.alive) continue;
        float dx = e.pos.x - px;
        float dz = e.pos.z - pz;
        float dist = std::sqrt(dx * dx + dz * dz);
        if (dist < plasmaRadius) {
            float damage = plasmaDamage * (1.0f - dist / plasmaRadius);
            if (damageBoostTimer > 0.0f) damage *= 2.0f;
            e.hp -= damage;
            if (e.hp <= 0 && e.alive) {
                e.alive = false;
                addScore(10);
                burst(e.pos, 25);
                if (rand() % 100 < 30) {
                    PowerUp pu;
                    pu.pos = e.pos;
                    pu.type = rand() % 3;
                    pu.life = 15.0f;
                    pu.rotation = 0;
                    powerUps.push_back(pu);
                }
            }
        }
    }
}

void Game::spawnEnemy() {
    float a = (rand() % 3600) * 0.017453f * 0.1f;
    float r = ARENA * 0.9f;
    Enemy e;
    e.pos = { std::cos(a) * r, 0.9f, std::sin(a) * r };
    e.phase = rand() % 628 * 0.01f;
    e.alive = true;
    e.hitFlash = 0;
    e.attackCooldown = 0;

    if (wave % 5 == 0 && enemies.empty()) {
        spawnBoss();
        return;
    }

    if (wave >= 3 && rand() % 4 == 0) {
        e.type = EnemyType::TANK;
        e.hp = 200.0f + wave * 20;
        e.maxHp = e.hp;
        e.moveSpeed = 1.5f;
    } else if (wave >= 2 && rand() % 4 == 1) {
        e.type = EnemyType::FAST;
        e.hp = 50.0f + wave * 5;
        e.maxHp = e.hp;
        e.moveSpeed = 6.0f;
    } else if (wave >= 4 && rand() % 4 == 2) {
        e.type = EnemyType::SHOOTER;
        e.hp = 80.0f + wave * 8;
        e.maxHp = e.hp;
        e.moveSpeed = 2.5f;
    } else {
        e.type = EnemyType::MELEE;
        e.hp = 100.0f + wave * 10;
        e.maxHp = e.hp;
        e.moveSpeed = 3.0f;
    }

    enemies.push_back(e);
}

void Game::spawnBoss() {
    Enemy e;
    e.pos = { ARENA * 0.7f, 1.8f, 0 };
    e.type = EnemyType::BOSS;
    e.hp = 500.0f + wave * 50;
    e.maxHp = e.hp;
    e.moveSpeed = 2.0f;
    e.phase = 0;
    e.alive = true;
    e.hitFlash = 0;
    e.attackCooldown = 0;
    enemies.push_back(e);
    printf("BOSS SPAWNED! HP: %.0f\n", e.hp);
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

void Game::activateNuclearBlast() {
    if (nuclearBlastCooldown > 0.0f) return;
    nuclearBlastCooldown = nuclearBlastMaxCooldown;

    for (auto& e : enemies) {
        if (e.alive) {
            e.alive = false;
            kills++;
            addScore(50);
            burst(e.pos, 30);
        }
    }
}

void Game::activateTimeSlow() {
    if (timeSlowCooldown > 0.0f) return;
    timeSlowCooldown = timeSlowMaxCooldown;
    timeSlowTimer = 3.0f;
}

void Game::activateShield() {
    if (shieldCooldown > 0.0f) return;
    shieldCooldown = shieldMaxCooldown;
    shieldTimer = 5.0f;
    hasShield = true;
}

void Game::updateSpecials(float dt) {
    if (nuclearBlastCooldown > 0.0f) nuclearBlastCooldown -= dt;
    if (timeSlowCooldown > 0.0f) timeSlowCooldown -= dt;
    if (shieldCooldown > 0.0f) shieldCooldown -= dt;

    if (timeSlowTimer > 0.0f) {
        timeSlowTimer -= dt;
        if (timeSlowTimer <= 0.0f) timeSlowTimer = 0.0f;
    }

    if (shieldTimer > 0.0f) {
        shieldTimer -= dt;
        if (shieldTimer <= 0.0f) {
            shieldTimer = 0.0f;
            hasShield = false;
        }
    }
}

void Game::applyUpgrade(int upgrade) {
    if (upgradePoints <= 0) return;
    switch (upgrade) {
        case 0: if (railgunLevel < maxUpgradeLevel) { railgunLevel++; upgradePoints--; } break;
        case 1: if (lightningLevel < maxUpgradeLevel) { lightningLevel++; lightningRange += 2.0f; upgradePoints--; } break;
        case 2: if (healthLevel < maxUpgradeLevel) { healthLevel++; maxHp += 25; hp = maxHp; upgradePoints--; } break;
        case 3: if (speedLevel < maxUpgradeLevel) { speedLevel++; upgradePoints--; } break;
    }
}

void Game::addScore(int points) {
    comboCount++;
    multiplierTimer = multiplierDecay;
    if (comboCount >= 10) scoreMultiplier = 5;
    else if (comboCount >= 5) scoreMultiplier = 3;
    else if (comboCount >= 3) scoreMultiplier = 2;
    score += points * scoreMultiplier;
    if (score > highScore) highScore = score;
}

void Game::addKillFeed(const std::string& text, Vec3 color) {
    KillFeedEntry entry;
    entry.text = text;
    entry.color = color;
    entry.life = 3.0f;
    killFeed.push_back(entry);
    if (killFeed.size() > 5) {
        killFeed.erase(killFeed.begin());
    }
}

void Game::updateKillFeed(float dt) {
    for (auto& entry : killFeed) {
        entry.life -= dt;
    }
    killFeed.erase(
        std::remove_if(killFeed.begin(), killFeed.end(),
            [](const KillFeedEntry& e) { return e.life <= 0.0f; }),
        killFeed.end());
}

void Game::addDamageNumber(Vec3 pos, int damage) {
    DamageNumber dn;
    dn.pos = pos;
    dn.damage = damage;
    dn.life = 1.0f;
    dn.vy = 2.0f;
    damageNumbers.push_back(dn);
}

void Game::updateDamageNumbers(float dt) {
    for (auto& dn : damageNumbers) {
        dn.life -= dt;
        dn.pos.y += dn.vy * dt;
        dn.vy -= 5.0f * dt;
    }
    damageNumbers.erase(
        std::remove_if(damageNumbers.begin(), damageNumbers.end(),
            [](const DamageNumber& d) { return d.life <= 0.0f; }),
        damageNumbers.end());
}

void Game::saveHighScore() {
    FILE* f = fopen("highscore.dat", "w");
    if (f) { fprintf(f, "%d", highScore); fclose(f); }
}

void Game::loadHighScore() {
    FILE* f = fopen("highscore.dat", "r");
    if (f) { fscanf(f, "%d", &highScore); fclose(f); }
}

std::vector<Vertex> Game::getArenaGeometry() const {
    std::vector<Vertex> verts;
    for (float i = -ARENA; i <= ARENA; i += 2.0f) {
        float fade = 1.0f - std::fabs(i) / ARENA;
        Vec3 c1(0.0f, 0.7f * fade + 0.1f, 0.9f * fade + 0.15f);
        Vec3 c2(0.0f, 0.7f * fade + 0.1f, 0.9f * fade + 0.15f);
        verts.push_back({ {i, 0, -ARENA}, c1 });
        verts.push_back({ {i, 0, ARENA}, c2 });
        verts.push_back({ {-ARENA, 0, i}, c1 });
        verts.push_back({ {ARENA, 0, i}, c2 });
    }
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
        Vec3 c;
        float s = 0.8f;
        switch (e.type) {
            case EnemyType::MELEE: c = Vec3(0.25f + fl * 0.75f, 0.6f + fl * 0.4f, 0.02f); break;
            case EnemyType::SHOOTER: c = Vec3(1.0f, 0.5f + fl * 0.5f, 0.0f); break;
            case EnemyType::TANK: c = Vec3(0.6f + fl * 0.4f, 0.0f, 0.8f); s = 1.0f; break;
            case EnemyType::FAST: c = Vec3(1.0f, 0.2f + fl * 0.3f, 0.0f); s = 0.6f; break;
            case EnemyType::BOSS: c = Vec3(1.0f, 0.8f + fl * 0.2f, 0.0f); s = 1.5f; break;
        }
        Vec3 pos = e.pos;
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
        verts.push_back({ {s.pos.x - size, s.pos.y - size, s.pos.z}, c });
        verts.push_back({ {s.pos.x + size, s.pos.y - size, s.pos.z}, c });
        verts.push_back({ {s.pos.x + size, s.pos.y + size, s.pos.z}, c });
        verts.push_back({ {s.pos.x - size, s.pos.y - size, s.pos.z}, c });
        verts.push_back({ {s.pos.x + size, s.pos.y + size, s.pos.z}, c });
        verts.push_back({ {s.pos.x - size, s.pos.y + size, s.pos.z}, c });
    }
    return verts;
}

std::vector<Vertex> Game::getLightningGeometry() const {
    std::vector<Vertex> verts;
    for (const auto& la : lightningArcs) {
        Vec3 color(0.5f, 0.7f, 1.0f);
        for (int i = 0; i < la.segments; i++) {
            float t1 = (float)i / la.segments;
            float t2 = (float)(i + 1) / la.segments;
            Vec3 p1 = la.start + (la.end - la.start) * t1;
            Vec3 p2 = la.start + (la.end - la.start) * t2;
            p1.x += (rand() % 100 - 50) * 0.01f;
            p1.z += (rand() % 100 - 50) * 0.01f;
            p2.x += (rand() % 100 - 50) * 0.01f;
            p2.z += (rand() % 100 - 50) * 0.01f;
            verts.push_back({ p1, color });
            verts.push_back({ p2, color });
        }
    }
    return verts;
}

std::vector<Vertex> Game::getPowerUpGeometry() const {
    std::vector<Vertex> verts;
    for (const auto& pu : powerUps) {
        float pulse = 1.0f + std::sin(pu.rotation * 3.0f) * 0.2f;
        float alpha = pu.life / 15.0f;
        Vec3 color;
        switch (pu.type) {
            case 0: color = Vec3(0.0f, 1.0f, 0.3f) * alpha; break;
            case 1: color = Vec3(1.0f, 0.8f, 0.0f) * alpha; break;
            case 2: color = Vec3(1.0f, 0.3f, 0.0f) * alpha; break;
            default: color = Vec3(1.0f, 1.0f, 1.0f) * alpha; break;
        }
        float s = 0.3f * pulse;
        verts.push_back({ {pu.pos.x - s, pu.pos.y, pu.pos.z}, color });
        verts.push_back({ {pu.pos.x + s, pu.pos.y, pu.pos.z}, color });
        verts.push_back({ {pu.pos.x, pu.pos.y + s, pu.pos.z}, color });
        verts.push_back({ {pu.pos.x + s, pu.pos.y, pu.pos.z}, color });
        verts.push_back({ {pu.pos.x, pu.pos.y - s, pu.pos.z}, color });
        verts.push_back({ {pu.pos.x - s, pu.pos.y, pu.pos.z}, color });
    }
    return verts;
}

std::vector<Vertex> Game::getHUDGeometry() const {
    std::vector<Vertex> verts;
    // Crosshair
    float chSize = 0.03f;
    Vec3 chColor(0.0f, 1.0f, 0.8f);
    verts.push_back({ {-chSize, 0, 0}, chColor });
    verts.push_back({ {chSize, 0, 0}, chColor });
    verts.push_back({ {0, -chSize, 0}, chColor });
    verts.push_back({ {0, chSize, 0}, chColor });

    // Health bar background
    float hbWidth = 0.4f;
    float hbHeight = 0.03f;
    float hbX = -0.8f;
    float hbY = -0.85f;
    Vec3 bgColor(0.2f, 0.2f, 0.2f);
    verts.push_back({ {hbX, hbY, 0}, bgColor });
    verts.push_back({ {hbX + hbWidth, hbY, 0}, bgColor });
    verts.push_back({ {hbX + hbWidth, hbY + hbHeight, 0}, bgColor });
    verts.push_back({ {hbX, hbY, 0}, bgColor });
    verts.push_back({ {hbX + hbWidth, hbY + hbHeight, 0}, bgColor });
    verts.push_back({ {hbX, hbY + hbHeight, 0}, bgColor });

    // Health bar fill
    float healthPct = (float)hp / maxHp;
    Vec3 hpColor(1.0f - healthPct, healthPct, 0.0f);
    verts.push_back({ {hbX, hbY, 0}, hpColor });
    verts.push_back({ {hbX + hbWidth * healthPct, hbY, 0}, hpColor });
    verts.push_back({ {hbX + hbWidth * healthPct, hbY + hbHeight, 0}, hpColor });
    verts.push_back({ {hbX, hbY, 0}, hpColor });
    verts.push_back({ {hbX + hbWidth * healthPct, hbY + hbHeight, 0}, hpColor });
    verts.push_back({ {hbX, hbY + hbHeight, 0}, hpColor });

    return verts;
}

std::vector<Vertex> Game::getMinimapGeometry() const {
    std::vector<Vertex> verts;
    float mapSize = 0.25f;
    float mapX = 0.95f - mapSize;
    float mapY = 0.95f - mapSize;
    float scale = mapSize / (ARENA * 2.0f);

    // Background
    verts.push_back({ {mapX, mapY, -0.1f}, Vec3(0.05f, 0.05f, 0.1f) });
    verts.push_back({ {mapX + mapSize, mapY, -0.1f}, Vec3(0.05f, 0.05f, 0.1f) });
    verts.push_back({ {mapX + mapSize, mapY + mapSize, -0.1f}, Vec3(0.05f, 0.05f, 0.1f) });
    verts.push_back({ {mapX, mapY + mapSize, -0.1f}, Vec3(0.05f, 0.05f, 0.1f) });

    // Player (white)
    float px = mapX + (px + ARENA) * scale;
    float py = mapY + (pz + ARENA) * scale;
    float ps = 0.015f;
    verts.push_back({ {px - ps, py - ps, -0.05f}, Vec3(1.0f, 1.0f, 1.0f) });
    verts.push_back({ {px + ps, py - ps, -0.05f}, Vec3(1.0f, 1.0f, 1.0f) });
    verts.push_back({ {px + ps, py + ps, -0.05f}, Vec3(1.0f, 1.0f, 1.0f) });
    verts.push_back({ {px - ps, py + ps, -0.05f}, Vec3(1.0f, 1.0f, 1.0f) });

    // Enemies (red/orange)
    for (auto& e : enemies) {
        if (!e.alive) continue;
        float bx = mapX + (e.pos.x + ARENA) * scale;
        float by = mapY + (e.pos.z + ARENA) * scale;
        float bs = 0.01f;
        Vec3 botColor = (e.type == EnemyType::BOSS) ? Vec3(1.0f, 0.8f, 0.0f) : Vec3(1.0f, 0.3f, 0.0f);
        verts.push_back({ {bx - bs, by - bs, -0.05f}, botColor });
        verts.push_back({ {bx + bs, by - bs, -0.05f}, botColor });
        verts.push_back({ {bx + bs, by + bs, -0.05f}, botColor });
        verts.push_back({ {bx - bs, by + bs, -0.05f}, botColor });
    }

    // Power-Ups
    for (auto& pu : powerUps) {
        float pux = mapX + (pu.pos.x + ARENA) * scale;
        float puy = mapY + (pu.pos.z + ARENA) * scale;
        float pus = 0.008f;
        Vec3 puColor;
        switch (pu.type) {
            case 0: puColor = Vec3(0.0f, 1.0f, 0.3f); break;
            case 1: puColor = Vec3(1.0f, 0.8f, 0.0f); break;
            case 2: puColor = Vec3(1.0f, 0.3f, 0.0f); break;
            default: puColor = Vec3(1.0f, 1.0f, 1.0f); break;
        }
        verts.push_back({ {pux - pus, puy - pus, -0.05f}, puColor });
        verts.push_back({ {pux + pus, puy - pus, -0.05f}, puColor });
        verts.push_back({ {pux + pus, puy + pus, -0.05f}, puColor });
        verts.push_back({ {pux - pus, puy + pus, -0.05f}, puColor });
    }

    return verts;
}

std::vector<Vertex> Game::getKillFeedGeometry() const {
    std::vector<Vertex> verts;
    float x = -0.95f;
    float y = 0.5f;
    for (const auto& entry : killFeed) {
        float alpha = entry.life / 3.0f;
        Vec3 color = entry.color * alpha;
        // Einfacher farbiger Balken für jeden Eintrag
        verts.push_back({ {x, y, -0.05f}, color });
        verts.push_back({ {x + 0.3f, y, -0.05f}, color });
        verts.push_back({ {x + 0.3f, y - 0.04f, -0.05f}, color });
        verts.push_back({ {x, y - 0.04f, -0.05f}, color });
        y -= 0.06f;
    }
    return verts;
}

std::vector<Vertex> Game::getDamageNumbersGeometry() const {
    std::vector<Vertex> verts;
    for (const auto& dn : damageNumbers) {
        float alpha = dn.life;
        Vec3 color(1.0f, 0.3f, 0.0f);
        color = color * alpha;
        float s = 0.02f;
        verts.push_back({ {dn.pos.x - s, dn.pos.y - s, dn.pos.z}, color });
        verts.push_back({ {dn.pos.x + s, dn.pos.y - s, dn.pos.z}, color });
        verts.push_back({ {dn.pos.x + s, dn.pos.y + s, dn.pos.z}, color });
        verts.push_back({ {dn.pos.x - s, dn.pos.y + s, dn.pos.z}, color });
    }
    return verts;
}
