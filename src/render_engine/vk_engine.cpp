#define VMA_IMPLEMENTATION
#include "../../include/render_engine/vk_mem_alloc.h"

#include "../../include/render_engine/vk_engine.h"
#include "../../include/render_engine/vk_images.h"
#include "../../include/render_engine/vk_descriptors.h"
#include "../../include/render_engine/vk_initializers.h"
#include "../../include/render_engine/vk_pipelines.h"
#include "../../include/render_engine/vk_types.h"

#include "../../third_party/imgui/imgui.h"
#include "../../third_party/imgui/backends/imgui_impl_sdl3.h"
#include "../../third_party/imgui/backends/imgui_impl_vulkan.h"

#include <glm/gtx/transform.hpp>

#include <VkBootstrap.h>

#include <iostream>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <chrono>
#include <thread>

std::string SHADERS_PATH = "../src/render_engine/shaders/";

constexpr bool bUseValidationLayers = true;

VulkanEngine *loadedEngine = nullptr;

VulkanEngine &VulkanEngine::Get() { return *loadedEngine; }

void VulkanEngine::init()
{
    // only one engine initialization is allowed with the application.
    assert(loadedEngine == nullptr);
    loadedEngine = this;

    // We initialize SDL and create a window with it.
    SDL_Init(SDL_INIT_VIDEO);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

    _window = SDL_CreateWindow(
        "Collaboration v1.0",
        _windowExtent.width,
        _windowExtent.height,
        window_flags);

    init_vulkan();

    init_swapchain();

    init_commands();

    init_sync_structures();

    init_descriptors();

    init_pipelines();

    init_imgui();

    _isInitialized = true;
}

void VulkanEngine::init_vulkan()
{
    vkb::InstanceBuilder builder;

    // make the vulkan instance, with basic debug features
    auto inst_ret = builder.set_app_name("Vulkan Application")
                        .request_validation_layers(bUseValidationLayers)
                        .use_default_debug_messenger()
                        .require_api_version(1, 3, 0)
                        .build();

    vkb::Instance vkb_inst = inst_ret.value();

    // grab the instance
    _instance = vkb_inst.instance;
    _debug_messenger = vkb_inst.debug_messenger;

    SDL_Vulkan_CreateSurface(_window, _instance, NULL, &_surface);

    VkPhysicalDeviceVulkan13Features features13{};
    features13.dynamicRendering = true;
    features13.synchronization2 = true;

    VkPhysicalDeviceVulkan12Features features12{};
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;
    features12.descriptorBindingPartiallyBound = true;
    features12.descriptorBindingVariableDescriptorCount = true;
    features12.runtimeDescriptorArray = true;

    VkPhysicalDeviceFeatures features{};
    features.fillModeNonSolid = true;
    features.geometryShader = true;

    // use vkbootstrap to select a gpu.
    // We want a gpu that can write to the SDL surface and supports vulkan 1.2
    vkb::PhysicalDeviceSelector selector{vkb_inst};
    vkb::PhysicalDevice physicalDevice = selector.set_minimum_version(1, 3).set_required_features(features).set_required_features_13(features13).set_required_features_12(features12).set_surface(_surface).select().value();

    // physicalDevice.features.
    // create the final vulkan device

    vkb::DeviceBuilder deviceBuilder{physicalDevice};

    vkb::Device vkbDevice = deviceBuilder.build().value();

    // Get the VkDevice handle used in the rest of a vulkan application
    _device = vkbDevice.device;
    _chosenGPU = physicalDevice.physical_device;

    // use vkbootstrap to get a Graphics queue
    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();

    _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    // initialize the memory allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _chosenGPU;
    allocatorInfo.device = _device;
    allocatorInfo.instance = _instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &_allocator);

    std::cout << "Vulkan initialized" << std::endl;
    std::cout << "GPU: " << physicalDevice.name << std::endl;
    std::cout << "Vulkan Instance Version: " << VK_VERSION_MAJOR(vkb_inst.api_version) << "." << VK_VERSION_MINOR(vkb_inst.api_version) << "." << VK_VERSION_PATCH(vkb_inst.api_version) << std::endl;
    std::cout << "Vulkan Device Version: " << VK_VERSION_MAJOR(vkbDevice.physical_device.properties.apiVersion) << "." << VK_VERSION_MINOR(vkbDevice.physical_device.properties.apiVersion) << "." << VK_VERSION_PATCH(vkbDevice.physical_device.properties.apiVersion) << std::endl;

    // Limits
    VkPhysicalDeviceLimits limits = physicalDevice.properties.limits;
    //print_physical_device_limits(limits);
}

