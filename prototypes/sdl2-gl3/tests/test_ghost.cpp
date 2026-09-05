#include <cstdio>
#include <cmath>
#include "../src/ghost_rules.h"

static int passed = 0, failed = 0;

#define T(name, expr) do { \
    printf("  Running %s... ", name); \
    fflush(stdout); \
    if (expr) { printf("PASSED\n"); passed++; } \
    else { printf("FAILED\n"); failed++; } \
} while (0)

int main() {
    printf("\n[Ghost Energy]\n");
    T("cannot_spend_on_cooldown",
        !GhostRules::canActivate(100.0f, GhostRules::SCANNER_COST, 1.0f));
    T("cannot_spend_if_poor",
        !GhostRules::canActivate(10.0f, GhostRules::SCANNER_COST, 0.0f));
    T("can_spend_when_ready",
        GhostRules::canActivate(40.0f, GhostRules::SCANNER_COST, 0.0f));
    T("spend_does_not_go_negative",
        GhostRules::spendEnergy(25.0f, GhostRules::SCANNER_COST) == 0.0f);
    T("energy_clamped_to_max",
        GhostRules::addEnergy(90.0f, 50.0f) == GhostRules::ENERGY_MAX);
    T("start_energy_can_cloak_or_scan",
        GhostRules::ENERGY_START >= GhostRules::SCANNER_COST &&
        GhostRules::ENERGY_START >= GhostRules::CLOAK_COST);

    printf("\n[Ghost Sniper]\n");
    T("miss_lockout_is_4s", GhostRules::sniperLockout(false, false) == 4.0f);
    T("hit_lockout_shorter", GhostRules::sniperLockout(true, false) == 1.5f);
    T("kill_lockout_shortest", GhostRules::sniperLockout(true, true) == 0.8f);
    T("ambush_doubles_damage",
        GhostRules::sniperDamage(true) == GhostRules::SNIPER_DAMAGE * 2.0f);
    T("hip_damage_is_200", GhostRules::sniperDamage(false) == 200.0f);
    T("marked_radius_wider",
        GhostRules::sniperHitRadius(true) > GhostRules::sniperHitRadius(false));
    T("stealth_kill_pays_more",
        GhostRules::energyFromKill(true) > GhostRules::energyFromKill(false));

    printf("\n[Hitscan]\n");
    {
        float t = -1.0f;
        bool hit = GhostRules::rayHitsSphere(
            0, 1, 0,  0, 0, -1,
            0, 1, -10,  0.85f, 120.0f, &t);
        T("ray_hits_bot_ahead", hit && t > 9.0f && t < 11.0f);
    }
    {
        bool hit = GhostRules::rayHitsSphere(
            0, 1, 0,  0, 0, -1,
            8, 1, -10,  0.85f, 120.0f, nullptr);
        T("ray_misses_offset_bot", !hit);
    }
    {
        bool hit = GhostRules::rayHitsSphere(
            0, 1, 0,  0, 0, -1,
            1.2f, 1, -10,  1.35f, 120.0f, nullptr);
        T("marked_radius_catches_near_miss", hit);
    }
    {
        bool hit = GhostRules::rayHitsSphere(
            0, 1, 0,  0, 0, -1,
            0, 1, 10,  0.85f, 120.0f, nullptr);
        T("ray_ignores_behind", !hit);
    }

    printf("\n[Cloak]\n");
    {
        auto hunt = GhostRules::huntPosition(true, 10, 20, 1, 2);
        T("cloaked_hunts_last_known", hunt.x == 1.0f && hunt.z == 2.0f);
    }
    {
        auto hunt = GhostRules::huntPosition(false, 10, 20, 1, 2);
        T("visible_hunts_player", hunt.x == 10.0f && hunt.z == 20.0f);
    }
    T("proximity_2m_breaks", GhostRules::cloakBreaksOnProximity(1.9f));
    T("proximity_far_holds", !GhostRules::cloakBreaksOnProximity(2.5f));

    printf("\n[Tac Nuke]\n");
    T("nuke_costs_more_than_start_energy",
        GhostRules::NUKE_COST > GhostRules::ENERGY_START);
    T("cannot_nuke_at_start",
        !GhostRules::canActivate(GhostRules::ENERGY_START, GhostRules::NUKE_COST, 0.0f));
    T("can_nuke_at_80",
        GhostRules::canActivate(80.0f, GhostRules::NUKE_COST, 0.0f));
    T("boss_survives_one_nuke",
        GhostRules::nukeDamageFor(true) < 750.0f);
    T("trash_dies_to_nuke",
        GhostRules::nukeDamageFor(false) >= 1000.0f);
    T("point_in_radius",
        GhostRules::inNukeRadius(0, 0, 0, 0));
    T("point_outside_radius",
        !GhostRules::inNukeRadius(20, 0, 0, 0));
    {
        float gx = 0, gz = 0;
        GhostRules::groundAim(0, 1.7f, 0,  0, -1, -1,  50.0f, 40.0f, &gx, &gz);
        T("ground_aim_hits_plane", fabsf(gz) > 0.5f);
    }
    {
        auto evade = GhostRules::evadeTarget(0, 0, 10, 0, 20);
        T("evade_runs_away_from_nuke", evade.x < 0.0f);
    }

    printf("\n[Detector]\n");
    {
        // Facing +X (yaw = pi/2), player in front
        float yaw = 1.570796f;
        T("detector_sees_ahead",
            GhostRules::inDetectorCone(0, 0, yaw, 8, 0));
        T("detector_misses_behind",
            !GhostRules::inDetectorCone(0, 0, yaw, -8, 0));
        T("detector_misses_far",
            !GhostRules::inDetectorCone(0, 0, yaw, 20, 0));
        T("detector_misses_side",
            !GhostRules::inDetectorCone(0, 0, yaw, 0, 8));
    }

    printf("\n=== Ghost tests: %d passed, %d failed ===\n", passed, failed);
    return failed ? 1 : 0;
}
