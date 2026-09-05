#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <cstdio>

// PerfProfiler: Frame-time heatmap, draw-call counter, GPU/CPU metrics
// Tracks: frame time, game logic, render, gpu wait, present, draw calls
// Output: histogram, averages, P99, min/max

class PerfProfiler {
public:
    struct FrameMetrics {
        uint32_t frameIndex;
        float totalMs;          // total frame time
        float gameLogicMs;      // game update
        float renderMs;         // render submit
        float gpuWaitMs;        // GPU synchronization wait
        float presentMs;        // present/vsync
        uint32_t drawCalls;
        uint32_t triangleCount;
        float fps;              // 1000 / totalMs
    };
    
    struct Histogram {
        static constexpr int NUM_BUCKETS = 20;
        static constexpr float MAX_MS = 33.33f; // 30 FPS cap
        float bucketWidth;
        int buckets[NUM_BUCKETS] = {0};
        int totalFrames = 0;
        
        void add(float ms);
        void print();
        float bucketToMs(int bucket) { return bucket * bucketWidth; }
    };
    
    struct Summary {
        float avgFps;
        float minFps;
        float maxFps;
        float p99FrameMs;
        float avgFrameMs;
        float avgDrawCalls;
        uint32_t totalFrames;
        uint32_t totalDrawCalls;
        
        void print();
    };
    
    static constexpr int MAX_FRAMES = 8192;
    
    // Begin/end frame capture
    static void beginFrame();
    static void endFrame();
    
    // Mark sections within a frame
    static void beginSection(const char* name);
    static void endSection(const char* name);
    
    // Add draw call count for current frame
    static void addDrawCalls(uint32_t count, uint32_t triangles = 0);
    
    // Get metrics
    static const FrameMetrics* getFrames() { return frames; }
    static int getFrameCount() { return frameCount; }
    static const FrameMetrics& getLastFrame() { return frames[frameCount - 1]; }
    
    // Analysis
    static Summary computeSummary();
    static void computeHistogram(Histogram& hist);
    
    // Print report
    static void printReport();
    static void printHistogram();
    
    // Reset
    static void reset();
    
    // Recording mode
    static void startRecording() { recording = true; }
    static void stopRecording() { recording = false; frameInProgress = false; }
    static bool isRecording() { return recording; }
    
private:
    static bool recording;
    static bool frameInProgress;
    static FrameMetrics frames[MAX_FRAMES];
    static int frameCount;
    static int writeIndex;
    static uint32_t currentFrameIndex;
    static float sectionStart[8];
    static uint32_t sectionActive;
    static char sectionNames[8][32];
    static int sectionCount;
    
    // Ring buffer for continuous recording
    static bool ringBuffer;
};