void VulkanEngine::print_physical_device_limits(const VkPhysicalDeviceLimits& limits) {
    std::cout << "Vulkan Physical Device Limits:" << std::endl;

    std::cout << "maxImageDimension1D: " << limits.maxImageDimension1D << std::endl;
    std::cout << "maxImageDimension2D: " << limits.maxImageDimension2D << std::endl;
    std::cout << "maxImageDimension3D: " << limits.maxImageDimension3D << std::endl;
    std::cout << "maxImageDimensionCube: " << limits.maxImageDimensionCube << std::endl;
    std::cout << "maxImageArrayLayers: " << limits.maxImageArrayLayers << std::endl;
    std::cout << "maxTexelBufferElements: " << limits.maxTexelBufferElements << std::endl;
    std::cout << "maxUniformBufferRange: " << limits.maxUniformBufferRange << std::endl;
    std::cout << "maxStorageBufferRange: " << limits.maxStorageBufferRange << std::endl;
    std::cout << "maxPushConstantsSize: " << limits.maxPushConstantsSize << std::endl;
    std::cout << "maxMemoryAllocationCount: " << limits.maxMemoryAllocationCount << std::endl;
    std::cout << "maxSamplerAllocationCount: " << limits.maxSamplerAllocationCount << std::endl;
    std::cout << "bufferImageGranularity: " << limits.bufferImageGranularity << std::endl;
    std::cout << "sparseAddressSpaceSize: " << limits.sparseAddressSpaceSize << std::endl;
    std::cout << "maxBoundDescriptorSets: " << limits.maxBoundDescriptorSets << std::endl;
    std::cout << "maxPerStageDescriptorSamplers: " << limits.maxPerStageDescriptorSamplers << std::endl;
    std::cout << "maxPerStageDescriptorUniformBuffers: " << limits.maxPerStageDescriptorUniformBuffers << std::endl;
    std::cout << "maxPerStageDescriptorStorageBuffers: " << limits.maxPerStageDescriptorStorageBuffers << std::endl;
    std::cout << "maxPerStageDescriptorSampledImages: " << limits.maxPerStageDescriptorSampledImages << std::endl;
    std::cout << "maxPerStageDescriptorStorageImages: " << limits.maxPerStageDescriptorStorageImages << std::endl;
    std::cout << "maxPerStageDescriptorInputAttachments: " << limits.maxPerStageDescriptorInputAttachments << std::endl;
    std::cout << "maxPerStageResources: " << limits.maxPerStageResources << std::endl;
    std::cout << "maxDescriptorSetSamplers: " << limits.maxDescriptorSetSamplers << std::endl;
    std::cout << "maxDescriptorSetUniformBuffers: " << limits.maxDescriptorSetUniformBuffers << std::endl;
    std::cout << "maxDescriptorSetUniformBuffersDynamic: " << limits.maxDescriptorSetUniformBuffersDynamic << std::endl;
    std::cout << "maxDescriptorSetStorageBuffers: " << limits.maxDescriptorSetStorageBuffers << std::endl;
    std::cout << "maxDescriptorSetStorageBuffersDynamic: " << limits.maxDescriptorSetStorageBuffersDynamic << std::endl;
    std::cout << "maxDescriptorSetSampledImages: " << limits.maxDescriptorSetSampledImages << std::endl;
    std::cout << "maxDescriptorSetStorageImages: " << limits.maxDescriptorSetStorageImages << std::endl;
    std::cout << "maxDescriptorSetInputAttachments: " << limits.maxDescriptorSetInputAttachments << std::endl;
    std::cout << "maxVertexInputAttributes: " << limits.maxVertexInputAttributes << std::endl;
    std::cout << "maxVertexInputBindings: " << limits.maxVertexInputBindings << std::endl;
    std::cout << "maxVertexInputAttributeOffset: " << limits.maxVertexInputAttributeOffset << std::endl;
    std::cout << "maxVertexInputBindingStride: " << limits.maxVertexInputBindingStride << std::endl;
    std::cout << "maxVertexOutputComponents: " << limits.maxVertexOutputComponents << std::endl;
    std::cout << "maxTessellationGenerationLevel: " << limits.maxTessellationGenerationLevel << std::endl;
    std::cout << "maxTessellationPatchSize: " << limits.maxTessellationPatchSize << std::endl;
    std::cout << "maxTessellationControlPerVertexInputComponents: " << limits.maxTessellationControlPerVertexInputComponents << std::endl;
    std::cout << "maxTessellationControlPerVertexOutputComponents: " << limits.maxTessellationControlPerVertexOutputComponents << std::endl;
    std::cout << "maxTessellationControlPerPatchOutputComponents: " << limits.maxTessellationControlPerPatchOutputComponents << std::endl;
    std::cout << "maxTessellationControlTotalOutputComponents: " << limits.maxTessellationControlTotalOutputComponents << std::endl;
    std::cout << "maxTessellationEvaluationInputComponents: " << limits.maxTessellationEvaluationInputComponents << std::endl;
    std::cout << "maxTessellationEvaluationOutputComponents: " << limits.maxTessellationEvaluationOutputComponents << std::endl;
    std::cout << "maxGeometryShaderInvocations: " << limits.maxGeometryShaderInvocations << std::endl;
    std::cout << "maxGeometryInputComponents: " << limits.maxGeometryInputComponents << std::endl;
    std::cout << "maxGeometryOutputComponents: " << limits.maxGeometryOutputComponents << std::endl;
    std::cout << "maxGeometryOutputVertices: " << limits.maxGeometryOutputVertices << std::endl;
    std::cout << "maxGeometryTotalOutputComponents: " << limits.maxGeometryTotalOutputComponents << std::endl;
    std::cout << "maxFragmentInputComponents: " << limits.maxFragmentInputComponents << std::endl;
    std::cout << "maxFragmentOutputAttachments: " << limits.maxFragmentOutputAttachments << std::endl;
    std::cout << "maxFragmentDualSrcAttachments: " << limits.maxFragmentDualSrcAttachments << std::endl;
    std::cout << "maxFragmentCombinedOutputResources: " << limits.maxFragmentCombinedOutputResources << std::endl;
    std::cout << "maxComputeSharedMemorySize: " << limits.maxComputeSharedMemorySize << std::endl;
    std::cout << "maxComputeWorkGroupCount[0]: " << limits.maxComputeWorkGroupCount[0] << std::endl;
    std::cout << "maxComputeWorkGroupCount[1]: " << limits.maxComputeWorkGroupCount[1] << std::endl;
    std::cout << "maxComputeWorkGroupCount[2]: " << limits.maxComputeWorkGroupCount[2] << std::endl;
    std::cout << "maxComputeWorkGroupInvocations: " << limits.maxComputeWorkGroupInvocations << std::endl;
    std::cout << "maxComputeWorkGroupSize[0]: " << limits.maxComputeWorkGroupSize[0] << std::endl;
    std::cout << "maxComputeWorkGroupSize[1]: " << limits.maxComputeWorkGroupSize[1] << std::endl;
    std::cout << "maxComputeWorkGroupSize[2]: " << limits.maxComputeWorkGroupSize[2] << std::endl;
    std::cout << "subPixelPrecisionBits: " << limits.subPixelPrecisionBits << std::endl;
    std::cout << "subTexelPrecisionBits: " << limits.subTexelPrecisionBits << std::endl;
    std::cout << "mipmapPrecisionBits: " << limits.mipmapPrecisionBits << std::endl;
    std::cout << "maxDrawIndexedIndexValue: " << limits.maxDrawIndexedIndexValue << std::endl;
    std::cout << "maxDrawIndirectCount: " << limits.maxDrawIndirectCount << std::endl;
    std::cout << "maxSamplerLodBias: " << limits.maxSamplerLodBias << std::endl;
    std::cout << "maxSamplerAnisotropy: " << limits.maxSamplerAnisotropy << std::endl;
    std::cout << "maxViewports: " << limits.maxViewports << std::endl;
    std::cout << "maxViewportDimensions[0]: " << limits.maxViewportDimensions[0] << std::endl;
    std::cout << "maxViewportDimensions[1]: " << limits.maxViewportDimensions[1] << std::endl;
    std::cout << "viewportBoundsRange[0]: " << limits.viewportBoundsRange[0] << std::endl;
    std::cout << "viewportBoundsRange[1]: " << limits.viewportBoundsRange[1] << std::endl;
    std::cout << "viewportSubPixelBits: " << limits.viewportSubPixelBits << std::endl;
    std::cout << "minMemoryMapAlignment: " << limits.minMemoryMapAlignment << std::endl;
    std::cout << "minTexelBufferOffsetAlignment: " << limits.minTexelBufferOffsetAlignment << std::endl;
    std::cout << "minUniformBufferOffsetAlignment: " << limits.minUniformBufferOffsetAlignment << std::endl;
    std::cout << "minStorageBufferOffsetAlignment: " << limits.minStorageBufferOffsetAlignment << std::endl;
    std::cout << "minTexelOffset: " << limits.minTexelOffset << std::endl;
    std::cout << "maxTexelOffset: " << limits.maxTexelOffset << std::endl;
    std::cout << "minTexelGatherOffset: " << limits.minTexelGatherOffset << std::endl;
    std::cout << "maxTexelGatherOffset: " << limits.maxTexelGatherOffset << std::endl;
    std::cout << "minInterpolationOffset: " << limits.minInterpolationOffset << std::endl;
    std::cout << "maxInterpolationOffset: " << limits.maxInterpolationOffset << std::endl;
    std::cout << "subPixelInterpolationOffsetBits: " << limits.subPixelInterpolationOffsetBits << std::endl;
    std::cout << "maxFramebufferWidth: " << limits.maxFramebufferWidth << std::endl;
    std::cout << "maxFramebufferHeight: " << limits.maxFramebufferHeight << std::endl;
    std::cout << "maxFramebufferLayers: " << limits.maxFramebufferLayers << std::endl;
    std::cout << "framebufferColorSampleCounts: " << limits.framebufferColorSampleCounts << std::endl;
    std::cout << "framebufferDepthSampleCounts: " << limits.framebufferDepthSampleCounts << std::endl;
    std::cout << "framebufferStencilSampleCounts: " << limits.framebufferStencilSampleCounts << std::endl;
    std::cout << "framebufferNoAttachmentsSampleCounts: " << limits.framebufferNoAttachmentsSampleCounts << std::endl;
    std::cout << "maxColorAttachments: " << limits.maxColorAttachments << std::endl;
    std::cout << "sampledImageColorSampleCounts: " << limits.sampledImageColorSampleCounts << std::endl;
    std::cout << "sampledImageIntegerSampleCounts: " << limits.sampledImageIntegerSampleCounts << std::endl;
    std::cout << "sampledImageDepthSampleCounts: " << limits.sampledImageDepthSampleCounts << std::endl;
    std::cout << "sampledImageStencilSampleCounts: " << limits.sampledImageStencilSampleCounts << std::endl;
    std::cout << "storageImageSampleCounts: " << limits.storageImageSampleCounts << std::endl;
    std::cout << "maxSampleMaskWords: " << limits.maxSampleMaskWords << std::endl;
    std::cout << "timestampComputeAndGraphics: " << limits.timestampComputeAndGraphics << std::endl;
    std::cout << "timestampPeriod: " << limits.timestampPeriod << std::endl;
    std::cout << "maxClipDistances: " << limits.maxClipDistances << std::endl;
    std::cout << "maxCullDistances: " << limits.maxCullDistances << std::endl;
    std::cout << "maxCombinedClipAndCullDistances: " << limits.maxCombinedClipAndCullDistances << std::endl;
    std::cout << "discreteQueuePriorities: " << limits.discreteQueuePriorities << std::endl;
    std::cout << "pointSizeRange[0]: " << limits.pointSizeRange[0] << std::endl;
    std::cout << "pointSizeRange[1]: " << limits.pointSizeRange[1] << std::endl;
    std::cout << "lineWidthRange[0]: " << limits.lineWidthRange[0] << std::endl;
    std::cout << "lineWidthRange[1]: " << limits.lineWidthRange[1] << std::endl;
    std::cout << "pointSizeGranularity: " << limits.pointSizeGranularity << std::endl;
    std::cout << "lineWidthGranularity: " << limits.lineWidthGranularity << std::endl;
    std::cout << "strictLines: " << limits.strictLines << std::endl;
    std::cout << "standardSampleLocations: " << limits.standardSampleLocations << std::endl;
    std::cout << "optimalBufferCopyOffsetAlignment: " << limits.optimalBufferCopyOffsetAlignment << std::endl;
    std::cout << "optimalBufferCopyRowPitchAlignment: " << limits.optimalBufferCopyRowPitchAlignment << std::endl;
    std::cout << "nonCoherentAtomSize: " << limits.nonCoherentAtomSize << std::endl;
}

