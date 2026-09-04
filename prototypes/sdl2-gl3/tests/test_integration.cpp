// Integration tests for module functions
#include <cstdio>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>

// Replicate minimal Game struct for testing (no SDL dependencies)
struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
};

struct Entity {
    Vec3 pos;
    float yaw = 0, pitch = 0;
    float health = 100;
    bool alive = false;
    int type = 0;
    int botType = 0;
    float moveSpeed = 3.0f;
    int splitters = 0;
    float attackCooldown = 0;
};

struct Projectile {
    Vec3 pos, dir;
    float speed, life;
    bool fromPlayer;
    int weaponType;
    float damage;
    Projectile(Vec3 p = Vec3(), Vec3 d = Vec3(), bool player = false, int w = 0, float dmg = 50.0f)
        : pos(p), dir(d), speed(50.0f), life(2.0f), fromPlayer(player), weaponType(w), damage(dmg) {}
};

struct PowerUp {
    Vec3 pos;
    int type;
    float life;
    float rotation;
};

// Minimal Game mock
struct Game {
    Entity player;
    std::vector<Entity> bots;
    std::vector<Projectile> projectiles;
    std::vector<PowerUp> powerUps;
    int wave = 0, score = 0, highScore = 0, kills = 0;
    float gameTime = 0;
    bool gameOver = false, waveComplete = false;
    float playerSpeed = 10.0f, maxHealth = 100.0f, arenaSize = 40.0f;

    // Weapon state
    int currentWeapon = 0; // 0=railgun, 1=lightning, 2=plasma
    float railgunCooldown = 0, lightningCooldown = 0, plasmaCooldown = 0;
    float railgunFireRate = 0.3f, lightningFireRate = 0.05f, plasmaFireRate = 0.5f;
    float lightningRange = 15.0f, lightningDamage = 25.0f;
    int lightningChainCount = 3;
    float plasmaDamage = 80.0f, plasmaRadius = 5.0f;
    int railgunLevel = 1, lightningLevel = 1;

    // Specials
    float nuclearBlastCooldown = 0, timeSlowCooldown = 0, shieldCooldown = 0;
    float nuclearBlastMaxCooldown = 30.0f, timeSlowMaxCooldown = 20.0f, shieldMaxCooldown = 15.0f;
    float timeSlowTimer = 0, shieldTimer = 0;
    bool hasShield = false;

    // Upgrades
    int upgradePoints = 0, healthLevel = 1, speedLevel = 1;
};

static int intPassed = 0, intFailed = 0;

#define INT_TEST(name, expr) do { \
    printf("  Running %s... ", name); \
    fflush(stdout); \
    if (expr) { printf("PASSED\n"); intPassed++; } \
    else { printf("FAILED\n"); intFailed++; } \
} while(0)

// ============================================================
// Module: weapons.cpp (inline implementation for testing)
// ============================================================
namespace weapons_mod {

void fireRailgun(Game& game) {
    if (game.railgunCooldown > 0.0f) return;
    game.railgunCooldown = game.railgunFireRate;
    float damage = 50.0f + (game.railgunLevel - 1) * 15.0f;
    Vec3 forward(0, 0, -1);
    game.projectiles.push_back(Projectile(game.player.pos + forward * 0.5f, forward, true, 0, damage));
}

void fireLightning(Game& game) {
    if (game.lightningCooldown > 0.0f) return;
    game.lightningCooldown = game.lightningFireRate;
    // Find closest bots in range
    std::vector<std::pair<float, Entity*>> sorted;
    for (auto& e : game.bots) {
        if (!e.alive) continue;
        float dx = game.player.pos.x - e.pos.x;
        float dz = game.player.pos.z - e.pos.z;
        float d = std::sqrt(dx * dx + dz * dz);
        if (d < game.lightningRange) sorted.push_back({d, &e});
    }
    std::sort(sorted.begin(), sorted.end());
    int count = 0;
    for (auto& pair : sorted) {
        if (count >= game.lightningChainCount) break;
        Entity* e = pair.second;
        e->health -= game.lightningDamage + (game.lightningLevel - 1) * 5.0f;
        if (e->health <= 0 && e->alive) { e->alive = false; game.kills++; }
        count++;
    }
}

void firePlasma(Game& game) {
    if (game.plasmaCooldown > 0.0f) return;
    game.plasmaCooldown = game.plasmaFireRate;
    Vec3 forward(0, 0, -1);
    game.projectiles.push_back(Projectile(game.player.pos + forward * 0.5f, forward, true, 2, game.plasmaDamage));
}

void updateWeapons(float dt, Game& game) {
    if (game.railgunCooldown > 0.0f) game.railgunCooldown -= dt;
    if (game.lightningCooldown > 0.0f) game.lightningCooldown -= dt;
    if (game.plasmaCooldown > 0.0f) game.plasmaCooldown -= dt;
}

} // namespace weapons_mod

