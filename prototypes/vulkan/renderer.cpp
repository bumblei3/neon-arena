// NEON ARENA - Vulkan + SDL2 Prototype
// Renderer implementation with Bloom post-processing, HUD, and Particles
#include "renderer.h"
#include <cstring>
#include <vector>
#include <fstream>

void Renderer::init() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("NEON ARENA - Vulkan + Bloom",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);

    createInstance();
    createSurface();
    pickPhysicalDevice();
    createDevice();
    createSwapchain();
    createImageViews();
    createRenderPasses();
    createDescriptorSetLayout();
    createPipelines();
    createFramebuffers();
    createCommandPool();
    createTriangleBuffer();
    createLineBuffer();
    createHudBuffer();
    createParticleBuffer();
    createUniformBuffer();
    createDescriptorPool();
    createSampler();
    createBloomImages();
    createBloomFramebuffers();
    createBloomDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
}

void Renderer::createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "NeonArena";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "NeonVulkan";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    uint32_t extCount = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &extCount, nullptr);
    std::vector<const char*> extensions(extCount);
    SDL_Vulkan_GetInstanceExtensions(window, &extCount, extensions.data());

    VkInstanceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &appInfo;
    ci.enabledExtensionCount = extCount;
    ci.ppEnabledExtensionNames = extensions.data();
    VK_CHECK(vkCreateInstance(&ci, nullptr, &instance));
}

void Renderer::createSurface() {
    if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
        fprintf(stderr, "SDL_Vulkan_CreateSurface failed: %s\n", SDL_GetError());
        std::exit(1);
    }
}

void Renderer::pickPhysicalDevice() {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, devices.data());

    for (auto& d : devices) {
        uint32_t qfCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(d, &qfCount, nullptr);
        std::vector<VkQueueFamilyProperties> qf(qfCount);
        vkGetPhysicalDeviceQueueFamilyProperties(d, &qfCount, qf.data());

        bool hasGraphics = false, hasPresent = false;
        for (uint32_t i = 0; i < qfCount; i++) {
            if (qf[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsFamily = i;
                hasGraphics = true;
            }
            VkBool32 present = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(d, i, surface, &present);
            if (present) {
                presentFamily = i;
                hasPresent = true;
            }
        }
        if (hasGraphics && hasPresent) {
            physicalDevice = d;
            return;
        }
    }
    fprintf(stderr, "No suitable GPU found\n");
    std::exit(1);
}

void Renderer::createDevice() {
    float prio = 1.0f;
    VkDeviceQueueCreateInfo qci{};
    qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.queueFamilyIndex = graphicsFamily;
    qci.queueCount = 1;
    qci.pQueuePriorities = &prio;

    const char* ext = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    VkDeviceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ci.queueCreateInfoCount = 1;
    ci.pQueueCreateInfos = &qci;
    ci.enabledExtensionCount = 1;
    ci.ppEnabledExtensionNames = &ext;
    VK_CHECK(vkCreateDevice(physicalDevice, &ci, nullptr, &device));

    vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentFamily, 0, &presentQueue);
}

void Renderer::createSwapchain() {
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);

    swapchainExtent = caps.currentExtent;
    if (swapchainExtent.width == UINT32_MAX) {
        swapchainExtent.width = (uint32_t)WIDTH;
        swapchainExtent.height = (uint32_t)HEIGHT;
    }

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
        imageCount = caps.maxImageCount;

    VkSwapchainCreateInfoKHR ci{};
    ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    ci.surface = surface;
    ci.minImageCount = imageCount;
    ci.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    ci.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    ci.imageExtent = swapchainExtent;
    ci.imageArrayLayers = 1;
    ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.preTransform = caps.currentTransform;
    ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    ci.clipped = VK_TRUE;
    VK_CHECK(vkCreateSwapchainKHR(device, &ci, nullptr, &swapchain));

    swapchainFormat = VK_FORMAT_B8G8R8A8_SRGB;
    uint32_t scCount = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &scCount, nullptr);
    swapchainImages.resize(scCount);
    vkGetSwapchainImagesKHR(device, swapchain, &scCount, swapchainImages.data());
}