void VulkanEngine::init_swapchain()
{
    create_swapchain(_windowExtent.width, _windowExtent.height);

    // depth image size will match the window
    VkExtent3D drawImageExtent = {
        _windowExtent.width,
        _windowExtent.height,
        1};

    // hardcoding the draw format to 64 bit signed float
    _drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    _drawImage.imageExtent = drawImageExtent;

    VkImageUsageFlags drawImageUsages{};
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo rimg_info = vkinit::image_create_info(_drawImage.imageFormat, drawImageUsages, drawImageExtent);

    // for the draw image, we want to allocate it from gpu local memory
    VmaAllocationCreateInfo rimg_allocinfo = {};
    rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // allocate and create the image
    vmaCreateImage(_allocator, &rimg_info, &rimg_allocinfo, &_drawImage.image, &_drawImage.allocation, nullptr);

    // build a image-view for the draw image to use for rendering
    VkImageViewCreateInfo rview_info = vkinit::imageview_create_info(_drawImage.imageFormat, _drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

    VK_CHECK(vkCreateImageView(_device, &rview_info, nullptr, &_drawImage.imageView));

    // create a depth image too
    // hardcoding the draw format to 32 bit float
    _depthImage.imageFormat = VK_FORMAT_D32_SFLOAT;
    _depthImage.imageExtent = drawImageExtent;
    VkImageUsageFlags depthImageUsages{};
    depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageCreateInfo dimg_info = vkinit::image_create_info(_depthImage.imageFormat, depthImageUsages, drawImageExtent);

    // allocate and create the image
    vmaCreateImage(_allocator, &dimg_info, &rimg_allocinfo, &_depthImage.image, &_depthImage.allocation, nullptr);

    // build a image-view for the draw image to use for rendering
    VkImageViewCreateInfo dview_info = vkinit::imageview_create_info(_depthImage.imageFormat, _depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

    VK_CHECK(vkCreateImageView(_device, &dview_info, nullptr, &_depthImage.imageView));

    // add to deletion queues
    _mainDeletionQueue.push_function([this]()
                                     {
		vkDestroyImageView(_device, _drawImage.imageView, nullptr);
		vmaDestroyImage(_allocator, _drawImage.image, _drawImage.allocation);

		vkDestroyImageView(_device, _depthImage.imageView, nullptr);
		vmaDestroyImage(_allocator, _depthImage.image, _depthImage.allocation); });
}

void VulkanEngine::create_swapchain(uint32_t width, uint32_t height)
{
    vkb::SwapchainBuilder swapchainBuilder{_chosenGPU, _device, _surface};

    _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapchainBuilder
                                      //.use_default_format_selection()
                                      .set_desired_format(VkSurfaceFormatKHR{.format = _swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
                                      // use vsync present mode
                                      .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                      .set_desired_extent(width, height)
                                      .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                                      .build()
                                      .value();

    _swapchainExtent = vkbSwapchain.extent;
    // store swapchain and its related images
    _swapchain = vkbSwapchain.swapchain;
    _swapchainImages = vkbSwapchain.get_images().value();
    _swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void VulkanEngine::destroy_swapchain()
{
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);

    // destroy swapchain resources
    for (int i = 0; i < _swapchainImageViews.size(); i++)
    {
        vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
    }
}

void VulkanEngine::resize_swapchain()
{
    vkDeviceWaitIdle(_device);

    destroy_swapchain();

    int w, h;
    SDL_GetWindowSize(_window, &w, &h);
    _windowExtent.width = w;
    _windowExtent.height = h;

    create_swapchain(_windowExtent.width, _windowExtent.height);

    resize_requested = false;
}

void VulkanEngine::init_commands()
{
    // create a command pool for commands submitted to the graphics queue.
    // we also want the pool to allow for resetting of individual command buffers
    VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for (int i = 0; i < FRAME_OVERLAP; i++)
    {
        VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_frames[i]._commandPool));

        // allocate the default command buffer that we will use for rendering
        VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_frames[i]._commandPool, 1);

        VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_frames[i]._mainCommandBuffer));
    }

    VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_immCommandPool));

    // allocate the command buffer for immediate submits
    VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_immCommandPool, 1);

    VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_immCommandBuffer));

    _mainDeletionQueue.push_function([this]()
                                     { vkDestroyCommandPool(_device, _immCommandPool, nullptr);
                                        vkDestroyCommandPool(_device, _frames[0]._commandPool, nullptr);
                                        vkDestroyCommandPool(_device, _frames[1]._commandPool, nullptr); });
}

