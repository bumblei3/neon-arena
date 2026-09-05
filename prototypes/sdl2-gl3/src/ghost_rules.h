#pragma once
#include <cmath>

// Pure Ghost-kit rules. No Game/SDL dependency — unit-tested in test_ghost.cpp.
namespace GhostRules {

constexpr float ENERGY_MAX = 100.0f;
constexpr float ENERGY_START = 40.0f;
constexpr float ENERGY_REGEN = 3.0f;
constexpr float ENERGY_KILL = 15.0f;
constexpr float ENERGY_STEALTH_KILL = 30.0f;

constexpr float SCANNER_COST = 25.0f;
constexpr float EMP_COST = 35.0f;
constexpr float CLOAK_COST = 40.0f;
constexpr float NUKE_COST = 80.0f;

constexpr float SCANNER_RADIUS = 25.0f;
constexpr float SCANNER_MARK = 3.0f;
constexpr float SCANNER_MARK_STEALTH = 5.0f;
constexpr float SCANNER_VISUAL = 0.5f;

constexpr float EMP_RADIUS = 15.0f;
constexpr float EMP_STUN = 1.5f;
constexpr float EMP_VISUAL = 0.6f;

constexpr float CLOAK_DURATION = 5.0f;
constexpr float CLOAK_KILL_DURATION = 2.0f;
constexpr float CLOAK_SPEED = 1.3f;
constexpr float CLOAK_BREAK_RANGE = 2.0f;
constexpr float AMBUSH_WINDOW = 2.0f;
constexpr float AMBUSH_MULT = 2.0f;

constexpr float SNIPER_DAMAGE = 200.0f;
constexpr float SNIPER_RANGE = 120.0f;
constexpr float SNIPER_HIT_RADIUS = 0.85f;
constexpr float SNIPER_MARKED_RADIUS = 1.35f;
constexpr float SNIPER_MISS_LOCKOUT = 4.0f;
constexpr float SNIPER_HIT_LOCKOUT = 1.5f;
constexpr float SNIPER_KILL_LOCKOUT = 0.8f;

constexpr float ADS_FOV = 0.65f;
constexpr float HIP_FOV = 1.1f;

constexpr float NUKE_PAINT = 1.5f;
constexpr float NUKE_INBOUND = 4.0f;
constexpr float NUKE_RADIUS = 18.0f;
constexpr float NUKE_BOSS_DAMAGE = 400.0f;
constexpr float NUKE_TRASH_DAMAGE = 10000.0f;
constexpr float NUKE_SELF_DAMAGE = 40.0f;
constexpr float NUKE_COOLDOWN = 45.0f;
constexpr float NUKE_MOVE_CANCEL = 0.6f;
constexpr float NUKE_MAX_AIM = 50.0f;

constexpr float DETECTOR_RANGE = 12.0f;
constexpr float DETECTOR_CONE_COS = 0.766f; // ~40° half-angle
constexpr float DETECTOR_SWARM = 4.0f;
constexpr int DETECTOR_WAVE = 8;

inline bool canActivate(float energy, float cost, float cooldown) {
    return cooldown <= 0.0f && energy >= cost;
}

inline float clampEnergy(float energy) {
    if (energy < 0.0f) return 0.0f;
    if (energy > ENERGY_MAX) return ENERGY_MAX;
    return energy;
}

inline float addEnergy(float energy, float amount) {
    return clampEnergy(energy + amount);
}

inline float spendEnergy(float energy, float cost) {
    return clampEnergy(energy - cost);
}

inline float sniperLockout(bool hit, bool killed) {
    if (!hit) return SNIPER_MISS_LOCKOUT;
    if (killed) return SNIPER_KILL_LOCKOUT;
    return SNIPER_HIT_LOCKOUT;
}

inline float sniperDamage(bool ambush) {
    return SNIPER_DAMAGE * (ambush ? AMBUSH_MULT : 1.0f);
}

inline float sniperHitRadius(bool marked) {
    return marked ? SNIPER_MARKED_RADIUS : SNIPER_HIT_RADIUS;
}

inline float energyFromKill(bool stealthBot) {
    return stealthBot ? ENERGY_STEALTH_KILL : ENERGY_KILL;
}

struct HuntPos { float x, z; };

inline HuntPos huntPosition(bool cloaked, float playerX, float playerZ,
                            float lastX, float lastZ) {
    if (cloaked) return {lastX, lastZ};
    return {playerX, playerZ};
}

inline bool cloakBreaksOnProximity(float dist) {
    return dist < CLOAK_BREAK_RANGE;
}

inline float nukeDamageFor(bool isBoss) {
    return isBoss ? NUKE_BOSS_DAMAGE : NUKE_TRASH_DAMAGE;
}

inline bool inNukeRadius(float x, float z, float nukeX, float nukeZ) {
    float dx = x - nukeX, dz = z - nukeZ;
    return dx * dx + dz * dz <= NUKE_RADIUS * NUKE_RADIUS;
}

// Look-ray vs ground plane y=0, clamped to the arena.
inline bool groundAim(float ox, float oy, float oz,
                      float dx, float dy, float dz,
                      float maxRange, float arena,
                      float* gx, float* gz) {
    float t;
    if (dy < -0.05f) t = -oy / dy;
    else t = 18.0f;
    if (t < 2.0f) t = 2.0f;
    if (t > maxRange) t = maxRange;
    float x = ox + dx * t;
    float z = oz + dz * t;
    if (x < -arena) x = -arena;
    if (x > arena) x = arena;
    if (z < -arena) z = -arena;
    if (z > arena) z = arena;
    if (gx) *gx = x;
    if (gz) *gz = z;
    return true;
}

// Bot yaw matches bots.cpp: atan2(dx, -dz), forward = (sin yaw, -cos yaw).
inline bool inDetectorCone(float botX, float botZ, float yaw,
                           float px, float pz, float range = DETECTOR_RANGE) {
    float dx = px - botX, dz = pz - botZ;
    float dist = std::sqrt(dx * dx + dz * dz);
    if (dist <= 0.4f) return true;
    if (dist > range) return false;
    float fx = std::sin(yaw);
    float fz = -std::cos(yaw);
    float dot = fx * (dx / dist) + fz * (dz / dist);
    return dot >= DETECTOR_CONE_COS;
}

inline HuntPos evadeTarget(float botX, float botZ, float nukeX, float nukeZ,
                           float run = 20.0f) {
    float dx = botX - nukeX, dz = botZ - nukeZ;
    float len = std::sqrt(dx * dx + dz * dz);
    if (len < 0.01f) {
        dx = 1.0f;
        dz = 0.0f;
        len = 1.0f;
    }
    return {botX + dx / len * run, botZ + dz / len * run};
}

// Closest point on ray vs sphere. dir must be normalized.
inline bool rayHitsSphere(float ox, float oy, float oz,
                          float dx, float dy, float dz,
                          float sx, float sy, float sz,
                          float radius, float maxRange, float* outT = nullptr) {
    float tox = sx - ox, toy = sy - oy, toz = sz - oz;
    float t = tox * dx + toy * dy + toz * dz;
    if (t < 0.0f || t > maxRange) return false;
    float cx = ox + dx * t;
    float cy = oy + dy * t;
    float cz = oz + dz * t;
    float ddx = cx - sx, ddy = cy - sy, ddz = cz - sz;
    if (ddx * ddx + ddy * ddy + ddz * ddz > radius * radius) return false;
    if (outT) *outT = t;
    return true;
}

} // namespace GhostRules
