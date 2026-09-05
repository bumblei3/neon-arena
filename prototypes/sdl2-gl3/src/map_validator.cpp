#include "map_validator.h"
#include <cstdio>
#include <cstdarg>
#include <cmath>
#include <algorithm>
#include <cstring>

void MapValidator::ValidationResult::addError(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    errors.push_back(buf);
    valid = false;
}

void MapValidator::ValidationResult::addWarning(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    warnings.push_back(buf);
}

MapValidator::ValidationResult MapValidator::validate(const MapData& map) {
    ValidationResult r;
    
    checkArenaSize(map, r);
    checkSpawnPoints(map, r);
    checkPlayerSpawns(map, r);
    checkCoverDensity(map, r);
    checkWaypoints(map, r);
    checkConnectivity(map, r);
    
    return r;
}

bool MapValidator::checkArenaSize(const MapData& map, ValidationResult& r) {
    r.arenaArea = 3.14159f * map.arenaRadius * map.arenaRadius;
    
    if (map.arenaRadius < MIN_ARENA_RADIUS) {
        r.addError("Arena too small: radius %.1f < %.1f", map.arenaRadius, MIN_ARENA_RADIUS);
        return false;
    }
    if (map.arenaRadius > MAX_ARENA_RADIUS) {
        r.addError("Arena too large: radius %.1f > %.1f", map.arenaRadius, MAX_ARENA_RADIUS);
        return false;
    }
    
    // Check that all spawns are within arena
    for (auto& s : map.spawns) {
        float d = std::sqrt(s.pos.x * s.pos.x + s.pos.y * s.pos.y);
        if (d > map.arenaRadius) {
            r.addError("Spawn %d outside arena (dist %.1f > radius %.1f)", s.id, d, map.arenaRadius);
            return false;
        }
    }
    
    return true;
}

bool MapValidator::checkSpawnPoints(const MapData& map, ValidationResult& r) {
    r.botSpawns = 0;
    r.playerSpawns = 0;
    
    for (auto& s : map.spawns) {
        if (s.isPlayer) r.playerSpawns++;
        else r.botSpawns++;
    }
    
    if (r.botSpawns < MIN_BOT_SPAWNS) {
        r.addError("Not enough bot spawns: %d < %d", r.botSpawns, MIN_BOT_SPAWNS);
        return false;
    }
    
    // Check minimum distance between bot spawns
    float minDist = 9999;
    float totalDist = 0;
    int pairs = 0;
    
    for (size_t i = 0; i < map.spawns.size(); i++) {
        if (map.spawns[i].isPlayer) continue;
        for (size_t j = i + 1; j < map.spawns.size(); j++) {
            if (map.spawns[j].isPlayer) continue;
            float d = map.spawns[i].pos.dist(map.spawns[j].pos);
            if (d < minDist) minDist = d;
            totalDist += d;
            pairs++;
        }
    }
    
    r.minSpawnDist = (minDist < 9999) ? minDist : 0;
    r.avgSpawnDist = (pairs > 0) ? totalDist / pairs : 0;
    
    if (r.minSpawnDist < MIN_SPAWN_DISTANCE) {
        r.addWarning("Bot spawns too close: %.1f < %.1f", r.minSpawnDist, MIN_SPAWN_DISTANCE);
    }
    
    return true;
}

bool MapValidator::checkPlayerSpawns(const MapData& map, ValidationResult& r) {
    if (r.playerSpawns < MIN_PLAYER_SPAWNS) {
        r.addError("No player spawns found (need at least %d)", MIN_PLAYER_SPAWNS);
        return false;
    }
    
    // Check player spawn distance from each other
    float maxDist = 0;
    std::vector<const SpawnPoint*> pspawns;
    for (auto& s : map.spawns) {
        if (s.isPlayer) pspawns.push_back(&s);
    }
    
    for (size_t i = 0; i < pspawns.size(); i++) {
        for (size_t j = i + 1; j < pspawns.size(); j++) {
            float d = pspawns[i]->pos.dist(pspawns[j]->pos);
            if (d > maxDist) maxDist = d;
        }
        // Distance from center
        float dcenter = std::sqrt(pspawns[i]->pos.x * pspawns[i]->pos.x + pspawns[i]->pos.y * pspawns[i]->pos.y);
        if (dcenter > r.maxPlayerDist) r.maxPlayerDist = dcenter;
    }
    
    if (maxDist < MIN_SPAWN_DISTANCE_PLAYER && pspawns.size() > 1) {
        r.addWarning("Player spawns very close: %.1f (coop may be chaotic)", maxDist);
    }
    
    // Check if any player spawn is at center
    for (auto s : pspawns) {
        if (s->pos.x == 0 && s->pos.y == 0) {
            r.hasCenterSpawn = true;
            r.addWarning("Player spawn at center — may be unfair");
        }
    }
    
    return true;
}