void VulkanEngine::init_sync_structures()
{
    // create syncronization structures
    // one fence to control when the gpu has finished rendering the frame,
    // and 2 semaphores to syncronize rendering with swapchain
    // we want the fence to start signalled so we can wait on it on the first
    // frame
    VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
    VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_immFence));

    _mainDeletionQueue.push_function([this]()
                                     { vkDestroyFence(_device, _immFence, nullptr); });

    for (int i = 0; i < FRAME_OVERLAP; i++)
    {

        VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_frames[i]._renderFence));

        VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

        VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._swapchainSemaphore));
        VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._renderSemaphore));

        _mainDeletionQueue.push_function([=, this]()
                                         {
            vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
            vkDestroySemaphore(_device, _frames[i]._swapchainSemaphore, nullptr);
            vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr); });
    }
}

void VulkanEngine::init_descriptors()
{
    // create a descriptor pool
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3},
    };

    globalDescriptorAllocator.init_pool(_device, 10, sizes);
    _mainDeletionQueue.push_function(
        [&]()
        { vkDestroyDescriptorPool(_device, globalDescriptorAllocator.pool, nullptr); });

    {
        DescriptorLayoutBuilder builder;
        builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        _drawImageDescriptorLayout = builder.build(_device, VK_SHADER_STAGE_COMPUTE_BIT);
    }

    _mainDeletionQueue.push_function([&]()
                                     {
                                         vkDestroyDescriptorSetLayout(_device, _drawImageDescriptorLayout, nullptr);
                                         // vkDestroyDescriptorSetLayout(_device, _gpuSceneDataDescriptorLayout, nullptr);
                                     });

    _drawImageDescriptors = globalDescriptorAllocator.allocate(_device, _drawImageDescriptorLayout);
    {
        DescriptorWriter writer;
        writer.write_image(0, _drawImage.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        writer.update_set(_device, _drawImageDescriptors);
    }

    for (int i = 0; i < FRAME_OVERLAP; i++)
    {
        // create a descriptor pool
        std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> frame_sizes = {
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4},
        };

        _frames[i]._frameDescriptors = DescriptorAllocatorGrowable{};
        _frames[i]._frameDescriptors.init(_device, 1000, frame_sizes);
        _mainDeletionQueue.push_function([&, i]()
                                         { _frames[i]._frameDescriptors.destroy_pools(_device); });
    }
}

