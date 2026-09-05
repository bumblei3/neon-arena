#include "perf_profiler.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>

bool PerfProfiler::recording = false;
bool PerfProfiler::frameInProgress = false;
PerfProfiler::FrameMetrics PerfProfiler::frames[MAX_FRAMES];
int PerfProfiler::frameCount = 0;
int PerfProfiler::writeIndex = 0;
uint32_t PerfProfiler::currentFrameIndex = 0;
float PerfProfiler::sectionStart[8] = {0};
uint32_t PerfProfiler::sectionActive = 0;
char PerfProfiler::sectionNames[8][32] = {{0}};
int PerfProfiler::sectionCount = 0;
bool PerfProfiler::ringBuffer = false;

void PerfProfiler::Histogram::add(float ms) {
    int bucket = (int)(ms / bucketWidth);
    if (bucket < 0) bucket = 0;
    if (bucket >= NUM_BUCKETS) bucket = NUM_BUCKETS - 1;
    buckets[bucket]++;
    totalFrames++;
}

void PerfProfiler::Histogram::print() {
    printf("\n=== Frame Time Histogram ===\n");
    int maxCount = 0;
    for (int i = 0; i < NUM_BUCKETS; i++) {
        if (buckets[i] > maxCount) maxCount = buckets[i];
    }
    if (maxCount == 0) maxCount = 1;
    
    for (int i = 0; i < NUM_BUCKETS; i++) {
        float ms = bucketToMs(i);
        int barLen = (int)((float)buckets[i] / maxCount * 40);
        printf("%6.2f ms |", ms);
        for (int j = 0; j < barLen; j++) printf("#");
        printf(" %d\n", buckets[i]);
    }
    printf("===========================\n");
}

void PerfProfiler::Summary::print() {
    printf("\n=== Performance Summary ===\n");
    printf("  Avg FPS:      %.1f\n", avgFps);
    printf("  Min FPS:      %.1f\n", minFps);
    printf("  Max FPS:      %.1f\n", maxFps);
    printf("  Avg Frame:    %.2f ms\n", avgFrameMs);
    printf("  P99 Frame:    %.2f ms\n", p99FrameMs);
    printf("  Avg Draws:    %.0f\n", avgDrawCalls);
    printf("  Total Frames: %u\n", totalFrames);
    printf("  Total Draws:  %u\n", totalDrawCalls);
    printf("==========================\n");
}

void PerfProfiler::beginFrame() {
    if (!recording) return;
    if (frameCount >= MAX_FRAMES) return;
    if (frameInProgress) return; // Already in a frame
    
    int idx = frameCount;
    frames[idx].frameIndex = currentFrameIndex++;
    frames[idx].totalMs = 0;
    frames[idx].gameLogicMs = 0;
    frames[idx].renderMs = 0;
    frames[idx].gpuWaitMs = 0;
    frames[idx].presentMs = 0;
    frames[idx].drawCalls = 0;
    frames[idx].triangleCount = 0;
    frames[idx].fps = 0;
    frameInProgress = true;
}

void PerfProfiler::endFrame() {
    if (!recording) return;
    if (!frameInProgress) return; // No matching beginFrame
    
    int idx = frameCount;
    
    // Calculate total
    frames[idx].totalMs = frames[idx].gameLogicMs + frames[idx].renderMs + 
                          frames[idx].gpuWaitMs + frames[idx].presentMs;
    if (frames[idx].totalMs > 0) {
        frames[idx].fps = 1000.0f / frames[idx].totalMs;
    }
    
    frameCount++;
    frameInProgress = false;
}

void PerfProfiler::beginSection(const char* name) {
    if (!recording || sectionCount >= 8) return;
    
    int idx = sectionCount;
    strncpy(sectionNames[idx], name, 31);
    sectionNames[idx][31] = 0;
    sectionStart[idx] = 0; // In real impl: SDL_GetTicks()
    sectionActive |= (1 << idx);
    sectionCount++;
}

void PerfProfiler::endSection(const char* name) {
    if (!recording) return;
    
    for (int i = 0; i < sectionCount; i++) {
        if (strcmp(sectionNames[i], name) == 0) {
            float elapsed = 0; // In real impl: SDL_GetTicks() - sectionStart[i]
            
            // Add to current frame
            if (frameCount < MAX_FRAMES) {
                int idx = frameCount;
                if (strcmp(name, "game") == 0) frames[idx].gameLogicMs += elapsed;
                else if (strcmp(name, "render") == 0) frames[idx].renderMs += elapsed;
                else if (strcmp(name, "gpu") == 0) frames[idx].gpuWaitMs += elapsed;
                else if (strcmp(name, "present") == 0) frames[idx].presentMs += elapsed;
            }
            
            sectionActive &= ~(1 << i);
            break;
        }
    }
}

void PerfProfiler::addDrawCalls(uint32_t count, uint32_t triangles) {
    if (!recording || frameCount >= MAX_FRAMES) return;
    int idx = frameCount;
    frames[idx].drawCalls += count;
    frames[idx].triangleCount += triangles;
}

PerfProfiler::Summary PerfProfiler::computeSummary() {
    Summary s = {};
    if (frameCount == 0) return s;
    
    float totalFrameMs = 0;
    float minFps = 9999;
    float maxFps = 0;
    float allFrames[MAX_FRAMES];
    uint32_t totalDraws = 0;
    
    for (int i = 0; i < frameCount; i++) {
        totalFrameMs += frames[i].totalMs;
        if (frames[i].fps < minFps) minFps = frames[i].fps;
        if (frames[i].fps > maxFps) maxFps = frames[i].fps;
        allFrames[i] = frames[i].totalMs;
        totalDraws += frames[i].drawCalls;
    }
    
    s.avgFrameMs = totalFrameMs / frameCount;
    s.avgFps = 1000.0f / s.avgFrameMs;
    s.minFps = minFps;
    s.maxFps = maxFps;
    s.totalFrames = frameCount;
    s.totalDrawCalls = totalDraws;
    s.avgDrawCalls = (float)totalDraws / frameCount;
    
    // P99 frame time
    std::sort(allFrames, allFrames + frameCount);
    int p99Idx = (int)(frameCount * 0.99f);
    if (p99Idx >= frameCount) p99Idx = frameCount - 1;
    s.p99FrameMs = allFrames[p99Idx];
    
    return s;
}

void PerfProfiler::computeHistogram(Histogram& hist) {
    hist.bucketWidth = Histogram::MAX_MS / Histogram::NUM_BUCKETS;
    hist.totalFrames = 0;
    memset(hist.buckets, 0, sizeof(hist.buckets));
    
    for (int i = 0; i < frameCount; i++) {
        hist.add(frames[i].totalMs);
    }
}

void PerfProfiler::printReport() {
    Summary s = computeSummary();
    s.print();
}

void PerfProfiler::printHistogram() {
    Histogram hist;
    computeHistogram(hist);
    hist.print();
}

void PerfProfiler::reset() {
    frameCount = 0;
    writeIndex = 0;
    currentFrameIndex = 0;
    sectionActive = 0;
    sectionCount = 0;
    frameInProgress = false;
}
