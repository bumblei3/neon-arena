// NEON ARENA - Vulkan + SDL2 Prototype
// Particle system: CPU update, GPU point rendering
#include "particle_system.h"
#include <cstring>
#include <cmath>
#include <cstdlib>

void ParticleSystem::init(VkDevice dev, VkPhysicalDevice phys) {
    device = dev;
    physicalDevice = phys;

    // Create particle buffer
    VkDeviceSize bufferSize = MAX_PARTICLES * sizeof(Particle);
    createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 particleBuffer, particleBufferMemory);

    // Initialize particles to dead
    void* data;
    vkMapMemory(device, particleBufferMemory, 0, bufferSize, 0, &data);
    Particle* particles = static_cast<Particle*>(data);
    memset(particles, 0, bufferSize);
    vkUnmapMemory(device, particleBufferMemory);
}

void ParticleSystem::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory) {
    VkBufferCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    ci.size = size;
    ci.usage = usage;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK(vkCreateBuffer(device, &ci, nullptr, &buffer));

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device, buffer, &memReq);

    VkMemoryAllocateInfo ai{};
    ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    ai.allocationSize = memReq.size;
    ai.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);
    VK_CHECK(vkAllocateMemory(device, &ai, nullptr, &memory));
    vkBindBufferMemory(device, buffer, memory, 0);
}

uint32_t ParticleSystem::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    fprintf(stderr, "Failed to find suitable memory type\n");
    std::exit(1);
}

void ParticleSystem::emit(uint32_t count, const float* pos, const float* color, float spread, float speed) {
    void* data;
    vkMapMemory(device, particleBufferMemory, 0, MAX_PARTICLES * sizeof(Particle), 0, &data);
    Particle* particles = static_cast<Particle*>(data);

    uint32_t emitted = 0;
    activeCount = 0;
    for (uint32_t i = 0; i < MAX_PARTICLES && emitted < count; i++) {
        if (particles[i].life <= 0.0f) {
            particles[i].pos[0] = pos[0] + ((rand() % 1000) / 1000.0f - 0.5f) * spread;
            particles[i].pos[1] = pos[1] + ((rand() % 1000) / 1000.0f - 0.5f) * spread;
            particles[i].pos[2] = pos[2] + ((rand() % 1000) / 1000.0f - 0.5f) * spread;
            particles[i].vel[0] = ((rand() % 1000) / 1000.0f - 0.5f) * speed;
            particles[i].vel[1] = (rand() % 1000) / 1000.0f * speed;
            particles[i].vel[2] = ((rand() % 1000) / 1000.0f - 0.5f) * speed;
            particles[i].life = 0.5f + (rand() % 30) * 0.01f;
            particles[i].size = 0.05f;
            particles[i].color[0] = color[0];
            particles[i].color[1] = color[1];
            particles[i].color[2] = color[2];
            emitted++;
        }
    }

    vkUnmapMemory(device, particleBufferMemory);
}

void ParticleSystem::update(float dt) {
    void* data;
    vkMapMemory(device, particleBufferMemory, 0, MAX_PARTICLES * sizeof(Particle), 0, &data);
    Particle* particles = static_cast<Particle*>(data);

    activeCount = 0;
    for (uint32_t i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0.0f) {
            particles[i].vel[1] -= 9.8f * dt;
            particles[i].pos[0] += particles[i].vel[0] * dt;
            particles[i].pos[1] += particles[i].vel[1] * dt;
            particles[i].pos[2] += particles[i].vel[2] * dt;
            if (particles[i].pos[1] < 0.05f) {
                particles[i].pos[1] = 0.05f;
                particles[i].vel[1] *= -0.4f;
                particles[i].vel[0] *= 0.8f;
                particles[i].vel[2] *= 0.8f;
            }
            particles[i].life -= dt;
            activeCount++;
        }
    }

    vkUnmapMemory(device, particleBufferMemory);
}

void ParticleSystem::cleanup() {
    vkDestroyBuffer(device, particleBuffer, nullptr);
    vkFreeMemory(device, particleBufferMemory, nullptr);
}
