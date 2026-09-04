// NEON ARENA - Vulkan + SDL2 Prototype
// Map loader implementation - simplified robust parser
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

// Find a key in JSON and return pointer to its value
static const char* findKey(const char* json, const char* key) {
    std::string search = "\"";
    search += key;
    search += "\"";
    const char* p = json;
    while (p && *p) {
        p = strstr(p, search.c_str());
        if (!p) return nullptr;
        p += search.length();
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
        if (*p == ':') {
            p++;
            while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
            return p;
        }
    }
    return nullptr;
}

// Parse a vec3 from JSON array
static bool parseVec3(const char* p, float* out) {
    if (!p || *p != '[') return false;
    p++;
    for (int i = 0; i < 3; i++) {
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
        char* end;
        out[i] = strtof(p, &end);
        if (end == p) return false;
        p = end;
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
        if (i < 2) {
            if (*p != ',') return false;
            p++;
        }
    }
    if (*p != ']') return false;
    return true;
}

bool MapLoader::parseJSON(const char* json, MapData& outMap) {
    outMap.vertices.clear();
    outMap.indices.clear();
    outMap.entities.clear();
    outMap.playerSpawns.clear();
    outMap.enemySpawns.clear();

    // Parse geometry array
    const char* geomVal = findKey(json, "geometry");
    if (!geomVal || *geomVal != '[') {
        fprintf(stderr, "No geometry array found\n");
        return false;
    }

    // Simple parser: find each geometry object and extract vertices/indices
    const char* p = geomVal + 1;
    while (*p && *p != ']') {
        // Find next '{'
        while (*p && *p != '{') p++;
        if (!*p) break;
        
        // Find matching '}'
        const char* objStart = p;
        int depth = 0;
        const char* objEnd = p;
        while (*objEnd) {
            if (*objEnd == '{') depth++;
            if (*objEnd == '}') depth--;
            if (depth == 0) break;
            objEnd++;
        }
        if (depth != 0) break;
        
        // Extract object substring
        std::string obj(objStart, objEnd - objStart + 1);
        
        // Check type
        const char* typeVal = findKey(obj.c_str(), "type");
        std::string type;
        if (typeVal && *typeVal == '"') {
            typeVal++;
            while (*typeVal && *typeVal != '"') {
                type += *typeVal;
                typeVal++;
            }
        }
        
        if (type == "floor" || type == "wall") {
            // Parse vertices
            const char* vertsVal = findKey(obj.c_str(), "vertices");
            if (vertsVal && *vertsVal == '[') {
                uint32_t baseIdx = (uint32_t)outMap.vertices.size();
                const char* vp = vertsVal + 1;
                while (*vp && *vp != ']') {
                    while (*vp && *vp != '{') vp++;
                    if (!*vp) break;
                    
                    // Find vertex object end
                    const char* vStart = vp;
                    int vd = 0;
                    const char* vEnd = vp;
                    while (*vEnd) {
                        if (*vEnd == '{') vd++;
                        if (*vEnd == '}') vd--;
                        if (vd == 0) break;
                        vEnd++;
                    }
                    
                    std::string vertObj(vStart, vEnd - vStart + 1);
                    MapVertex mv = {};
                    
                    float pos[3];
                    const char* posVal = findKey(vertObj.c_str(), "pos");
                    if (posVal && parseVec3(posVal, pos)) {
                        mv.pos[0] = pos[0];
                        mv.pos[1] = pos[1];
                        mv.pos[2] = pos[2];
                    }
                    
                    float col[3];
                    const char* colVal = findKey(vertObj.c_str(), "color");
                    if (colVal && parseVec3(colVal, col)) {
                        mv.color[0] = col[0];
                        mv.color[1] = col[1];
                        mv.color[2] = col[2];
                        mv.color[3] = 1.0f;
                    }
                    
                    outMap.vertices.push_back(mv);
                    vp = vEnd + 1;
                    while (*vp && *vp != '{' && *vp != ']') vp++;
                }
                
                // Parse indices
                const char* idxVal = findKey(obj.c_str(), "indices");
                if (idxVal && *idxVal == '[') {
                    uint32_t baseIdx = (uint32_t)outMap.vertices.size() - 4;
                    const char* ip = idxVal + 1;
                    while (*ip && *ip != ']') {
                        while (*ip == ' ' || *ip == '\t' || *ip == '\n' || *ip == '\r') ip++;
                        char* end;
                        long idx = strtol(ip, &end, 10);
                        if (end == ip) break;
                        outMap.indices.push_back(baseIdx + (uint32_t)idx);
                        ip = end;
                        while (*ip == ' ' || *ip == '\t' || *ip == '\n' || *ip == '\r') ip++;
                        if (*ip == ',') ip++;
                    }
                }
            }
        } else if (type == "pillar" || type == "platform" || type == "ramp") {
            float pos[3] = {0}, size[3] = {1}, color[4] = {0.5, 0.5, 0.5, 1};
            
            const char* posVal = findKey(obj.c_str(), "pos");
            if (posVal) parseVec3(posVal, pos);
            
            const char* sizeVal = findKey(obj.c_str(), "size");
            if (sizeVal) parseVec3(sizeVal, size);
            
            const char* colVal = findKey(obj.c_str(), "color");
            if (colVal) {
                float col[3];
                if (parseVec3(colVal, col)) {
                    color[0] = col[0]; color[1] = col[1]; color[2] = col[2];
                }
            }
            
            // Generate box
            float hx = size[0] * 0.5f, hy = size[1] * 0.5f, hz = size[2] * 0.5f;
            uint32_t baseIdx = (uint32_t)outMap.vertices.size();
            
            for (int face = 0; face < 6; face++) {
                float v[4][3];
                switch (face) {
                    case 0: v[0][0]=-hx;v[0][1]=-hy;v[0][2]=hz; v[1][0]=hx;v[1][1]=-hy;v[1][2]=hz; v[2][0]=hx;v[2][1]=hy;v[2][2]=hz; v[3][0]=-hx;v[3][1]=hy;v[3][2]=hz; break;
                    case 1: v[0][0]=hx;v[0][1]=-hy;v[0][2]=-hz; v[1][0]=-hx;v[1][1]=-hy;v[1][2]=-hz; v[2][0]=-hx;v[2][1]=hy;v[2][2]=-hz; v[3][0]=hx;v[3][1]=hy;v[3][2]=-hz; break;
                    case 2: v[0][0]=-hx;v[0][1]=hy;v[0][2]=hz; v[1][0]=hx;v[1][1]=hy;v[1][2]=hz; v[2][0]=hx;v[2][1]=hy;v[2][2]=-hz; v[3][0]=-hx;v[3][1]=hy;v[3][2]=-hz; break;
                    case 3: v[0][0]=-hx;v[0][1]=-hy;v[0][2]=-hz; v[1][0]=hx;v[1][1]=-hy;v[1][2]=-hz; v[2][0]=hx;v[2][1]=-hy;v[2][2]=hz; v[3][0]=-hx;v[3][1]=-hy;v[3][2]=hz; break;
                    case 4: v[0][0]=hx;v[0][1]=-hy;v[0][2]=hz; v[1][0]=hx;v[1][1]=-hy;v[1][2]=-hz; v[2][0]=hx;v[2][1]=hy;v[2][2]=-hz; v[3][0]=hx;v[3][1]=hy;v[3][2]=hz; break;
                    case 5: v[0][0]=-hx;v[0][1]=-hy;v[0][2]=-hz; v[1][0]=-hx;v[1][1]=-hy;v[1][2]=hz; v[2][0]=-hx;v[2][1]=hy;v[2][2]=hz; v[3][0]=-hx;v[3][1]=hy;v[3][2]=-hz; break;
                }
                for (int i = 0; i < 4; i++) {
                    MapVertex mv;
                    mv.pos[0] = pos[0] + v[i][0];
                    mv.pos[1] = pos[1] + v[i][1];
                    mv.pos[2] = pos[2] + v[i][2];
                    mv.color[0] = color[0]; mv.color[1] = color[1]; mv.color[2] = color[2]; mv.color[3] = 1.0f;
                    outMap.vertices.push_back(mv);
                }
                outMap.indices.push_back(baseIdx + face * 4 + 0);
                outMap.indices.push_back(baseIdx + face * 4 + 1);
                outMap.indices.push_back(baseIdx + face * 4 + 2);
                outMap.indices.push_back(baseIdx + face * 4 + 0);
                outMap.indices.push_back(baseIdx + face * 4 + 2);
                outMap.indices.push_back(baseIdx + face * 4 + 3);
            }
        }
        
        p = objEnd + 1;
        while (*p && *p != '{' && *p != ']') p++;
    }

    // Parse entities
    const char* entVal = findKey(json, "entities");
    if (entVal && *entVal == '[') {
        const char* ep = entVal + 1;
        while (*ep && *ep != ']') {
            while (*ep && *ep != '{') ep++;
            if (!*ep) break;
            
            const char* eStart = ep;
            int ed = 0;
            const char* eEnd = ep;
            while (*eEnd) {
                if (*eEnd == '{') ed++;
                if (*eEnd == '}') ed--;
                if (ed == 0) break;
                eEnd++;
            }
            
            std::string entObj(eStart, eEnd - eStart + 1);
            MapEntity ent;
            
            const char* cnVal = findKey(entObj.c_str(), "classname");
            if (cnVal && *cnVal == '"') {
                cnVal++;
                while (*cnVal && *cnVal != '"') {
                    ent.classname += *cnVal;
                    cnVal++;
                }
            }
            
            float orig[3];
            const char* origVal = findKey(entObj.c_str(), "origin");
            if (origVal && parseVec3(origVal, orig)) {
                ent.origin[0] = orig[0];
                ent.origin[1] = orig[1];
                ent.origin[2] = orig[2];
            }
            
            outMap.entities.push_back(ent);
            
            if (ent.classname == "info_player_deathmatch") {
                outMap.playerSpawns.push_back({ent.origin[0], ent.origin[1], ent.origin[2]});
            } else if (ent.classname == "enemy_spawn") {
                outMap.enemySpawns.push_back({ent.origin[0], ent.origin[1], ent.origin[2]});
            }
            
            ep = eEnd + 1;
            while (*ep && *ep != '{' && *ep != ']') ep++;
        }
    }

    printf("Map loaded: %zu vertices, %zu indices, %zu entities\n",
           outMap.vertices.size(), outMap.indices.size(), outMap.entities.size());

    return !outMap.vertices.empty();
}

