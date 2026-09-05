#include "echo.h"
#include <algorithm>
#include <cmath>

EchoSystem::EchoSystem() : writeIndex(0), frameCount(0), playIndex(0),
    recording(false), playing(false), active(false), lifetimeRemaining(0.0f),
    boostTimer(0.0f) {
}

EchoSystem::~EchoSystem() {
    shutdown();
}

void EchoSystem::init() {
    reset();
}

void EchoSystem::shutdown() {
    reset();
}

void EchoSystem::reset() {
    for (int i = 0; i < MAX_ECHO_FRAMES; i++) {
        frames[i].active = false;
    }
    writeIndex = 0;
    frameCount = 0;
    playIndex = 0;
    recording = false;
    playing = false;
    active = false;
    lifetimeRemaining = 0.0f;
    boostTimer = 0.0f;
    ghost.active = false;
    ghost.alpha = 0.0f;
    effects.clear();
}

void EchoSystem::startRecording() {
    recording = true;
    writeIndex = 0;
    frameCount = 0;
}

void EchoSystem::stopRecording() {
    recording = false;
}

void EchoSystem::recordFrame(float x, float y, float z, float vx, float vy, float vz, float yaw, float pitch, float gameTime) {
    if (!recording) return;

    EchoFrame& frame = frames[writeIndex];
    frame.posX = x;
    frame.posY = y;
    frame.posZ = z;
    frame.velX = vx;
    frame.velY = vy;
    frame.velZ = vz;
    frame.yaw = yaw;
    frame.pitch = pitch;
    frame.speed = std::sqrt(vx * vx + vy * vy + vz * vz);
    frame.gameTime = gameTime;
    frame.active = true;

    writeIndex = (writeIndex + 1) % MAX_ECHO_FRAMES;
    if (frameCount < MAX_ECHO_FRAMES) frameCount++;
}

void EchoSystem::play() {
    if (frameCount == 0) return;
    playing = true;
    active = true;
    playIndex = 0;
    lifetimeRemaining = ECHO_LIFETIME;
    ghost.active = true;
    ghost.alpha = 1.0f;
}

void EchoSystem::pause() {
    playing = false;
}

void EchoSystem::update(float dt) {
    if (!active) return;

    // Lifetime decay
    lifetimeRemaining -= dt;
    if (lifetimeRemaining <= 0.0f) {
        active = false;
        playing = false;
        ghost.active = false;
        return;
    }

    // Playback
    if (playing && frameCount > 0) {
        updateGhost();
        playIndex = (playIndex + 1) % frameCount;
    }

    // Boost timer
    if (boostTimer > 0.0f) {
        boostTimer -= dt;
        if (boostTimer <= 0.0f) boostTimer = 0.0f;
    }

    // Effects
    updateEffects(dt);

    // Ghost alpha fade (letzte 3 Sekunden ausblenden)
    if (lifetimeRemaining < 3.0f) {
        ghost.alpha = lifetimeRemaining / 3.0f;
    }
}

void EchoSystem::updateGhost() {
    if (frameCount == 0) return;

    int attempts = 0;
    while (attempts < MAX_ECHO_FRAMES && !frames[playIndex].active) {
        playIndex = (playIndex + 1) % MAX_ECHO_FRAMES;
        attempts++;
    }

    if (!frames[playIndex].active) return;

    const EchoFrame& frame = frames[playIndex];
    ghost.posX = frame.posX;
    ghost.posY = frame.posY;
    ghost.posZ = frame.posZ;
    ghost.yaw = frame.yaw;
    ghost.pitch = frame.pitch;
}

void EchoSystem::updateEffects(float dt) {
    for (int i = (int)effects.size() - 1; i >= 0; i--) {
        effects[i].timer += dt;
        if (effects[i].timer >= effects[i].duration) {
            effects.erase(effects.begin() + i);
        }
    }
}

bool EchoSystem::checkPlayerBoost(float playerX, float playerY, float playerZ, float& playerSpeed) {
    if (!active || !ghost.active) return false;

    float dx = playerX - ghost.posX;
    float dy = playerY - ghost.posY;
    float dz = playerZ - ghost.posZ;
    float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

    if (dist < ECHO_RADIUS && dy > 0 && dy < 48.0f) {
        boostTimer = BOOST_DURATION;
        playerSpeed *= BOOST_MULTIPLIER;
        ghost.active = false;
        ghost.alpha = 0.0f;
        effects.push_back(EchoEffect(EchoEffectType::PLAYER_BOOST, ghost.posX, ghost.posY, ghost.posZ));
        triggerCount++;
        return true;
    }
    return false;
}

void EchoSystem::checkBotCollision(float botX, float botY, float botZ, int botIndex) {
    if (!active || !ghost.active) return;

    float dx = botX - ghost.posX;
    float dy = botY - ghost.posY;
    float dz = botZ - ghost.posZ;
    float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

    if (dist < ECHO_RADIUS) {
        ghost.active = false;
        ghost.alpha = 0.0f;
        effects.push_back(EchoEffect(EchoEffectType::BOT_STUN, ghost.posX, ghost.posY, ghost.posZ));
    }
}
