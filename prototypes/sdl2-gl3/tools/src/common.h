#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

// Common helpers for NeonArena tools

// Trim whitespace from string
inline std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Split string by delimiter
inline std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> parts;
    size_t pos = 0;
    while (pos < s.size()) {
        size_t next = s.find(delim, pos);
        if (next == std::string::npos) {
            parts.push_back(s.substr(pos));
            break;
        }
        parts.push_back(s.substr(pos, next - pos));
        pos = next + 1;
    }
    return parts;
}

// Read file contents to string
inline std::string readFile(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return "";
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::string content(size, '\0');
    fread(&content[0], 1, size, f);
    fclose(f);
    return content;
}

// Write string to file
inline bool writeFile(const char* filename, const std::string& content) {
    FILE* f = fopen(filename, "w");
    if (!f) return false;
    fwrite(content.c_str(), 1, content.size(), f);
    fclose(f);
    return true;
}