void Renderer::createImageViews() {
    swapchainImageViews.resize(swapchainImages.size());
    for (size_t i = 0; i < swapchainImages.size(); i++) {
        VkImageViewCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ci.image = swapchainImages[i];
        ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ci.format = swapchainFormat;
        ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ci.subresourceRange.levelCount = 1;
        ci.subresourceRange.layerCount = 1;
        VK_CHECK(vkCreateImageView(device, &ci, nullptr, &swapchainImageViews[i]));
    }
}

void Renderer::createRenderPasses() {
    VkAttachmentReference ref{};
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &ref;

    // Scene render pass
    {
        VkAttachmentDescription color{};
        color.format = swapchainFormat;
        color.samples = VK_SAMPLE_COUNT_1_BIT;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkRenderPassCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        ci.attachmentCount = 1;
        ci.pAttachments = &color;
        ci.subpassCount = 1;
        ci.pSubpasses = &subpass;
        VK_CHECK(vkCreateRenderPass(device, &ci, nullptr, &renderPass));
    }

    // Bloom render pass
    {
        VkAttachmentDescription color{};
        color.format = swapchainFormat;
        color.samples = VK_SAMPLE_COUNT_1_BIT;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkRenderPassCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        ci.attachmentCount = 1;
        ci.pAttachments = &color;
        ci.subpassCount = 1;
        ci.pSubpasses = &subpass;
        VK_CHECK(vkCreateRenderPass(device, &ci, nullptr, &bloomRenderPass));
    }

    // Composite render pass
    {
        VkAttachmentDescription color{};
        color.format = swapchainFormat;
        color.samples = VK_SAMPLE_COUNT_1_BIT;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkRenderPassCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        ci.attachmentCount = 1;
        ci.pAttachments = &color;
        ci.subpassCount = 1;
        ci.pSubpasses = &subpass;
        VK_CHECK(vkCreateRenderPass(device, &ci, nullptr, &compositeRenderPass));
    }
}

void Renderer::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboBinding{};
    uboBinding.binding = 0;
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1;
    uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ci.bindingCount = 1;
    ci.pBindings = &uboBinding;
    VK_CHECK(vkCreateDescriptorSetLayout(device, &ci, nullptr, &descriptorSetLayout));

    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 0;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.descriptorCount = 2;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo sci{};
    sci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    sci.bindingCount = 1;
    sci.pBindings = &samplerBinding;
    VK_CHECK(vkCreateDescriptorSetLayout(device, &sci, nullptr, &bloomDescriptorSetLayout));
}

