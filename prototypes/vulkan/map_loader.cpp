// NEON ARENA - Vulkan + SDL2 Prototype
// Map loader implementation
#include "map_loader.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>

std::string MapLoader::readFile(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return "";
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::string buf(size, '\0');
    fread(&buf[0], 1, size, f);
    fclose(f);
    return buf;
}

bool MapLoader::loadFromJSON(const char* path, MapData& outMap) {
    std::string json = readFile(path);
    if (json.empty()) {
        fprintf(stderr, "Failed to load map: %s\n", path);
        return false;
    }
    return parseJSON(json.c_str(), outMap);
}

// Simple JSON parser for our map format
bool MapLoader::parseJSON(const char* json, MapData& outMap) {
    // For now, just generate default arena
    // Full JSON parsing would require a library or manual parser
    generateDefaultArena(outMap);
    return true;
}

void MapLoader::generateDefaultArena(MapData& outMap) {
    // Create a simple arena: floor + 4 walls
    outMap.vertices.clear();
    outMap.indices.clear();
    outMap.entities.clear();
    
    // Floor (large quad)
    MapVertex floor[4];
    float size = 24.0f;
    float y = 0.0f;
    
    // Floor vertices
    floor[0] = {{-size, y, -size}, {0, 1, 0}, {0, 0}, {0.03f, 0.04f, 0.08f, 1}};
    floor[1] = {{ size, y, -size}, {0, 1, 0}, {1, 0}, {0.03f, 0.04f, 0.08f, 1}};
    floor[2] = {{ size, y,  size}, {0, 1, 0}, {1, 1}, {0.03f, 0.04f, 0.08f, 1}};
    floor[3] = {{-size, y,  size}, {0, 1, 0}, {0, 1}, {0.03f, 0.04f, 0.08f, 1}};
    
    uint32_t baseIndex = (uint32_t)outMap.vertices.size();
    for (int i = 0; i < 4; i++) outMap.vertices.push_back(floor[i]);
    
    outMap.indices.push_back(baseIndex + 0);
    outMap.indices.push_back(baseIndex + 1);
    outMap.indices.push_back(baseIndex + 2);
    outMap.indices.push_back(baseIndex + 0);
    outMap.indices.push_back(baseIndex + 2);
    outMap.indices.push_back(baseIndex + 3);
    
    // Walls
    float wallHeight = 6.0f;
    float wallColor[4] = {0.8f, 0.2f, 0.6f, 1};
    
    MapVertex walls[16];
    // Front wall
    walls[0] = {{-size, y, -size}, {0, 0, 1}, {0, 0}, {0.8f, 0.2f, 0.6f, 1}};
    walls[1] = {{ size, y, -size}, {0, 0, 1}, {1, 0}, {0.8f, 0.2f, 0.6f, 1}};
    walls[2] = {{ size, y + wallHeight, -size}, {0, 0, 1}, {1, 1}, {0.8f, 0.2f, 0.6f, 1}};
    walls[3] = {{-size, y + wallHeight, -size}, {0, 0, 1}, {0, 1}, {0.8f, 0.2f, 0.6f, 1}};
    
    baseIndex = (uint32_t)outMap.vertices.size();
    for (int i = 0; i < 4; i++) outMap.vertices.push_back(walls[i]);
    outMap.indices.push_back(baseIndex + 0);
    outMap.indices.push_back(baseIndex + 1);
    outMap.indices.push_back(baseIndex + 2);
    outMap.indices.push_back(baseIndex + 0);
    outMap.indices.push_back(baseIndex + 2);
    outMap.indices.push_back(baseIndex + 3);
    
    // Player spawn
    MapEntity playerSpawn;
    playerSpawn.classname = "info_player_deathmatch";
    playerSpawn.origin[0] = 0;
    playerSpawn.origin[1] = 2.0f;
    playerSpawn.origin[2] = 0;
    outMap.entities.push_back(playerSpawn);
    outMap.playerSpawns.push_back({0, 2.0f, 0});
    
    outMap.boundsMin[0] = -size;
    outMap.boundsMin[1] = 0;
    outMap.boundsMin[2] = -size;
    outMap.boundsMax[0] = size;
    outMap.boundsMax[1] = wallHeight;
    outMap.boundsMax[2] = size;
}

std::vector<Vertex> MapLoader::extractGeometry(const MapData& map) {
    std::vector<Vertex> verts;
    for (uint32_t i = 0; i < map.indices.size(); i++) {
        const MapVertex& mv = map.vertices[map.indices[i]];
        Vertex v;
        v.pos = {mv.pos[0], mv.pos[1], mv.pos[2]};
        v.color = {mv.color[0], mv.color[1], mv.color[2]};
        verts.push_back(v);
    }
    return verts;
}

void MapLoader::extractCollision(const MapData& map, std::vector<Vec3>& outBoxes) {
    // Simple AABB collision boxes
    outBoxes.clear();
    // Add walls as collision boxes
    // For now, just arena bounds
}
