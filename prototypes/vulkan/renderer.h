// NEON ARENA - Vulkan + SDL2 Prototype
// Renderer: Vulkan instance, device, swapchain, vertex/uniform buffers
#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>

#define VK_CHECK(call)                                                          \
    do {                                                                        \
        VkResult _r = (call);                                                   \
        if (_r != VK_SUCCESS) {                                                 \
            fprintf(stderr, "Vulkan error %d at %s:%d\n", _r, __FILE__, __LINE__); \
            std::exit(1);                                                       \
        }                                                                       \
    } while (0)

struct Vertex {
    float pos[3];
    float color[3];
};

struct UniformBufferObject {
    float view[16];
    float proj[16];
};

class Renderer {
public:
    SDL_Window* window = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkFormat swapchainFormat;
    VkExtent2D swapchainExtent;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline trianglePipeline = VK_NULL_HANDLE;
    VkPipeline linePipeline = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;
    VkSemaphore imageAvailable = VK_NULL_HANDLE;
    VkSemaphore renderFinished = VK_NULL_HANDLE;
    VkFence inFlight = VK_NULL_HANDLE;
    uint32_t graphicsFamily = 0;
    uint32_t presentFamily = 0;

    // Buffers
    VkBuffer triangleBuffer = VK_NULL_HANDLE;
    VkDeviceMemory triangleBufferMemory = VK_NULL_HANDLE;
    VkBuffer lineBuffer = VK_NULL_HANDLE;
    VkDeviceMemory lineBufferMemory = VK_NULL_HANDLE;
    VkBuffer uniformBuffer = VK_NULL_HANDLE;
    VkDeviceMemory uniformBufferMemory = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

    uint32_t triangleVerts_ = 0;
    uint32_t lineVerts_ = 0;

    static const int WIDTH = 1280;
    static const int HEIGHT = 720;
    static const VkDeviceSize MAX_VERTICES = 1024 * 1024; // 1M vertices max

    void init();
    void cleanup();
    void updateTriangles(const std::vector<Vertex>& verts);
    void updateLines(const std::vector<Vertex>& verts);
    void updateUniform(const UniformBufferObject& ubo);
    void drawFrame();

private:
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createDevice();
    void createSwapchain();
    void createImageViews();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createPipelines();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void createTriangleBuffer();
    void createLineBuffer();
    void createUniformBuffer();
    void createDescriptorPool();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    std::vector<char> readFile(const char* path);
    VkShaderModule createShaderModule(const std::vector<char>& code);
};
