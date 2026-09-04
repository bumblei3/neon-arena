// NEON ARENA - Vulkan + SDL2 Prototype
// Compute particle demo (separate binary)
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

struct Particle {
    float pos[3];
    float vel[3];
    float life;
    float size;
    float color[3];
};

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    
    printf("Compute particle demo - placeholder\n");
    printf("The compute shader is compiled but not yet integrated into the main renderer.\n");
    printf("Use the main neon-arena binary for the full game.\n");
    
    return 0;
}
