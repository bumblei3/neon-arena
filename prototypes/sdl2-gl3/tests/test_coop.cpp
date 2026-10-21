#include <cassert>
#include <cstdio>
#include "../src/coop.h"
#include "../src/math.h"

int main() {
    CoopManager::init();
    CoopManager::setCoopActive(true);
    assert(CoopManager::isCoopActive());
    Vec3 eye, center; float fov;
    CoopManager::calculateSharedCamera(eye, center, fov, Vec3(0,0,0), Vec3(10,0,0), 50.0f, 1.33f);
    // eye should be between players
    assert(eye.x > 0 && eye.x < 10);
    printf("[PASS] coop_shared_camera\n");
    auto& r = CoopManager::getRevive();
    assert(!r.playerDown);
    assert(r.reviveDuration > 0);
    printf("[PASS] coop_revive_state\n");
    CoopManager::shutdown();
    printf("=== Coop Tests: 2 passed, 0 failed ===\n");
    return 0;
}
