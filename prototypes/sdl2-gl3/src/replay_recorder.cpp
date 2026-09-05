#include "replay_recorder.h"
#include <cstdio>
#include <cstring>
#include <cmath>

bool ReplayRecorder::recording = false;
bool ReplayRecorder::playing = false;
bool ReplayRecorder::ringBuffer = false;
ReplayRecorder::InputEvent ReplayRecorder::events[MAX_EVENTS];
int ReplayRecorder::eventCount = 0;
int ReplayRecorder::writeIndex = 0;
int ReplayRecorder::readIndex = 0;
uint32_t ReplayRecorder::recordStartMs = 0;

void ReplayRecorder::start() {
    recording = true;
    playing = false;
    eventCount = 0;
    writeIndex = 0;
    readIndex = 0;
    recordStartMs = 0;
}

void ReplayRecorder::stop() {
    recording = false;
}

bool ReplayRecorder::recordEvent(InputType type, float x, float y, uint8_t buttons) {
    if (!recording) return false;
    
    if (eventCount >= MAX_EVENTS) {
        if (ringBuffer) {
            // Overwrite oldest
            writeIndex = (writeIndex + 1) % MAX_EVENTS;
            eventCount = MAX_EVENTS;
        } else {
            return false; // Buffer full
        }
    } else {
        eventCount++;
    }
    
    int idx = (writeIndex + eventCount - 1) % MAX_EVENTS;
    events[idx] = InputEvent(recordStartMs, type, x, y, buttons);
    
    return true;
}

void ReplayRecorder::startPlayback() {
    if (eventCount == 0) return;
    playing = true;
    recording = false;
    readIndex = 0;
}

void ReplayRecorder::stopPlayback() {
    playing = false;
}

bool ReplayRecorder::getNextEvent(InputEvent& out) {
    if (!playing || readIndex >= eventCount) {
        playing = false;
        return false;
    }
    
    int idx = (writeIndex + readIndex) % MAX_EVENTS;
    out = events[idx];
    readIndex++;
    
    if (readIndex >= eventCount) {
        playing = false;
    }
    
    return true;
}

void ReplayRecorder::resetPlayback() {
    readIndex = 0;
}

bool ReplayRecorder::saveToFile(const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) return false;
    
    ReplayHeader header;
    header.eventCount = eventCount;
    header.durationMs = getDurationMs();
    header.mapNameLen = 0;
    
    fwrite(&header, sizeof(header), 1, f);
    fwrite(events, sizeof(InputEvent), eventCount, f);
    
    fclose(f);
    return true;
}

bool ReplayRecorder::loadFromFile(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return false;
    
    ReplayHeader header;
    if (fread(&header, sizeof(header), 1, f) != 1) {
        fclose(f);
        return false;
    }
    
    if (header.magic[0] != 'N' || header.magic[1] != 'R' || header.magic[2] != 'P' || header.magic[3] != 'Y') {
        fclose(f);
        return false;
    }
    
    eventCount = header.eventCount < MAX_EVENTS ? header.eventCount : MAX_EVENTS;
    fread(events, sizeof(InputEvent), eventCount, f);
    
    fclose(f);
    return true;
}

bool ReplayRecorder::exportToJSON(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return false;
    
    fprintf(f, "{\n");
    fprintf(f, "  \"version\": 1,\n");
    fprintf(f, "  \"eventCount\": %d,\n", eventCount);
    fprintf(f, "  \"durationMs\": %u,\n", getDurationMs());
    fprintf(f, "  \"events\": [\n");
    
    for (int i = 0; i < eventCount; i++) {
        int idx = (writeIndex + i) % MAX_EVENTS;
        auto& e = events[idx];
        fprintf(f, "    {\"ts\":%u,\"type\":\"%s\",\"x\":%.3f,\"y\":%.3f,\"btns\":%u}%s\n",
                e.timestampMs, inputTypeToString(e.type), e.x, e.y, e.buttons,
                (i < eventCount - 1) ? "," : "");
    }
    
    fprintf(f, "  ]\n");
    fprintf(f, "}\n");
    
    fclose(f);
    return true;
}

bool ReplayRecorder::importFromJSON(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return false;
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* buffer = (char*)malloc(size + 1);
    if (!buffer) { fclose(f); return false; }
    
    size_t n = fread(buffer, 1, size, f);
    buffer[n] = '\0';
    fclose(f);
    
    // Simple JSON parsing
    eventCount = 0;
    writeIndex = 0;
    
    const char* p = buffer;
    while (*p && eventCount < MAX_EVENTS) {
        // Find "ts":
        const char* tsPtr = strstr(p, "\"ts\":");
        if (!tsPtr) break;
        tsPtr += 5;
        
        // Find "type":
        const char* typePtr = strstr(tsPtr, "\"type\":\"");
        if (!typePtr) break;
        typePtr += 8;
        
        // Find "x":
        const char* xPtr = strstr(typePtr, "\"x\":");
        if (!xPtr) break;
        xPtr += 4;
        
        // Find "y":
        const char* yPtr = strstr(xPtr, "\"y\":");
        if (!yPtr) break;
        yPtr += 4;
        
        // Find "btns":
        const char* btnPtr = strstr(yPtr, "\"btns\":");
        if (!btnPtr) break;
        btnPtr += 7;
        
        // Parse values
        uint32_t ts = (uint32_t)atoi(tsPtr);
        
        // Extract type string
        char typeBuf[32] = {0};
        const char* te = strchr(typePtr, '"');
        if (te) {
            int len = te - typePtr;
            if (len < 31) { memcpy(typeBuf, typePtr, len); typeBuf[len] = 0; }
        }
        
        float x = strtof(xPtr, nullptr);
        float y = strtof(yPtr, nullptr);
        uint8_t btns = (uint8_t)atoi(btnPtr);
        
        InputEvent e;
        e.timestampMs = ts;
        e.type = stringToInputType(typeBuf);
        e.x = x;
        e.y = y;
        e.buttons = btns;
        
        events[eventCount++] = e;
        
        p = btnPtr + 1;
    }
    
    free(buffer);
    return eventCount > 0;
}

const char* ReplayRecorder::inputTypeToString(InputType t) {
    switch (t) {
        case InputType::MOVE: return "MOVE";
        case InputType::AIM: return "AIM";
        case InputType::FIRE: return "FIRE";
        case InputType::USE: return "USE";
        case InputType::JUMP: return "JUMP";
        case InputType::WEAPON_SWITCH: return "WEAPON_SWITCH";
        case InputType::PAUSE: return "PAUSE";
        case InputType::MENU: return "MENU";
    }
    return "UNKNOWN";
}

ReplayRecorder::InputType ReplayRecorder::stringToInputType(const char* s) {
    if (!s) return InputType::MOVE;
    if (strcmp(s, "MOVE") == 0) return InputType::MOVE;
    if (strcmp(s, "AIM") == 0) return InputType::AIM;
    if (strcmp(s, "FIRE") == 0) return InputType::FIRE;
    if (strcmp(s, "USE") == 0) return InputType::USE;
    if (strcmp(s, "JUMP") == 0) return InputType::JUMP;
    if (strcmp(s, "WEAPON_SWITCH") == 0) return InputType::WEAPON_SWITCH;
    if (strcmp(s, "PAUSE") == 0) return InputType::PAUSE;
    if (strcmp(s, "MENU") == 0) return InputType::MENU;
    return InputType::MOVE;
}
