// Tests for game state features: splitter, camera shake, HUD timing
#include <cstdio>
#include <cassert>
#include <cmath>
#include <vector>

// Minimal mock for new game state features
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
    float health;
    bool alive;
    int botType;
    float moveSpeed;
    int splitters;
    Entity() : pos(0, 0, 0), health(100), alive(false), botType(0), moveSpeed(3.0f), splitters(0) {}
};

struct GameState {
    Entity player;
    std::vector<Entity> bots;
    int wave = 0;
    int kills = 0;
    int score = 0;
    int scoreMultiplier = 1;
    int comboCount = 0;
    float multiplierTimer = 0;
    const float multiplierDecay = 5.0f;

    // HUD state
    float hitFeedbackTimer = 0;
    float waveAnnounceTimer = 0;
    int killStreak = 0;
    int lastKillTime = 0;

    // Camera shake
    float shakeAmount = 0.0f;
    float shakeDecay = 5.0f;
    Vec3 shakeOffset;

    void addScore(int points) {
        comboCount++; multiplierTimer = multiplierDecay;
        if (comboCount >= 10) scoreMultiplier = 5;
        else if (comboCount >= 5) scoreMultiplier = 3;
        else if (comboCount >= 3) scoreMultiplier = 2;
        score += points * scoreMultiplier;
    }

    void update(float dt) {
        if (multiplierTimer > 0) {
            multiplierTimer -= dt;
            if (multiplierTimer <= 0) { multiplierTimer = 0; comboCount = 0; scoreMultiplier = 1; }
        }
        if (hitFeedbackTimer > 0) { hitFeedbackTimer -= dt; if (hitFeedbackTimer < 0.0f) hitFeedbackTimer = 0.0f; }
        if (waveAnnounceTimer > 0) { waveAnnounceTimer -= dt; if (waveAnnounceTimer < 0.0f) waveAnnounceTimer = 0.0f; }

        // Shake decay
        if (shakeAmount > 0) {
            shakeAmount -= shakeDecay * dt;
            if (shakeAmount < 0) { shakeAmount = 0; shakeOffset = Vec3(0, 0, 0); }
        }
    }

    void onBotKill(bool isBoss = false) {
        kills++;
        killStreak++;
        addScore(isBoss ? 50 : 10);
        shakeAmount = isBoss ? 4.0f : 2.0f + botType * 1.0f;
    }

    void onPlayerHit() {
        player.health -= 20;
        shakeAmount = 3.0f;
        hitFeedbackTimer = 0.3f;
    }

    void onWaveComplete(int waveNum) {
        waveComplete = true;
        waveBreak = 0;
        killStreak = 0;
    }

    void nextWave() {
        wave++;
        waveComplete = false;
        waveBreak = 0;
        waveAnnounceTimer = 2.0f;
        // spawnWave would go here
    }

    bool waveComplete = false;
    int waveBreak = 0;
    int botType = 0;

    // Splitter mechanic
    void spawnSplitters(const Entity& deadBot) {
        if (deadBot.splitters <= 0) return;
        for (int s = 0; s < deadBot.splitters; s++) {
            Entity mini;
            float angle = (float)s / deadBot.splitters * 6.28318f;
            mini.pos = deadBot.pos + Vec3(cosf(angle) * 2.0f, 0, sinf(angle) * 2.0f);
            mini.health = 30.0f + wave * 3;
            mini.alive = true;
            mini.moveSpeed = deadBot.moveSpeed * 1.3f;
            mini.splitters = deadBot.splitters - 1;
            bots.push_back(mini);
        }
    }

    float distance(Vec3 a, Vec3 b) { return (a - b).length(); }
};

static int gsPassed = 0, gsFailed = 0;

#define GS_TEST(name, expr) do { \
    printf("  Running %s... ", name); \
    fflush(stdout); \
    if (expr) { printf("PASSED\n"); gsPassed++; } \
    else { printf("FAILED\n"); gsFailed++; } \
} while(0)