void Renderer::createPipelines() {
    auto vertSrc = readFile(SHADER_DIR "/triangle.vert.spv");
    auto fragSrc = readFile(SHADER_DIR "/triangle.frag.spv");

    VkShaderModule vertModule = createShaderModule(vertSrc);
    VkShaderModule fragModule = createShaderModule(fragSrc);

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vertModule;
    stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fragModule;
    stages[1].pName = "main";

    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = sizeof(Vertex);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attrs[2]{};
    attrs[0].binding = 0;
    attrs[0].location = 0;
    attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attrs[0].offset = offsetof(Vertex, pos);
    attrs[1].binding = 0;
    attrs[1].location = 1;
    attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attrs[1].offset = offsetof(Vertex, color);

    VkPipelineVertexInputStateCreateInfo vi{};
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.vertexBindingDescriptionCount = 1;
    vi.pVertexBindingDescriptions = &binding;
    vi.vertexAttributeDescriptionCount = 2;
    vi.pVertexAttributeDescriptions = attrs;

    VkPipelineInputAssemblyStateCreateInfo iaTri{};
    iaTri.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    iaTri.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineInputAssemblyStateCreateInfo iaLine{};
    iaLine.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    iaLine.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    VkPipelineInputAssemblyStateCreateInfo iaPoint{};
    iaPoint.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    iaPoint.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

    VkViewport viewport{0, 0, (float)swapchainExtent.width, (float)swapchainExtent.height, 0, 1};
    VkRect2D scissor{{0, 0}, swapchainExtent};
    VkPipelineViewportStateCreateInfo vp{};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.pViewports = &viewport;
    vp.scissorCount = 1;
    vp.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rs{};
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.lineWidth = 2.0f;
    rs.cullMode = VK_CULL_MODE_NONE;

    VkPipelineMultisampleStateCreateInfo ms{};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState blendAttach{};
    blendAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAttach.blendEnable = VK_TRUE;
    blendAttach.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttach.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttach.colorBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo cb{};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = &blendAttach;

    VkPipelineLayoutCreateInfo plci{};
    plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plci.setLayoutCount = 1;
    plci.pSetLayouts = &descriptorSetLayout;
    VK_CHECK(vkCreatePipelineLayout(device, &plci, nullptr, &pipelineLayout));

    VkGraphicsPipelineCreateInfo pci{};
    pci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pci.stageCount = 2;
    pci.pStages = stages;
    pci.pVertexInputState = &vi;
    pci.pViewportState = &vp;
    pci.pRasterizationState = &rs;
    pci.pMultisampleState = &ms;
    pci.pColorBlendState = &cb;
    pci.layout = pipelineLayout;
    pci.renderPass = renderPass;

    pci.pInputAssemblyState = &iaTri;
    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pci, nullptr, &trianglePipeline));

    pci.pInputAssemblyState = &iaLine;
    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pci, nullptr, &linePipeline));

    pci.pInputAssemblyState = &iaPoint;
    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pci, nullptr, &particlePipeline));

    vkDestroyShaderModule(device, vertModule, nullptr);
    vkDestroyShaderModule(device, fragModule, nullptr);

    // HUD pipeline
    VkPipelineVertexInputStateCreateInfo hudVi{};
    hudVi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VkVertexInputBindingDescription hudBinding{};
    hudBinding.binding = 0;
    hudBinding.stride = sizeof(HudVertex);
    hudBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription hudAttrs[3]{};
    hudAttrs[0].binding = 0;
    hudAttrs[0].location = 0;
    hudAttrs[0].format = VK_FORMAT_R32G32_SFLOAT;
    hudAttrs[0].offset = offsetof(HudVertex, pos);
    hudAttrs[1].binding = 0;
    hudAttrs[1].location = 1;
    hudAttrs[1].format = VK_FORMAT_R32G32_SFLOAT;
    hudAttrs[1].offset = offsetof(HudVertex, uv);
    hudAttrs[2].binding = 0;
    hudAttrs[2].location = 2;
    hudAttrs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    hudAttrs[2].offset = offsetof(HudVertex, color);

    hudVi.vertexBindingDescriptionCount = 1;
    hudVi.pVertexBindingDescriptions = &hudBinding;
    hudVi.vertexAttributeDescriptionCount = 3;
    hudVi.pVertexAttributeDescriptions = hudAttrs;

    VkPipelineLayoutCreateInfo hudPlci{};
    hudPlci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    VK_CHECK(vkCreatePipelineLayout(device, &hudPlci, nullptr, &hudPipelineLayout));

    VkGraphicsPipelineCreateInfo hudPci{};
    hudPci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    hudPci.stageCount = 2;
    hudPci.pStages = stages;
    hudPci.pVertexInputState = &hudVi;
    hudPci.pInputAssemblyState = &iaTri;
    hudPci.pViewportState = &vp;
    hudPci.pRasterizationState = &rs;
    hudPci.pMultisampleState = &ms;
    hudPci.pColorBlendState = &cb;
    hudPci.layout = hudPipelineLayout;
    hudPci.renderPass = renderPass;
    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &hudPci, nullptr, &hudPipeline));

    // Bloom pipelines
    VkPipelineVertexInputStateCreateInfo nullVi{};
    nullVi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VkPipelineInputAssemblyStateCreateInfo iaTriOnly{};
    iaTriOnly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    iaTriOnly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPushConstantRange pcr{};
    pcr.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pcr.offset = 0;
    pcr.size = sizeof(float) * 3;

    VkPipelineLayoutCreateInfo bplci{};
    bplci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    bplci.setLayoutCount = 1;
    bplci.pSetLayouts = &bloomDescriptorSetLayout;
    bplci.pushConstantRangeCount = 1;
    bplci.pPushConstantRanges = &pcr;
    VK_CHECK(vkCreatePipelineLayout(device, &bplci, nullptr, &bloomPipelineLayout));

    VkGraphicsPipelineCreateInfo bpci{};
    bpci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    bpci.pVertexInputState = &nullVi;
    bpci.pInputAssemblyState = &iaTriOnly;
    bpci.pViewportState = &vp;
    bpci.pRasterizationState = &rs;
    bpci.pMultisampleState = &ms;
    bpci.pColorBlendState = &cb;
    bpci.layout = bloomPipelineLayout;
    bpci.renderPass = bloomRenderPass;

    auto createBloomPipeline = [this, &bpci](const char* vertPath, const char* fragPath, VkPipeline& pipeline) {
        auto vSrc = readFile(vertPath);
        auto fSrc = readFile(fragPath);
        VkShaderModule vMod = createShaderModule(vSrc);
        VkShaderModule fMod = createShaderModule(fSrc);
        VkPipelineShaderStageCreateInfo sStages[2]{};
        sStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        sStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        sStages[0].module = vMod;
        sStages[0].pName = "main";
        sStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        sStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        sStages[1].module = fMod;
        sStages[1].pName = "main";
        bpci.stageCount = 2;
        bpci.pStages = sStages;
        VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &bpci, nullptr, &pipeline));
        vkDestroyShaderModule(device, vMod, nullptr);
        vkDestroyShaderModule(device, fMod, nullptr);
    };

    createBloomPipeline(SHADER_DIR "/fullscreen.vert.spv", SHADER_DIR "/brightpass.frag.spv", brightPipeline);
    createBloomPipeline(SHADER_DIR "/fullscreen.vert.spv", SHADER_DIR "/blur.frag.spv", blurPipeline);
    bpci.renderPass = compositeRenderPass;
    createBloomPipeline(SHADER_DIR "/fullscreen.vert.spv", SHADER_DIR "/composite.frag.spv", compositePipeline);
}

