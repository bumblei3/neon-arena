// game.h - Game logic for neon arena prototype
#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include "renderer.h"
#include "math.h"
#include "particle.h"
#include "text.h"
#include "audio_manager.h"
#include "spatial_hash.h"
#include "savegame.h"
#include "music_generator.h"
#include "particle_ecs.h"
#include "wave_config.h"
#include "overclock.h"
#include "echo.h"
#include "bot_ai.h"
#include <vector>
#include <cmath>
#include <cstdio>
#include <string>

// Game states
enum class GameState {
    MENU,
    OPTIONS,
    PLAYING,
    PAUSED,
    GAME_OVER,
    WAVE_COMPLETE
};

// AudioManager
extern AudioManager* g_audio;
extern MusicGenerator* g_music;

// Play sound through AudioManager
inline void playSnd(int soundHandle) {
    if (g_audio && soundHandle >= 0) g_audio->playSFX(soundHandle);
}

struct Entity {
    Vec3 pos;
    float yaw;
    float pitch;
    float health;
    bool alive;
    int type;  // 0 = player, 1 = bot
    int botType;  // 0 = melee, 1 = shooter, 2 = tank, 3 = fast, 4 = boss
    float attackCooldown = 0;
    float moveSpeed = 3.0f;
    bool isBoss = false;
    float bossPhase = 0;  // For boss attack patterns
    float vx, vy, vz;
    int splitters = 0;    // Number of mini-bots to spawn on death
    BotAI::BotState aiState;  // AI state machine for this bot

    Entity() : pos(0,0,0), yaw(0), pitch(0), health(100), alive(false), type(0), botType(0), attackCooldown(0), moveSpeed(3.0f), vx(0), vy(0), vz(0), splitters(0) {}
};

enum class WeaponType {
    RAILGUN,
    LIGHTNING_GUN,
    PLASMA_RIFLE
};

struct Projectile {
    Vec3 pos;
    Vec3 dir;
    float speed;
    float life;
    bool fromPlayer;
    WeaponType weapon;
    float damage;

    Projectile() : pos(0,0,0), dir(0,0,0), speed(50.0f), life(2.0f), fromPlayer(false), weapon(WeaponType::RAILGUN), damage(50.0f) {}
    Projectile(Vec3 p, Vec3 d, bool player, WeaponType w = WeaponType::RAILGUN, float dmg = 50.0f)
        : pos(p), dir(d.normalized()), speed(50.0f), life(2.0f), fromPlayer(player), weapon(w), damage(dmg) {}
};

struct LightningArc {
    Vec3 start;
    Vec3 end;
    Vec3 color;
    float life;
    float maxLife;
    int segments;

    LightningArc(Vec3 s, Vec3 e, Vec3 c, float l = 0.15f, int seg = 8)
        : start(s), end(e), color(c), life(l), maxLife(l), segments(seg) {}
};

// Power-up types
enum class PowerUpType {
    HEALTH = 0,
    SCORE = 1,
    DAMAGE_BOOST = 2
};

struct PowerUp {
    Vec3 pos;
    int type;  // 0 = health, 1 = score bonus, 2 = damage boost
    float life;
    float rotation;
};

// Special ability types
enum class SpecialType {
    NUCLEAR_BLAST,
    TIME_SLOW,
    SHIELD
};

// Kill feed entry
struct KillFeedEntry {
    std::string text;
    Vec3 color;
    float life;
};

// Damage number popup
struct DamageNumber {
    Vec3 pos;
    int damage;
    float life;
    float vy;
};

class Game {
public:
    Game();
    ~Game();

    bool init(SDL_Window* window);
    void shutdown();
    void run();

    void handleInput(float dt);
    void update(float dt);
    void render();

    Vec3 playerPos() const { return player.pos; }
    float playerYaw() const { return player.yaw; }
    float playerPitch() const { return player.pitch; }

    int getWave() const { return wave; }
    int getScore() const { return score; }
    int getKills() const { return kills; }
    float getGameTime() const { return gameTime; }
    bool isGameOver() const { return gameOver; }
    bool isWaveComplete() const { return waveComplete; }

