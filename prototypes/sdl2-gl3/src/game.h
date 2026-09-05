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
#include "achievements.h"
#include "music_generator.h"
#include "particle_ecs.h"
#include "wave_config.h"
#include "overclock.h"
#include "echo.h"
#include "bot_ai.h"
#include "coop.h"
#include "audio_polish.h"
#include "wave_editor.h"
#include "ghost_rules.h"
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
    int botType;  // 0 melee, 1 shooter, 2 tank, 3 fast, 4 boss, 5 stealth, 6 detector
    float attackCooldown = 0;
    float moveSpeed = 3.0f;
    bool isBoss = false;
    float bossPhase = 0;
    float vx, vy, vz;
    int splitters = 0;
    BotAI::BotState aiState;
    int ghostMarked = 0;  // 1 = marked by scanner (visible)
    float ghostMarkTimer = 0.0f;

    Entity() : pos(0,0,0), yaw(0), pitch(0), health(100), alive(false), type(0), botType(0), attackCooldown(0), moveSpeed(3.0f), vx(0), vy(0), vz(0), splitters(0), ghostMarked(0), ghostMarkTimer(0.0f) {}
};

enum class WeaponType {
    RAILGUN,
    LIGHTNING_GUN,
    PLASMA_RIFLE,
    GHOST_SNIPER
};

enum class Loadout {
    ARENA,
    GHOST
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

enum class PowerUpType {
    HEALTH = 0,
    SCORE = 1,
    DAMAGE_BOOST = 2
};

struct PowerUp {
    Vec3 pos;
    int type;
    float life;
    float rotation;
};

enum class SpecialType {
    NUCLEAR_BLAST,
    TIME_SLOW,
    SHIELD,
    SCANNER_SWEEP,
    EMP_BLAST,
    TAC_NUKE,
    CLOAK
};

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

    // Friend declarations
    friend void switchWeapon(Game& game, WeaponType w);
    friend void fireRailgun(Game& game);
    friend void fireLightning(Game& game);
    friend void firePlasma(Game& game);
    friend void fireGhost(Game& game);
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
    friend void activateScannerSweep(Game& game);
    friend void activateEMPBlast(Game& game);
    friend void activateTacNuke(Game& game);
    friend void cancelNukePaint(Game& game);
    friend void detonateTacNuke(Game& game);
    friend void activateCloak(Game& game);
    friend void updateSpecials(float dt, Game& game);
    friend void renderSpecialEffects(Game& game);
    friend void renderSpecialsHUD(Game& game);
    friend void renderGhostEnergyBar(Game& game);
    friend void addGhostEnergy(Game& game, float amount);
    friend bool spendGhostEnergy(Game& game, float amount);
    friend void breakCloak(Game& game);
    friend void grantKillCloak(Game& game);
    friend void notifyPlayerHit(Game& game, float damage);
    friend void registerBotKill(Game& game, Entity& bot, WeaponType weapon, bool ambush, const char* feedOverride);
    
    friend void renderHUD(Game& game);
    friend void renderMinimap(Game& game);
    friend void renderUpgradeMenu(Game& game);
    friend class SpatialHash;
    friend class SavegameManager;
    friend class OverclockManager;
    friend class EchoSystem;
    friend class CoopManager;
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

    void updateMenu(float dt);
    void renderMenu();
    void renderOptions();
    void renderPauseMenu();
    void renderGameOver();
    void handleMenuInput(SDL_Event& event);
    void updateMenuItems();
    void resetGame();
    void startNewRun(Loadout kit);

    void handleMouse();

    Vec3 normalize(Vec3 v);
    float distance(Vec3 a, Vec3 b);

    Renderer* renderer_ = nullptr;
    SDL_Window* window_ = nullptr;

    GameState state = GameState::MENU;
    int menuSelection = 0;
    std::vector<std::string> menuItems;
    float stateTimer = 0;

    TextRenderer text_;

    SpatialHash* spatialHash = nullptr;
    ParticleSystem* particleSystem = nullptr;

    float shakeAmount = 0.0f;
    float shakeDecay = 5.0f;
    Vec3 shakeOffset;

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

    int scoreMultiplier = 1;
    int killStreak = 0;
    float hitFeedbackTimer = 0;
    float waveAnnounceTimer = 0;
    float multiplierTimer = 0.0f;
    const float multiplierDecay = 5.0f;
    int comboCount = 0;

    std::vector<KillFeedEntry> killFeed;
    std::vector<DamageNumber> damageNumbers;

    bool keys[SDL_NUM_SCANCODES];
    float mouseX = 0, mouseY = 0;
    bool shootRequested = false;
    bool shootLightning = false;

    float playerSpeed = 10.0f;
    float playerHeight = 1.7f;
    float mouseSensitivity = 0.002f;
    float maxHealth = 100.0f;
    float arenaSize = 40.0f;

    Loadout loadout = Loadout::ARENA;
    WeaponType currentWeapon = WeaponType::RAILGUN;
    bool adsHeld = false;
    float railgunCooldown = 0.0f;
    float lightningCooldown = 0.0f;
    const float railgunFireRate = 0.3f;
    const float lightningFireRate = 0.05f;
    float lightningRange = 15.0f;
    const float lightningDamage = 25.0f;
    int lightningChainCount = 3;

    float plasmaCooldown = 0.0f;
    float plasmaFireRate = 0.5f;
    const float plasmaDamage = 80.0f;
    const float plasmaRadius = 5.0f;
    const float plasmaSpeed = 30.0f;

