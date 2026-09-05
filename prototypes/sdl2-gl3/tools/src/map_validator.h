#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>

// MapValidator: Validates arena maps for NeonArena
// Checks: arena size, spawn points, player spawns, cover density, connectivity

class MapValidator {
public:
    struct Vec2 {
        float x, y;
        Vec2(float x = 0, float y = 0) : x(x), y(y) {}
        float dist(const Vec2& o) const { return std::sqrt((x-o.x)*(x-o.x) + (y-o.y)*(y-o.y)); }
    };
    
    struct SpawnPoint {
        Vec2 pos;
        bool isPlayer;
        int id;
    };
    
    struct CoverObject {
        Vec2 pos;
        float radius;
    };
    
    struct MapData {
        std::string name;
        float arenaRadius;
        std::vector<SpawnPoint> spawns;
        std::vector<CoverObject> cover;
        std::vector<Vec2> waypoints;
    };
    
    struct ValidationResult {
        bool valid = true;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
        
        // Metrics
        int playerSpawns = 0;
        int botSpawns = 0;
        float avgSpawnDist = 0;
        float minSpawnDist = 0;
        float coverDensity = 0;       // cover objects per 100 units radius
        float arenaArea = 0;
        int waypointCount = 0;
        bool hasCenterSpawn = false;
        float maxPlayerDist = 0;      // max distance from center for player spawns
        
        void addError(const char* fmt, ...);
        void addWarning(const char* fmt, ...);
    };
    
    // Validate a map
    static ValidationResult validate(const MapData& map);
    
    // Quick checks
    static bool checkArenaSize(const MapData& map, ValidationResult& r);
    static bool checkSpawnPoints(const MapData& map, ValidationResult& r);
    static bool checkPlayerSpawns(const MapData& map, ValidationResult& r);
    static bool checkCoverDensity(const MapData& map, ValidationResult& r);
    static bool checkWaypoints(const MapData& map, ValidationResult& r);
    static bool checkConnectivity(const MapData& map, ValidationResult& r);
    
    // Generate a default valid map
    static MapData generateDefaultMap(const char* name, float radius = 50.0f);
    
    // Print validation report
    static void printReport(const ValidationResult& r);
    
    // Configurable limits
    static constexpr float MIN_ARENA_RADIUS = 20.0f;
    static constexpr float MAX_ARENA_RADIUS = 200.0f;
    static constexpr int MIN_PLAYER_SPAWNS = 1;
    static constexpr int MIN_BOT_SPAWNS = 4;
    static constexpr int MIN_SPAWN_DISTANCE = 5.0f;
    static constexpr int MIN_SPAWN_DISTANCE_PLAYER = 10.0f;
    static constexpr float MIN_COVER_DENSITY = 0.1f;
    static constexpr float MAX_COVER_DENSITY = 5.0f;
    static constexpr int MIN_WAYPOINTS = 4;
};
