// NEON ARENA - Vulkan + SDL2 Prototype
// GPU particle system via compute shader
#pragma once

#include "types.h" // For Vertex, Vec3
#include <vector>
#include <cstring>

class ParticleSystem {
public:
    static const uint32_t MAX_PARTICLES = 4096;

    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    // Buffers
    VkBuffer particleBuffer = VK_NULL_HANDLE;
    VkDeviceMemory particleBufferMemory = VK_NULL_HANDLE;

    // Compute pipeline
    VkPipeline computePipeline = VK_NULL_HANDLE;
    VkPipelineLayout computePipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout computeDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet computeDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorPool computeDescriptorPool = VK_NULL_HANDLE;

    // Render pipeline for particles
    VkPipeline renderPipeline = VK_NULL_HANDLE;
    VkPipelineLayout renderPipelineLayout = VK_NULL_HANDLE;

    uint32_t activeCount = 0;
    std::vector<Vertex> vertices;

    const std::vector<Vertex>& getVertices() const { return vertices; }
    uint32_t getActiveCount() const { return activeCount; }

    void init(VkDevice dev, VkPhysicalDevice phys);
    void cleanup();

    void emit(uint32_t count, const float* pos, const float* color, float spread, float speed);
    void update(float dt);
    void render(VkCommandBuffer cmd, VkPipelineLayout sceneLayout, const float* view, const float* proj, float aspect);

private:
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createComputePipeline();
    void createRenderPipeline();
    void createDescriptorSet();

    std::vector<char> readFile(const char* path);
    VkShaderModule createShaderModule(const std::vector<char>& code);
};
