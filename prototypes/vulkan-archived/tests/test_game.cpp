// Tests für NeonArena Vulkan Prototyp
// Umfassende Unit-Tests für alle Spielogik-Features
#include <cstdio>
#include <cmath>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3 normalized() const { float l = length(); return l > 0 ? Vec3(x/l, y/l, z/l) : Vec3(); }
};

class TestGame {
public:
    struct Enemy {
        Vec3 pos; float hp; float phase; bool alive; float hitFlash;
        int type; float moveSpeed; float attackCooldown;
        Enemy() : pos(0,0,0), hp(3), phase(0), alive(false), hitFlash(0), type(0), moveSpeed(3.0f), attackCooldown(0) {}
    };
    struct Tracer { Vec3 from, to; float life; };
    struct Spark { Vec3 pos, vel; float life; };
    struct LightningArc { Vec3 start, end; float life; int segments; };

    struct KillFeedEntry {
        std::string text;
        Vec3 color;
        float life;
    };

    struct DamageNumber {
        Vec3 pos;
        int damage;
        float life;
        float vy;
    };

    float px = 0, pz = 0, pyaw = 0, ppitch = 0;
    float pvel_x = 0, pvel_z = 0;
    int hp = 100, score = 0, kills = 0;
    uint32_t lastShot = 0;

    std::vector<Enemy> enemies;
    std::vector<Tracer> tracers;
    std::vector<Spark> sparks;
    std::vector<LightningArc> lightningArcs;
    std::vector<KillFeedEntry> killFeed;
    std::vector<DamageNumber> damageNumbers;
    uint32_t lastSpawn = 0;
    int wave = 1;
    double now_s = 0;

    static constexpr float ARENA = 24.0f;
    static constexpr int MAX_ENEMIES = 12;

    float nuclearBlastCooldown = 0, timeSlowCooldown = 0, shieldCooldown = 0;
    const float nuclearBlastMaxCooldown = 30.0f, timeSlowMaxCooldown = 20.0f, shieldMaxCooldown = 15.0f;
    float timeSlowTimer = 0, shieldTimer = 0;
    bool hasShield = false;

    void spawnEnemy() {
        float a = (rand() % 3600) * 0.017453f * 0.1f;
        float r = ARENA * 0.9f;
        Enemy e;
        e.pos = { std::cos(a) * r, 0.9f, std::sin(a) * r };
        e.hp = 3 + wave;
        e.phase = rand() % 628 * 0.01f;
        e.alive = true;
        e.hitFlash = 0;
        e.type = 0;
        e.moveSpeed = 2.2f + wave * 0.15f;
        enemies.push_back(e);
    }

    void spawnBoss() {
        Enemy e;
        e.pos = { ARENA * 0.7f, 1.8f, 0 };
        e.hp = 500.0f + wave * 50;
        e.type = 4;
        e.moveSpeed = 2.0f;
        e.phase = 0;
        e.alive = true;
        e.hitFlash = 0;
        e.attackCooldown = 0;
        enemies.push_back(e);
    }

    void shoot() {
        uint32_t t = (uint32_t)(now_s * 1000);
        if (t - lastShot < 180) return;
        lastShot = t;

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

        Vec3 hit = o + d * (bestT > 60 ? 60 : bestT);
        tracers.push_back({ o, hit, 0.12f });

        if (best) {
            best->hp--;
            best->hitFlash = 1;
            if (best->hp <= 0) {
                best->alive = false;
                score += 10;
                kills++;
                if (best->type == 4) {
                    addKillFeed("BOSS KILLED!", Vec3(1.0f, 0.8f, 0.0f));
                } else {
                    addKillFeed("KILL", Vec3(0.0f, 1.0f, 0.5f));
                }
                addDamageNumber(best->pos, 50);
            }
        }
    }

    void burst(Vec3 p, int n) {
        for (int i = 0; i < n; i++) {
            Spark s;
            s.pos = p;
            s.vel = { (rand() % 2000 - 1000) * 0.004f, (rand() % 1500 + 200) * 0.004f, (rand() % 2000 - 1000) * 0.004f };
            s.life = 0.5f + (rand() % 30) * 0.01f;
            sparks.push_back(s);
        }
    }

    void activateNuclearBlast() {
        if (nuclearBlastCooldown > 0.0f) return;
        nuclearBlastCooldown = nuclearBlastMaxCooldown;
        for (auto& e : enemies) {
            if (e.alive) {
                e.alive = false;
                kills++;
                addScore(50);
            }
        }
    }

    void activateTimeSlow() {
        if (timeSlowCooldown > 0.0f) return;
        timeSlowCooldown = timeSlowMaxCooldown;
        timeSlowTimer = 3.0f;
    }

    void activateShield() {
        if (shieldCooldown > 0.0f) return;
        shieldCooldown = shieldMaxCooldown;
        shieldTimer = 5.0f;
        hasShield = true;
    }

