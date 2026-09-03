// NEON ARENA - Vulkan + SDL2 Prototype
// Map loader implementation with JSON parsing
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

// Minimal JSON parser for our map format
static const char* skipWhitespace(const char* p) {
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    return p;
}

static const char* parseString(const char* p, std::string& out) {
    if (*p != '"') return nullptr;
    p++;
    std::string result;
    while (*p && *p != '"') {
        if (*p == '\\') {
            p++;
            switch (*p) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                default: result += *p; break;
            }
        } else {
            result += *p;
        }
        p++;
    }
    if (*p == '"') p++;
    out = result;
    return p;
}

static const char* parseNumber(const char* p, float& out) {
    char* end;
    out = strtof(p, &end);
    return end;
}

static const char* parseVec3(const char* p, float* out) {
    if (*p != '[') return nullptr;
    p++;
    for (int i = 0; i < 3; i++) {
        p = skipWhitespace(p);
        p = parseNumber(p, out[i]);
        p = skipWhitespace(p);
        if (i < 2) {
            if (*p != ',') return nullptr;
            p++;
        }
    }
    if (*p != ']') return nullptr;
    p++;
    return p;
}

static const char* parseColor(const char* p, float* out) {
    return parseVec3(p, out);
}

static const char* findKey(const char* json, const char* key) {
    std::string search = "\"";
    search += key;
    search += "\"";
    const char* p = json;
    while ((p = strstr(p, search.c_str()))) {
        p += search.length();
        p = skipWhitespace(p);
        if (*p == ':') {
            p++;
            return skipWhitespace(p);
        }
    }
    return nullptr;
}

bool MapLoader::loadFromJSON(const char* path, MapData& outMap) {
    std::string json = readFile(path);
    if (json.empty()) {
        fprintf(stderr, "Failed to load map: %s\n", path);
        return false;
    }
    return parseJSON(json.c_str(), outMap);
}

