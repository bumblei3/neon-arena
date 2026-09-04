#include "bot_ai.h"
#include <algorithm>
#include <cmath>

void BotAI::setState(BotState& bot, State newState) {
    if (bot.state != newState) {
        bot.state = newState;
        bot.stateTimer = 0.0f;
    }
}

void BotAI::update(BotState& bot, float dt, float playerX, float playerZ,
                   float botX, float botZ, float botHealth, float maxHealth,
                   float arenaSize, int wave, bool playerIsMoving) {
    bot.stateTimer += dt;
    bot.decisionTimer += dt;

    if (bot.state == State::STUNNED) {
        if (bot.stateTimer > 1.0f) {
            setState(bot, State::HUNT);
        }
        return;
    }

    if (bot.state == State::DEAD) return;

    float dx = playerX - botX;
    float dz = playerZ - botZ;
    float dist = std::sqrt(dx * dx + dz * dz);

    // Make decisions more frequently (every 0.2s for more responsive AI)
    if (bot.decisionTimer >= 0.2f) {
        bot.decisionTimer = 0.0f;
        decideState(bot, dist, botHealth, maxHealth, wave);
    }

    // Execute current state
    executeState(bot, playerX, playerZ, botX, botZ, arenaSize);
}

void BotAI::decideState(BotState& bot, float distToPlayer, float botHealth,
                       float maxHealth, int wave) {
    float healthPct = (maxHealth > 0.0f) ? botHealth / maxHealth : 1.0f;

    // Boss behavior
    if (bot.personality == Personality::BOSS) {
        if (healthPct < 0.2f) {
            setState(bot, State::RETREAT);
        } else if (distToPlayer > 15.0f) {
            setState(bot, State::HUNT);
        } else {
            setState(bot, State::ATTACK);
        }
        return;
    }

    // Low health → retreat (aggressive bots retreat later)
    float retreatThreshold = (bot.personality == Personality::AGGRESSIVE) ? 0.15f : 0.25f;
    if (healthPct < retreatThreshold) {
        setState(bot, State::RETREAT);
        return;
    }

    // Stunned bots recover
    if (bot.state == State::STUNNED && bot.stateTimer > 1.0f) {
        setState(bot, State::HUNT);
        return;
    }

    // Personality-based decisions
    switch (bot.personality) {
        case Personality::AGGRESSIVE:
            if (distToPlayer > 3.0f) {
                setState(bot, State::HUNT);
            } else {
                setState(bot, State::ATTACK);
            }
            break;

        case Personality::DEFENSIVE:
            if (distToPlayer > 15.0f) {
                setState(bot, State::HUNT);
            } else if (distToPlayer < 6.0f) {
                setState(bot, State::RETREAT);
            } else {
                setState(bot, State::ATTACK);
            }
            break;

        case Personality::FLANKER:
            if (distToPlayer > 12.0f) {
                setState(bot, State::HUNT);
            } else if (distToPlayer < 5.0f) {
                setState(bot, State::FLANK);
            } else {
                setState(bot, State::ATTACK);
            }
            break;

        case Personality::SWARM:
            if (distToPlayer > 18.0f) {
                setState(bot, State::HUNT);
            } else {
                setState(bot, State::ATTACK);
            }
            break;

        default:
            if (distToPlayer > 10.0f) {
                setState(bot, State::HUNT);
            } else {
                setState(bot, State::ATTACK);
            }
            break;
    }
}

void BotAI::executeState(BotState& bot, float playerX, float playerZ,
                        float botX, float botZ, float arenaSize) {
    float dx, dz, dist;

    switch (bot.state) {
        case State::IDLE:
            if (bot.stateTimer > 2.0f) {
                bot.targetX = botX + (rand() % 100 / 50.0f - 1.0f) * 5.0f;
                bot.targetZ = botZ + (rand() % 100 / 50.0f - 1.0f) * 5.0f;
                bot.stateTimer = 0.0f;
            }
            break;

        case State::HUNT:
            bot.targetX = playerX;
            bot.targetZ = playerZ;
            break;

        case State::ATTACK:
            dx = playerX - botX;
            dz = playerZ - botZ;
            dist = std::sqrt(dx * dx + dz * dz);
            if (dist > 0.01f) {
                float preferred = 4.0f;
                bot.targetX = playerX - (dx / dist) * preferred;
                bot.targetZ = playerZ - (dz / dist) * preferred;
            }
            break;

        case State::RETREAT:
            dx = playerX - botX;
            dz = playerZ - botZ;
            dist = std::sqrt(dx * dx + dz * dz);
            if (dist > 0.01f) {
                bot.targetX = botX - (dx / dist) * 10.0f;
                bot.targetZ = botZ - (dz / dist) * 10.0f;
            }
            break;

        case State::FLANK:
            dx = playerX - botX;
            dz = playerZ - botZ;
            bot.targetX = playerX + dz * 0.5f;
            bot.targetZ = playerZ - dx * 0.5f;
            break;

        case State::STUNNED:
        case State::DEAD:
            break;
    }
}

void BotAI::swarmUpdate(std::vector<BotState>& bots, float playerX, float playerZ) {
    int swarmCount = 0;
    float centerX = 0, centerZ = 0;

    for (auto& bot : bots) {
        if (bot.state != State::DEAD && bot.state != State::STUNNED) {
            centerX += bot.targetX;
            centerZ += bot.targetZ;
            swarmCount++;
        }
    }

    if (swarmCount == 0) return;

    centerX /= swarmCount;
    centerZ /= swarmCount;

    for (auto& bot : bots) {
        if (bot.state == State::DEAD || bot.state == State::STUNNED) continue;
        if (bot.personality != Personality::SWARM) continue;

        float toCenterX = bot.targetX - centerX;
        float toCenterZ = bot.targetZ - centerZ;
        float centerDist = std::sqrt(toCenterX * toCenterX + toCenterZ * toCenterZ);

        if (centerDist < 5.0f) {
            float push = 0.5f;
            // Always push outward slightly (even if centerDist is 0, give random push)
            if (centerDist > 0.01f) {
                bot.targetX += toCenterX * push;
                bot.targetZ += toCenterZ * push;
            } else {
                bot.targetX += ((rand() % 100) / 100.0f - 0.5f) * 3.0f;
                bot.targetZ += ((rand() % 100) / 100.0f - 0.5f) * 3.0f;
            }
        }
    }
}

bool BotAI::shouldAttack(const BotState& bot, float distToPlayer, int botType) {
    if (bot.state == State::STUNNED || bot.state == State::DEAD) return false;
    if (bot.state == State::RETREAT) return false;

    switch (botType) {
        case 0: return distToPlayer < 4.0f;
        case 1: return distToPlayer < 20.0f;
        case 2: return distToPlayer < 5.0f;
        case 3: return distToPlayer < 3.0f;
        case 4: return distToPlayer < 25.0f;
        default: return distToPlayer < 5.0f;
    }
}
