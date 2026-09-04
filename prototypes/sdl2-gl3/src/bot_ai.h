#pragma once
// bot_ai.h - Bot AI with State Machine, Pathfinding, and Swarm Intelligence
#include <vector>
#include <cmath>
#include <cstdlib>

class BotAI {
public:
    // Bot behavior states
    enum class State {
        IDLE,       // No target, wandering
        HUNT,       // Target acquired, moving to attack position
        ATTACK,     // In range, attacking
        RETREAT,    // Low health, fleeing
        FLANK,      // Trying to get behind player
        STUNNED,    // Temporarily disabled
        DEAD
    };

    // Bot personality types
    enum class Personality {
        AGGRESSIVE,  // Always charges
        DEFENSIVE,   // Keeps distance, cautious
        FLANKER,     // Tries to attack from sides
        SWARM,       // Coordinates with other bots
        BOSS         // Special boss behavior
    };

    struct BotState {
        State state = State::IDLE;
        Personality personality = Personality::AGGRESSIVE;
        float stateTimer = 0.0f;
        float decisionTimer = 0.0f;
        float targetX = 0, targetZ = 0;  // Current move target
        int swarmId = -1;                // Which swarm this bot belongs to
        float aggression = 1.0f;         // 0.0 = passive, 1.0 = aggressive
        float lastDamageTaken = 0.0f;
        float lastDamageDealt = 0.0f;
    };

    static void update(BotState& bot, float dt, float playerX, float playerZ,
                       float botX, float botZ, float botHealth, float maxHealth,
                       float arenaSize, int wave, bool playerIsMoving);

    static void decideState(BotState& bot, float distToPlayer, float botHealth,
                           float maxHealth, int wave);

    static void executeState(BotState& bot, float playerX, float playerZ,
                            float botX, float botZ, float arenaSize);

    static void swarmUpdate(std::vector<BotState>& bots, float playerX, float playerZ);

    static bool shouldAttack(const BotState& bot, float distToPlayer, int botType);

    static void setState(BotState& bot, State newState);
};