void MapLoader::generateDefaultArena(MapData& outMap) {
    outMap.vertices.clear();
    outMap.indices.clear();
    outMap.entities.clear();
    
    float size = 24.0f;
    MapVertex v;
    
    // Floor
    v = {{-size, 0, -size}, {0, 1, 0}, {0, 0}, {0.03f, 0.04f, 0.08f, 1}};
    outMap.vertices.push_back(v);
    v = {{ size, 0, -size}, {0, 1, 0}, {1, 0}, {0.03f, 0.04f, 0.08f, 1}};
    outMap.vertices.push_back(v);
    v = {{ size, 0,  size}, {0, 1, 0}, {1, 1}, {0.03f, 0.04f, 0.08f, 1}};
    outMap.vertices.push_back(v);
    v = {{-size, 0,  size}, {0, 1, 0}, {0, 1}, {0.03f, 0.04f, 0.08f, 1}};
    outMap.vertices.push_back(v);
    
    outMap.indices.push_back(0); outMap.indices.push_back(1); outMap.indices.push_back(2);
    outMap.indices.push_back(0); outMap.indices.push_back(2); outMap.indices.push_back(3);
    
    outMap.playerSpawns.push_back({0, 2.0f, 0});
}

std::vector<Vertex> MapLoader::extractGeometry(const MapData& map) {
    std::vector<Vertex> verts;
    for (uint32_t i = 0; i < map.indices.size(); i++) {
        uint32_t idx = map.indices[i];
        if (idx >= map.vertices.size()) continue;
        const MapVertex& mv = map.vertices[idx];
        Vertex v;
        v.pos = {mv.pos[0], mv.pos[1], mv.pos[2]};
        v.color = {mv.color[0], mv.color[1], mv.color[2]};
        verts.push_back(v);
    }
    return verts;
}

void MapLoader::extractCollision(const MapData& map, std::vector<Vec3>& outBoxes) {
    outBoxes.clear();
}
