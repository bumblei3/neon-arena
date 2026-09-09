// config.h - Central constants for NeonArena Prototype
#pragma once

// --- Display ---
constexpr int DEFAULT_WINDOW_WIDTH = 1280;
constexpr int DEFAULT_WINDOW_HEIGHT = 720;
constexpr int GL_VERSION_MAJOR = 3;
constexpr int GL_VERSION_MINOR = 3;
constexpr int GL_SAMPLE_COUNT = 4;
constexpr int TARGET_FPS = 60;
constexpr float TARGET_FRAME_MS = 1000.0f / TARGET_FPS;

// --- Input ---
constexpr float DEFAULT_MOUSE_SENSITIVITY = 0.002f;
constexpr float MIN_MOUSE_SENSITIVITY = 0.0005f;
constexpr float MAX_MOUSE_SENSITIVITY = 0.005f;
constexpr float MOUSE_SENSITIVITY_STEP = 0.0005f;
constexpr int COOP_DEADZONE = 8000;

// --- Gameplay ---
constexpr float DEFAULT_PLAYER_SPEED = 10.0f;
constexpr float DEFAULT_PLAYER_HEIGHT = 1.7f;
constexpr float DEFAULT_MAX_HEALTH = 100.0f;
constexpr float DEFAULT_ARENA_SIZE = 40.0f;
constexpr int DEFAULT_START_WAVE = 1;
constexpr float WAVE_BREAK_DELAY = 3.0f;
constexpr int WAVE_ANNOUNCE_DURATION_MS = 2000;
constexpr float NEXT_WAVE_DELAY = 3.0f;

// --- Railgun ---
constexpr float RAILGUN_FIRE_RATE = 0.3f;
constexpr float RAILGUN_DAMAGE = 50.0f;
constexpr float RAILGUN_PROJECTILE_SPEED = 50.0f;
constexpr int RAILGUN_MAX_LEVEL = 5;
constexpr float RAILGUN_UPGRADE_MULT = 0.25f;

// --- Lightning Gun ---
constexpr float LIGHTNING_FIRE_RATE = 0.05f;
constexpr float LIGHTNING_DAMAGE = 25.0f;
constexpr float LIGHTNING_RANGE = 15.0f;
constexpr int LIGHTNING_CHAIN_COUNT = 3;
constexpr int LIGHTNING_MAX_LEVEL = 5;
constexpr float LIGHTNING_UPGRADE_MULT = 0.2f;

// --- Plasma Rifle ---
constexpr float PLASMA_FIRE_RATE = 0.5f;
constexpr float PLASMA_DAMAGE = 80.0f;
constexpr float PLASMA_RADIUS = 5.0f;
constexpr float PLASMA_SPEED = 30.0f;
constexpr int PLASMA_MAX_LEVEL = 5;
constexpr float PLASMA_UPGRADE_MULT = 0.3f;



// --- Combo System ---
constexpr float COMBO_WINDOW = 3.0f;
constexpr float COMBO_MAX_LEVEL = 5;
constexpr float MULTIPLIER_DECAY = 5.0f;

// --- Upgrades ---
constexpr int MAX_UPGRADE_LEVEL = 5;
constexpr float UPGRADE_COST_LOW = 1.0f;
constexpr float UPGRADE_COST_HIGH = 2.0f;
constexpr int UPGRADE_COST_THRESHOLD = 3;

// --- Upgrade Effects ---
constexpr float HEALTH_PER_LEVEL = 25.0f;
constexpr float HEALTH_MAX_BONUS = 150.0f;
constexpr float DAMAGE_PER_LEVEL = 0.10f;
constexpr int SPEED_MAX_LEVEL = 5;

// --- Power-Up Drop Rate ---
constexpr int POWERUP_DROP_CHANCE = 30; // percent

// --- Bot Balance ---
constexpr float BOT_BASE_SPEED = 3.0f;
constexpr float BOT_SPEED_INCREMENT = 0.15f;
constexpr int BOT_MAX_SKILL = 5;

// --- Special Cooldowns ---
constexpr float SPECIAL_NUKE_COOLDOWN = 30.0f;
constexpr float SPECIAL_TIME_SLOW_COOLDOWN = 20.0f;
constexpr float SPECIAL_SHIELD_COOLDOWN = 15.0f;

// --- Score ---
constexpr int SCORE_PER_KILL = 10;
constexpr float SCORE_DECAY_PER_SEC = 60.0f;

// --- Arena ---
constexpr float ARENA_MAX_SIZE = 60.0f;
constexpr int SCORE_PER_KILL_FEED = 10;

// --- Visual ---
constexpr float CAMERA_SMOOTHING = 5.0f;
constexpr float CAMERA_DISTANCE = 8.0f;
constexpr float CAMERA_HEIGHT = 5.0f;
constexpr float SHAKE_DECAY = 5.0f;

// --- Audio ---
constexpr int AUDIO_RATE = 44100;
constexpr int AUDIO_CHANNELS = 2;
constexpr int AUDIO_CHUNKSIZE = 2048;

// --- Replay ---
constexpr int REPLAY_MAX_EVENTS = 10000;
constexpr float REPLAY_TICK_RATE = 1.0f / 60.0f;

// --- Perf Profiler ---
constexpr int PERF_MAX_SAMPLES = 300;
constexpr float PERF_UPDATE_INTERVAL = 0.5f;


constexpr int SEASONAL_WEEKS = 16;
constexpr int SEASONAL_TOP_N = 5;
