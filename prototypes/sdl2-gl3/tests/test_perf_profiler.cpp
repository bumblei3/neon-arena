// PerfProfiler tests - frame timing, draw calls, summary, histogram
#include <cstdio>
#include <cassert>
#include <cstring>
#include <algorithm>
#include "perf_profiler.h"

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

// === Test Frame Recording ===

void testFrameRecording() {
    printf("\n[Frame Recording Tests]\n");
    
    PerfProfiler::reset();
    PerfProfiler::startRecording();
    
    TEST("is_recording", PerfProfiler::isRecording());
    
    // Record 10 frames
    for (int i = 0; i < 10; i++) {
        PerfProfiler::beginFrame();
        PerfProfiler::addDrawCalls(100 + i * 10, 1000 + i * 100);
        PerfProfiler::endFrame();
    }
    
    TEST("frame_count_10", PerfProfiler::getFrameCount() == 10);
    
    // Check frame data
    const auto& f0 = PerfProfiler::getFrames()[0];
    TEST("frame0_draws", f0.drawCalls == 100);
    TEST("frame0_tris", f0.triangleCount == 1000);
    
    const auto& f9 = PerfProfiler::getFrames()[9];
    TEST("frame9_draws", f9.drawCalls == 190);
    TEST("frame9_tris", f9.triangleCount == 1900);
    
    PerfProfiler::stopRecording();
    TEST("stopped", !PerfProfiler::isRecording());
}

// === Test Summary ===

void testSummary() {
    printf("\n[Summary Tests]\n");
    
    PerfProfiler::reset();
    PerfProfiler::startRecording();
    
    // Record frames with known values
    for (int i = 0; i < 100; i++) {
        PerfProfiler::beginFrame();
        PerfProfiler::addDrawCalls(50);
        PerfProfiler::endFrame();
    }
    
    auto s = PerfProfiler::computeSummary();
    
    TEST("summary_frames", s.totalFrames == 100);
    TEST("summary_draws", s.totalDrawCalls == 5000);
    TEST("summary_avgDraws", s.avgDrawCalls == 50.0f);
    
    PerfProfiler::stopRecording();
}

// === Test Histogram ===

void testHistogram() {
    printf("\n[Histogram Tests]\n");
    
    PerfProfiler::reset();
    PerfProfiler::startRecording();
    
    // Record frames
    for (int i = 0; i < 50; i++) {
        PerfProfiler::beginFrame();
        PerfProfiler::endFrame();
    }
    
    PerfProfiler::Histogram hist;
    PerfProfiler::computeHistogram(hist);
    
    TEST("hist_total", hist.totalFrames == 50);
    TEST("hist_buckets", hist.bucketWidth > 0);
    
    // Check all frames are in buckets
    int sum = 0;
    for (int i = 0; i < PerfProfiler::Histogram::NUM_BUCKETS; i++) {
        sum += hist.buckets[i];
    }
    TEST("hist_sum", sum == 50);
    
    PerfProfiler::stopRecording();
}

// === Test Reset ===

void testReset() {
    printf("\n[Reset Test]\n");
    
    PerfProfiler::reset(); // Ensure clean state from previous test
    PerfProfiler::startRecording();
    PerfProfiler::beginFrame();
    PerfProfiler::endFrame();
    
    TEST("has_1_frame", PerfProfiler::getFrameCount() == 1);
    
    PerfProfiler::reset();
    TEST("reset_count", PerfProfiler::getFrameCount() == 0);
    
    PerfProfiler::stopRecording();
}

// === Test Max Frames ===

void testMaxFrames() {
    printf("\n[Max Frames Test]\n");
    
    PerfProfiler::reset();
    PerfProfiler::startRecording();
    
    // Record more than max
    for (int i = 0; i < PerfProfiler::MAX_FRAMES + 100; i++) {
        PerfProfiler::beginFrame();
        PerfProfiler::endFrame();
    }
    
    TEST("max_frames", PerfProfiler::getFrameCount() == PerfProfiler::MAX_FRAMES);
    
    PerfProfiler::stopRecording();
}

// === Test Draw Call Accumulation ===

void testDrawCallAccumulation() {
    printf("\n[Draw Call Accumulation Test]\n");
    
    PerfProfiler::reset();
    PerfProfiler::startRecording();
    
    PerfProfiler::beginFrame();
    PerfProfiler::addDrawCalls(10, 100);
    PerfProfiler::addDrawCalls(20, 200);
    PerfProfiler::addDrawCalls(5, 50);
    PerfProfiler::endFrame();
    
    const auto& f = PerfProfiler::getFrames()[0];
    TEST("accum_draws", f.drawCalls == 35);
    TEST("accum_tris", f.triangleCount == 350);
    
    PerfProfiler::stopRecording();
}

// === Test Not Recording ===

void testNotRecording() {
    printf("\n[Not Recording Test]\n");
    
    PerfProfiler::reset();
    // Not recording
    
    PerfProfiler::beginFrame();
    PerfProfiler::addDrawCalls(100);
    PerfProfiler::endFrame();
    
    TEST("not_recording_no_frames", PerfProfiler::getFrameCount() == 0);
}

// === Test Print Functions ===

void testPrintFunctions() {
    printf("\n[Print Functions Test]\n");
    
    PerfProfiler::reset();
    PerfProfiler::startRecording();
    
    for (int i = 0; i < 10; i++) {
        PerfProfiler::beginFrame();
        PerfProfiler::addDrawCalls(50 + i * 5);
        PerfProfiler::endFrame();
    }
    
    // Just make sure they don't crash
    PerfProfiler::printReport();
    PerfProfiler::printHistogram();
    
    TEST("print_no_crash", true);
    
    PerfProfiler::stopRecording();
}

// === Test Constants ===

void testConstants() {
    printf("\n[Constants Test]\n");
    
    TEST("max_frames", PerfProfiler::MAX_FRAMES == 8192);
    TEST("hist_buckets", PerfProfiler::Histogram::NUM_BUCKETS == 20);
    TEST("hist_max_ms", PerfProfiler::Histogram::MAX_MS == 33.33f);
}

// === Main ===

int main() {
    printf("=== Perf Profiler Test Suite ===\n");
    
    testFrameRecording();
    testSummary();
    testHistogram();
    testReset();
    testMaxFrames();
    testDrawCallAccumulation();
    testNotRecording();
    testPrintFunctions();
    testConstants();
    
    printf("\n=== Results: %d passed, %d failed ===\n", testsPassed, testsFailed);
    
    return testsFailed > 0 ? 1 : 0;
}