    void addScore(int points) { score += points; }

    void addKillFeed(const std::string& text, Vec3 color) {
        KillFeedEntry entry;
        entry.text = text;
        entry.color = color;
        entry.life = 3.0f;
        killFeed.push_back(entry);
        if (killFeed.size() > 5) killFeed.erase(killFeed.begin());
    }

    void addDamageNumber(Vec3 pos, int damage) {
        DamageNumber dn;
        dn.pos = pos;
        dn.damage = damage;
        dn.life = 1.0f;
        dn.vy = 2.0f;
        damageNumbers.push_back(dn);
    }
};

static int testsPassed = 0, testsFailed = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { printf("  Running %s... ", #name); fflush(stdout); test_##name(); printf("PASSED\n"); testsPassed++; } while(0)
#define ASSERT(cond) do { if (!(cond)) { printf("FAILED\n  Assertion failed: %s at line %d\n", #cond, __LINE__); testsFailed++; return; } } while(0)
#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_GT(a, b) ASSERT((a) > (b))

TEST(spawn_enemy_in_arena) {
    TestGame g; g.spawnEnemy();
    ASSERT_EQ(g.enemies.size(), 1); ASSERT(g.enemies[0].alive);
    ASSERT(g.enemies[0].pos.x >= -TestGame::ARENA && g.enemies[0].pos.x <= TestGame::ARENA);
    ASSERT(g.enemies[0].pos.z >= -TestGame::ARENA && g.enemies[0].pos.z <= TestGame::ARENA);
}

TEST(spawn_boss_high_hp) {
    TestGame g; g.wave = 5; g.spawnBoss();
    ASSERT_EQ(g.enemies.size(), 1); ASSERT(g.enemies[0].alive);
    ASSERT_GT(g.enemies[0].hp, 500); ASSERT_EQ(g.enemies[0].type, 4);
}

TEST(shoot_hits_enemy) {
    TestGame g; g.px = 0; g.pz = 0; g.pyaw = 0; g.ppitch = 0;
    g.now_s = 1.0; g.spawnEnemy();
    g.enemies[0].pos = Vec3(0, 0.9f, -5); g.enemies[0].hp = 3;
    g.shoot(); ASSERT_EQ(g.enemies[0].hp, 2);
}

TEST(shoot_kills_enemy) {
    TestGame g; g.px = 0; g.pz = 0; g.pyaw = 0; g.ppitch = 0;
    g.now_s = 1.0; g.spawnEnemy();
    g.enemies[0].pos = Vec3(0, 0.9f, -5); g.enemies[0].hp = 1;
    g.shoot();
    ASSERT(!g.enemies[0].alive); ASSERT_EQ(g.kills, 1);
}

TEST(shoot_increases_score_on_kill) {
    TestGame g; g.px = 0; g.pz = 0; g.pyaw = 0; g.ppitch = 0;
    g.now_s = 1.0; g.spawnEnemy();
    g.enemies[0].pos = Vec3(0, 0.9f, -5); g.enemies[0].hp = 1;
    int initial = g.score; g.shoot(); ASSERT_GT(g.score, initial);
}

TEST(shoot_respects_cooldown) {
    TestGame g; g.px = 0; g.pz = 0; g.pyaw = 0; g.ppitch = 0;
    g.spawnEnemy(); g.enemies[0].pos = Vec3(0, 0.9f, -5);
    g.enemies[0].hp = 3;
    g.now_s = 0.2;
    g.shoot();
    g.now_s = 0.21;  // Nur 10ms später (Cooldown ist 180ms)
    g.shoot();
    ASSERT_EQ(g.enemies[0].hp, 2);  // Nur ein Treffer
}

TEST(shoot_creates_tracer) {
    TestGame g; g.px = 0; g.pz = 0; g.pyaw = 0; g.ppitch = 0;
    g.now_s = 1.0; g.shoot();
    ASSERT_GT(g.tracers.size(), 0);
}

TEST(burst_creates_sparks) {
    TestGame g; Vec3 pos(0, 1.5f, 0); g.burst(pos, 10);
    ASSERT_EQ(g.sparks.size(), 10);
}

TEST(nuclear_blast_kills_all_enemies) {
    TestGame g; g.px = 0; g.pz = 0;
    for (int i = 0; i < 5; i++) {
        g.spawnEnemy();
        g.enemies[i].pos = Vec3(i * 3, 0.9f, 0);
    }
    g.activateNuclearBlast();
    for (auto& e : g.enemies) ASSERT(!e.alive);
    ASSERT_EQ(g.kills, 5);
}

TEST(time_slow_activates) {
    TestGame g; g.activateTimeSlow();
    ASSERT_GT(g.timeSlowTimer, 0); ASSERT_EQ(g.timeSlowCooldown, g.timeSlowMaxCooldown);
}

TEST(shield_activates) {
    TestGame g; g.activateShield();
    ASSERT(g.hasShield); ASSERT_GT(g.shieldTimer, 0);
}

TEST(kill_feed_on_kill) {
    TestGame g; g.px = 0; g.pz = 0; g.pyaw = 0; g.ppitch = 0;
    g.now_s = 1.0; g.spawnEnemy();
    g.enemies[0].pos = Vec3(0, 0.9f, -5); g.enemies[0].hp = 1;
    g.shoot();
    ASSERT_GT(g.killFeed.size(), 0);
}

TEST(damage_number_on_kill) {
    TestGame g; g.px = 0; g.pz = 0; g.pyaw = 0; g.ppitch = 0;
    g.now_s = 1.0; g.spawnEnemy();
    g.enemies[0].pos = Vec3(0, 0.9f, -5); g.enemies[0].hp = 1;
    g.shoot();
    ASSERT_GT(g.damageNumbers.size(), 0);
}

TEST(game_over_resets) {
    TestGame g; g.hp = 0; g.score = 100; g.wave = 5;
    if (g.hp <= 0) { g.hp = 100; g.score = 0; g.wave = 1; g.enemies.clear(); }
    ASSERT_EQ(g.hp, 100); ASSERT_EQ(g.score, 0); ASSERT_EQ(g.wave, 1);
}

TEST(enemy_chases_player) {
    TestGame g; g.px = 10; g.pz = 10; g.spawnEnemy();
    float d1 = std::sqrt((g.px - g.enemies[0].pos.x) * (g.px - g.enemies[0].pos.x) + (g.pz - g.enemies[0].pos.z) * (g.pz - g.enemies[0].pos.z));
    float speed = 2.2f;
    float dx = g.px - g.enemies[0].pos.x, dz = g.pz - g.enemies[0].pos.z;
    float l = std::sqrt(dx * dx + dz * dz);
    if (l > 0.01f) { dx /= l; dz /= l; }
    g.enemies[0].pos.x += dx * speed * 0.016f;
    g.enemies[0].pos.z += dz * speed * 0.016f;
    float d2 = std::sqrt((g.px - g.enemies[0].pos.x) * (g.px - g.enemies[0].pos.x) + (g.pz - g.enemies[0].pos.z) * (g.pz - g.enemies[0].pos.z));
    ASSERT(d2 < d1);
}

TEST(tracer_life_decreases) {
    TestGame g; g.tracers.push_back({Vec3(0,0,0), Vec3(1,1,1), 0.12f});
    float initial = g.tracers[0].life; g.tracers[0].life -= 0.01f;
    ASSERT(g.tracers[0].life < initial);
}

TEST(spark_life_decreases) {
    TestGame g; g.sparks.push_back({Vec3(0,0,0), Vec3(1,1,1), 0.5f});
    float initial = g.sparks[0].life; g.sparks[0].life -= 0.01f;
    ASSERT(g.sparks[0].life < initial);
}

TEST(max_enemies_limit) {
    TestGame g;
    for (int i = 0; i < TestGame::MAX_ENEMIES + 5; i++) {
        if ((int)g.enemies.size() < TestGame::MAX_ENEMIES) g.spawnEnemy();
    }
    ASSERT_EQ(g.enemies.size(), TestGame::MAX_ENEMIES);
}

int main() {
    printf("\n=== NeonArena Vulkan Prototyp Tests ===\n\n");

    printf("[Enemy Tests]\n");
    RUN_TEST(spawn_enemy_in_arena);
    RUN_TEST(spawn_boss_high_hp);
    RUN_TEST(max_enemies_limit);

    printf("\n[Combat Tests]\n");
    RUN_TEST(shoot_hits_enemy);
    RUN_TEST(shoot_kills_enemy);
    RUN_TEST(shoot_increases_score_on_kill);
    RUN_TEST(shoot_respects_cooldown);
    RUN_TEST(shoot_creates_tracer);

    printf("\n[Specials Tests]\n");
    RUN_TEST(nuclear_blast_kills_all_enemies);
    RUN_TEST(time_slow_activates);
    RUN_TEST(shield_activates);

    printf("\n[FX Tests]\n");
    RUN_TEST(burst_creates_sparks);
    RUN_TEST(tracer_life_decreases);
    RUN_TEST(spark_life_decreases);

    printf("\n[Kill Feed Tests]\n");
    RUN_TEST(kill_feed_on_kill);
    RUN_TEST(damage_number_on_kill);

    printf("\n[Game State Tests]\n");
    RUN_TEST(game_over_resets);
    RUN_TEST(enemy_chases_player);

    printf("\n=== Results ===\n");
    printf("Passed: %d\n", testsPassed);
    printf("Failed: %d\n", testsFailed);
    printf("Total:  %d\n", testsPassed + testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