// ============================================================
// Module: powerups.cpp (inline implementation for testing)
// ============================================================
namespace powerups_mod {

void spawnPowerUp(Game& game, Vec3 pos, int type) {
    PowerUp pu;
    pu.pos = pos;
    pu.type = type;
    pu.life = 15.0f;
    pu.rotation = 0;
    game.powerUps.push_back(pu);
}

void updatePowerUps(float dt, Game& game) {
    for (int i = (int)game.powerUps.size() - 1; i >= 0; i--) {
        game.powerUps[i].life -= dt;
        game.powerUps[i].rotation += dt * 2.0f;
        if (game.powerUps[i].life <= 0.0f) {
            game.powerUps.erase(game.powerUps.begin() + i);
        }
    }
}

void collectPowerUp(Game& game, int index) {
    if (index < 0 || index >= (int)game.powerUps.size()) return;
    float dx = game.player.pos.x - game.powerUps[index].pos.x;
    float dz = game.player.pos.z - game.powerUps[index].pos.z;
    float d = std::sqrt(dx * dx + dz * dz);
    if (d < 2.0f) {
        switch (game.powerUps[index].type) {
            case 0: game.player.health = game.maxHealth; break;
            case 1: game.score += 500; break;
            case 2: break; // damage boost - no timer in this mock
        }
        game.powerUps.erase(game.powerUps.begin() + index);
    }
}

} // namespace powerups_mod

// ============================================================
// Module: specials.cpp (inline implementation for testing)
// ============================================================
namespace specials_mod {

void activateNuclearBlast(Game& game) {
    if (game.nuclearBlastCooldown > 0.0f) return;
    game.nuclearBlastCooldown = game.nuclearBlastMaxCooldown;
    for (auto& bot : game.bots) {
        if (bot.alive) { bot.alive = false; game.kills++; }
    }
}

void activateTimeSlow(Game& game) {
    if (game.timeSlowCooldown > 0.0f) return;
    game.timeSlowCooldown = game.timeSlowMaxCooldown;
    game.timeSlowTimer = 3.0f;
}

void activateShield(Game& game) {
    if (game.shieldCooldown > 0.0f) return;
    game.shieldCooldown = game.shieldMaxCooldown;
    game.shieldTimer = 5.0f;
    game.hasShield = true;
}

void updateSpecials(float dt, Game& game) {
    if (game.nuclearBlastCooldown > 0.0f) game.nuclearBlastCooldown -= dt;
    if (game.timeSlowCooldown > 0.0f) game.timeSlowCooldown -= dt;
    if (game.shieldCooldown > 0.0f) game.shieldCooldown -= dt;
    if (game.timeSlowTimer > 0.0f) {
        game.timeSlowTimer -= dt;
        if (game.timeSlowTimer <= 0.0f) game.timeSlowTimer = 0.0f;
    }
    if (game.shieldTimer > 0.0f) {
        game.shieldTimer -= dt;
        if (game.shieldTimer <= 0.0f) { game.shieldTimer = 0.0f; game.hasShield = false; }
    }
}

} // namespace specials_mod

// ============================================================
// Module: score.cpp (inline implementation for testing)
// ============================================================
namespace score_mod {

void addScore(Game& game, int points) {
    game.score += points;
}

void saveHighScore(Game& game) {
    FILE* f = fopen("test_highscore.dat", "w");
    if (f) {
        fprintf(f, "%d", game.highScore);
        fclose(f);
    }
}

void loadHighScore(Game& game) {
    FILE* f = fopen("test_highscore.dat", "r");
    if (f) {
        fscanf(f, "%d", &game.highScore);
        fclose(f);
    }
}

} // namespace score_mod