AllocatedBuffer VulkanEngine::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    // allocate buffer
    VkBufferCreateInfo bufferInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.pNext = nullptr;
    bufferInfo.size = allocSize;

    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = memoryUsage;
    vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    AllocatedBuffer newBuffer;

    // allocate the buffer
    VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation,
                             &newBuffer.info));

    return newBuffer;
}

void VulkanEngine::destroy_buffer(const AllocatedBuffer &buffer)
{
    vmaDestroyBuffer(_allocator, buffer.buffer, buffer.allocation);
}

void VulkanEngine::init_pipelines()
{
    init_background_pipelines();

    init_triangle_pipeline();
}

void VulkanEngine::init_triangle_pipeline()
{
    VkShaderModule triangleFragShader;
    if (!vkutil::load_shader_module("../src/render_engine/shaders/colored_triangle.frag.spv", _device, &triangleFragShader))
    {
        fmt::print("Error when building the triangle fragment shader module");
    }
    else
    {
        fmt::print("Triangle fragment shader succesfully loaded\n");
    }

    VkShaderModule triangleVertexShader;
    if (!vkutil::load_shader_module("../src/render_engine/shaders/colored_triangle.vert.spv", _device, &triangleVertexShader))
    {
        fmt::print("Error when building the triangle vertex shader module");
    }
    else
    {
        fmt::print("Triangle vertex shader succesfully loaded\n");
    }

    // build the pipeline layout that controls the inputs/outputs of the shader
    // we are not using descriptor sets or other systems yet, so no need to use anything other than empty default
    VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();
    VK_CHECK(vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr, &_trianglePipelineLayout));

    PipelineBuilder pipelineBuilder;

    // use the triangle layout we created
    pipelineBuilder._pipelineLayout = _trianglePipelineLayout;
    // connecting the vertex and pixel shaders to the pipeline
    pipelineBuilder.set_shaders(triangleVertexShader, triangleFragShader);
    // it will draw triangles
    pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    // filled triangles
    pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
    // no backface culling
    pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
    // no multisampling
    pipelineBuilder.set_multisampling_none();
    // no blending
    pipelineBuilder.disable_blending();
    // no depth testing
    pipelineBuilder.disable_depthtest();

    // connect the image format we will draw into, from draw image
    pipelineBuilder.set_color_attachment_format(_drawImage.imageFormat);
    pipelineBuilder.set_depth_format(VK_FORMAT_UNDEFINED);

    // finally build the pipeline
    _trianglePipeline = pipelineBuilder.build_pipeline(_device);

    // clean structures
    vkDestroyShaderModule(_device, triangleFragShader, nullptr);
    vkDestroyShaderModule(_device, triangleVertexShader, nullptr);

    _mainDeletionQueue.push_function([this]()
                                     {
		vkDestroyPipelineLayout(_device, _trianglePipelineLayout, nullptr);
		vkDestroyPipeline(_device, _trianglePipeline, nullptr); });
}