    // Friend declarations for module free functions
    friend void switchWeapon(Game& game, WeaponType w);
    friend void fireRailgun(Game& game);
    friend void fireLightning(Game& game);
    friend void firePlasma(Game& game);
    friend void updateWeapons(float dt, Game& game);
    friend void renderLightning(Game& game);
    friend void findLightningTargets(Vec3 pos, Game& game, std::vector<Entity*>& targets);
    
    friend void spawnWave(Game& game);
    friend void updateBots(Game& game, float dt);
    friend void renderBots(Game& game);
    friend void renderSolidBots(Game& game);
    friend void spawnExplosion(Game& game, Vec3 pos, Vec3 color, int count);
    
    friend void updateScore(float dt, Game& game);
    friend void addScore(Game& game, int points);
    friend void saveHighScore(Game& game);
    friend void loadHighScore(Game& game);
    friend void addKillFeed(Game& game, const std::string& text, Vec3 color);
    friend void updateKillFeed(float dt, Game& game);
    friend void renderKillFeed(Game& game);
    friend void addDamageNumber(Game& game, Vec3 pos, int damage);
    friend void updateDamageNumbers(float dt, Game& game);
    friend void renderDamageNumbers(Game& game);
    
    friend void spawnPowerUp(Game& game, Vec3 pos, int type);
    friend void updatePowerUps(float dt, Game& game);
    friend void renderPowerUps(Game& game);
    friend void collectPowerUp(Game& game, int index);
    
    friend void activateNuclearBlast(Game& game);
    friend void activateTimeSlow(Game& game);
    friend void activateShield(Game& game);
    friend void updateSpecials(float dt, Game& game);
    friend void renderSpecialEffects(Game& game);
    friend void renderSpecialsHUD(Game& game);
    
    friend void renderHUD(Game& game);
    friend void renderMinimap(Game& game);
    friend void renderUpgradeMenu(Game& game);
    friend class SpatialHash;
    friend class SavegameManager;
    friend class OverclockManager;
    friend class EchoSystem;
    friend void handleUpgradeInput(Game& game, SDL_Event& event);
    friend void applyUpgrade(Game& game, int selection);
    friend void resetUpgrades(Game& game);

private:
    void setupArena();
    void updatePlayer(float dt);
    void updateProjectiles(float dt);
    void updateParticles(float dt);
    void renderParticles();
    void renderArena();
    void renderProjectiles();
    void renderMinimap();
    void checkCollisions();
    void updateCamera(float dt);
    void nextWave();
    void gameOverScreen();

    // Menu system
    void updateMenu(float dt);
    void renderMenu();
    void renderOptions();
    void renderPauseMenu();
    void renderGameOver();
    void handleMenuInput(SDL_Event& event);
    void updateMenuItems();
    void resetGame();

    // Camera
    void handleMouse();

    // Utilities
    Vec3 normalize(Vec3 v);
    float distance(Vec3 a, Vec3 b);

    // Renderer
    Renderer* renderer_ = nullptr;
    SDL_Window* window_ = nullptr;

    // State
    GameState state = GameState::MENU;
    int menuSelection = 0;
    std::vector<std::string> menuItems;
    float stateTimer = 0;

    // Text
    TextRenderer text_;

    // Spatial hash for collision detection
    SpatialHash* spatialHash = nullptr;

    // Particle system (ECS - no vector needed)
    ParticleSystem* particleSystem = nullptr;

    // Camera shake
    float shakeAmount = 0.0f;
    float shakeDecay = 5.0f;
    Vec3 shakeOffset;

    // Game state
    Entity player;
    std::vector<Entity> bots;
    std::vector<Projectile> projectiles;
    std::vector<Particle> particles;
    int wave = 0;
    int score = 0;
    int highScore = 0;
    int kills = 0;
    float gameTime = 0;
    bool running = true;
    bool gameOver = false;
    bool waveComplete = false;
    float waveBreak = 0;
    float nextWaveDelay = 3.0f;

