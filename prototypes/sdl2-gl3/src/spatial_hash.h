// spatial_hash.h - Grid-based spatial hashing for collision detection
#pragma once
#include <vector>
#include <unordered_map>
#include <cmath>
#include "math.h"

// Simple spatial hash grid for O(1) neighbor lookups
class SpatialHash {
public:
    SpatialHash(float cellSize = 5.0f) : cellSize(cellSize) {}
    
    void clear() {
        grid.clear();
    }
    
    // Insert a position into the grid with associated ID
    void insert(int id, float x, float z) {
        int key = hash(x, z);
        grid[key].push_back(id);
    }
    
    // Query IDs near a position (within cell radius)
    std::vector<int> queryNearby(float x, float z, float radius = 1.5f) {
        std::vector<int> result;
        
        int minX = static_cast<int>(std::floor((x - radius) / cellSize));
        int maxX = static_cast<int>(std::floor((x + radius) / cellSize));
        int minZ = static_cast<int>(std::floor((z - radius) / cellSize));
        int maxZ = static_cast<int>(std::floor((z + radius) / cellSize));
        
        for (int cx = minX; cx <= maxX; cx++) {
            for (int cz = minZ; cz <= maxZ; cz++) {
                int key = hashCell(cx, cz);
                auto it = grid.find(key);
                if (it != grid.end()) {
                    result.insert(result.end(), it->second.begin(), it->second.end());
                }
            }
        }
        
        return result;
    }
    
    size_t getCellCount() const { return grid.size(); }
    
    float getCellSize() const { return cellSize; }
    void setCellSize(float size) { cellSize = size; }
    
private:
    float cellSize;
    std::unordered_map<int, std::vector<int>> grid;
    
    int hash(float x, float z) const {
        int ix = static_cast<int>(std::floor(x / cellSize));
        int iz = static_cast<int>(std::floor(z / cellSize));
        return hashCell(ix, iz);
    }
    
    int hashCell(int x, int z) const {
        return x * 73856093 ^ z * 19349663;
    }
};

extern SpatialHash* g_spatialHash;
