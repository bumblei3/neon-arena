// MapValidator tests - arena validation, spawn checking, cover density
#include <cstdio>
#include <cassert>
#include <cstring>
#include "map_validator.h"

using MapData = MapValidator::MapData;

static int testsPassed = 0;
static int testsFailed = 0;

#define TEST(name, expr) do { \
    if (expr) { \
        printf("  [PASS] %s\n", name); \
        testsPassed++; \
    } else { \
        printf("  [FAIL] %s\n", name); \
        testsFailed++; \
    } \
} while(0)

// === Test Default Map ===

void testDefaultMap() {
    printf("\n[Default Map Tests]\n");
    
    MapData map = MapValidator::generateDefaultMap("test_arena", 50.0f);
    
    TEST("default_name", strcmp(map.name.c_str(), "test_arena") == 0);
    TEST("default_radius", map.arenaRadius == 50.0f);
    TEST("default_player_spawns", map.spawns.size() >= 4);
    
    int playerCount = 0, botCount = 0;
    for (auto& s : map.spawns) {
        if (s.isPlayer) playerCount++;
        else botCount++;
    }
    
    TEST("default_4_players", playerCount == 4);
    TEST("default_8_bots", botCount == 8);
    TEST("default_waypoints", map.waypoints.size() >= 4);
    TEST("default_cover", map.cover.size() > 0);
}

// === Test Validation Pass ===

void testValidationPass() {
    printf("\n[Validation Pass Tests]\n");
    
    MapData map = MapValidator::generateDefaultMap("valid_arena", 50.0f);
    auto result = MapValidator::validate(map);
    
    TEST("valid_default_map", result.valid);
    TEST("no_errors", result.errors.empty());
    TEST("player_count", result.playerSpawns == 4);
    TEST("bot_count", result.botSpawns == 8);
    TEST("arena_area", result.arenaArea > 0);
}

// === Test Arena Size ===

void testArenaSize() {
    printf("\n[Arena Size Tests]\n");
    
    // Too small
    {
        MapData map = MapValidator::generateDefaultMap("tiny", 10.0f);
        auto result = MapValidator::validate(map);
        TEST("too_small_invalid", !result.valid);
    }
    
    // Too large
    {
        MapData map = MapValidator::generateDefaultMap("huge", 300.0f);
        auto result = MapValidator::validate(map);
        TEST("too_large_invalid", !result.valid);
    }
    
    // Just right
    {
        MapData map = MapValidator::generateDefaultMap("ok", 50.0f);
        auto result = MapValidator::validate(map);
        TEST("normal_size_valid", result.valid);
    }
}

// === Test No Player Spawns ===

void testNoPlayerSpawns() {
    printf("\n[No Player Spawns Test]\n");
    
    MapData map;
    map.name = "no_players";
    map.arenaRadius = 50.0f;
    
    // Only bot spawns
    map.spawns.push_back({MapValidator::Vec2(40, 0), false, 0});
    map.spawns.push_back({MapValidator::Vec2(-40, 0), false, 1});
    map.spawns.push_back({MapValidator::Vec2(0, 40), false, 2});
    map.spawns.push_back({MapValidator::Vec2(0, -40), false, 3});
    
    // Add waypoints to pass waypoint check
    map.waypoints.push_back({MapValidator::Vec2(10, 10)});
    map.waypoints.push_back({MapValidator::Vec2(-10, 10)});
    map.waypoints.push_back({MapValidator::Vec2(10, -10)});
    map.waypoints.push_back({MapValidator::Vec2(-10, -10)});
    
    auto result = MapValidator::validate(map);
    TEST("no_player_spawns_invalid", !result.valid);
    TEST("has_error_message", !result.errors.empty());
}

// === Test Spawn Distance ===

void testSpawnDistance() {
    printf("\n[Spawn Distance Tests]\n");
    
    // Too close spawns
    {
        MapData map;
        map.name = "close_spawns";
        map.arenaRadius = 50.0f;
        
        map.spawns.push_back({MapValidator::Vec2(0, 0), true, 0});
        map.spawns.push_back({MapValidator::Vec2(1, 0), false, 1});   // too close
        map.spawns.push_back({MapValidator::Vec2(2, 0), false, 2});   // too close
        map.spawns.push_back({MapValidator::Vec2(3, 0), false, 3});   // too close
        map.spawns.push_back({MapValidator::Vec2(4, 0), false, 4});   // too close
        
        map.waypoints.push_back({MapValidator::Vec2(10, 10)});
        map.waypoints.push_back({MapValidator::Vec2(-10, 10)});
        map.waypoints.push_back({MapValidator::Vec2(10, -10)});
        map.waypoints.push_back({MapValidator::Vec2(-10, -10)});
        
        auto result = MapValidator::validate(map);
        TEST("close_spawns_warning", !result.warnings.empty());
    }
    
    // Well-spaced spawns
    {
        MapData map;
        map.name = "spaced_spawns";
        map.arenaRadius = 50.0f;
        
        map.spawns.push_back({MapValidator::Vec2(0, 0), true, 0});
        map.spawns.push_back({MapValidator::Vec2(30, 0), false, 1});
        map.spawns.push_back({MapValidator::Vec2(-30, 0), false, 2});
        map.spawns.push_back({MapValidator::Vec2(0, 30), false, 3});
        map.spawns.push_back({MapValidator::Vec2(0, -30), false, 4});
        
        map.waypoints.push_back({MapValidator::Vec2(10, 10)});
        map.waypoints.push_back({MapValidator::Vec2(-10, 10)});
        map.waypoints.push_back({MapValidator::Vec2(10, -10)});
        map.waypoints.push_back({MapValidator::Vec2(-10, -10)});
        
        auto result = MapValidator::validate(map);
        TEST("spaced_spawns_valid", result.valid);
    }
}