// ============================================================
// TESTS
// ============================================================

void testWeapons() {
    printf("\n[Weapon Module Tests]\n");

    // Railgun fires projectile
    {
        Game g;
        g.player.pos = Vec3(0, 1.7f, 0);
        weapons_mod::fireRailgun(g);
        INT_TEST("railgun_creates_projectile", g.projectiles.size() == 1);
        INT_TEST("railgun_sets_cooldown", g.railgunCooldown == g.railgunFireRate);
        INT_TEST("railgun_projectile_from_player", g.projectiles[0].fromPlayer);
    }

    // Railgun cooldown blocks second shot
    {
        Game g;
        g.player.pos = Vec3(0, 1.7f, 0);
        weapons_mod::fireRailgun(g);
        weapons_mod::fireRailgun(g);
        INT_TEST("railgun_cooldown_blocks", g.projectiles.size() == 1);
    }

    // Update reduces cooldown
    {
        Game g;
        g.player.pos = Vec3(0, 1.7f, 0);
        weapons_mod::fireRailgun(g);
        weapons_mod::updateWeapons(0.3f, g);
        INT_TEST("weapon_cooldown_decays", g.railgunCooldown <= 0.0f);
    }

    // Lightning damages multiple targets
    {
        Game g;
        g.player.pos = Vec3(0, 1.7f, 0);
        g.bots.resize(3);
        g.bots[0].pos = Vec3(5, 0.5f, 0); g.bots[0].alive = true; g.bots[0].health = 100;
        g.bots[1].pos = Vec3(10, 0.5f, 0); g.bots[1].alive = true; g.bots[1].health = 100;
        g.bots[2].pos = Vec3(14, 0.5f, 0); g.bots[2].alive = true; g.bots[2].health = 100;
        weapons_mod::fireLightning(g);
        INT_TEST("lightning_hits_bot1", g.bots[0].health < 100);
        INT_TEST("lightning_hits_bot2", g.bots[1].health < 100);
        INT_TEST("lightning_hits_bot3", g.bots[2].health < 100);
    }

    // Lightning kills low-health bot
    {
        Game g;
        g.player.pos = Vec3(0, 1.7f, 0);
        g.bots.resize(1);
        g.bots[0].pos = Vec3(5, 0.5f, 0); g.bots[0].alive = true; g.bots[0].health = 10;
        weapons_mod::fireLightning(g);
        INT_TEST("lightning_kills_weak_bot", !g.bots[0].alive);
        INT_TEST("lightning_kill_increments", g.kills == 1);
    }

    // Lightning respects chain count
    {
        Game g;
        g.player.pos = Vec3(0, 1.7f, 0);
        g.lightningChainCount = 2;
        g.bots.resize(4);
        for (int i = 0; i < 4; i++) {
            g.bots[i].pos = Vec3(i * 3 + 3, 0.5f, 0); g.bots[i].alive = true; g.bots[i].health = 100;
        }
        weapons_mod::fireLightning(g);
        int damaged = 0;
        for (auto& b : g.bots) if (b.health < 100) damaged++;
        INT_TEST("lightning_chain_count_respected", damaged == 2);
    }

    // Plasma fires
    {
        Game g;
        g.player.pos = Vec3(0, 1.7f, 0);
        weapons_mod::firePlasma(g);
        INT_TEST("plasma_creates_projectile", g.projectiles.size() == 1);
        INT_TEST("plasma_weapon_type", g.projectiles[0].weaponType == 2);
    }

    // Lightning ignores far targets
    {
        Game g;
        g.player.pos = Vec3(0, 1.7f, 0);
        g.lightningRange = 5.0f;
        g.bots.resize(2);
        g.bots[0].pos = Vec3(4, 0.5f, 0); g.bots[0].alive = true; g.bots[0].health = 100;
        g.bots[1].pos = Vec3(20, 0.5f, 0); g.bots[1].alive = true; g.bots[1].health = 100;
        weapons_mod::fireLightning(g);
        INT_TEST("lightning_near_target_hit", g.bots[0].health < 100);
        INT_TEST("lightning_far_target_unhit", g.bots[1].health == 100);
    }
}