bool MapLoader::parseJSON(const char* json, MapData& outMap) {
    outMap.vertices.clear();
    outMap.indices.clear();
    outMap.entities.clear();
    outMap.playerSpawns.clear();
    outMap.enemySpawns.clear();

    // Parse name
    const char* nameVal = findKey(json, "name");
    if (nameVal) {
        std::string name;
        parseString(nameVal, name);
        printf("Loading map: %s\n", name.c_str());
    }

    // Parse geometry array
    const char* geomVal = findKey(json, "geometry");
    if (!geomVal) {
        fprintf(stderr, "No geometry found in map\n");
        return false;
    }

    if (*geomVal != '[') return false;
    geomVal++;

    while (*geomVal && *geomVal != ']') {
        geomVal = skipWhitespace(geomVal);
        if (*geomVal == '{') {
            // Parse geometry object
            std::string type;
            const char* typeVal = findKey(geomVal, "type");
            if (typeVal) parseString(typeVal, type);

            if (type == "floor" || type == "wall") {
                // Parse vertices
                const char* vertsVal = findKey(geomVal, "vertices");
                if (vertsVal && *vertsVal == '[') {
                    uint32_t baseIdx = (uint32_t)outMap.vertices.size();
                    vertsVal++;
                    while (*vertsVal && *vertsVal != ']') {
                        vertsVal = skipWhitespace(vertsVal);
                        if (*vertsVal == '{') {
                            MapVertex mv = {};
                            const char* posVal = findKey(vertsVal, "pos");
                            if (posVal) parseVec3(posVal, mv.pos);
                            const char* colVal = findKey(vertsVal, "color");
                            if (colVal) parseColor(colVal, mv.color);
                            mv.color[3] = 1.0f;
                            outMap.vertices.push_back(mv);
                            // Skip to next vertex
                            int depth = 1;
                            while (*vertsVal && depth > 0) {
                                if (*vertsVal == '{') depth++;
                                if (*vertsVal == '}') depth--;
                                if (depth > 0) vertsVal++;
                            }
                            if (*vertsVal == '}') vertsVal++;
                        }
                        vertsVal = skipWhitespace(vertsVal);
                        if (*vertsVal == ',') vertsVal++;
                    }
                }

                // Parse indices
                const char* idxVal = findKey(geomVal, "indices");
                if (idxVal && *idxVal == '[') {
                    uint32_t baseIdx = (uint32_t)outMap.vertices.size() - 4; // HACK: assumes 4 verts
                    idxVal++;
                    while (*idxVal && *idxVal != ']') {
                        idxVal = skipWhitespace(idxVal);
                        char* end;
                        uint32_t idx = (uint32_t)strtol(idxVal, &end, 10);
                        outMap.indices.push_back(baseIdx + idx);
                        idxVal = end;
                        idxVal = skipWhitespace(idxVal);
                        if (*idxVal == ',') idxVal++;
                    }
                }
            } else if (type == "pillar" || type == "platform" || type == "ramp") {
                // Parse box geometry
                float pos[3] = {0}, size[3] = {1}, color[4] = {0.5, 0.5, 0.5, 1};
                const char* posVal = findKey(geomVal, "pos");
                if (posVal) parseVec3(posVal, pos);
                const char* sizeVal = findKey(geomVal, "size");
                if (sizeVal) parseVec3(sizeVal, size);
                const char* colVal = findKey(geomVal, "color");
                if (colVal) parseColor(colVal, color);

                // Generate box vertices
                float hx = size[0] * 0.5f, hy = size[1] * 0.5f, hz = size[2] * 0.5f;
                uint32_t baseIdx = (uint32_t)outMap.vertices.size();

                // 6 faces, 4 vertices each
                for (int face = 0; face < 6; face++) {
                    float v[4][3];
                    switch (face) {
                        case 0: // Front
                            v[0][0] = pos[0]-hx; v[0][1] = pos[1]-hy; v[0][2] = pos[2]+hz;
                            v[1][0] = pos[0]+hx; v[1][1] = pos[1]-hy; v[1][2] = pos[2]+hz;
                            v[2][0] = pos[0]+hx; v[2][1] = pos[1]+hy; v[2][2] = pos[2]+hz;
                            v[3][0] = pos[0]-hx; v[3][1] = pos[1]+hy; v[3][2] = pos[2]+hz;
                            break;
                        case 1: // Back
                            v[0][0] = pos[0]+hx; v[0][1] = pos[1]-hy; v[0][2] = pos[2]-hz;
                            v[1][0] = pos[0]-hx; v[1][1] = pos[1]-hy; v[1][2] = pos[2]-hz;
                            v[2][0] = pos[0]-hx; v[2][1] = pos[1]+hy; v[2][2] = pos[2]-hz;
                            v[3][0] = pos[0]+hx; v[3][1] = pos[1]+hy; v[3][2] = pos[2]-hz;
                            break;
                        case 2: // Top
                            v[0][0] = pos[0]-hx; v[0][1] = pos[1]+hy; v[0][2] = pos[2]+hz;
                            v[1][0] = pos[0]+hx; v[1][1] = pos[1]+hy; v[1][2] = pos[2]+hz;
                            v[2][0] = pos[0]+hx; v[2][1] = pos[1]+hy; v[2][2] = pos[2]-hz;
                            v[3][0] = pos[0]-hx; v[3][1] = pos[1]+hy; v[3][2] = pos[2]-hz;
                            break;
                        case 3: // Bottom
                            v[0][0] = pos[0]-hx; v[0][1] = pos[1]-hy; v[0][2] = pos[2]-hz;
                            v[1][0] = pos[0]+hx; v[1][1] = pos[1]-hy; v[1][2] = pos[2]-hz;
                            v[2][0] = pos[0]+hx; v[2][1] = pos[1]-hy; v[2][2] = pos[2]+hz;
                            v[3][0] = pos[0]-hx; v[3][1] = pos[1]-hy; v[3][2] = pos[2]+hz;
                            break;
                        case 4: // Right
                            v[0][0] = pos[0]+hx; v[0][1] = pos[1]-hy; v[0][2] = pos[2]+hz;
                            v[1][0] = pos[0]+hx; v[1][1] = pos[1]-hy; v[1][2] = pos[2]-hz;
                            v[2][0] = pos[0]+hx; v[2][1] = pos[1]+hy; v[2][2] = pos[2]-hz;
                            v[3][0] = pos[0]+hx; v[3][1] = pos[1]+hy; v[3][2] = pos[2]+hz;
                            break;
                        case 5: // Left
                            v[0][0] = pos[0]-hx; v[0][1] = pos[1]-hy; v[0][2] = pos[2]-hz;
                            v[1][0] = pos[0]-hx; v[1][1] = pos[1]-hy; v[1][2] = pos[2]+hz;
                            v[2][0] = pos[0]-hx; v[2][1] = pos[1]+hy; v[2][2] = pos[2]+hz;
                            v[3][0] = pos[0]-hx; v[3][1] = pos[1]+hy; v[3][2] = pos[2]-hz;
                            break;
                    }
                    for (int i = 0; i < 4; i++) {
                        MapVertex mv;
                        mv.pos[0] = v[i][0]; mv.pos[1] = v[i][1]; mv.pos[2] = v[i][2];
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

            // Skip to next geometry object
            int depth = 1;
            while (*geomVal && depth > 0) {
                if (*geomVal == '{') depth++;
                if (*geomVal == '}') depth--;
                if (depth > 0) geomVal++;
            }
            if (*geomVal == '}') geomVal++;
        }
        geomVal = skipWhitespace(geomVal);
        if (*geomVal == ',') geomVal++;
    }

    // Parse entities
    const char* entVal = findKey(json, "entities");
    if (entVal && *entVal == '[') {
        entVal++;
        while (*entVal && *entVal != ']') {
            entVal = skipWhitespace(entVal);
            if (*entVal == '{') {
                MapEntity ent;
                const char* cnVal = findKey(entVal, "classname");
                if (cnVal) parseString(cnVal, ent.classname);
                const char* origVal = findKey(entVal, "origin");
                if (origVal) parseVec3(origVal, ent.origin);
                outMap.entities.push_back(ent);

                if (ent.classname == "info_player_deathmatch") {
                    outMap.playerSpawns.push_back({ent.origin[0], ent.origin[1], ent.origin[2]});
                } else if (ent.classname == "enemy_spawn") {
                    outMap.enemySpawns.push_back({ent.origin[0], ent.origin[1], ent.origin[2]});
                }

                int depth = 1;
                while (*entVal && depth > 0) {
                    if (*entVal == '{') depth++;
                    if (*entVal == '}') depth--;
                    if (depth > 0) entVal++;
                }
                if (*entVal == '}') entVal++;
            }
            entVal = skipWhitespace(entVal);
            if (*entVal == ',') entVal++;
        }
    }

    printf("Map loaded: %zu vertices, %zu indices, %zu entities\n",
           outMap.vertices.size(), outMap.indices.size(), outMap.entities.size());

    return true;
}

void MapLoader::generateDefaultArena(MapData& outMap) {
    // Fallback: generate simple arena
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
        const MapVertex& mv = map.vertices[map.indices[i]];
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
