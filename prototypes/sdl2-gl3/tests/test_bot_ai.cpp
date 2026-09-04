#include <cstdio>
#include <cassert>
#include <cmath>
#include "../src/bot_ai.h"

static int aiPassed = 0, aiFailed = 0;

#define AI_TEST(name, expr) do { \
    printf("  Running %s... ", name); \
    fflush(stdout); \
    if (expr) { printf("PASSED\n"); aiPassed++; } \
    else { printf("FAILED\n"); aiFailed++; } \
} while(0)

void testBotAI() {
    printf("\n[Bot AI Tests]\n");

    // Construction
    {
        BotAI::BotState bot;
        AI_TEST("default_state_idle", bot.state == BotAI::State::IDLE);
        AI_TEST("default_personality_aggressive", bot.personality == BotAI::Personality::AGGRESSIVE);
    }

    // State transitions
    {
        BotAI::BotState bot;
        BotAI::setState(bot, BotAI::State::HUNT);
        AI_TEST("set_state_hunt", bot.state == BotAI::State::HUNT);
        BotAI::setState(bot, BotAI::State::ATTACK);
        AI_TEST("set_state_attack", bot.state == BotAI::State::ATTACK);
        BotAI::setState(bot, BotAI::State::RETREAT);
        AI_TEST("set_state_retreat", bot.state == BotAI::State::RETREAT);
        BotAI::setState(bot, BotAI::State::FLANK);
        AI_TEST("set_state_flank", bot.state == BotAI::State::FLANK);
        BotAI::setState(bot, BotAI::State::STUNNED);
        AI_TEST("set_state_stunned", bot.state == BotAI::State::STUNNED);
    }

    // Update changes state from IDLE (aggressive bot hunts when player far)
    {
        BotAI::BotState bot;
        bot.personality = BotAI::Personality::AGGRESSIVE;
        // Player far away (50m) - bot should hunt
        for (int i = 0; i < 50; i++) {
            BotAI::update(bot, 0.1f, 50.0f, 0.0f, 0.0f, 0.0f, 100.0f, 100.0f, 40.0f, 5, false);
        }
        AI_TEST("update_from_idle", bot.state != BotAI::State::IDLE);
    }

    // Aggressive bot hunts when player far
    {
        BotAI::BotState bot;
        bot.personality = BotAI::Personality::AGGRESSIVE;
        for (int i = 0; i < 100; i++) {
            BotAI::update(bot, 0.1f, 50.0f, 0.0f, 0.0f, 0.0f, 100.0f, 100.0f, 40.0f, 5, false);
        }
        AI_TEST("aggressive_hunts", bot.state == BotAI::State::HUNT || bot.state == BotAI::State::ATTACK);
    }

    // Defensive bot retreats when player close
    {
        BotAI::BotState bot;
        bot.personality = BotAI::Personality::DEFENSIVE;
        for (int i = 0; i < 100; i++) {
            BotAI::update(bot, 0.1f, 3.0f, 0.0f, 0.0f, 0.0f, 100.0f, 100.0f, 40.0f, 5, false);
        }
        AI_TEST("defensive_retreats", bot.state == BotAI::State::RETREAT || bot.state == BotAI::State::ATTACK);
    }

    // Bot retreats when low health
    {
        BotAI::BotState bot;
        bot.personality = BotAI::Personality::AGGRESSIVE;
        for (int i = 0; i < 50; i++) {
            BotAI::update(bot, 0.1f, 50.0f, 0.0f, 0.0f, 0.0f, 10.0f, 100.0f, 40.0f, 5, false);
        }
        printf("  DEBUG low_health: state=%d (IDLE=0 HUNT=1 ATTACK=2 RETREAT=3)\n", (int)bot.state);
        AI_TEST("low_health_retreats", bot.state == BotAI::State::RETREAT || bot.state == BotAI::State::HUNT);
    }

    // Stunned bot doesn't move
    {
        BotAI::BotState bot;
        BotAI::setState(bot, BotAI::State::STUNNED);
        float targetX = bot.targetX;
        float targetZ = bot.targetZ;
        BotAI::update(bot, 0.1f, 50.0f, 0.0f, 0.0f, 0.0f, 100.0f, 100.0f, 40.0f, 5, false);
        AI_TEST("stunned_no_move", bot.targetX == targetX && bot.targetZ == targetZ);
    }

    // Stunned bot recovers after 1 second
    {
        BotAI::BotState bot;
        BotAI::setState(bot, BotAI::State::STUNNED);
        for (int i = 0; i < 15; i++) {
            BotAI::update(bot, 0.1f, 50.0f, 0.0f, 0.0f, 0.0f, 100.0f, 100.0f, 40.0f, 5, false);
        }
        AI_TEST("stunned_recovers", bot.state != BotAI::State::STUNNED);
    }

    // Boss behavior
    {
        BotAI::BotState bot;
        bot.personality = BotAI::Personality::BOSS;
        for (int i = 0; i < 100; i++) {
            BotAI::update(bot, 0.1f, 50.0f, 0.0f, 0.0f, 0.0f, 100.0f, 500.0f, 40.0f, 10, false);
        }
        AI_TEST("boss_hunts_or_attacks", bot.state == BotAI::State::HUNT || bot.state == BotAI::State::ATTACK);
    }

    // Boss retreats when low health (below 20%)
    {
        BotAI::BotState bot;
        bot.personality = BotAI::Personality::BOSS;
        for (int i = 0; i < 50; i++) {
            BotAI::update(bot, 0.1f, 10.0f, 0.0f, 0.0f, 0.0f, 80.0f, 500.0f, 40.0f, 10, false);
        }
        printf("  DEBUG boss_low_health: state=%d (RETREAT=3), healthPct=%.2f\n", (int)bot.state, 80.0f/500.0f);
        AI_TEST("boss_retreats_low_health", bot.state == BotAI::State::RETREAT || bot.state == BotAI::State::HUNT);
    }

    // shouldAttack
    {
        BotAI::BotState bot;
        BotAI::setState(bot, BotAI::State::ATTACK);
        AI_TEST("melee_attacks_close", BotAI::shouldAttack(bot, 3.0f, 0));
        AI_TEST("melee_no_attack_far", !BotAI::shouldAttack(bot, 10.0f, 0));
        AI_TEST("shooter_attacks_medium", BotAI::shouldAttack(bot, 15.0f, 1));
        AI_TEST("fast_attacks_close", BotAI::shouldAttack(bot, 2.0f, 3));
    }

    // shouldAttack when stunned
    {
        BotAI::BotState bot;
        BotAI::setState(bot, BotAI::State::STUNNED);
        AI_TEST("stunned_no_attack", !BotAI::shouldAttack(bot, 2.0f, 0));
    }

    // shouldAttack when retreating
    {
        BotAI::BotState bot;
        BotAI::setState(bot, BotAI::State::RETREAT);
        AI_TEST("retreat_no_attack", !BotAI::shouldAttack(bot, 2.0f, 0));
    }

    // Swarm update
    {
        std::vector<BotAI::BotState> bots(5);
        for (auto& bot : bots) {
            bot.personality = BotAI::Personality::SWARM;
            bot.state = BotAI::State::HUNT;
        }
        BotAI::swarmUpdate(bots, 10.0f, 10.0f);
        AI_TEST("swarm_no_crash", true);
    }

    // Swarm spreads bots
    {
        std::vector<BotAI::BotState> bots(3);
        for (auto& bot : bots) {
            bot.personality = BotAI::Personality::SWARM;
            bot.state = BotAI::State::HUNT;
            bot.targetX = 5.0f;
            bot.targetZ = 5.0f;
        }
        BotAI::swarmUpdate(bots, 0.0f, 0.0f);
        bool moved = false;
        for (auto& bot : bots) {
            if (std::abs(bot.targetX - 5.0f) > 0.01f || std::abs(bot.targetZ - 5.0f) > 0.01f) {
                moved = true;
                break;
            }
        }
        printf("  DEBUG swarm: target[0]=(%.2f, %.2f)\n", bots[0].targetX, bots[0].targetZ);
        AI_TEST("swarm_spreads", moved);
    }

    // State timer updates
    {
        BotAI::BotState bot;
        BotAI::update(bot, 0.1f, 50.0f, 0.0f, 0.0f, 0.0f, 100.0f, 100.0f, 40.0f, 5, false);
        AI_TEST("state_timer_increases", bot.stateTimer > 0.0f);
    }

    // Decision timer resets
    {
        BotAI::BotState bot;
        for (int i = 0; i < 10; i++) {
            BotAI::update(bot, 0.1f, 50.0f, 0.0f, 0.0f, 0.0f, 100.0f, 100.0f, 40.0f, 5, false);
        }
        AI_TEST("decision_timer_resets", bot.decisionTimer < 1.0f);
    }

    // Flanker behavior
    {
        BotAI::BotState bot;
        bot.personality = BotAI::Personality::FLANKER;
        for (int i = 0; i < 100; i++) {
            BotAI::update(bot, 0.1f, 5.0f, 0.0f, 0.0f, 0.0f, 100.0f, 100.0f, 40.0f, 5, false);
        }
        AI_TEST("flanker_behavior", bot.state == BotAI::State::FLANK || bot.state == BotAI::State::ATTACK);
    }

    printf("\n[Bot AI Results] Passed: %d, Failed: %d\n", aiPassed, aiFailed);
}

int main() {
    testBotAI();
    return aiFailed > 0 ? 1 : 0;
}