void Renderer::createFramebuffers() {
    framebuffers.resize(swapchainImageViews.size());
    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        VkFramebufferCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        ci.renderPass = compositeRenderPass;
        ci.attachmentCount = 1;
        ci.pAttachments = &swapchainImageViews[i];
        ci.width = swapchainExtent.width;
        ci.height = swapchainExtent.height;
        ci.layers = 1;
        VK_CHECK(vkCreateFramebuffer(device, &ci, nullptr, &framebuffers[i]));
    }
}

void Renderer::createCommandPool() {
    VkCommandPoolCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    ci.queueFamilyIndex = graphicsFamily;
    ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(device, &ci, nullptr, &commandPool));
}

void Renderer::createCommandBuffers() {
    commandBuffers.resize(framebuffers.size());
    VkCommandBufferAllocateInfo ai{};
    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandPool = commandPool;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = (uint32_t)commandBuffers.size();
    VK_CHECK(vkAllocateCommandBuffers(device, &ai, commandBuffers.data()));
}

void Renderer::createSyncObjects() {
    VkSemaphoreCreateInfo sci{};
    sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fci{};
    fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VK_CHECK(vkCreateSemaphore(device, &sci, nullptr, &imageAvailable));
    VK_CHECK(vkCreateSemaphore(device, &sci, nullptr, &renderFinished));
    VK_CHECK(vkCreateFence(device, &fci, nullptr, &inFlight));
}

void Renderer::createTriangleBuffer() {
    VkDeviceSize size = MAX_VERTICES * sizeof(Vertex);
    createBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, triangleBuffer, triangleBufferMemory);
}

void Renderer::createLineBuffer() {
    VkDeviceSize size = MAX_VERTICES * sizeof(Vertex);
    createBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, lineBuffer, lineBufferMemory);
}

void Renderer::createHudBuffer() {
    VkDeviceSize size = MAX_HUD_VERTICES * sizeof(HudVertex);
    createBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, hudBuffer, hudBufferMemory);
}

void Renderer::createParticleBuffer() {
    VkDeviceSize size = MAX_PARTICLES * sizeof(Vertex);
    createBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, particleBuffer, particleBufferMemory);
}

void Renderer::createUniformBuffer() {
    VkDeviceSize size = sizeof(UniformBufferObject);
    createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer, uniformBufferMemory);
}

