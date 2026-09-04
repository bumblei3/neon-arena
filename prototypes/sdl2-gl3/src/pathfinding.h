// pathfinding.h - A* pathfinding for bot navigation
#pragma once
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>

struct PathNode {
    int x, z;
    float gCost, hCost;
    int parentX, parentZ;
    bool walked;
    
    float fCost() const { return gCost + hCost; }
};

class Pathfinder {
public:
    static constexpr int GRID_SIZE = 20;
    static constexpr float CELL_SIZE = 4.0f;
    
    static std::vector<std::pair<float, float>> findPath(
        float startX, float startZ,
        float targetX, float targetZ,
        float arenaSize) 
    {
        std::vector<std::pair<float, float>> path;
        
        // Convert to grid coordinates
        int sx = worldToGrid(startX);
        int sz = worldToGrid(startZ);
        int tx = worldToGrid(targetX);
        int tz = worldToGrid(targetZ);
        
        // Bounds check — if target outside grid, clamp to arena edge
        if (sx < 0 || sx >= GRID_SIZE || sz < 0 || sz >= GRID_SIZE)
            return directPath(startX, startZ, targetX, targetZ);
        if (tx < 0 || tx >= GRID_SIZE || tz < 0 || tz >= GRID_SIZE) {
            // Clamp target to arena bounds
            float clampedX = targetX;
            float clampedZ = targetZ;
            float limit = arenaSize - 2.0f;
            if (clampedX > limit) clampedX = limit;
            if (clampedX < -limit) clampedX = -limit;
            if (clampedZ > limit) clampedZ = limit;
            if (clampedZ < -limit) clampedZ = -limit;
            return directPath(startX, startZ, clampedX, clampedZ);
        }
        
        // A* search
        PathNode grid[GRID_SIZE][GRID_SIZE];
        for (int i = 0; i < GRID_SIZE; i++)
            for (int j = 0; j < GRID_SIZE; j++) {
                grid[i][j] = {i, j, 9999.0f, 9999.0f, -1, -1, false};
            }
        
        grid[sx][sz].gCost = 0.0f;
        grid[sx][sz].hCost = heuristic(sx, sz, tx, tz);
        
        int openList[GRID_SIZE * GRID_SIZE][2];
        int openCount = 1;
        openList[0][0] = sx;
        openList[0][1] = sz;
        
        int iterations = 0;
        const int MAX_ITERATIONS = 200;
        
        while (openCount > 0 && iterations < MAX_ITERATIONS) {
            iterations++;
            
            // Find lowest fCost
            int bestIdx = 0;
            float bestF = grid[openList[0][0]][openList[0][1]].fCost();
            for (int i = 1; i < openCount; i++) {
                float f = grid[openList[i][0]][openList[i][1]].fCost();
                if (f < bestF) {
                    bestF = f;
                    bestIdx = i;
                }
            }
            
            int cx = openList[bestIdx][0];
            int cz = openList[bestIdx][1];
            
            // Remove from open
            openList[bestIdx][0] = openList[openCount-1][0];
            openList[bestIdx][1] = openList[openCount-1][1];
            openCount--;
            
            grid[cx][cz].walked = true;
            
            if (cx == tx && cz == tz) {
                // Reconstruct path
                int rx = tx, rz = tz;
                while (rx != sx || rz != sz) {
                    path.push_back({gridToWorld(rx), gridToWorld(rz)});
                    int px = grid[rx][rz].parentX;
                    int pz = grid[rx][rz].parentZ;
                    rx = px;
                    rz = pz;
                }
                std::reverse(path.begin(), path.end());
                return path;
            }
            
            // Check neighbors (8-directional)
            for (int dx = -1; dx <= 1; dx++) {
                for (int dz = -1; dz <= 1; dz++) {
                    if (dx == 0 && dz == 0) continue;
                    
                    int nx = cx + dx;
                    int nz = cz + dz;
                    
                    if (nx < 0 || nx >= GRID_SIZE || nz < 0 || nz >= GRID_SIZE)
                        continue;
                    if (grid[nx][nz].walked) continue;
                    
                    // Check arena bounds
                    float wx = gridToWorld(nx);
                    float wz = gridToWorld(nz);
                    if (std::abs(wx) > arenaSize - 2 || std::abs(wz) > arenaSize - 2)
                        continue;
                    
                    float newG = grid[cx][cz].gCost + ((dx != 0 && dz != 0) ? 1.414f : 1.0f);
                    
                    bool inOpen = false;
                    for (int i = 0; i < openCount; i++) {
                        if (openList[i][0] == nx && openList[i][1] == nz) {
                            inOpen = true;
                            break;
                        }
                    }
                    
                    if (!inOpen || newG < grid[nx][nz].gCost) {
                        grid[nx][nz].gCost = newG;
                        grid[nx][nz].hCost = heuristic(nx, nz, tx, tz);
                        grid[nx][nz].parentX = cx;
                        grid[nx][nz].parentZ = cz;
                        
                        if (!inOpen) {
                            openList[openCount][0] = nx;
                            openList[openCount][1] = nz;
                            openCount++;
                        }
                    }
                }
            }
        }
        
        return directPath(startX, startZ, targetX, targetZ);
    }
    
    static std::pair<float, float> getNextWaypoint(
        float startX, float startZ,
        float targetX, float targetZ,
        float arenaSize) 
    {
        auto path = findPath(startX, startZ, targetX, targetZ, arenaSize);
        if (path.size() > 1) {
            return path[1]; // Next waypoint after start
        } else if (!path.empty()) {
            return path[0];
        }
        return {targetX, targetZ};
    }

private:
    static int worldToGrid(float world) {
        return static_cast<int>((world + GRID_SIZE * CELL_SIZE * 0.5f) / CELL_SIZE);
    }
    
    static float gridToWorld(int grid) {
        return grid * CELL_SIZE - GRID_SIZE * CELL_SIZE * 0.5f + CELL_SIZE * 0.5f;
    }
    
    static float heuristic(int x1, int z1, int x2, int z2) {
        int dx = std::abs(x2 - x1);
        int dz = std::abs(z2 - z1);
        // Octile distance
        if (dx > dz)
            return dx + (1.414f - 1.0f) * dz;
        return dz + (1.414f - 1.0f) * dx;
    }
    
    static std::vector<std::pair<float, float>> directPath(
        float startX, float startZ,
        float targetX, float targetZ) 
    {
        return {{targetX, targetZ}};
    }
};