bool MapValidator::checkCoverDensity(const MapData& map, ValidationResult& r) {
    if (map.cover.empty()) {
        r.addWarning("No cover objects — arena is open");
        r.coverDensity = 0;
        return true;
    }
    
    // Density: cover per 100 unit radius circle
    float area100 = 3.14159f * 10000; // pi * 100^2
    r.coverDensity = (float)map.cover.size() / (r.arenaArea / area100);
    
    if (r.coverDensity < MIN_COVER_DENSITY) {
        r.addWarning("Low cover density: %.2f (may be too open)", r.coverDensity);
    }
    if (r.coverDensity > MAX_COVER_DENSITY) {
        r.addWarning("High cover density: %.2f (may be too cluttered)", r.coverDensity);
    }
    
    // Check cover is within arena
    for (size_t i = 0; i < map.cover.size(); i++) {
        float d = std::sqrt(map.cover[i].pos.x * map.cover[i].pos.x + map.cover[i].pos.y * map.cover[i].pos.y);
        if (d + map.cover[i].radius > map.arenaRadius) {
            r.addError("Cover %zu extends outside arena (dist %.1f + radius %.1f > %.1f)",
                      i, d, map.cover[i].radius, map.arenaRadius);
            return false;
        }
    }
    
    return true;
}

bool MapValidator::checkWaypoints(const MapData& map, ValidationResult& r) {
    r.waypointCount = (int)map.waypoints.size();
    
    if (r.waypointCount < MIN_WAYPOINTS) {
        r.addError("Not enough waypoints: %d < %d", r.waypointCount, MIN_WAYPOINTS);
        return false;
    }
    
    // Check waypoints within arena
    for (size_t i = 0; i < map.waypoints.size(); i++) {
        float d = std::sqrt(map.waypoints[i].x * map.waypoints[i].x + map.waypoints[i].y * map.waypoints[i].y);
        if (d > map.arenaRadius) {
            r.addError("Waypoint %zu outside arena (dist %.1f > radius %.1f)",
                      i, d, map.arenaRadius);
            return false;
        }
    }
    
    return true;
}

bool MapValidator::checkConnectivity(const MapData& map, ValidationResult& r) {
    // Simple connectivity: each waypoint should be reachable from at least one other
    // within 2x arena radius / sqrt(waypoints)
    float maxStep = 2.0f * map.arenaRadius / std::sqrt((float)map.waypoints.size());
    
    if (map.waypoints.empty()) return true;
    
    int isolated = 0;
    for (size_t i = 0; i < map.waypoints.size(); i++) {
        bool hasNeighbor = false;
        for (size_t j = 0; j < map.waypoints.size(); j++) {
            if (i == j) continue;
            if (map.waypoints[i].dist(map.waypoints[j]) < maxStep * 2) {
                hasNeighbor = true;
                break;
            }
        }
        if (!hasNeighbor) isolated++;
    }
    
    if (isolated > 0) {
        r.addWarning("%d isolated waypoints (no neighbors within %.1f units)", isolated, maxStep * 2);
    }
    
    return true;
}

MapValidator::MapData MapValidator::generateDefaultMap(const char* name, float radius) {
    MapValidator::MapData map;
    map.name = name;
    map.arenaRadius = radius;
    
    // Player spawns (4 corners-ish)
    map.spawns.push_back({Vec2(0, 0), true, 0});  // center
    map.spawns.push_back({Vec2(radius * 0.7f, 0), true, 1});
    map.spawns.push_back({Vec2(-radius * 0.7f, 0), true, 2});
    map.spawns.push_back({Vec2(0, radius * 0.7f), true, 3});
    
    // Bot spawns around perimeter
    int numBotSpawns = 8;
    for (int i = 0; i < numBotSpawns; i++) {
        float angle = 2.0f * 3.14159f * i / numBotSpawns;
        map.spawns.push_back({Vec2(radius * 0.9f * cosf(angle), radius * 0.9f * sinf(angle)), false, 100 + i});
    }
    
    // Waypoints (grid pattern)
    int gridN = (int)std::sqrt(radius / 5);
    if (gridN < 2) gridN = 2;
    float step = radius * 1.6f / gridN;
    for (int ix = -gridN; ix <= gridN; ix++) {
        for (int iy = -gridN; iy <= gridN; iy++) {
            float x = ix * step;
            float y = iy * step;
            if (x*x + y*y < radius*radius) {
                map.waypoints.push_back(Vec2(x, y));
            }
        }
    }
    
    // Cover objects (scattered)
    int numCover = (int)(radius / 10);
    for (int i = 0; i < numCover; i++) {
        float angle = 2.0f * 3.14159f * i / numCover;
        float dist = radius * 0.3f + (i % 3) * radius * 0.2f;
        map.cover.push_back({Vec2(dist * cosf(angle), dist * sinf(angle)), 2.0f});
    }
    
    return map;
}

void MapValidator::printReport(const ValidationResult& r) {
    printf("\n=== Map Validation Report ===\n");
    printf("Status: %s\n", r.valid ? "\033[1;32mVALID\033[0m" : "\033[1;31mINVALID\033[0m");
    printf("  Player Spawns:  %d\n", r.playerSpawns);
    printf("  Bot Spawns:     %d\n", r.botSpawns);
    printf("  Min Spawn Dist: %.1f\n", r.minSpawnDist);
    printf("  Avg Spawn Dist: %.1f\n", r.avgSpawnDist);
    printf("  Waypoints:      %d\n", r.waypointCount);
    printf("  Cover Density:  %.2f\n", r.coverDensity);
    printf("  Arena Area:     %.0f\n", r.arenaArea);
    
    if (!r.errors.empty()) {
        printf("\n  \033[1;31mErrors:\033[0m\n");
        for (auto& e : r.errors) printf("    - %s\n", e.c_str());
    }
    if (!r.warnings.empty()) {
        printf("\n  \033[1;33mWarnings:\033[0m\n");
        for (auto& w : r.warnings) printf("    - %s\n", w.c_str());
    }
    printf("============================\n\n");
}