void VulkanEngine::init_background_pipelines()
{
    VkPipelineLayoutCreateInfo computeLayout{};
    computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computeLayout.pNext = nullptr;
    computeLayout.pSetLayouts = &_drawImageDescriptorLayout;
    computeLayout.setLayoutCount = 1;

    VkPushConstantRange pushConstant{};
    pushConstant.offset = 0;
    pushConstant.size = sizeof(ComputePushConstants);
    pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    computeLayout.pPushConstantRanges = &pushConstant;
    computeLayout.pushConstantRangeCount = 1;

    VK_CHECK(vkCreatePipelineLayout(_device, &computeLayout, nullptr, &_gradientPipelineLayout));

    // layout code
    VkShaderModule gradientShader;
    if (!vkutil::load_shader_module("../src/render_engine/shaders/gradient_color.comp.spv", _device, &gradientShader))
    {
        fmt::print("Error when building the compute shader \n");
    }

    VkShaderModule skyShader;
    if (!vkutil::load_shader_module("../src/render_engine/shaders/sky.comp.spv", _device, &skyShader))
    {
        fmt::print("Error when building the compute shader \n");
    }

    VkShaderModule gradient2Shader;
    if (!vkutil::load_shader_module("../src/render_engine/shaders/gradient.comp.spv", _device, &gradient2Shader))
    {
        fmt::print("Error when building the compute shader \n");
    }

    VkPipelineShaderStageCreateInfo stageinfo{};
    stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageinfo.pNext = nullptr;
    stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageinfo.module = gradientShader;
    stageinfo.pName = "main";

    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.layout = _gradientPipelineLayout;
    computePipelineCreateInfo.stage = stageinfo;

    ComputeEffect gradient;
    gradient.layout = _gradientPipelineLayout;
    gradient.name = "gradient";
    gradient.data = {};

    // default colors
    gradient.data.data1 = glm::vec4(1, 0, 0, 1);
    gradient.data.data2 = glm::vec4(0, 0, 1, 1);

    VK_CHECK(vkCreateComputePipelines(_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &gradient.pipeline));

    // change the shader module only to create the sky shader
    computePipelineCreateInfo.stage.module = skyShader;

    ComputeEffect sky;
    sky.layout = _gradientPipelineLayout;
    sky.name = "sky";
    sky.data = {};
    // default sky parameters
    sky.data.data1 = glm::vec4(0.1, 0.2, 0.4, 0.97);

    VK_CHECK(vkCreateComputePipelines(_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &sky.pipeline));

    ComputeEffect gradient2;
    gradient2.layout = _gradientPipelineLayout;
    gradient2.name = "gradient2";
    gradient2.data = {};

    gradient2.data.data1 = glm::vec4(0, 1, 1, 1);
    gradient2.data.data2 = glm::vec4(1, 1, 0, 1);

    computePipelineCreateInfo.stage.module = gradient2Shader;

    VK_CHECK(vkCreateComputePipelines(_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &gradient2.pipeline));

    // add the 2 background effects into the array
    backgroundEffects.push_back(sky);
    backgroundEffects.push_back(gradient);
    backgroundEffects.push_back(gradient2);

    // destroy structures properly
    vkDestroyShaderModule(_device, gradientShader, nullptr);
    vkDestroyShaderModule(_device, skyShader, nullptr);
    vkDestroyShaderModule(_device, gradient2Shader, nullptr);

    _mainDeletionQueue.push_function([this, sky, gradient, gradient2]()
                                     {
	vkDestroyPipelineLayout(_device, _gradientPipelineLayout, nullptr);
	vkDestroyPipeline(_device, sky.pipeline, nullptr);
	vkDestroyPipeline(_device, gradient.pipeline, nullptr);
    vkDestroyPipeline(_device, gradient2.pipeline, nullptr); });
}

