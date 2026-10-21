#include <cassert>
#include <cstdio>
#include "../src/balancing.h"

int main() {
    const BalancingDB& db = BalancingDB::get();
    auto r = db.analyze();
    assert(r.totalDpsP1 > 0);
    assert(r.totalDpsP10 >= r.totalDpsP1);
    assert(r.timeToKillW1 > 0);
    assert(r.survivalScoreW10 > 0);
    assert(r.recommendedBotCap > 0);
    printf("[PASS] balancing_analyze_sane\n");
    // weapon lookup
    assert(db.getWeapon("Railgun") != nullptr);
    assert(db.getWeapon("Nonexistent") == nullptr);
    printf("[PASS] balancing_weapon_lookup\n");
    // wave difficulty monotonic-ish
    float d1 = db.getDifficulty(1);
    float d10 = db.getDifficulty(10);
    assert(d10 >= d1);
    printf("[PASS] balancing_difficulty_monotonic\n");
    printf("=== Balancing Tests: 3 passed, 0 failed ===\n");
    return 0;
}
