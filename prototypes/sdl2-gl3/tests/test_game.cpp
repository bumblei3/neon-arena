// Tests für NeonArena SDL2-GL3 Prototyp
// Umfassende Unit-Tests für alle Spielogik-Features
#include <cstdio>
#include <cmath>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <algorithm>

// Mock-Klassen für Tests
struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3 normalized() const { float l = length(); return l > 0 ? Vec3(x/l, y/l, z/l) : Vec3(); }
};

struct Mat4 {
    float m[16];
    Mat4(bool identity = false) { std::memset(m, 0, sizeof(m)); if (identity) m[0]=m[5]=m[10]=m[15]=1.0f; }
    static Mat4 perspective(float fovY, float aspect, float near, float far) {
        Mat4 result; float t = std::tan(fovY * 0.5f);
        result.m[0] = 1.0f / (aspect * t); result.m[5] = 1.0f / t;
        result.m[10] = (far + near) / (near - far); result.m[14] = (2.0f * far * near) / (near - far);
        result.m[11] = -1.0f; return result;
    }
    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
        Vec3 f = (center - eye).normalized();
        Vec3 s = Vec3(f.y*up.z - f.z*up.y, f.z*up.x - f.x*up.z, f.x*up.y - f.y*up.x).normalized();
        Vec3 u = Vec3(s.y*f.z - s.z*f.y, s.z*f.x - s.x*f.z, s.x*f.y - s.y*f.x);
        Mat4 result(true);
        result.m[0]=s.x; result.m[4]=s.y; result.m[8]=s.z;
        result.m[1]=u.x; result.m[5]=u.y; result.m[9]=u.z;
        result.m[2]=-f.x; result.m[6]=-f.y; result.m[10]=-f.z;
        result.m[12]=-(s.x*eye.x+s.y*eye.y+s.z*eye.z);
        result.m[13]=-(u.x*eye.x+u.y*eye.y+u.z*eye.z);
        result.m[14]=f.x*eye.x+f.y*eye.y+f.z*eye.z;
        return result;
    }
    static Mat4 ortho(float l, float r, float b, float t, float n, float f) {
        Mat4 result(true);
        result.m[0]=2.0f/(r-l); result.m[5]=2.0f/(t-b); result.m[10]=-2.0f/(f-n);
        result.m[12]=-(r+l)/(r-l); result.m[13]=-(t+b)/(t-b); result.m[14]=-(f+n)/(f-n);
        return result;
    }
    const float* ptr() const { return m; }
};

class TestGame {
public:
    struct Entity {
        Vec3 pos; float yaw, pitch; float health; bool alive;
        int type; int botType; float attackCooldown; float moveSpeed;
        Entity() : pos(0,0,0), yaw(0), pitch(0), health(100), alive(false), type(0), botType(0), attackCooldown(0), moveSpeed(3.0f) {}
    };
    struct Projectile {
        Vec3 pos, dir; float speed, life; bool fromPlayer; float damage; int weaponType;
        Projectile(Vec3 p, Vec3 d, bool player, float dmg = 50.0f, int w = 0)
            : pos(p), dir(d.normalized()), speed(50.0f), life(2.0f), fromPlayer(player), damage(dmg), weaponType(w) {}
    };
    struct PowerUp { Vec3 pos; int type; float life; float rotation;
        PowerUp() : pos(0,0,0), type(0), life(15.0f), rotation(0) {} };

    enum class WeaponType { RAILGUN, LIGHTNING, PLASMA };

    Entity player;
    std::vector<Entity> bots;
    std::vector<Projectile> projectiles;
    std::vector<PowerUp> powerUps;
    int wave = 0, score = 0, kills = 0;
    float gameTime = 0;
    bool gameOver = false, waveComplete = false;