void Renderer::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = (uint32_t)swapchainImages.size();

    VkDescriptorPoolCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ci.poolSizeCount = 1;
    ci.pPoolSizes = &poolSize;
    ci.maxSets = (uint32_t)swapchainImages.size();
    VK_CHECK(vkCreateDescriptorPool(device, &ci, nullptr, &descriptorPool));

    std::vector<VkDescriptorSetLayout> layouts(swapchainImages.size(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo ai{};
    ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    ai.descriptorPool = descriptorPool;
    ai.descriptorSetCount = (uint32_t)swapchainImages.size();
    ai.pSetLayouts = layouts.data();
    descriptorSets.resize(swapchainImages.size());
    VK_CHECK(vkAllocateDescriptorSets(device, &ai, descriptorSets.data()));

    for (size_t i = 0; i < swapchainImages.size(); i++) {
        VkDescriptorBufferInfo bi{};
        bi.buffer = uniformBuffer;
        bi.offset = 0;
        bi.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptorSets[i];
        write.dstBinding = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.pBufferInfo = &bi;
        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
    }
}

void Renderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory) {
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

uint32_t Renderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    fprintf(stderr, "Failed to find suitable memory type\n");
    std::exit(1);
}

std::vector<char> Renderer::readFile(const char* path) {
    std::ifstream f(path, std::ios::ate | std::ios::binary);
    if (!f.is_open()) {
        fprintf(stderr, "Failed to open %s\n", path);
        return {};
    }
    size_t size = f.tellg();
    std::vector<char> buf(size);
    f.seekg(0);
    f.read(buf.data(), size);
    return buf;
}

VkShaderModule Renderer::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = code.size();
    ci.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule mod;
    VK_CHECK(vkCreateShaderModule(device, &ci, nullptr, &mod));
    return mod;
}

void Renderer::updateTriangles(const std::vector<Vertex>& verts) {
    VkDeviceSize size = verts.size() * sizeof(Vertex);
    void* data;
    vkMapMemory(device, triangleBufferMemory, 0, size, 0, &data);
    memcpy(data, verts.data(), size);
    vkUnmapMemory(device, triangleBufferMemory);
    triangleVerts_ = (uint32_t)verts.size();
}

void Renderer::updateLines(const std::vector<Vertex>& verts) {
    VkDeviceSize size = verts.size() * sizeof(Vertex);
    void* data;
    vkMapMemory(device, lineBufferMemory, 0, size, 0, &data);
    memcpy(data, verts.data(), size);
    vkUnmapMemory(device, lineBufferMemory);
    lineVerts_ = (uint32_t)verts.size();
}

void Renderer::updateHud(const std::vector<HudVertex>& verts) {
    VkDeviceSize size = verts.size() * sizeof(HudVertex);
    void* data;
    vkMapMemory(device, hudBufferMemory, 0, size, 0, &data);
    memcpy(data, verts.data(), size);
    vkUnmapMemory(device, hudBufferMemory);
    hudVerts_ = (uint32_t)verts.size();
}

void Renderer::updateParticles(const std::vector<Vertex>& verts, uint32_t count) {
    VkDeviceSize size = verts.size() * sizeof(Vertex);
    void* data;
    vkMapMemory(device, particleBufferMemory, 0, size, 0, &data);
    memcpy(data, verts.data(), size);
    vkUnmapMemory(device, particleBufferMemory);
    particleVerts_ = count;
}

void Renderer::updateUniform(const UniformBufferObject& ubo) {
    void* data;
    vkMapMemory(device, uniformBufferMemory, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, uniformBufferMemory);
}

void Renderer::createSampler() {
    VkSamplerCreateInfo sci{};
    sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sci.magFilter = VK_FILTER_LINEAR;
    sci.minFilter = VK_FILTER_LINEAR;
    sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sci.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    VK_CHECK(vkCreateSampler(device, &sci, nullptr, &textureSampler));
}