// === Test Cover Density ===

void testCoverDensity() {
    printf("\n[Cover Density Tests]\n");
    
    // No cover
    {
        MapData map = MapValidator::generateDefaultMap("no_cover", 50.0f);
        map.cover.clear();
        auto result = MapValidator::validate(map);
        TEST("no_cover_warning", !result.warnings.empty());
        TEST("cover_density_zero", result.coverDensity == 0);
    }
    
    // Normal cover
    {
        MapData map = MapValidator::generateDefaultMap("normal_cover", 50.0f);
        auto result = MapValidator::validate(map);
        TEST("normal_cover_ok", result.coverDensity > 0);
    }
}

// === Test Waypoints ===

void testWaypoints() {
    printf("\n[Waypoint Tests]\n");
    
    // Not enough waypoints
    {
        MapData map = MapValidator::generateDefaultMap("few_wps", 50.0f);
        map.waypoints.clear();
        map.waypoints.push_back({MapValidator::Vec2(0, 0)});  // only 1
        auto result = MapValidator::validate(map);
        TEST("few_waypoints_invalid", !result.valid);
    }
    
    // Waypoint outside arena
    {
        MapData map = MapValidator::generateDefaultMap("outside_wp", 50.0f);
        map.waypoints.push_back({MapValidator::Vec2(100, 100)});  // outside!
        auto result = MapValidator::validate(map);
        TEST("outside_waypoint_invalid", !result.valid);
    }
}

// === Test Connectivity ===

void testConnectivity() {
    printf("\n[Connectivity Tests]\n");
    
    // Isolated waypoints
    {
        MapData map;
        map.name = "isolated";
        map.arenaRadius = 50.0f;
        
        map.spawns.push_back({MapValidator::Vec2(0, 0), true, 0});
        map.spawns.push_back({MapValidator::Vec2(40, 0), false, 1});
        map.spawns.push_back({MapValidator::Vec2(-40, 0), false, 2});
        map.spawns.push_back({MapValidator::Vec2(0, 40), false, 3});
        map.spawns.push_back({MapValidator::Vec2(0, -40), false, 4});
        
        // Two isolated waypoints
        map.waypoints.push_back({MapValidator::Vec2(5, 5)});
        map.waypoints.push_back({MapValidator::Vec2(10, 10)});
        map.waypoints.push_back({MapValidator::Vec2(-5, -5)});
        map.waypoints.push_back({MapValidator::Vec2(-10, -10)});
        
        auto result = MapValidator::validate(map);
        TEST("has_waypoints", result.waypointCount == 4);
    }
}

// === Test Bounds ===

void testBounds() {
    printf("\n[Bounds Tests]\n");
    
    // Cover extends outside
    {
        MapData map;
        map.name = "cover_outside";
        map.arenaRadius = 50.0f;
        
        map.spawns.push_back({MapValidator::Vec2(0, 0), true, 0});
        map.spawns.push_back({MapValidator::Vec2(40, 0), false, 1});
        map.spawns.push_back({MapValidator::Vec2(-40, 0), false, 2});
        map.spawns.push_back({MapValidator::Vec2(0, 40), false, 3});
        map.spawns.push_back({MapValidator::Vec2(0, -40), false, 4});
        
        map.waypoints.push_back({MapValidator::Vec2(5, 5)});
        map.waypoints.push_back({MapValidator::Vec2(-5, 5)});
        map.waypoints.push_back({MapValidator::Vec2(5, -5)});
        map.waypoints.push_back({MapValidator::Vec2(-5, -5)});
        
        // Cover at edge with large radius → extends outside
        map.cover.push_back({MapValidator::Vec2(48, 0), 5.0f});
        
        auto result = MapValidator::validate(map);
        TEST("cover_outside_invalid", !result.valid);
    }
}

// === Test Min/Max Constants ===

void testConstants() {
    printf("\n[Constants Tests]\n");
    
    TEST("min_radius", MapValidator::MIN_ARENA_RADIUS == 20.0f);
    TEST("max_radius", MapValidator::MAX_ARENA_RADIUS == 200.0f);
    TEST("min_player_spawns", MapValidator::MIN_PLAYER_SPAWNS == 1);
    TEST("min_bot_spawns", MapValidator::MIN_BOT_SPAWNS == 4);
    TEST("min_waypoints", MapValidator::MIN_WAYPOINTS == 4);
}

// === Test Report Printing ===

void testReportPrinting() {
    printf("\n[Report Printing Test]\n");
    
    MapData map = MapValidator::generateDefaultMap("report_test", 50.0f);
    auto result = MapValidator::validate(map);
    
    // Just make sure it doesn't crash
    MapValidator::printReport(result);
    TEST("report_no_crash", true);
    
    // Also test invalid map
    MapData bad = MapValidator::generateDefaultMap("bad", 5.0f);
    auto badResult = MapValidator::validate(bad);
    MapValidator::printReport(badResult);
    TEST("report_invalid_no_crash", true);
}

// === Main ===

int main() {
    printf("=== Map Validator Test Suite ===\n");
    
    testDefaultMap();
    testValidationPass();
    testArenaSize();
    testNoPlayerSpawns();
    testSpawnDistance();
    testCoverDensity();
    testWaypoints();
    testConnectivity();
    testBounds();
    testConstants();
    testReportPrinting();
    
    printf("\n=== Results: %d passed, %d failed ===\n", testsPassed, testsFailed);
    
    return testsFailed > 0 ? 1 : 0;
}