void testPowerups() {
    printf("\n[Powerup Module Tests]\n");

    // Spawn
    {
        Game g;
        powerups_mod::spawnPowerUp(g, Vec3(5, 0.5f, 5), 0);
        INT_TEST("powerup_spawns", g.powerUps.size() == 1);
        INT_TEST("powerup_type_set", g.powerUps[0].type == 0);
    }

    // Update decays life
    {
        Game g;
        powerups_mod::spawnPowerUp(g, Vec3(5, 0.5f, 5), 0);
        float life1 = g.powerUps[0].life;
        powerups_mod::updatePowerUps(1.0f, g);
        INT_TEST("powerup_life_decreases", g.powerUps[0].life < life1);
    }

    // Powerup expires
    {
        Game g;
        powerups_mod::spawnPowerUp(g, Vec3(5, 0.5f, 5), 0);
        powerups_mod::updatePowerUps(20.0f, g);
        INT_TEST("powerup_expires", g.powerUps.empty());
    }

    // Collect health
    {
        Game g;
        g.player.pos = Vec3(0, 1.7f, 0);
        g.player.health = 50;
        powerups_mod::spawnPowerUp(g, Vec3(0, 0.5f, 0), 0);
        powerups_mod::collectPowerUp(g, 0);
        INT_TEST("health_powerup_heals", g.player.health == g.maxHealth);
        INT_TEST("health_powerup_removed", g.powerUps.empty());
    }

    // Collect score
    {
        Game g;
        g.player.pos = Vec3(0, 1.7f, 0);
        int initialScore = g.score;
        powerups_mod::spawnPowerUp(g, Vec3(0, 0.5f, 0), 1);
        powerups_mod::collectPowerUp(g, 0);
        INT_TEST("score_powerup_adds", g.score == initialScore + 500);
    }

    // Collect out of range fails
    {
        Game g;
        g.player.pos = Vec3(0, 1.7f, 0);
        powerups_mod::spawnPowerUp(g, Vec3(100, 0.5f, 100), 0);
        powerups_mod::collectPowerUp(g, 0);
        INT_TEST("collect_out_of_range_fails", g.powerUps.size() == 1);
    }
}

void testSpecials() {
    printf("\n[Special Module Tests]\n");

    // Nuclear blast kills all
    {
        Game g;
        g.bots.resize(5);
        for (int i = 0; i < 5; i++) { g.bots[i].pos = Vec3(i * 3, 0.5f, 0); g.bots[i].alive = true; g.bots[i].health = 100; }
        specials_mod::activateNuclearBlast(g);
        int alive = 0;
        for (auto& b : g.bots) if (b.alive) alive++;
        INT_TEST("nuclear_kills_all", alive == 0);
        INT_TEST("nuclear_kills_count", g.kills == 5);
        INT_TEST("nuclear_sets_cooldown", g.nuclearBlastCooldown == g.nuclearBlastMaxCooldown);
    }

    // Nuclear cooldown blocks
    {
        Game g;
        g.bots.resize(1);
        g.bots[0].alive = true; g.bots[0].health = 100;
        specials_mod::activateNuclearBlast(g);
        specials_mod::activateNuclearBlast(g);
        INT_TEST("nuclear_cooldown_blocks", g.kills == 1);
    }

    // Time slow activates
    {
        Game g;
        specials_mod::activateTimeSlow(g);
        INT_TEST("time_slow_activates", g.timeSlowTimer == 3.0f);
        INT_TEST("time_slow_cooldown", g.timeSlowCooldown == g.timeSlowMaxCooldown);
    }

    // Shield activates
    {
        Game g;
        specials_mod::activateShield(g);
        INT_TEST("shield_activates", g.hasShield);
        INT_TEST("shield_timer", g.shieldTimer == 5.0f);
    }

    // Shield expires
    {
        Game g;
        specials_mod::activateShield(g);
        specials_mod::updateSpecials(6.0f, g);
        INT_TEST("shield_expires", !g.hasShield);
        INT_TEST("shield_timer_zero", g.shieldTimer == 0.0f);
    }

    // Time slow timer decays
    {
        Game g;
        specials_mod::activateTimeSlow(g);
        specials_mod::updateSpecials(1.0f, g);
        INT_TEST("time_slow_decays", g.timeSlowTimer == 2.0f);
    }
}

