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

struct HudVertex {
    float pos[2];
    float uv[2];
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
    VkRenderPass bloomRenderPass = VK_NULL_HANDLE;
    VkRenderPass compositeRenderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout bloomDescriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout bloomPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout hudPipelineLayout = VK_NULL_HANDLE;
    VkPipeline trianglePipeline = VK_NULL_HANDLE;
    VkPipeline linePipeline = VK_NULL_HANDLE;
    VkPipeline hudPipeline = VK_NULL_HANDLE;
    VkPipeline brightPipeline = VK_NULL_HANDLE;
    VkPipeline blurPipeline = VK_NULL_HANDLE;
    VkPipeline compositePipeline = VK_NULL_HANDLE;
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
    VkBuffer hudBuffer = VK_NULL_HANDLE;
    VkDeviceMemory hudBufferMemory = VK_NULL_HANDLE;
    VkBuffer uniformBuffer = VK_NULL_HANDLE;
    VkDeviceMemory uniformBufferMemory = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

    // Bloom resources
    VkImage bloomImage1 = VK_NULL_HANDLE;  // Scene
    VkDeviceMemory bloomMemory1 = VK_NULL_HANDLE;
    VkImageView bloomView1 = VK_NULL_HANDLE;
    VkImage bloomImage2 = VK_NULL_HANDLE;  // Bright result
    VkDeviceMemory bloomMemory2 = VK_NULL_HANDLE;
    VkImageView bloomView2 = VK_NULL_HANDLE;
    VkImage bloomImage3 = VK_NULL_HANDLE;  // Blur H result
    VkDeviceMemory bloomMemory3 = VK_NULL_HANDLE;
    VkImageView bloomView3 = VK_NULL_HANDLE;
    VkFramebuffer bloomFramebuffer1 = VK_NULL_HANDLE;
    VkFramebuffer bloomFramebuffer2 = VK_NULL_HANDLE;
    VkFramebuffer bloomFramebuffer3 = VK_NULL_HANDLE;
    VkDescriptorPool bloomDescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet bloomDescriptorSet1 = VK_NULL_HANDLE;  // Bright: reads scene
    VkDescriptorSet bloomDescriptorSet2 = VK_NULL_HANDLE;  // Blur H: reads bright
    VkDescriptorSet bloomDescriptorSet3 = VK_NULL_HANDLE;  // Blur V: reads blur H
    VkDescriptorSet compositeDescriptorSet = VK_NULL_HANDLE;  // Composite: reads scene + blur V
    VkSampler textureSampler = VK_NULL_HANDLE;

    uint32_t triangleVerts_ = 0;
    uint32_t lineVerts_ = 0;
    uint32_t hudVerts_ = 0;

    static const int WIDTH = 1280;
    static const int HEIGHT = 720;
    static const VkDeviceSize MAX_VERTICES = 1024 * 1024;
    static const VkDeviceSize MAX_HUD_VERTICES = 65536;

    void init();
    void cleanup();
    void updateTriangles(const std::vector<Vertex>& verts);
    void updateLines(const std::vector<Vertex>& verts);
    void updateHud(const std::vector<HudVertex>& verts);
    void updateUniform(const UniformBufferObject& ubo);
    void drawFrame();

private:
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createDevice();
    void createSwapchain();
    void createImageViews();
    void createRenderPasses();
    void createDescriptorSetLayout();
    void createPipelines();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void createTriangleBuffer();
    void createLineBuffer();
    void createHudBuffer();
    void createUniformBuffer();
    void createDescriptorPool();
    void createSampler();
    void createBloomImages();
    void createBloomFramebuffers();
    void createBloomDescriptorSets();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    std::vector<char> readFile(const char* path);
    VkShaderModule createShaderModule(const std::vector<char>& code);
};