AllocatedImage VulkanEngine::create_image(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped)
{
    AllocatedImage newImage;
    newImage.imageFormat = format;
    newImage.imageExtent = size;

    VkImageCreateInfo img_info = vkinit::image_create_info(format, usage, size);
    if (mipmapped)
    {
        img_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
    }

    // always allocate images on dedicated GPU memory
    VmaAllocationCreateInfo allocinfo = {};
    allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // allocate and create the image
    VK_CHECK(vmaCreateImage(_allocator, &img_info, &allocinfo, &newImage.image, &newImage.allocation, nullptr));

    // if the format is a depth format, we will need to have it use the correct
    // aspect flag
    VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
    if (format == VK_FORMAT_D32_SFLOAT)
    {
        aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    // build a image-view for the image
    VkImageViewCreateInfo view_info = vkinit::imageview_create_info(format, newImage.image, aspectFlag);
    view_info.subresourceRange.levelCount = img_info.mipLevels;

    VK_CHECK(vkCreateImageView(_device, &view_info, nullptr, &newImage.imageView));

    return newImage;
}

//> create_mip_2
AllocatedImage VulkanEngine::create_image(void *data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped)
{
    size_t data_size = size.depth * size.width * size.height * 4;
    AllocatedBuffer uploadbuffer = create_buffer(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    memcpy(uploadbuffer.info.pMappedData, data, data_size);

    AllocatedImage new_image = create_image(size, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, mipmapped);

    immediate_submit([&](VkCommandBuffer cmd)
                     {
        vkutil::transition_image(cmd, new_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkBufferImageCopy copyRegion = {};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;

        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageExtent = size;

        // copy the buffer into the image
        vkCmdCopyBufferToImage(cmd, uploadbuffer.buffer, new_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
            &copyRegion);

        if (mipmapped) {
            vkutil::generate_mipmaps(cmd, new_image.image,VkExtent2D{new_image.imageExtent.width,new_image.imageExtent.height});
        } else {
            vkutil::transition_image(cmd, new_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        } });
    destroy_buffer(uploadbuffer);
    return new_image;
}
//< create_mip_2

FrameData &VulkanEngine::get_last_frame()
{
    return _frames[(_frameNumber - 1) % FRAME_OVERLAP];
}

void VulkanEngine::immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function)
{
    VK_CHECK(vkResetFences(_device, 1, &_immFence));
    VK_CHECK(vkResetCommandBuffer(_immCommandBuffer, 0));

    VkCommandBuffer cmd = _immCommandBuffer;

    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    function(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);
    VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, nullptr, nullptr);

    // submit command buffer to the queue and execute it.
    //  _renderFence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit, _immFence));

    VK_CHECK(vkWaitForFences(_device, 1, &_immFence, true, 9999999999));
}

void VulkanEngine::destroy_image(const AllocatedImage &img)
{
    vkDestroyImageView(_device, img.imageView, nullptr);
    vmaDestroyImage(_allocator, img.image, img.allocation);
}

void VulkanEngine::cleanup()
{
    if (_isInitialized)
    {
        // make sure the gpu has stopped doing its things
        vkDeviceWaitIdle(_device);

        _mainDeletionQueue.flush();

        destroy_swapchain();

        vkDestroySurfaceKHR(_instance, _surface, nullptr);

        vmaDestroyAllocator(_allocator);

        vkDestroyDevice(_device, nullptr);
        vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
        vkDestroyInstance(_instance, nullptr);

        SDL_DestroyWindow(_window);
    }
}

void VulkanEngine::init_imgui()
{
    // 1: create descriptor pool for IMGUI
    //  the size of the pool is very oversize, but it's copied from imgui demo
    //  itself.
    VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                         {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VkDescriptorPool imguiPool;
    VK_CHECK(vkCreateDescriptorPool(_device, &pool_info, nullptr, &imguiPool));

    // 2: initialize imgui library

    // this initializes the core structures of imgui
    ImGui::CreateContext();

    // this initializes imgui for SDL
    ImGui_ImplSDL3_InitForVulkan(_window);

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = _instance;
    init_info.PhysicalDevice = _chosenGPU;
    init_info.Device = _device;
    init_info.Queue = _graphicsQueue;
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.UseDynamicRendering = true;

    // dynamic rendering parameters for imgui to use
    init_info.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &_swapchainImageFormat;

    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info);

    ImGui_ImplVulkan_CreateFontsTexture();

    // add the destroy the imgui created structures
    _mainDeletionQueue.push_function([=, this]()
                                     {
		ImGui_ImplVulkan_Shutdown();
		vkDestroyDescriptorPool(_device, imguiPool, nullptr); });
}

void VulkanEngine::draw_background(VkCommandBuffer cmd)
{
    ComputeEffect &effect = backgroundEffects[currentBackgroundEffect];

    // bind the background compute pipeline
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, effect.pipeline);

    // bind the descriptor set containing the draw image for the compute pipeline
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _gradientPipelineLayout, 0, 1, &_drawImageDescriptors, 0, nullptr);

    vkCmdPushConstants(cmd, _gradientPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &effect.data);
    // execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
    vkCmdDispatch(cmd, std::ceil(_drawExtent.width / 8.0), std::ceil(_drawExtent.height / 8.0), 1);
}

void VulkanEngine::draw_main(VkCommandBuffer cmd)
{
    ComputeEffect &effect = backgroundEffects[currentBackgroundEffect];

    // bind the background compute pipeline
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, effect.pipeline);

    // bind the descriptor set containing the draw image for the compute pipeline
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _gradientPipelineLayout, 0, 1, &_drawImageDescriptors, 0, nullptr);

    vkCmdPushConstants(cmd, _gradientPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &effect.data);
    // execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
    vkCmdDispatch(cmd, std::ceil(_drawExtent.width / 16.0), std::ceil(_drawExtent.height / 16.0), 1);

    // draw the triangle

    VkRenderingAttachmentInfo colorAttachment = vkinit::attachment_info(_drawImage.imageView, nullptr, VK_IMAGE_LAYOUT_GENERAL);
    VkRenderingAttachmentInfo depthAttachment = vkinit::depth_attachment_info(_depthImage.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    VkRenderingInfo renderInfo = vkinit::rendering_info(_drawExtent, &colorAttachment, &depthAttachment);

    vkCmdBeginRendering(cmd, &renderInfo);
    auto start = std::chrono::system_clock::now();
    draw_geometry(cmd);

    auto end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    stats.mesh_draw_time = elapsed.count() / 1000.f;

    vkCmdEndRendering(cmd);
}