void testGameState() {
    printf("\n[Game State Feature Tests]\n");

    // --- Splitter Tests ---
    {
        GameState g;
        g.wave = 5;
        Entity boss;
        boss.pos = Vec3(10, 0, 10);
        boss.splitters = 2;
        boss.moveSpeed = 4.0f;
        g.spawnSplitters(boss);
        GS_TEST("splitter_spawns_minis", g.bots.size() == 2);
        GS_TEST("splitter_depth_decremented", g.bots[0].splitters == 1);
        GS_TEST("splitter_faster_move_speed", g.bots[0].moveSpeed > boss.moveSpeed);
    }

    {
        GameState g;
        Entity normal;
        normal.splitters = 0;
        g.spawnSplitters(normal);
        GS_TEST("splitter_zero_no_spawns", g.bots.empty());
    }

    {
        GameState g;
        g.wave = 10;
        Entity boss;
        boss.splitters = 3;
        boss.moveSpeed = 5.0f;
        g.spawnSplitters(boss);
        GS_TEST("splitter_three_spawns", g.bots.size() == 3);
        GS_TEST("splitter_depth_two", g.bots[0].splitters == 2);
        GS_TEST("splitter_depth_one", g.bots[2].splitters == 2);
    }

    // --- Camera Shake Tests ---
    {
        GameState g;
        g.onBotKill(false);
        GS_TEST("kill_triggers_shake", g.shakeAmount > 0);
    }

    {
        GameState g;
        g.onBotKill(true);
        GS_TEST("boss_kill_stronger_shake", g.shakeAmount >= 4.0f);
    }

    {
        GameState g;
        g.onPlayerHit();
        GS_TEST("player_hit_strong_shake", g.shakeAmount == 3.0f);
    }

    {
        GameState g;
        g.shakeAmount = 5.0f;
        g.update(0.1f);
        GS_TEST("shake_decays", g.shakeAmount < 5.0f);
    }

    {
        GameState g;
        g.shakeAmount = 0.5f;
        g.update(1.0f);
        GS_TEST("shake_never_negative", g.shakeAmount == 0.0f);
    }

    // --- Kill Streak Tests ---
    {
        GameState g;
        g.onBotKill(); g.onBotKill(); g.onBotKill();
        GS_TEST("kill_streak_increments", g.killStreak == 3);
    }

    {
        GameState g;
        for (int i = 0; i < 5; i++) g.onBotKill();
        GS_TEST("kill_streak_five", g.killStreak == 5);
    }

    {
        GameState g;
        g.onBotKill(); g.onBotKill(); g.onBotKill();
        g.onWaveComplete(1);
        GS_TEST("wave_complete_resets_streak", g.killStreak == 0);
    }

    // --- Hit Feedback Timer Tests ---
    {
        GameState g;
        g.onPlayerHit();
        GS_TEST("hit_feedback_set", g.hitFeedbackTimer == 0.3f);
    }

    {
        GameState g;
        g.onPlayerHit();
        g.update(0.2f);
        GS_TEST("hit_feedback_decays", g.hitFeedbackTimer <= 0.1f + 0.001f);
    }

    {
        GameState g;
        g.onPlayerHit();
        g.update(0.5f);
        GS_TEST("hit_feedback_expires", g.hitFeedbackTimer == 0.0f);
    }

    // --- Wave Announce Timer Tests ---
    {
        GameState g;
        g.nextWave();
        GS_TEST("wave_announce_set", g.waveAnnounceTimer == 2.0f);
    }

    {
        GameState g;
        g.nextWave();
        g.update(1.0f);
        GS_TEST("wave_announce_decays", g.waveAnnounceTimer == 1.0f);
    }

    {
        GameState g;
        g.nextWave();
        g.update(3.0f);
        GS_TEST("wave_announce_expires", g.waveAnnounceTimer == 0.0f);
    }

    // --- Combo Multiplier with Timer Decay ---
    {
        GameState g;
        g.addScore(10);
        g.addScore(10);
        g.addScore(10);
        GS_TEST("combo_three_multiplier_two", g.scoreMultiplier == 2);
    }

    {
        GameState g;
        g.addScore(10); g.addScore(10); g.addScore(10);
        g.update(6.0f); // combo decay
        GS_TEST("combo_decays_to_one", g.scoreMultiplier == 1);
        GS_TEST("combo_count_resets", g.comboCount == 0);
    }

    {
        GameState g;
        for (int i = 0; i < 10; i++) g.addScore(10);
        GS_TEST("combo_max_five", g.scoreMultiplier == 5);
    }

    {
        GameState g;
        g.addScore(10);
        GS_TEST("combo_one_multiplier_one", g.scoreMultiplier == 1);
    }

    printf("\n[Game State Results] Passed: %d, Failed: %d\n", gsPassed, gsFailed);
}

int main() {
    testGameState();
    return gsFailed > 0 ? 1 : 0;
}