void Renderer::createBloomImages() {
    VkImageCreateInfo ii{};
    ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ii.imageType = VK_IMAGE_TYPE_2D;
    ii.format = swapchainFormat;
    ii.extent = {swapchainExtent.width, swapchainExtent.height, 1};
    ii.mipLevels = 1;
    ii.arrayLayers = 1;
    ii.samples = VK_SAMPLE_COUNT_1_BIT;
    ii.tiling = VK_IMAGE_TILING_OPTIMAL;
    ii.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    auto createImageWithView = [this](VkImage& img, VkDeviceMemory& mem, VkImageView& view) {
        VK_CHECK(vkCreateImage(device, &ii, nullptr, &img));
        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(device, img, &req);
        VkMemoryAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize = req.size;
        ai.memoryTypeIndex = findMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK(vkAllocateMemory(device, &ai, nullptr, &mem));
        vkBindImageMemory(device, img, mem, 0);

        VkImageViewCreateInfo ivci{};
        ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.image = img;
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.format = swapchainFormat;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ivci.subresourceRange.levelCount = 1;
        ivci.subresourceRange.layerCount = 1;
        VK_CHECK(vkCreateImageView(device, &ivci, nullptr, &view));
    };

    createImageWithView(bloomImage1, bloomMemory1, bloomView1);
    createImageWithView(bloomImage2, bloomMemory2, bloomView2);
    createImageWithView(bloomImage3, bloomMemory3, bloomView3);
}

void Renderer::createBloomFramebuffers() {
    VkFramebufferCreateInfo fci{};
    fci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fci.renderPass = bloomRenderPass;
    fci.attachmentCount = 1;
    fci.width = swapchainExtent.width;
    fci.height = swapchainExtent.height;
    fci.layers = 1;

    fci.pAttachments = &bloomView1;
    VK_CHECK(vkCreateFramebuffer(device, &fci, nullptr, &bloomFramebuffer1));
    fci.pAttachments = &bloomView2;
    VK_CHECK(vkCreateFramebuffer(device, &fci, nullptr, &bloomFramebuffer2));
    fci.pAttachments = &bloomView3;
    VK_CHECK(vkCreateFramebuffer(device, &fci, nullptr, &bloomFramebuffer3));
}

void Renderer::createBloomDescriptorSets() {
    VkDescriptorPoolSize samplerSize{};
    samplerSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerSize.descriptorCount = 6;

    VkDescriptorPoolCreateInfo dpci{};
    dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpci.poolSizeCount = 1;
    dpci.pPoolSizes = &samplerSize;
    dpci.maxSets = 6;
    VK_CHECK(vkCreateDescriptorPool(device, &dpci, nullptr, &bloomDescriptorPool));

    std::vector<VkDescriptorSetLayout> bloomLayouts(6, bloomDescriptorSetLayout);
    VkDescriptorSetAllocateInfo dsai{};
    dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsai.descriptorPool = bloomDescriptorPool;
    dsai.descriptorSetCount = 6;
    dsai.pSetLayouts = bloomLayouts.data();
    std::vector<VkDescriptorSet> bloomSets(6);
    VK_CHECK(vkAllocateDescriptorSets(device, &dsai, bloomSets.data()));

    VkDescriptorImageInfo sceneImg{};
    sceneImg.imageView = bloomView1;
    sceneImg.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    sceneImg.sampler = textureSampler;

    VkDescriptorImageInfo blur1Img{};
    blur1Img.imageView = bloomView2;
    blur1Img.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    blur1Img.sampler = textureSampler;

    VkDescriptorImageInfo blur2Img{};
    blur2Img.imageView = bloomView3;
    blur2Img.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    blur2Img.sampler = textureSampler;

    VkWriteDescriptorSet writes[6]{};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = bloomSets[0];
    writes[0].dstBinding = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[0].pImageInfo = &sceneImg;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = bloomSets[1];
    writes[1].dstBinding = 0;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].pImageInfo = &blur1Img;

    writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[2].dstSet = bloomSets[2];
    writes[2].dstBinding = 0;
    writes[2].descriptorCount = 1;
    writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[2].pImageInfo = &blur2Img;

    VkDescriptorImageInfo compositeImgs[2] = {sceneImg, blur2Img};
    writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[3].dstSet = bloomSets[3];
    writes[3].dstBinding = 0;
    writes[3].descriptorCount = 2;
    writes[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[3].pImageInfo = compositeImgs;

    vkUpdateDescriptorSets(device, 4, writes, 0, nullptr);

    bloomDescriptorSet1 = bloomSets[0];
    bloomDescriptorSet2 = bloomSets[1];
    bloomDescriptorSet3 = bloomSets[2];
    compositeDescriptorSet = bloomSets[3];
}

