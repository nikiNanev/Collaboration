#pragma once

#include "vk_types.h"

#include <deque>
#include <functional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include "vk_mem_alloc.h"

#include "camera.h"
#include "vk_descriptors.h"
#include "vk_loaders.h"
#include "vk_pipelines.h"

struct MeshAsset;
namespace fastgltf
{
    struct Mesh;
}

struct RenderObject
{
    uint32_t indexCount;
    uint32_t firstIndex;
    VkBuffer indexBuffer;

    MaterialInstance *material;
    Bounds bounds;
    glm::mat4 transform;
    VkDeviceAddress vertexBufferAddress;
};

struct DrawContext
{
    std::vector<RenderObject> OpaqueSurfaces;
    std::vector<RenderObject> TransparentSurfaces;
};

struct EngineStats
{
    float frametime;
    int triangle_count;
    int drawcall_count;
    float mesh_draw_time;
};

struct GLTFMetallic_Roughness
{
    MaterialPipeline opaquePipeline;
    MaterialPipeline transparentPipeline;

    VkDescriptorSetLayout materialLayout;

    struct MaterialConstants
    {
        glm::vec4 colorFactors;
        glm::vec4 metal_rough_factors;
        // padding, we need it anyway for uniform buffers
        uint32_t colorTexID;
        uint32_t metalRoughTexID;
        uint32_t pad1;
        uint32_t pad2;
        glm::vec4 extra[13];
    };

    struct MaterialResources
    {
        AllocatedImage colorImage;
        VkSampler colorSampler;
        AllocatedImage metalRoughImage;
        VkSampler metalRoughSampler;
        VkBuffer dataBuffer;
        uint32_t dataBufferOffset;
    };

    DescriptorWriter writer;

    void build_pipelines(VulkanEngine *engine);
    void clear_resources(VkDevice device);

    MaterialInstance write_material(VkDevice device, MaterialPass pass, const MaterialResources &resources, DescriptorAllocatorGrowable &descriptorAllocator);
};

struct MeshNode : public Node
{

    std::shared_ptr<MeshAsset> mesh;

    virtual void Draw(const glm::mat4 &topMatrix, DrawContext &ctx) override;
};

struct TextureID
{
    uint32_t Index;
};

struct TextureCache
{
    std::vector<VkDescriptorImageInfo> Cache;
    std::unordered_map<std::string, TextureID> NameMap;
    TextureID AddTexture(const VkImageView &image, VkSampler sampler);
};

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

    DescriptorAllocatorGrowable _frameDescriptors;

    DeletionQueue _deletionQueue;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanEngine
{
public:
    FrameData _frames[FRAME_OVERLAP];

    FrameData &get_current_frame() { return _frames[_frameNumber % FRAME_OVERLAP]; };
    FrameData &get_last_frame();
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
    AllocatedImage _depthImage;
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

    VkPipelineLayout _movingSquaresPipelineLayout;
    VkPipeline _movingSquaresPipeline;

    void immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function);

    AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
    GPUMeshBuffers uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);

    VkPipelineLayout _meshPipelineLayout;
    VkPipeline _meshPipeline;

    VkDescriptorSetLayout _gpuSceneDataDescriptorLayout;

    std::vector<std::shared_ptr<MeshAsset>> testMeshes;

    AllocatedImage create_image(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped);
    AllocatedImage create_image(void *data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped);

    AllocatedImage _whiteImage;
    AllocatedImage _blackImage;
    AllocatedImage _greyImage;
    AllocatedImage _errorCheckerboardImage;

    VkSampler _defaultSamplerLinear;
    VkSampler _defaultSamplerNearest;

    TextureCache texCache;

    GLTFMetallic_Roughness metalRoughMaterial;

    GPUMeshBuffers rectangle;
    DrawContext drawCommands;

    GPUSceneData sceneData;

    Camera mainCamera;

    EngineStats stats;

    std::unordered_map<std::string, std::shared_ptr<LoadedGLTF>> loadedScenes;
    std::vector<std::shared_ptr<LoadedGLTF>> brickadiaScene;

    bool resize_requested{false};
    bool freeze_rendering{false};

    void destroy_image(const AllocatedImage &img);
    // initializes everything in the engine
    void init();

    // shuts down the engine
    void cleanup();

    void destroy_buffer(const AllocatedBuffer &buffer);

    // draw loop
    void draw();

    void draw_background(VkCommandBuffer cmd);

    void draw_geometry(VkCommandBuffer cmd);

    void draw_main(VkCommandBuffer cmd);

    bool is_visible(const RenderObject &obj, const glm::mat4 &viewproj);

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
    void init_squares_pipeline();
    void init_mesh_pipeline();

    void init_default_data();
    void init_renderables();
    void update_scene();

    void init_imgui();

    void resize_swapchain();
    void draw_imgui(VkCommandBuffer cmd, VkImageView targetImageView);

    void create_swapchain(uint32_t width, uint32_t height);
    void destroy_swapchain();
};