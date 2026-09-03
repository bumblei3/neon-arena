// game.h - Game logic for neon arena prototype
#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include "renderer.h"
#include "math.h"
#include "particle.h"
#include <vector>
#include <cmath>
#include <cstdio>
#include <string>

// Game states
enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER,
    WAVE_COMPLETE
};

// Global sound pointers (set in main.cpp)
extern Mix_Chunk* g_sndShoot;
extern Mix_Chunk* g_sndExplosion;
extern Mix_Chunk* g_sndWave;
extern Mix_Chunk* g_sndGameOver;

inline void playSnd(Mix_Chunk* snd) {
    if (snd) Mix_PlayChannel(-1, snd, 0);
}

struct Entity {
    Vec3 pos;
    float yaw;
    float pitch;
    float health;
    bool alive;
    int type;  // 0 = player, 1 = bot
    float vx, vy, vz;

    Entity() : pos(0,0,0), yaw(0), pitch(0), health(100), alive(false), type(0), vx(0), vy(0), vz(0) {}
};

struct Projectile {
    Vec3 pos;
    Vec3 dir;
    float speed;
    float life;
    bool fromPlayer;

    Projectile(Vec3 p, Vec3 d, bool player)
        : pos(p), dir(d.normalized()), speed(50.0f), life(2.0f), fromPlayer(player) {}
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

private:
    void setupArena();
    void spawnWave();
    void updatePlayer(float dt);
    void updateBots(float dt);
    void updateProjectiles(float dt);
    void updateParticles(float dt);
    void spawnExplosion(Vec3 pos, Vec3 color, int count);
    void renderParticles();
    void checkCollisions();
    void renderArena();
    void renderBots();
    void renderProjectiles();
    void renderHUD();
    void updateCamera(float dt);
    void nextWave();
    void gameOverScreen();

    // Menu system
    void updateMenu(float dt);
    void renderMenu();
    void renderPauseMenu();
    void renderGameOver();
    void handleMenuInput(SDL_Event& event);
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
    std::vector<std::string> menuItems = {"Start Game", "Options", "Quit"};
    float stateTimer = 0;

    // Game state
    Entity player;
    std::vector<Entity> bots;
    std::vector<Projectile> projectiles;
    std::vector<Particle> particles;
    int wave = 0;
    int score = 0;
    int kills = 0;
    float gameTime = 0;
    bool running = true;
    bool gameOver = false;
    bool waveComplete = false;
    float waveBreak = 0;
    float nextWaveDelay = 3.0f;

    // Input
    bool keys[SDL_NUM_SCANCODES];
    float mouseX = 0, mouseY = 0;
    bool shootRequested = false;

    // Config
    float playerSpeed = 10.0f;
    float playerHeight = 1.7f;
    float mouseSensitivity = 0.002f;
    float maxHealth = 100.0f;
    float arenaSize = 40.0f;
};