void Renderer::drawFrame() {
    vkWaitForFences(device, 1, &inFlight, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &inFlight);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(commandBuffers[imageIndex], 0);
    VkCommandBufferBeginInfo bi{};
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VK_CHECK(vkBeginCommandBuffer(commandBuffers[imageIndex], &bi));

    // --- Pass 1: Render scene + HUD + Particles to offscreen buffer ---
    VkRenderPassBeginInfo scenePass{};
    scenePass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    scenePass.renderPass = renderPass;
    scenePass.framebuffer = bloomFramebuffer1;
    scenePass.renderArea.extent = swapchainExtent;
    VkClearValue clear{{0.02f, 0.02f, 0.05f, 1.0f}};
    scenePass.clearValueCount = 1;
    scenePass.pClearValues = &clear;
    vkCmdBeginRenderPass(commandBuffers[imageIndex], &scenePass, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[imageIndex], 0, nullptr);

    if (lineVerts_ > 0) {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffers[imageIndex], 0, 1, &lineBuffer, &offset);
        vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, linePipeline);
        vkCmdDraw(commandBuffers[imageIndex], lineVerts_, 1, 0, 0);
    }

    if (triangleVerts_ > 0) {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffers[imageIndex], 0, 1, &triangleBuffer, &offset);
        vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline);
        vkCmdDraw(commandBuffers[imageIndex], triangleVerts_, 1, 0, 0);
    }

    if (particleVerts_ > 0) {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffers[imageIndex], 0, 1, &particleBuffer, &offset);
        vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, particlePipeline);
        vkCmdDraw(commandBuffers[imageIndex], particleVerts_, 1, 0, 0);
    }

    if (hudVerts_ > 0) {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffers[imageIndex], 0, 1, &hudBuffer, &offset);
        vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, hudPipeline);
        vkCmdDraw(commandBuffers[imageIndex], hudVerts_, 1, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffers[imageIndex]);

    // --- Pass 2: Bright pass ---
    VkRenderPassBeginInfo brightPassInfo{};
    brightPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    brightPassInfo.renderPass = bloomRenderPass;
    brightPassInfo.framebuffer = bloomFramebuffer2;
    brightPassInfo.renderArea.extent = swapchainExtent;
    vkCmdBeginRenderPass(commandBuffers[imageIndex], &brightPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, brightPipeline);
    vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, bloomPipelineLayout, 0, 1, &bloomDescriptorSet1, 0, nullptr);
    vkCmdDraw(commandBuffers[imageIndex], 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffers[imageIndex]);

    // --- Pass 3: Blur H ---
    VkRenderPassBeginInfo blurPassInfo{};
    blurPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    blurPassInfo.renderPass = bloomRenderPass;
    blurPassInfo.framebuffer = bloomFramebuffer3;
    blurPassInfo.renderArea.extent = swapchainExtent;
    vkCmdBeginRenderPass(commandBuffers[imageIndex], &blurPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, blurPipeline);
    vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, bloomPipelineLayout, 0, 1, &bloomDescriptorSet2, 0, nullptr);
    float dirH[3] = {1.0f, 0.0f, 2.0f};
    vkCmdPushConstants(commandBuffers[imageIndex], bloomPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(dirH), dirH);
    vkCmdDraw(commandBuffers[imageIndex], 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffers[imageIndex]);

    // --- Pass 4: Blur V ---
    blurPassInfo.framebuffer = bloomFramebuffer2;
    vkCmdBeginRenderPass(commandBuffers[imageIndex], &blurPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, blurPipeline);
    vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, bloomPipelineLayout, 0, 1, &bloomDescriptorSet3, 0, nullptr);
    float dirV[3] = {0.0f, 1.0f, 2.0f};
    vkCmdPushConstants(commandBuffers[imageIndex], bloomPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(dirV), dirV);
    vkCmdDraw(commandBuffers[imageIndex], 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffers[imageIndex]);

    // --- Pass 5: Composite ---
    VkRenderPassBeginInfo compositePass{};
    compositePass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    compositePass.renderPass = compositeRenderPass;
    compositePass.framebuffer = framebuffers[imageIndex];
    compositePass.renderArea.extent = swapchainExtent;
    VkClearValue compositeClear{{0.0f, 0.0f, 0.0f, 1.0f}};
    compositePass.clearValueCount = 1;
    compositePass.pClearValues = &compositeClear;
    vkCmdBeginRenderPass(commandBuffers[imageIndex], &compositePass, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, compositePipeline);
    vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, bloomPipelineLayout, 0, 1, &compositeDescriptorSet, 0, nullptr);
    vkCmdDraw(commandBuffers[imageIndex], 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffers[imageIndex]);

    VK_CHECK(vkEndCommandBuffer(commandBuffers[imageIndex]));

    VkSubmitInfo si{};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {imageAvailable};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = waitSemaphores;
    si.pWaitDstStageMask = waitStages;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &commandBuffers[imageIndex];
    VkSemaphore signalSemaphores[] = {renderFinished};
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = signalSemaphores;
    VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &si, inFlight));

    VkPresentInfoKHR pi{};
    pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = signalSemaphores;
    pi.swapchainCount = 1;
    pi.pSwapchains = &swapchain;
    pi.pImageIndices = &imageIndex;
    vkQueuePresentKHR(presentQueue, &pi);
}

