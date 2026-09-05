#pragma once
#include <vector>
#include <cstdint>
#include <cstring>

// ReplayRecorder: Record and playback player inputs for bug reports
// Records: timestamp, input type (move/aim/fire), value (x/y buttons)
// Format: lightweight binary ring buffer + export to JSON

class ReplayRecorder {
public:
    enum class InputType : uint8_t {
        MOVE = 0,
        AIM = 1,
        FIRE = 2,
        USE = 3,
        JUMP = 4,
        WEAPON_SWITCH = 5,
        PAUSE = 6,
        MENU = 7,
    };
    
    struct InputEvent {
        uint32_t timestampMs;   // ms since recording start
        InputType type;
        float x, y;             // value (e.g. aim delta, move direction)
        uint8_t buttons;         // button state bitmask
        
        InputEvent() : timestampMs(0), type(InputType::MOVE), x(0), y(0), buttons(0) {}
        InputEvent(uint32_t ts, InputType t, float x, float y, uint8_t b) 
            : timestampMs(ts), type(t), x(x), y(y), buttons(b) {}
    };
    
    struct ReplayHeader {
        char magic[4] = {'N', 'R', 'P', 'Y'};
        uint16_t version = 1;
        uint16_t reserved = 0;
        uint32_t mapNameLen;
        uint32_t eventCount;
        uint32_t durationMs;
    };
    
    static constexpr int MAX_EVENTS = 65536;
    
    // Start/stop recording
    static void start();
    static void stop();
    static bool isRecording() { return recording; }
    
    // Record an event
    static bool recordEvent(InputType type, float x = 0, float y = 0, uint8_t buttons = 0);
    
    // Playback
    static void startPlayback();
    static void stopPlayback();
    static bool isPlaying() { return playing; }
    
    // Get next event during playback (returns false if done)
    static bool getNextEvent(InputEvent& out);
    
    // Reset playback to start
    static void resetPlayback();
    
    // Save to file (binary format)
    static bool saveToFile(const char* filename);
    
    // Load from file
    static bool loadFromFile(const char* filename);
    
    // Export to JSON (human-readable)
    static bool exportToJSON(const char* filename);
    
    // Import from JSON
    static bool importFromJSON(const char* filename);
    
    // Statistics
    static int getEventCount() { return eventCount; }
    static int getMaxEvents() { return MAX_EVENTS; }
    static uint32_t getDurationMs() { return eventCount > 0 ? events[eventCount-1].timestampMs : 0; }
    
    // Ring buffer mode
    static void setRingBuffer(bool enable) { ringBuffer = enable; }
    static bool isRingBuffer() { return ringBuffer; }
    
    // JSON helpers
    static const char* inputTypeToString(InputType t);
    static InputType stringToInputType(const char* s);
    
private:
    static bool recording;
    static bool playing;
    static bool ringBuffer;
    static InputEvent events[MAX_EVENTS];
    static int eventCount;
    static int writeIndex;
    static int readIndex;
    static uint32_t recordStartMs;
};
