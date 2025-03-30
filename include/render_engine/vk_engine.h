#pragma once

#include "vk_types.h"
#include "vk_descriptors.h"

#include <glm/vec4.hpp>

struct ComputePushConstants
{
    glm::vec4 data1;
    glm::vec4 data2;
    glm::vec4 data3;
    glm::vec4 data4;
};

struct ComputeEffect
{
    const char *name;

    VkPipeline pipeline;
    VkPipelineLayout layout;

    ComputePushConstants data;
};

struct DeletionQueue
{
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()> &&function)
    {
        deletors.push_back(function);
    }

    void flush()
    {
        // reverse iterate the deletion queue to execute all the functions
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
        {
            (*it)(); // call functors
        }

        deletors.clear();
    }
};

struct FrameData
{
    VkCommandPool _commandPool;
    VkCommandBuffer _mainCommandBuffer;

    VkSemaphore _swapchainSemaphore, _renderSemaphore;
    VkFence _renderFence;

    DeletionQueue _deletionQueue;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanEngine
{
public:
    FrameData _frames[FRAME_OVERLAP];

    FrameData &get_current_frame() { return _frames[_frameNumber % FRAME_OVERLAP]; };

    VkQueue _graphicsQueue;
    uint32_t _graphicsQueueFamily;

    bool _isInitialized{false};
    int _frameNumber{0};
    bool stop_rendering{false};
    VkExtent2D _windowExtent{1700, 900};

    struct SDL_Window *_window{nullptr};

    static VulkanEngine &Get();

    VkInstance _instance;                      // Vulkan library handle
    VkDebugUtilsMessengerEXT _debug_messenger; // Vulkan debug output handle
    VkPhysicalDevice _chosenGPU;               // GPU chosen as the default device
    VkDevice _device;                          // Vulkan device for commands
    VkSurfaceKHR _surface;                     // Vulkan window surface

    VkSwapchainKHR _swapchain;
    VkFormat _swapchainImageFormat;

    std::vector<VkImage> _swapchainImages;
    std::vector<VkImageView> _swapchainImageViews;
    VkExtent2D _swapchainExtent;

    DeletionQueue _mainDeletionQueue;

    VmaAllocator _allocator;

    // draw resources
    AllocatedImage _drawImage;
    VkExtent2D _drawExtent;

    DescriptorAllocator globalDescriptorAllocator;

    VkDescriptorSet _drawImageDescriptors;
    VkDescriptorSetLayout _drawImageDescriptorLayout;

    VkPipeline _gradientPipeline;
    VkPipelineLayout _gradientPipelineLayout;

    // immediate submit structures
    VkFence _immFence;
    VkCommandBuffer _immCommandBuffer;
    VkCommandPool _immCommandPool;

    std::vector<ComputeEffect> backgroundEffects;
    int currentBackgroundEffect{0};

    VkPipelineLayout _trianglePipelineLayout;
    VkPipeline _trianglePipeline;

    void immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function);

    AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
    GPUMeshBuffers uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);

    VkPipelineLayout _meshPipelineLayout;
    VkPipeline _meshPipeline;

    GPUMeshBuffers rectangle;

    // initializes everything in the engine
    void init();

    // shuts down the engine
    void cleanup();

    void destroy_buffer(const AllocatedBuffer &buffer);

    // draw loop
    void draw();

    void draw_background(VkCommandBuffer cmd);

    void draw_geometry(VkCommandBuffer cmd);

    // run main loop
    void run();

private:
    void init_vulkan();
    void init_swapchain();
    void init_commands();
    void init_sync_structures();
    void init_descriptors();

    void init_pipelines();
    void init_background_pipelines();
    void init_triangle_pipeline();
    void init_mesh_pipeline();

    void init_default_data();

    void init_imgui();

    void draw_imgui(VkCommandBuffer cmd, VkImageView targetImageView);

    void create_swapchain(uint32_t width, uint32_t height);
    void destroy_swapchain();
};
