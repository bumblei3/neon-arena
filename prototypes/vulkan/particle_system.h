// NEON ARENA - Vulkan + SDL2 Prototype
// Particle system: CPU update, GPU point rendering
#pragma once

#include "types.h"
#include <vector>
#include <cstdint>

struct Particle {
    float pos[3];
    float vel[3];
    float life;
    float size;
    float color[3];
    float padding[2];
};

class ParticleSystem {
public:
    static const uint32_t MAX_PARTICLES = 4096;

    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    VkBuffer particleBuffer = VK_NULL_HANDLE;
    VkDeviceMemory particleBufferMemory = VK_NULL_HANDLE;

    uint32_t activeCount = 0;

    void init(VkDevice dev, VkPhysicalDevice phys);
    void cleanup();

    void emit(uint32_t count, const float* pos, const float* color, float spread, float speed);
    void update(float dt);

    const std::vector<Vertex>& getVertices() const { return vertices; }
    uint32_t getActiveCount() const { return activeCount; }

private:
    std::vector<Vertex> vertices;

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};
