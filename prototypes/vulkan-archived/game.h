// NEON ARENA - Vulkan + SDL2 Prototype
// Game state: arena, enemies, waves, player, audio, upgrades, specials
#pragma once

#include "types.h"
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>
#include <cstring>

class AudioSystem;

// Gegnertypen
enum class EnemyType {
    MELEE = 0,
    SHOOTER = 1,
    TANK = 2,
    FAST = 3,
    BOSS = 4
};

// Waffentypen
enum class WeaponType {
    RAILGUN = 0,
    LIGHTNING = 1,
    PLASMA = 2
};

// Specials
enum class SpecialType {
    NUCLEAR_BLAST = 0,
    TIME_SLOW = 1,
    SHIELD = 2
};

struct Enemy {
    Vec3 pos;
    float hp;
    float maxHp;
    float phase;
    bool alive;
    float hitFlash;
    EnemyType type;
    float attackCooldown;
    float moveSpeed;
};

struct Tracer {
    Vec3 from, to;
    float life;
};

struct Spark {
    Vec3 pos, vel;
    float life;
};

struct LightningArc {
    Vec3 start, end;
    float life;
    int segments;
};

struct PowerUp {
    Vec3 pos;
    int type;
    float life;
    float rotation;
};

class Game {
public:
    static const float ARENA;
    static const int MAX_ENEMIES = 15;

    // Player state
    float px = 0, pz = 0, pyaw = 0, ppitch = 0;
    float pvel_x = 0, pvel_z = 0;
    int hp = 100;
    int maxHp = 100;
    int score = 0;
    int highScore = 0;
    uint32_t lastShot = 0;
    float recoil = 0;

    // Enemies / fx
    std::vector<Enemy> enemies;
    std::vector<Tracer> tracers;
    std::vector<Spark> sparks;
    std::vector<LightningArc> lightningArcs;
    std::vector<PowerUp> powerUps;
    uint32_t lastSpawn = 0;
    int wave = 1;
    double now_s = 0;
    int kills = 0;

    // Input state
    bool keyW = false, keyA = false, keyS = false, keyD = false;
    bool keySpace = false;

    // Weapon system
    WeaponType currentWeapon = WeaponType::RAILGUN;
    float railgunCooldown = 0;
    float lightningCooldown = 0;
    float plasmaCooldown = 0;
    const float railgunFireRate = 0.3f;
    const float lightningFireRate = 0.05f;
    const float plasmaFireRate = 0.5f;
    float lightningRange = 15.0f;
    float lightningDamage = 25.0f;
    const int lightningChainCount = 3;
    const float plasmaDamage = 80.0f;
    const float plasmaRadius = 5.0f;

    // Specials
    float nuclearBlastCooldown = 0.0f;
    const float nuclearBlastMaxCooldown = 30.0f;
    float timeSlowCooldown = 0.0f;
    const float timeSlowMaxCooldown = 20.0f;
    float shieldCooldown = 0.0f;
    const float shieldMaxCooldown = 15.0f;
    float timeSlowTimer = 0.0f;
    float shieldTimer = 0.0f;
    bool hasShield = false;

    // Upgrade system
    int upgradePoints = 0;
    int railgunLevel = 1;
    int lightningLevel = 1;
    int healthLevel = 1;
    int speedLevel = 1;
    const int maxUpgradeLevel = 5;
    bool showUpgradeMenu = false;
    int upgradeSelection = 0;

    // Score system
    int scoreMultiplier = 1;
    float multiplierTimer = 0.0f;
    const float multiplierDecay = 5.0f;
    int comboCount = 0;
    float damageBoostTimer = 0.0f;

    // Kill Feed
    struct KillFeedEntry {
        std::string text;
        Vec3 color;
        float life;
    };
    std::vector<KillFeedEntry> killFeed;
    void addKillFeed(const std::string& text, Vec3 color);
    void updateKillFeed(float dt);

    // Damage Numbers
    struct DamageNumber {
        Vec3 pos;
        int damage;
        float life;
        float vy;
    };
    std::vector<DamageNumber> damageNumbers;
    void addDamageNumber(Vec3 pos, int damage);
    void updateDamageNumbers(float dt);

    void setAudio(AudioSystem* a) { m_audio = a; }

    void update(float dt);
    void shoot();
    void shootRailgun();
    void shootLightning();
    void shootPlasma();
    void spawnEnemy();
    void spawnBoss();
    void burst(Vec3 p, int n);

    // Specials
    void activateNuclearBlast();
    void activateTimeSlow();
    void activateShield();
    void updateSpecials(float dt);

    // Upgrades
    void applyUpgrade(int upgrade);
    void addScore(int points);
    void saveHighScore();
    void loadHighScore();

    // Get view matrix from player camera
    void getViewMatrix(float* out) const;
    void getProjMatrix(float* out, float aspect) const;

    // Generate arena geometry
    std::vector<Vertex> getArenaGeometry() const;

    // Generate enemy geometry
    std::vector<Vertex> getEnemyGeometry() const;

    // Generate tracer geometry
    std::vector<Vertex> getTracerGeometry() const;

    // Generate spark geometry
    std::vector<Vertex> getSparkGeometry() const;

    // Generate lightning geometry
    std::vector<Vertex> getLightningGeometry() const;

    // Generate power-up geometry
    std::vector<Vertex> getPowerUpGeometry() const;

    // Generate HUD geometry
    std::vector<Vertex> getHUDGeometry() const;

    // Generate Minimap geometry
    std::vector<Vertex> getMinimapGeometry() const;

    // Generate Kill Feed geometry
    std::vector<Vertex> getKillFeedGeometry() const;

    // Generate Damage Numbers geometry
    std::vector<Vertex> getDamageNumbersGeometry() const;

private:
    AudioSystem* m_audio = nullptr;

    static void mat4_identity(float* m);
    static void mat4_lookAt(float* m, const Vec3& eye, const Vec3& center, const Vec3& up);
    static void mat4_perspective(float* m, float fovy, float aspect, float zn, float zf);
    static void mat4_mul(float* out, const float* a, const float* b);
};
