// NEON ARENA - Vulkan + SDL2 Prototype
// Map loader: simple JSON-based map format
#pragma once

#include "game.h"
#include <vector>
#include <string>
#include <cstdint>

struct MapVertex {
    float pos[3];
    float normal[3];
    float uv[2];
    float color[4];
};

struct MapEntity {
    std::string classname;
    float origin[3];
    float angle;
    // Key-value pairs
    std::vector<std::pair<std::string, std::string>> keys;
};

struct MapData {
    std::vector<MapVertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<MapEntity> entities;
    
    // Arena bounds
    float boundsMin[3];
    float boundsMax[3];
    
    // Spawn points
    std::vector<Vec3> playerSpawns;
    std::vector<Vec3> enemySpawns;
};

class MapLoader {
public:
    // Load map from JSON file
    static bool loadFromJSON(const char* path, MapData& outMap);
    
    // Generate default arena map
    static void generateDefaultArena(MapData& outMap);
    
    // Convert map to renderable geometry
    static std::vector<Vertex> extractGeometry(const MapData& map);
    
    // Extract collision data (simplified AABB)
    static void extractCollision(const MapData& map, std::vector<Vec3>& outBoxes);

private:
    static bool parseJSON(const char* json, MapData& outMap);
    static std::string readFile(const char* path);
};