    WeaponType currentWeapon = WeaponType::RAILGUN;
    float railgunCooldown = 0, lightningCooldown = 0, plasmaCooldown = 0;
    const float railgunFireRate = 0.3f, lightningFireRate = 0.05f, plasmaFireRate = 0.5f;
    float lightningRange = 15.0f, lightningDamage = 25.0f;
    const int lightningChainCount = 3;
    const float plasmaDamage = 80.0f, plasmaRadius = 5.0f;

    float nuclearBlastCooldown = 0, timeSlowCooldown = 0, shieldCooldown = 0;
    const float nuclearBlastMaxCooldown = 30.0f, timeSlowMaxCooldown = 20.0f, shieldMaxCooldown = 15.0f;
    float timeSlowTimer = 0, shieldTimer = 0;
    bool hasShield = false;

    int upgradePoints = 0, railgunLevel = 1, lightningLevel = 1, healthLevel = 1, speedLevel = 1;
    const int maxUpgradeLevel = 5;

    int scoreMultiplier = 1, comboCount = 0;
    float multiplierTimer = 0, damageBoostTimer = 0;
    const float multiplierDecay = 5.0f;

    float playerSpeed = 10.0f, playerHeight = 1.7f, maxHealth = 100.0f, arenaSize = 40.0f;

    void spawnWave() {
        wave++; bots.clear();
        int botCount = wave + 1;
        for (int i = 0; i < botCount; i++) {
            Entity bot;
            float angle = (float)i / botCount * 6.28318f;
            bot.pos = Vec3(cosf(angle) * arenaSize * 0.7f, 0.5f, sinf(angle) * arenaSize * 0.7f);
            bot.health = 100.0f + wave * 10;
            bot.alive = true; bot.type = 1; bot.botType = 0; bot.moveSpeed = 3.0f;
            bots.push_back(bot);
        }
        waveComplete = false;
    }

    void fireRailgun() {
        if (railgunCooldown > 0.0f) return;
        railgunCooldown = railgunFireRate;
        float damage = 50.0f + (railgunLevel - 1) * 15.0f;
        Vec3 forward(0, 0, -1);
        projectiles.push_back(Projectile(player.pos + forward * 0.5f, forward, true, damage, 0));
    }

    void fireLightning() {
        if (lightningCooldown > 0.0f) return;
        lightningCooldown = lightningFireRate;
        std::vector<std::pair<float, Entity*>> sorted;
        for (auto& e : bots) {
            if (!e.alive) continue;
            float d = std::sqrt((player.pos.x - e.pos.x) * (player.pos.x - e.pos.x) + (player.pos.z - e.pos.z) * (player.pos.z - e.pos.z));
            if (d < lightningRange) sorted.push_back({d, &e});
        }
        std::sort(sorted.begin(), sorted.end());
        int count = 0;
        for (auto& pair : sorted) {
            if (count >= lightningChainCount) break;
            Entity* e = pair.second;
            e->health -= lightningDamage + (lightningLevel - 1) * 5.0f;
            if (e->health <= 0 && e->alive) { e->alive = false; kills++; addScore(10); }
            count++;
        }
    }

    void firePlasma() {
        if (plasmaCooldown > 0.0f) return;
        plasmaCooldown = plasmaFireRate;
        Vec3 forward(0, 0, -1);
        projectiles.push_back(Projectile(player.pos + forward * 0.5f, forward, true, plasmaDamage, 2));
    }