    std::vector<LightningArc> lightningArcs;

    int railgunLevel = 1;
    int lightningLevel = 1;

    // Ghost sniper
    float ghostCooldown = 0.0f;
    const float ghostFireRate = GhostRules::SNIPER_MISS_LOCKOUT;
    int ghostKills = 0;
    bool ghostAmbushActive = false;
    float ghostAmbushTimer = 0.0f;
    float ghostAmbushDamageMult = GhostRules::AMBUSH_MULT;
    float lastKnownPlayerX = 0.0f;
    float lastKnownPlayerZ = 0.0f;

    // === GHOST ENERGY SYSTEM ===
    float ghostEnergy = 0.0f;
    const float GHOST_ENERGY_MAX = GhostRules::ENERGY_MAX;
    float ghostEnergyRegen = GhostRules::ENERGY_REGEN;
    float ghostEnergyFromKill = GhostRules::ENERGY_KILL;
    float ghostEnergyFromStealthKill = GhostRules::ENERGY_STEALTH_KILL;
    float ghostEnergyFromCombo = 2.0f;

    const float SCANNER_COST = GhostRules::SCANNER_COST;
    const float EMP_COST = GhostRules::EMP_COST;
    const float CLOAK_COST = GhostRules::CLOAK_COST;
    const float NUKE_COST = GhostRules::NUKE_COST;

    // Ghost specials
    float scannerCooldown = 0.0f;
    const float scannerMaxCooldown = 15.0f;
    float scannerTimer = 0.0f;

    float empCooldown = 0.0f;
    const float empMaxCooldown = 25.0f;
    float empTimer = 0.0f;
    float empStunTimer = 0.0f;
    const float EMP_STUN_DURATION = GhostRules::EMP_STUN;
    float empRadius = GhostRules::EMP_RADIUS;

    float nukeCooldown = 0.0f;
    const float nukeMaxCooldown = GhostRules::NUKE_COOLDOWN;
    float nukePaintTimer = 0.0f;
    float nukeInboundTimer = 0.0f;
    float nukeX = 0.0f;
    float nukeZ = 0.0f;
    float nukePaintOriginX = 0.0f;
    float nukePaintOriginZ = 0.0f;
    float nukeFlashTimer = 0.0f;
    float detectorSwarmTimer = 0.0f;

    float cloakCooldown = 0.0f;
    const float cloakMaxCooldown = 20.0f;
    float cloakTimer = 0.0f;
    float cloakSpeedBoost = GhostRules::CLOAK_SPEED;
    float cloakShieldTimer = 0.0f;

    // Combo system
    int ghostComboCount = 0;
    float ghostComboTimer = 0.0f;
    const float COMBO_WINDOW = 3.0f;
    const float COMBO_MAX_LEVEL = 5;
    float comboBonusDamage = 1.0f;

    // Manager specials
    float nuclearBlastCooldown = 0.0f;
    const float nuclearBlastMaxCooldown = 30.0f;
    float timeSlowCooldown = 0.0f;
    const float timeSlowMaxCooldown = 20.0f;
    float shieldCooldown = 0.0f;
    float shieldMaxCooldown = 15.0f;
    float timeSlowTimer = 0.0f;
    float shieldTimer = 0.0f;
    bool hasShield = false;

    std::vector<PowerUp> powerUps;
    float damageBoostTimer = 0.0f;

    int upgradePoints = 0;
    int healthLevel = 1;
    int speedLevel = 1;
    bool showUpgradeMenu = false;
    int upgradeSelection = 0;
    const int maxUpgradeLevel = 5;

    float railgunFeedbackChance = 0.0f;
    float plasmaOverheatPenalty = 0.0f;
    float lightningBacklashChance = 0.0f;
    float shieldCrashChance = 0.0f;
    int   splitterVirusLevel = 0;
    bool  splitterFriendlyFire = false;
    float scoreMultiplierFloat = 1.0f;
    float scoreDecayRate = 0.0f;
    float phaseGlitchChance = 0.0f;
    int   phaseShiftKills = 0;
    float phaseShiftTimer = 0.0f;

    AchievementSystem::AchievementProgress achievementProgress;
    bool tookDamageThisWave = false;
    float achievementPopupTimer = 0.0f;
    std::string achievementPopupText;
    std::string achievementPopupDesc;
    Vec3 achievementPopupColor;
    std::vector<AchievementSystem::ID> pendingAchievements;
    int killStreakSinceBonus = 0;
    int killStreakBonusGranted = 0;
    bool thermalVision = false;
    float thermalVisionTimer = 0.0f;

    bool coopActive = false;
    Entity player2;
    int player2Score = 0;
    int player2Kills = 0;
    float player2ShootCooldown = 0.0f;
    bool player2Shoot = false;
    float player2ShootTimer = 0.0f;
    
    Vec3 coopCameraEye;
    Vec3 coopCameraTarget;
    float coopCameraFov = 60.0f;

    WaveFusion currentFusion = WaveFusion::NONE;
    float arenaShrinkTimer = 0.0f;
    float fusionDisplayTimer = 0.0f;

    struct RivalGhost {
        bool active = false;
        float posX = 0, posY = 0, posZ = 0;
        float alpha = 1.0f;
        float heartbeatTimer = 0.0f;
        int rivalScore = 0;
        int rivalWave = 0;
    };
    RivalGhost rivalGhost;
    OverclockManager* overclock = nullptr;
    EchoSystem* echoSystem = nullptr;
};