void VulkanEngine::draw()
{
    // wait until the gpu has finished rendering the last frame. Timeout of 1 second
    VK_CHECK(vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true, 1000000000));

    get_current_frame()._deletionQueue.flush();
    get_current_frame()._frameDescriptors.clear_pools(_device);
    // request image from the swapchain
    uint32_t swapchainImageIndex;

    VkResult e = vkAcquireNextImageKHR(_device, _swapchain, 1000000000, get_current_frame()._swapchainSemaphore, nullptr, &swapchainImageIndex);
    if (e == VK_ERROR_OUT_OF_DATE_KHR)
    {
        resize_requested = true;
        return;
    }
    _drawExtent.height = std::min(_swapchainExtent.height, _drawImage.imageExtent.height) * 1.f;
    _drawExtent.width = std::min(_swapchainExtent.width, _drawImage.imageExtent.width) * 1.f;

    VK_CHECK(vkResetFences(_device, 1, &get_current_frame()._renderFence));

    // now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
    VK_CHECK(vkResetCommandBuffer(get_current_frame()._mainCommandBuffer, 0));

    // naming it cmd for shorter writing
    VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

    // begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    // transition our main draw image into general layout so we can write into it
    // we will overwrite it all so we dont care about what was the older layout
    vkutil::transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    draw_background(cmd);

    vkutil::transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    draw_geometry(cmd);

    // transtion the draw image and the swapchain image into their correct transfer layouts
    vkutil::transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vkutil::transition_image(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkExtent2D extent;
    extent.height = _windowExtent.height;
    extent.width = _windowExtent.width;
    // extent.depth = 1;

    // execute a copy from the draw image into the swapchain
    vkutil::copy_image_to_image(cmd, _drawImage.image, _swapchainImages[swapchainImageIndex], _drawExtent, _swapchainExtent);

    // set swapchain image layout to Attachment Optimal so we can draw it
    vkutil::transition_image(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // draw imgui into the swapchain image
    draw_imgui(cmd, _swapchainImageViews[swapchainImageIndex]);

    // set swapchain image layout to Present so we can draw it
    vkutil::transition_image(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // finalize the command buffer (we can no longer add commands, but it can now be executed)
    VK_CHECK(vkEndCommandBuffer(cmd));

    // prepare the submission to the queue.
    // we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
    // we will signal the _renderSemaphore, to signal that rendering has finished

    VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);

    VkSemaphoreSubmitInfo waitInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, get_current_frame()._swapchainSemaphore);
    VkSemaphoreSubmitInfo signalInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, get_current_frame()._renderSemaphore);

    VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, &signalInfo, &waitInfo);

    // submit command buffer to the queue and execute it.
    //  _renderFence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit, get_current_frame()._renderFence));

    // prepare present
    //  this will put the image we just rendered to into the visible window.
    //  we want to wait on the _renderSemaphore for that,
    //  as its necessary that drawing commands have finished before the image is displayed to the user
    VkPresentInfoKHR presentInfo = vkinit::present_info();

    presentInfo.pSwapchains = &_swapchain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &get_current_frame()._renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapchainImageIndex;

    VkResult presentResult = vkQueuePresentKHR(_graphicsQueue, &presentInfo);
    if (e == VK_ERROR_OUT_OF_DATE_KHR)
    {
        resize_requested = true;
        return;
    }
    // increase the number of frames drawn
    _frameNumber++;
}

void VulkanEngine::draw_geometry(VkCommandBuffer cmd)
{
    
    // begin a render pass  connected to our draw image
    VkRenderingAttachmentInfo colorAttachment = vkinit::attachment_info(_drawImage.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkRenderingInfo renderInfo = vkinit::rendering_info(_drawExtent, &colorAttachment, nullptr);
    vkCmdBeginRendering(cmd, &renderInfo);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _trianglePipeline);

    // set dynamic viewport and scissor
    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = _drawExtent.width;
    viewport.height = _drawExtent.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = _drawExtent.width;
    scissor.extent.height = _drawExtent.height;

    vkCmdSetScissor(cmd, 0, 1, &scissor);

    // launch a draw command to draw 12 vertices
    vkCmdDraw(cmd, 12, 1, 0, 0);

    vkCmdEndRendering(cmd);
}

void VulkanEngine::draw_imgui(VkCommandBuffer cmd, VkImageView targetImageView)
{
    VkRenderingAttachmentInfo colorAttachment = vkinit::attachment_info(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = vkinit::rendering_info(_swapchainExtent, &colorAttachment, nullptr);

    vkCmdBeginRendering(cmd, &renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vkCmdEndRendering(cmd);
}

void VulkanEngine::run()
{
    SDL_Event e;
    bool bQuit = false;

    // main loop
    while (!bQuit)
    {
        auto start = std::chrono::system_clock::now();

        // Handle events on queue
        while (SDL_PollEvent(&e) != 0)
        {
            // close the window when user alt-f4s or clicks the X button
            if (e.type == SDL_EVENT_QUIT)
                bQuit = true;

            if (e.key.scancode == SDL_SCANCODE_ESCAPE)
                bQuit = true;

            if (e.type == SDL_EVENT_WINDOW_SHOWN)
            {
                if (e.window.type == SDL_EVENT_WINDOW_MINIMIZED)
                {
                    stop_rendering = true;
                }
                if (e.window.type == SDL_EVENT_WINDOW_RESTORED)
                {
                    stop_rendering = false;
                }
            }

            // send SDL event to imgui for handling
            ImGui_ImplSDL3_ProcessEvent(&e);
        }

        if (freeze_rendering)
            continue;

        if (resize_requested)
        {
            resize_swapchain();
        }

        // imgui new frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();

        ImGui::NewFrame();

        ImGui::Begin("Stats");

        ImGui::Text("frametime %f ms", stats.frametime);
        ImGui::Text("drawtime %f ms", stats.mesh_draw_time);
        ImGui::Text("triangles %i", stats.triangle_count);
        ImGui::Text("draws %i", stats.drawcall_count);
        ImGui::End();

        if (ImGui::Begin("background"))
        {

            ComputeEffect &selected = backgroundEffects[currentBackgroundEffect];

            ImGui::Text("Selected effect: %s", selected.name);

            ImGui::SliderInt("Effect Index", &currentBackgroundEffect, 0, backgroundEffects.size() - 1);

            ImGui::InputFloat4("data1", (float *)&selected.data.data1);
            ImGui::InputFloat4("data2", (float *)&selected.data.data2);
            ImGui::InputFloat4("data3", (float *)&selected.data.data3);
            ImGui::InputFloat4("data4", (float *)&selected.data.data4);

            ImGui::End();
        }

        ImGui::Render();

        draw();

        auto end = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        stats.frametime = elapsed.count() / 1000.f;
    }
}