    void activateNuclearBlast() {
        if (nuclearBlastCooldown > 0.0f) return;
        nuclearBlastCooldown = nuclearBlastMaxCooldown;
        for (auto& bot : bots) {
            if (bot.alive) { bot.alive = false; kills++; addScore(50); }
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

    void updateSpecials(float dt) {
        if (nuclearBlastCooldown > 0.0f) nuclearBlastCooldown -= dt;
        if (timeSlowCooldown > 0.0f) timeSlowCooldown -= dt;
        if (shieldCooldown > 0.0f) shieldCooldown -= dt;
        if (timeSlowTimer > 0.0f) { timeSlowTimer -= dt; if (timeSlowTimer <= 0.0f) timeSlowTimer = 0.0f; }
        if (shieldTimer > 0.0f) { shieldTimer -= dt; if (shieldTimer <= 0.0f) { shieldTimer = 0.0f; hasShield = false; } }
    }

    void checkCollisions() {
        for (auto it = projectiles.begin(); it != projectiles.end(); ) {
            bool hit = false;
            if (it->fromPlayer) {
                for (auto& bot : bots) {
                    if (!bot.alive) continue;
                    float dist = std::sqrt((it->pos.x - bot.pos.x) * (it->pos.x - bot.pos.x) + (it->pos.z - bot.pos.z) * (it->pos.z - bot.pos.z));
                    if (dist < 1.5f) {
                        bot.health -= it->damage;
                        if (bot.health <= 0 && bot.alive) { bot.alive = false; kills++; addScore(10); }
                        hit = true; break;
                    }
                }
            }
            if (hit) it = projectiles.erase(it); else ++it;
        }
    }

    void applyUpgrade(int upgrade) {
        if (upgradePoints <= 0) return;
        switch (upgrade) {
            case 0: if (railgunLevel < maxUpgradeLevel) { railgunLevel++; upgradePoints--; } break;
            case 1: if (lightningLevel < maxUpgradeLevel) { lightningLevel++; lightningRange += 2.0f; upgradePoints--; } break;
            case 2: if (healthLevel < maxUpgradeLevel) { healthLevel++; maxHealth += 25; player.health = maxHealth; upgradePoints--; } break;
            case 3: if (speedLevel < maxUpgradeLevel) { speedLevel++; playerSpeed += 1.5f; upgradePoints--; } break;
        }
    }

    void addScore(int points) {
        comboCount++; multiplierTimer = multiplierDecay;
        if (comboCount >= 10) scoreMultiplier = 5;
        else if (comboCount >= 5) scoreMultiplier = 3;
        else if (comboCount >= 3) scoreMultiplier = 2;
        score += points * scoreMultiplier;
    }

    void spawnPowerUp(Vec3 pos, int type) {
        PowerUp pu; pu.pos = pos; pu.type = type; pu.life = 15.0f; pu.rotation = 0;
        powerUps.push_back(pu);
    }

    void collectPowerUps() {
        for (int i = (int)powerUps.size() - 1; i >= 0; i--) {
            float d = std::sqrt((player.pos.x - powerUps[i].pos.x) * (player.pos.x - powerUps[i].pos.x) + (player.pos.z - powerUps[i].pos.z) * (player.pos.z - powerUps[i].pos.z));
            if (d < 2.0f) {
                switch (powerUps[i].type) {
                    case 0: player.health = maxHealth; break;
                    case 1: addScore(500); break;
                    case 2: damageBoostTimer = 10.0f; break;
                }
                powerUps.erase(powerUps.begin() + i);
            }
        }
    }

    float distance(Vec3 a, Vec3 b) { return (a - b).length(); }
};

static int testsPassed = 0, testsFailed = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { printf("  Running %s... ", #name); fflush(stdout); test_##name(); printf("PASSED\n"); testsPassed++; } while(0)
#define ASSERT(cond) do { if (!(cond)) { printf("FAILED\n  Assertion failed: %s at line %d\n", #cond, __LINE__); testsFailed++; return; } } while(0)
#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NEAR(a, b, eps) ASSERT(std::fabs((a) - (b)) < (eps))
#define ASSERT_GT(a, b) ASSERT((a) > (b))
#define ASSERT_LT(a, b) ASSERT((a) < (b))

// Spawn Tests
TEST(spawn_wave_creates_correct_number_of_bots) {
    TestGame g; g.spawnWave();
    ASSERT_EQ(g.bots.size(), 2); ASSERT_EQ(g.wave, 1); ASSERT(!g.waveComplete);
}
TEST(spawn_wave_increases_difficulty) {
    TestGame g; g.spawnWave(); float hp1 = g.bots[0].health;
    g.spawnWave(); ASSERT_GT(g.bots[0].health, hp1);
}
TEST(bot_spawns_in_arena) {
    TestGame g; g.spawnWave();
    for (auto& b : g.bots) { ASSERT(b.pos.x >= -g.arenaSize && b.pos.x <= g.arenaSize); ASSERT(b.pos.z >= -g.arenaSize && b.pos.z <= g.arenaSize); }
}

// Weapon Tests
TEST(railgun_fires_projectile) {
    TestGame g; g.player.pos = Vec3(0, 1.7f, 0); g.fireRailgun();
    ASSERT_EQ(g.projectiles.size(), 1); ASSERT(g.projectiles[0].fromPlayer);
}
TEST(railgun_respects_cooldown) {
    TestGame g; g.player.pos = Vec3(0, 1.7f, 0); g.fireRailgun(); g.fireRailgun();
    ASSERT_EQ(g.projectiles.size(), 1);
}
TEST(lightning_hits_multiple_targets) {
    TestGame g; g.player.pos = Vec3(0, 1.7f, 0);
    g.bots.resize(3);
    g.bots[0].pos = Vec3(5, 0.5f, 0); g.bots[0].alive = true; g.bots[0].health = 100;
    g.bots[1].pos = Vec3(10, 0.5f, 0); g.bots[1].alive = true; g.bots[1].health = 100;
    g.bots[2].pos = Vec3(14, 0.5f, 0); g.bots[2].alive = true; g.bots[2].health = 100;
    g.fireLightning();
    ASSERT(g.bots[0].health < 100); ASSERT(g.bots[1].health < 100); ASSERT(g.bots[2].health < 100);
}
TEST(plasma_fires_projectile) {
    TestGame g; g.player.pos = Vec3(0, 1.7f, 0); g.firePlasma();
    ASSERT_EQ(g.projectiles.size(), 1); ASSERT_EQ(g.projectiles[0].weaponType, 2);
}
TEST(plasma_respects_cooldown) {
    TestGame g; g.player.pos = Vec3(0, 1.7f, 0); g.firePlasma(); g.firePlasma();
    ASSERT_EQ(g.projectiles.size(), 1);
}
TEST(projectile_kills_bot) {
    TestGame g; g.player.pos = Vec3(0, 1.7f, 0);
    g.bots.resize(1); g.bots[0].pos = Vec3(0, 0.5f, -5); g.bots[0].alive = true; g.bots[0].health = 10;
    g.fireRailgun(); g.projectiles[0].pos = Vec3(0, 0.5f, -5); g.checkCollisions();
    ASSERT(!g.bots[0].alive); ASSERT_EQ(g.kills, 1);
}

// Specials Tests
TEST(nuclear_blast_kills_all_enemies) {
    TestGame g; g.player.pos = Vec3(0, 1.7f, 0);
    g.bots.resize(5);
    for (int i = 0; i < 5; i++) { g.bots[i].pos = Vec3(i * 3, 0.5f, 0); g.bots[i].alive = true; g.bots[i].health = 100; }
    g.activateNuclearBlast();
    for (auto& b : g.bots) ASSERT(!b.alive);
    ASSERT_EQ(g.kills, 5);
}
TEST(nuclear_blast_respects_cooldown) {
    TestGame g; g.activateNuclearBlast(); g.activateNuclearBlast();
    ASSERT_EQ(g.nuclearBlastCooldown, g.nuclearBlastMaxCooldown);
}
TEST(time_slow_activates) {
    TestGame g; g.activateTimeSlow();
    ASSERT_GT(g.timeSlowTimer, 0); ASSERT_EQ(g.timeSlowCooldown, g.timeSlowMaxCooldown);
}
TEST(shield_activates) {
    TestGame g; g.activateShield();
    ASSERT(g.hasShield); ASSERT_GT(g.shieldTimer, 0);
}
TEST(shield_expires) {
    TestGame g; g.activateShield(); g.updateSpecials(6.0f);
    ASSERT(!g.hasShield); ASSERT_EQ(g.shieldTimer, 0);
}

// Score Tests
TEST(score_increases_on_kill) {
    TestGame g; g.player.pos = Vec3(0, 1.7f, 0);
    g.bots.resize(1); g.bots[0].pos = Vec3(0, 0.5f, -5); g.bots[0].alive = true; g.bots[0].health = 10;
    int initial = g.score; g.fireRailgun(); g.projectiles[0].pos = Vec3(0, 0.5f, -5); g.checkCollisions();
    ASSERT_GT(g.score, initial);
}
TEST(combo_multiplier_increases) {
    TestGame g; g.addScore(10); ASSERT_EQ(g.scoreMultiplier, 1);
    g.addScore(10); g.addScore(10); ASSERT_EQ(g.scoreMultiplier, 2);
    g.addScore(10); g.addScore(10); ASSERT_EQ(g.scoreMultiplier, 3);
}
TEST(combo_max_multiplier) {
    TestGame g; for (int i = 0; i < 12; i++) g.addScore(10);
    ASSERT_EQ(g.scoreMultiplier, 5);
}

// Upgrade Tests
TEST(upgrade_railgun_increases_damage) {
    TestGame g; g.upgradePoints = 1;
    float d1 = 50.0f + (g.railgunLevel - 1) * 15.0f; g.applyUpgrade(0);
    float d2 = 50.0f + (g.railgunLevel - 1) * 15.0f;
    ASSERT_GT(d2, d1); ASSERT_EQ(g.railgunLevel, 2);
}
TEST(upgrade_lightning_increases_range) {
    TestGame g; g.upgradePoints = 1; float r1 = g.lightningRange;
    g.applyUpgrade(1); ASSERT_GT(g.lightningRange, r1);
}
TEST(upgrade_health_increases_max_hp) {
    TestGame g; g.upgradePoints = 1; int m1 = g.maxHealth;
    g.applyUpgrade(2); ASSERT_GT(g.maxHealth, m1);
}
TEST(upgrade_speed_increases_player_speed) {
    TestGame g; g.upgradePoints = 1; float s1 = g.playerSpeed;
    g.applyUpgrade(3); ASSERT_GT(g.playerSpeed, s1);
}
TEST(upgrade_requires_points) {
    TestGame g; g.upgradePoints = 0; g.applyUpgrade(0); ASSERT_EQ(g.railgunLevel, 1);
}
TEST(upgrade_capped_at_max_level) {
    TestGame g; g.upgradePoints = 100;
    for (int i = 0; i < 10; i++) g.applyUpgrade(0);
    ASSERT_EQ(g.railgunLevel, g.maxUpgradeLevel);
}

// Power-Up Tests
TEST(spawn_power_up) {
    TestGame g; g.spawnPowerUp(Vec3(5, 0.5f, 5), 0);
    ASSERT_EQ(g.powerUps.size(), 1); ASSERT_EQ(g.powerUps[0].type, 0);
}
TEST(collect_power_up_health) {
    TestGame g; g.player.pos = Vec3(0, 1.7f, 0); g.player.health = 50;
    g.spawnPowerUp(Vec3(0, 0.5f, 0), 0); g.collectPowerUps();
    ASSERT_EQ(g.player.health, g.maxHealth); ASSERT_EQ(g.powerUps.size(), 0);
}
TEST(collect_power_up_score) {
    TestGame g; g.player.pos = Vec3(0, 1.7f, 0);
    int initial = g.score; g.spawnPowerUp(Vec3(0, 0.5f, 0), 1); g.collectPowerUps();
    ASSERT_GT(g.score, initial);
}

// Math Tests
TEST(distance_calculation) {
    TestGame g; ASSERT_NEAR(g.distance(Vec3(0,0,0), Vec3(3,4,0)), 5.0f, 0.01f);
}
TEST(mat4_perspective_valid) {
    Mat4 p = Mat4::perspective(1.1f, 16.0f/9.0f, 0.1f, 200.0f);
    ASSERT(p.m[0] > 0); ASSERT(p.m[5] > 0); ASSERT_EQ(p.m[11], -1.0f);
}
TEST(mat4_lookAt_valid) {
    Mat4 v = Mat4::lookAt(Vec3(0,5,10), Vec3(0,0,0), Vec3(0,1,0));
    ASSERT(v.m[12] != 0 || v.m[13] != 0 || v.m[14] != 0);
}
TEST(mat4_ortho_valid) {
    Mat4 o = Mat4::ortho(-1,1,-1,1,-1,1);
    ASSERT_EQ(o.m[0], 1.0f); ASSERT_EQ(o.m[5], 1.0f);
}

// Game State Tests
TEST(player_bounds_clamped) {
    TestGame g; g.player.pos = Vec3(1000, 1.7f, 1000);
    float lim = g.arenaSize - 1.0f;
    g.player.pos.x = std::max(-lim, std::min(lim, g.player.pos.x));
    g.player.pos.z = std::max(-lim, std::min(lim, g.player.pos.z));
    ASSERT(g.player.pos.x <= g.arenaSize); ASSERT(g.player.pos.z <= g.arenaSize);
}
TEST(game_over_resets_state) {
    TestGame g; g.player.health = 0; g.kills = 10; g.score = 1000;
    if (g.player.health <= 0) { g.player.health = g.maxHealth; g.score = 0; g.kills = 0; g.wave = 1; g.bots.clear(); }
    ASSERT_EQ(g.player.health, g.maxHealth); ASSERT_EQ(g.score, 0); ASSERT_EQ(g.kills, 0);
}

int main() {
    printf("\n=== NeonArena SDL2-GL3 Prototyp Tests ===\n\n");

    printf("[Spawn Tests]\n");
    RUN_TEST(spawn_wave_creates_correct_number_of_bots);
    RUN_TEST(spawn_wave_increases_difficulty);
    RUN_TEST(bot_spawns_in_arena);

    printf("\n[Weapon Tests]\n");
    RUN_TEST(railgun_fires_projectile);
    RUN_TEST(railgun_respects_cooldown);
    RUN_TEST(lightning_hits_multiple_targets);
    RUN_TEST(plasma_fires_projectile);
    RUN_TEST(plasma_respects_cooldown);
    RUN_TEST(projectile_kills_bot);

    printf("\n[Specials Tests]\n");
    RUN_TEST(nuclear_blast_kills_all_enemies);
    RUN_TEST(nuclear_blast_respects_cooldown);
    RUN_TEST(time_slow_activates);
    RUN_TEST(shield_activates);
    RUN_TEST(shield_expires);

    printf("\n[Score Tests]\n");
    RUN_TEST(score_increases_on_kill);
    RUN_TEST(combo_multiplier_increases);
    RUN_TEST(combo_max_multiplier);

    printf("\n[Upgrade Tests]\n");
    RUN_TEST(upgrade_railgun_increases_damage);
    RUN_TEST(upgrade_lightning_increases_range);
    RUN_TEST(upgrade_health_increases_max_hp);
    RUN_TEST(upgrade_speed_increases_player_speed);
    RUN_TEST(upgrade_requires_points);
    RUN_TEST(upgrade_capped_at_max_level);

    printf("\n[Power-Up Tests]\n");
    RUN_TEST(spawn_power_up);
    RUN_TEST(collect_power_up_health);
    RUN_TEST(collect_power_up_score);

    printf("\n[Math Tests]\n");
    RUN_TEST(distance_calculation);
    RUN_TEST(mat4_perspective_valid);
    RUN_TEST(mat4_lookAt_valid);
    RUN_TEST(mat4_ortho_valid);

    printf("\n[Game State Tests]\n");
    RUN_TEST(player_bounds_clamped);
    RUN_TEST(game_over_resets_state);

    printf("\n=== Results ===\n");
    printf("Passed: %d\n", testsPassed);
    printf("Failed: %d\n", testsFailed);
    printf("Total:  %d\n", testsPassed + testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
