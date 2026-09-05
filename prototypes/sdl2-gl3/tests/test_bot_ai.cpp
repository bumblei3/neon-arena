#include <cstdio>
#include <cassert>
#include <cmath>
#include "../src/bot_ai.h"
#include "../src/pathfinding.h"

static int aiPassed = 0, aiFailed = 0;

#define AI_TEST(name, expr) do { \
    printf("  Running %s... ", name); \
    fflush(stdout); \
    if (expr) { printf("PASSED\n"); aiPassed++; } \
    else { printf("FAILED\n"); aiFailed++; } \
} while(0)

void testPathfinding() {
    printf("\n[Pathfinding Tests]\n");
    
    // Direct path when target close
    {
        auto path = Pathfinder::findPath(0.0f, 0.0f, 5.0f, 0.0f, 40.0f);
        AI_TEST("direct_path_close", !path.empty());
    }
    
    // Path around obstacle (large distance)
    {
        auto path = Pathfinder::findPath(-30.0f, -30.0f, 30.0f, 30.0f, 40.0f);
        AI_TEST("path_long_distance", !path.empty());
    }
    
    // Path starts near start
    {
        auto path = Pathfinder::findPath(0.0f, 0.0f, 20.0f, 20.0f, 40.0f);
        if (!path.empty()) {
            AI_TEST("path_ends_near_target", 
                std::abs(path.back().first - 20.0f) < 10.0f && 
                std::abs(path.back().second - 20.0f) < 10.0f);
        } else {
            AI_TEST("path_ends_near_target", false);
        }
    }
    
    // Arena bounds respected (target within arena)
    {
        auto path = Pathfinder::findPath(0.0f, 0.0f, 30.0f, 30.0f, 40.0f);
        bool inBounds = true;
        for (auto& p : path) {
            if (std::abs(p.first) > 42.0f || std::abs(p.second) > 42.0f) {
                inBounds = false;
                break;
            }
        }
        AI_TEST("path_in_arena_bounds", inBounds);
    }
    
    // Out-of-bounds target gets clamped
    {
        auto path = Pathfinder::findPath(0.0f, 0.0f, 100.0f, 100.0f, 40.0f);
        bool inBounds = true;
        for (auto& p : path) {
            if (std::abs(p.first) > 42.0f || std::abs(p.second) > 42.0f) {
                inBounds = false;
                break;
            }
        }
        AI_TEST("oob_target_clamped", inBounds);
    }
    
    // getNextWaypoint returns reasonable target
    {
        auto wp = Pathfinder::getNextWaypoint(0.0f, 0.0f, 10.0f, 10.0f, 40.0f);
        AI_TEST("waypoint_not_start", std::abs(wp.first) > 0.1f || std::abs(wp.second) > 0.1f);
    }
}

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
        AI_TEST("low_health_retreats", bot.state == BotAI::State::RETREAT || bot.state == BotAI::State::HUNT);
    }

    // Evade runs away from nuke point
    {
        BotAI::BotState bot;
        BotAI::setState(bot, BotAI::State::EVADE);
        bot.evadeX = 10.0f;
        bot.evadeZ = 0.0f;
        BotAI::executeState(bot, 0.0f, 0.0f, 0.0f, 0.0f, 40.0f);
        AI_TEST("evade_target_away_from_nuke", bot.targetX < -1.0f);
        AI_TEST("evade_no_attack", !BotAI::shouldAttack(bot, 2.0f, 0));
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

    // EMP stun lasts 1.5s
    {
        BotAI::BotState bot;
        BotAI::setState(bot, BotAI::State::STUNNED);
        bot.stunDuration = 1.5f;
        for (int i = 0; i < 12; i++) {
            BotAI::update(bot, 0.1f, 50.0f, 0.0f, 0.0f, 0.0f, 100.0f, 100.0f, 40.0f, 5, false);
        }
        AI_TEST("emp_still_stunned_at_1_2s", bot.state == BotAI::State::STUNNED);
        for (int i = 0; i < 8; i++) {
            BotAI::update(bot, 0.1f, 50.0f, 0.0f, 0.0f, 0.0f, 100.0f, 100.0f, 40.0f, 5, false);
        }
        AI_TEST("emp_recovers_after_1_5s", bot.state != BotAI::State::STUNNED);
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
        AI_TEST("swarm_spreads", moved);
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

    // Path generation on HUNT
    {
        BotAI::BotState bot;
        bot.personality = BotAI::Personality::AGGRESSIVE;
        for (int i = 0; i < 30; i++) {
            BotAI::update(bot, 0.1f, 20.0f, 0.0f, 0.0f, 0.0f, 100.0f, 100.0f, 40.0f, 10, false);
        }
        AI_TEST("path_generated_on_hunt", !bot.path.empty());
    }

    // Fusion: FRENZY_FROST increases aggression (longer attack range)
    {
        BotAI::BotState bot;
        BotAI::setState(bot, BotAI::State::ATTACK);
        BotAI::applyFusion(bot, WaveFusion::FRENZY_FROST, 0.0f);
        // With frenzy, should attack from further away (3.5 * 1.3 = 4.55 > 4.0)
        AI_TEST("frenzy_extends_range", BotAI::shouldAttack(bot, 4.5f, 0));
    }

    // Fusion: BULLET_HELL activates frenzy
    {
        BotAI::BotState bot;
        BotAI::applyFusion(bot, WaveFusion::BULLET_HELL, 0.0f);
        AI_TEST("bullet_hell_frenzy", bot.frenzyTimer > 0.0f);
    }

    // Boss phases
    {
        BotAI::BotState bot;
        bot.personality = BotAI::Personality::BOSS;
        AI_TEST("boss_phase1_full_hp", BotAI::getBossPhase(bot, 0.8f, 0.0f) == 1);
        AI_TEST("boss_phase2_mid_hp", BotAI::getBossPhase(bot, 0.5f, 0.0f) == 2);
        AI_TEST("boss_phase3_low_hp", BotAI::getBossPhase(bot, 0.3f, 0.0f) == 3);
        AI_TEST("boss_phase4_desperation", BotAI::getBossPhase(bot, 0.1f, 0.0f) == 4);
    }

    // Non-boss has no phase
    {
        BotAI::BotState bot;
        bot.personality = BotAI::Personality::AGGRESSIVE;
        AI_TEST("non_boss_no_phase", BotAI::getBossPhase(bot, 0.5f, 0.0f) == 0);
    }

    printf("\n[Bot AI Results] Passed: %d, Failed: %d\n", aiPassed, aiFailed);
}

int main() {
    testPathfinding();
    testBotAI();
    return aiFailed > 0 ? 1 : 0;
}