void Renderer::cleanup() {
    vkDestroySemaphore(device, imageAvailable, nullptr);
    vkDestroySemaphore(device, renderFinished, nullptr);
    vkDestroyFence(device, inFlight, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    for (auto fb : framebuffers) vkDestroyFramebuffer(device, fb, nullptr);
    vkDestroyFramebuffer(device, bloomFramebuffer1, nullptr);
    vkDestroyFramebuffer(device, bloomFramebuffer2, nullptr);
    vkDestroyFramebuffer(device, bloomFramebuffer3, nullptr);
    vkDestroyPipeline(device, trianglePipeline, nullptr);
    vkDestroyPipeline(device, linePipeline, nullptr);
    vkDestroyPipeline(device, hudPipeline, nullptr);
    vkDestroyPipeline(device, particlePipeline, nullptr);
    vkDestroyPipeline(device, brightPipeline, nullptr);
    vkDestroyPipeline(device, blurPipeline, nullptr);
    vkDestroyPipeline(device, compositePipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyPipelineLayout(device, hudPipelineLayout, nullptr);
    vkDestroyPipelineLayout(device, bloomPipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, bloomDescriptorSetLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    vkDestroyRenderPass(device, bloomRenderPass, nullptr);
    vkDestroyRenderPass(device, compositeRenderPass, nullptr);
    for (auto iv : swapchainImageViews) vkDestroyImageView(device, iv, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyBuffer(device, triangleBuffer, nullptr);
    vkFreeMemory(device, triangleBufferMemory, nullptr);
    vkDestroyBuffer(device, lineBuffer, nullptr);
    vkFreeMemory(device, lineBufferMemory, nullptr);
    vkDestroyBuffer(device, hudBuffer, nullptr);
    vkFreeMemory(device, hudBufferMemory, nullptr);
    vkDestroyBuffer(device, particleBuffer, nullptr);
    vkFreeMemory(device, particleBufferMemory, nullptr);
    vkDestroyBuffer(device, uniformBuffer, nullptr);
    vkFreeMemory(device, uniformBufferMemory, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorPool(device, bloomDescriptorPool, nullptr);
    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImage(device, bloomImage1, nullptr);
    vkFreeMemory(device, bloomMemory1, nullptr);
    vkDestroyImageView(device, bloomView1, nullptr);
    vkDestroyImage(device, bloomImage2, nullptr);
    vkFreeMemory(device, bloomMemory2, nullptr);
    vkDestroyImageView(device, bloomView2, nullptr);
    vkDestroyImage(device, bloomImage3, nullptr);
    vkFreeMemory(device, bloomMemory3, nullptr);
    vkDestroyImageView(device, bloomView3, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