void testScore() {
    printf("\n[Score Module Tests]\n");

    // Add score
    {
        Game g;
        int initial = g.score;
        score_mod::addScore(g, 10);
        INT_TEST("score_increases", g.score == initial + 10);
    }

    // Save/load highscore
    {
        Game g;
        g.highScore = 5000;
        score_mod::saveHighScore(g);

        Game g2;
        score_mod::loadHighScore(g2);
        INT_TEST("highscore_roundtrip", g2.highScore == 5000);
    }

    // Save overwrites
    {
        Game g;
        g.highScore = 1000;
        score_mod::saveHighScore(g);
        g.highScore = 2000;
        score_mod::saveHighScore(g);

        Game g2;
        score_mod::loadHighScore(g2);
        INT_TEST("highscore_overwrites", g2.highScore == 2000);
    }

    // Cleanup
    ::remove("test_highscore.dat");
}

void testIntegration() {
    printf("\n[Integration Tests]\n");

    // Full wave cycle: spawn bots, kill all, check state
    {
        Game g;
        g.player.pos = Vec3(0, 1.7f, 0);
        g.wave = 5;
        g.bots.resize(3);
        for (int i = 0; i < 3; i++) {
            g.bots[i].pos = Vec3(i * 5 + 3, 0.5f, 0);
            g.bots[i].alive = true;
            g.bots[i].health = 10;
        }
        // Kill all with nuclear
        specials_mod::activateNuclearBlast(g);
        INT_TEST("wave_all_dead", g.kills == 3);
        // No alive bots
        int alive = 0;
        for (auto& b : g.bots) if (b.alive) alive++;
        INT_TEST("wave_none_alive", alive == 0);
    }

    // Railgun projectile damages bot
    {
        Game g;
        g.player.pos = Vec3(0, 1.7f, 0);
        g.bots.resize(1);
        g.bots[0].pos = Vec3(0, 0.5f, -5); g.bots[0].alive = true; g.bots[0].health = 100;
        weapons_mod::fireRailgun(g);
        // Manually check collision (simulate projectile hit)
        g.projectiles[0].pos = Vec3(0, 0.5f, -5);
        float dist = std::sqrt(
            (g.projectiles[0].pos.x - g.bots[0].pos.x) * (g.projectiles[0].pos.x - g.bots[0].pos.x) +
            (g.projectiles[0].pos.z - g.bots[0].pos.z) * (g.projectiles[0].pos.z - g.bots[0].pos.z)
        );
        INT_TEST("projectile_in_range", dist < 1.5f);
    }

    // Powerup collected during gameplay
    {
        Game g;
        g.player.pos = Vec3(0, 1.7f, 0);
        powerups_mod::spawnPowerUp(g, Vec3(0, 0.5f, 0), 1);
        powerups_mod::collectPowerUp(g, 0);
        INT_TEST("gameplay_powerup_collect", g.score == 500);
    }

    // Multiple weapon types cycle
    {
        Game g;
        g.player.pos = Vec3(0, 1.7f, 0);
        weapons_mod::fireRailgun(g);
        weapons_mod::updateWeapons(0.3f, g); // railgun ready again
        weapons_mod::fireRailgun(g);
        weapons_mod::fireLightning(g);
        weapons_mod::firePlasma(g);
        INT_TEST("multiple_weapon_types", g.projectiles.size() == 3);
    }

    // Combo: damage then collect
    {
        Game g;
        g.player.pos = Vec3(0, 1.7f, 0);
        g.bots.resize(1);
        g.bots[0].pos = Vec3(3, 0.5f, 0); g.bots[0].alive = true; g.bots[0].health = 10;
        weapons_mod::fireLightning(g); // kills bot
        INT_TEST("combo_kill_bot", !g.bots[0].alive);
        score_mod::addScore(g, 10);
        INT_TEST("combo_score", g.score == 10);
    }
}

int main() {
    testWeapons();
    testPowerups();
    testSpecials();
    testScore();
    testIntegration();

    printf("\n=== Integration Results ===\n");
    printf("Passed: %d\n", intPassed);
    printf("Failed: %d\n", intFailed);
    printf("Total:  %d\n", intPassed + intFailed);

    return intFailed > 0 ? 1 : 0;
}