    // Score system
    int scoreMultiplier = 1;
    int lastKillTime = 0;       // For kill-streak tracking
    int killStreak = 0;         // Current kill streak
    float hitFeedbackTimer = 0; // Crosshair pulse on hit
    float waveAnnounceTimer = 0; // Wave announcement display
    float multiplierTimer = 0.0f;
    const float multiplierDecay = 5.0f;
    int comboCount = 0;

    // Kill Feed
    std::vector<KillFeedEntry> killFeed;

    // Damage Numbers
    std::vector<DamageNumber> damageNumbers;

    // Input
    bool keys[SDL_NUM_SCANCODES];
    float mouseX = 0, mouseY = 0;
    bool shootRequested = false;
    bool shootLightning = false;

    // Config
    float playerSpeed = 10.0f;
    float playerHeight = 1.7f;
    float mouseSensitivity = 0.002f;
    float maxHealth = 100.0f;
    float arenaSize = 40.0f;

    // Weapon system
    WeaponType currentWeapon = WeaponType::RAILGUN;
    float railgunCooldown = 0.0f;
    float lightningCooldown = 0.0f;
    const float railgunFireRate = 0.3f;
    const float lightningFireRate = 0.05f;
    float lightningRange = 15.0f;
    const float lightningDamage = 25.0f;
    int lightningChainCount = 3;

    // Plasma Rifle
    float plasmaCooldown = 0.0f;
    float plasmaFireRate = 0.5f;
    const float plasmaDamage = 80.0f;
    const float plasmaRadius = 5.0f;
    const float plasmaSpeed = 30.0f;

    // Lightning arcs for rendering
    std::vector<LightningArc> lightningArcs;

    // Weapon upgrade levels
    int railgunLevel = 1;
    int lightningLevel = 1;

    // Specials/Ultimates
    float nuclearBlastCooldown = 0.0f;
    const float nuclearBlastMaxCooldown = 30.0f;
    float timeSlowCooldown = 0.0f;
    const float timeSlowMaxCooldown = 20.0f;
    float shieldCooldown = 0.0f;
    float shieldMaxCooldown = 15.0f;
    float timeSlowTimer = 0.0f;
    float shieldTimer = 0.0f;
    bool hasShield = false;

    // Power-ups
    std::vector<PowerUp> powerUps;
    float damageBoostTimer = 0.0f;

    // Upgrade system
    int upgradePoints = 0;
    int healthLevel = 1;
    int speedLevel = 1;
    bool showUpgradeMenu = false;
    int upgradeSelection = 0;
    const int maxUpgradeLevel = 5;

    // === Overclock Bug Effects ===
    float railgunFeedbackChance = 0.0f;  // Self-damage on miss
    float plasmaOverheatPenalty = 0.0f;  // Extra cooldown on overheat
    float lightningBacklashChance = 0.0f; // Self-hit chance
    float shieldCrashChance = 0.0f;      // Shield stun chance
    int   splitterVirusLevel = 0;        // Mini-bots from bots
    bool  splitterFriendlyFire = false;  // Mini-bots hit player
    float scoreMultiplierFloat = 1.0f;   // Score gain multiplier
    float scoreDecayRate = 0.0f;         // Score lost per second
    float phaseGlitchChance = 0.0f;      // Fall through floor
    int   phaseShiftKills = 0;           // Invuln kills count
    float phaseShiftTimer = 0.0f;        // Invuln timer

    // === Wave-Fusion System ===
    WaveFusion currentFusion = WaveFusion::NONE;
    float arenaShrinkTimer = 0.0f;       // Tracks arena shrink over time
    float fusionDisplayTimer = 0.0f;     // How long to show fusion announcement

    // === Rival Ghost System ===
    struct RivalGhost {
        bool active = false;
        float posX = 0, posY = 0, posZ = 0;
        float alpha = 1.0f;              // Visibility (fades in/out)
        float heartbeatTimer = 0.0f;     // Pulsing effect
        int rivalScore = 0;
        int rivalWave = 0;
    };
    RivalGhost rivalGhost;
    OverclockManager* overclock = nullptr;
    EchoSystem* echoSystem = nullptr;
};
