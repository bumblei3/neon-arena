// ReplayRecorder tests - record, playback, save/load, JSON export
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cmath>
#include "replay_recorder.h"

static int testsPassed = 0;
static int testsFailed = 0;

#define TEST(name, expr) do { \
    if (expr) { \
        printf("  [PASS] %s\n", name); \
        testsPassed++; \
    } else { \
        printf("  [FAIL] %s\n", name); \
        testsFailed++; \
    } \
} while(0)

// === Test Recording ===

void testRecordPlayback() {
    printf("\n[Record/Playback Tests]\n");
    
    // Not recording → fails
    ReplayRecorder::stop();
    bool recorded = ReplayRecorder::recordEvent(ReplayRecorder::InputType::FIRE);
    TEST("not_recording_fails", !recorded);
    
    // Start recording
    ReplayRecorder::start();
    TEST("is_recording", ReplayRecorder::isRecording());
    
    // Record events
    ReplayRecorder::recordEvent(ReplayRecorder::InputType::MOVE, 1.0f, 0.0f, 0);
    ReplayRecorder::recordEvent(ReplayRecorder::InputType::AIM, 0.5f, 0.5f, 0);
    ReplayRecorder::recordEvent(ReplayRecorder::InputType::FIRE, 0.0f, 0.0f, 1);
    ReplayRecorder::recordEvent(ReplayRecorder::InputType::JUMP, 0.0f, 0.0f, 0);
    ReplayRecorder::recordEvent(ReplayRecorder::InputType::USE, 0.0f, 0.0f, 0);
    
    TEST("recorded_5_events", ReplayRecorder::getEventCount() == 5);
    
    ReplayRecorder::stop();
    TEST("stopped_recording", !ReplayRecorder::isRecording());
    
    // Playback
    ReplayRecorder::startPlayback();
    TEST("is_playing", ReplayRecorder::isPlaying());
    
    ReplayRecorder::InputEvent e;
    TEST("play_event1", ReplayRecorder::getNextEvent(e));
    TEST("event1_type", e.type == ReplayRecorder::InputType::MOVE);
    TEST("event1_x", e.x == 1.0f);
    
    TEST("play_event2", ReplayRecorder::getNextEvent(e));
    TEST("event2_type", e.type == ReplayRecorder::InputType::AIM);
    TEST("event2_y", e.y == 0.5f);
    
    TEST("play_event3", ReplayRecorder::getNextEvent(e));
    TEST("event3_type", e.type == ReplayRecorder::InputType::FIRE);
    
    TEST("play_event4", ReplayRecorder::getNextEvent(e));
    TEST("event4_type", e.type == ReplayRecorder::InputType::JUMP);
    
    TEST("play_event5", ReplayRecorder::getNextEvent(e));
    TEST("event5_type", e.type == ReplayRecorder::InputType::USE);
    
    // No more events
    TEST("play_done", !ReplayRecorder::getNextEvent(e));
    TEST("not_playing_after_done", !ReplayRecorder::isPlaying());
}

// === Test Reset Playback ===

void testResetPlayback() {
    printf("\n[Reset Playback Test]\n");
    
    ReplayRecorder::startPlayback();
    ReplayRecorder::InputEvent e;
    
    // Skip first two
    ReplayRecorder::getNextEvent(e);
    ReplayRecorder::getNextEvent(e);
    
    // Reset
    ReplayRecorder::resetPlayback();
    
    // Should get first again
    TEST("reset_get_first", ReplayRecorder::getNextEvent(e));
    TEST("reset_first_type", e.type == ReplayRecorder::InputType::MOVE);
}

// === Test Buffer Full ===

void testBufferFull() {
    printf("\n[Buffer Full Test]\n");
    
    ReplayRecorder::start();
    
    // Fill buffer
    int recorded = 0;
    for (int i = 0; i < ReplayRecorder::MAX_EVENTS + 100; i++) {
        if (ReplayRecorder::recordEvent(ReplayRecorder::InputType::MOVE)) recorded++;
    }
    
    TEST("buffer_full_count", recorded == ReplayRecorder::MAX_EVENTS);
    TEST("event_count_max", ReplayRecorder::getEventCount() == ReplayRecorder::MAX_EVENTS);
    
    ReplayRecorder::stop();
}

// === Test Ring Buffer ===

void testRingBuffer() {
    printf("\n[Ring Buffer Test]\n");
    
    ReplayRecorder::setRingBuffer(true);
    ReplayRecorder::start();
    
    // Fill buffer
    for (int i = 0; i < ReplayRecorder::MAX_EVENTS; i++) {
        ReplayRecorder::recordEvent(ReplayRecorder::InputType::MOVE, (float)i, 0);
    }
    
    // Overwrite
    for (int i = 0; i < 10; i++) {
        ReplayRecorder::recordEvent(ReplayRecorder::InputType::FIRE, (float)(i + 1000));
    }
    
    TEST("ring_count_stays_max", ReplayRecorder::getEventCount() == ReplayRecorder::MAX_EVENTS);
    
    ReplayRecorder::stop();
    ReplayRecorder::setRingBuffer(false);
}

// === Test Input Types ===

void testInputTypes() {
    printf("\n[Input Type Tests]\n");
    
    TEST("type_move", ReplayRecorder::stringToInputType("MOVE") == ReplayRecorder::InputType::MOVE);
    TEST("type_aim", ReplayRecorder::stringToInputType("AIM") == ReplayRecorder::InputType::AIM);
    TEST("type_fire", ReplayRecorder::stringToInputType("FIRE") == ReplayRecorder::InputType::FIRE);
    TEST("type_use", ReplayRecorder::stringToInputType("USE") == ReplayRecorder::InputType::USE);
    TEST("type_jump", ReplayRecorder::stringToInputType("JUMP") == ReplayRecorder::InputType::JUMP);
    TEST("type_unknown", ReplayRecorder::stringToInputType("UNKNOWN") == ReplayRecorder::InputType::MOVE);
    TEST("type_null", ReplayRecorder::stringToInputType(nullptr) == ReplayRecorder::InputType::MOVE);
}

// === Test Save/Load Binary ===

void testSaveLoad() {
    printf("\n[Save/Load Binary Tests]\n");
    
    ReplayRecorder::start();
    ReplayRecorder::recordEvent(ReplayRecorder::InputType::MOVE, 1.0f, 0.0f);
    ReplayRecorder::recordEvent(ReplayRecorder::InputType::FIRE, 0.0f, 0.0f, 1);
    ReplayRecorder::recordEvent(ReplayRecorder::InputType::AIM, 0.5f, 0.5f);
    int origCount = ReplayRecorder::getEventCount();
    ReplayRecorder::stop();
    
    // Save
    const char* testFile = "/tmp/test_replay.bin";
    bool saved = ReplayRecorder::saveToFile(testFile);
    TEST("save_binary", saved);
    
    // Clear and reload
    ReplayRecorder::start();
    TEST("cleared", ReplayRecorder::getEventCount() == 0);
    ReplayRecorder::stop();
    
    bool loaded = ReplayRecorder::loadFromFile(testFile);
    TEST("load_binary", loaded);
    TEST("loaded_count", ReplayRecorder::getEventCount() == origCount);
    
    // Verify content
    ReplayRecorder::startPlayback();
    ReplayRecorder::InputEvent e;
    TEST("play_loaded_1", ReplayRecorder::getNextEvent(e));
    TEST("loaded_type", e.type == ReplayRecorder::InputType::MOVE);
    TEST("loaded_x", e.x == 1.0f);
}

// === Test JSON Export ===

void testJSONExport() {
    printf("\n[JSON Export Tests]\n");
    
    ReplayRecorder::start();
    ReplayRecorder::recordEvent(ReplayRecorder::InputType::MOVE, 1.0f, 2.0f);
    ReplayRecorder::recordEvent(ReplayRecorder::InputType::FIRE, 0.0f, 0.0f, 1);
    ReplayRecorder::stop();
    
    const char* testFile = "/tmp/test_replay.json";
    bool saved = ReplayRecorder::exportToJSON(testFile);
    TEST("export_json", saved);
    
    // Check file
    FILE* f = fopen(testFile, "r");
    TEST("json_file_exists", f != nullptr);
    if (f) {
        char buf[1024];
        size_t n = fread(buf, 1, sizeof(buf)-1, f);
        buf[n] = 0;
        TEST("json_has_events", strstr(buf, "\"events\"") != nullptr);
        TEST("json_has_move", strstr(buf, "MOVE") != nullptr);
        TEST("json_has_fire", strstr(buf, "FIRE") != nullptr);
        fclose(f);
    }
}

// === Test JSON Import ===

void testJSONImport() {
    printf("\n[JSON Import Tests]\n");
    
    const char* testFile = "/tmp/test_replay.json";
    
    // Clear
    ReplayRecorder::start();
    ReplayRecorder::stop();
    
    bool loaded = ReplayRecorder::importFromJSON(testFile);
    TEST("import_json", loaded);
    TEST("imported_count", ReplayRecorder::getEventCount() == 2);
    
    if (ReplayRecorder::getEventCount() == 2) {
        ReplayRecorder::startPlayback();
        ReplayRecorder::InputEvent e;
        ReplayRecorder::getNextEvent(e);
        TEST("imported_type", e.type == ReplayRecorder::InputType::MOVE);
        TEST("imported_x", e.x == 1.0f);
    }
}

// === Test Duration ===

void testDuration() {
    printf("\n[Duration Test]\n");
    
    ReplayRecorder::start();
    ReplayRecorder::recordEvent(ReplayRecorder::InputType::MOVE);
    uint32_t d = ReplayRecorder::getDurationMs();
    TEST("duration_zero", d == 0);  // no timestamps set in test
    ReplayRecorder::stop();
}

// === Test Max Events ===

void testMaxEvents() {
    printf("\n[Max Events Test]\n");
    
    TEST("max_events_value", ReplayRecorder::MAX_EVENTS == 65536);
}

// === Main ===

int main() {
    printf("=== Replay Recorder Test Suite ===\n");
    
    testRecordPlayback();
    testResetPlayback();
    testBufferFull();
    testRingBuffer();
    testInputTypes();
    testSaveLoad();
    testJSONExport();
    testJSONImport();
    testDuration();
    testMaxEvents();
    
    printf("\n=== Results: %d passed, %d failed ===\n", testsPassed, testsFailed);
    
    return testsFailed > 0 ? 1 : 0;
}
