#pragma once

#include "windowContext.hpp"
#include "vulkanDevice.hpp"
#include "vulkanSwapchain.hpp"
#include "vulkanBuffer.hpp"
#include "vulkanPipeline.hpp"

class Renderer {
public: 
    void run();
    void addCube(glm::vec3 center, float size, glm::vec3 color);
    void clearGeometry();
private:
    // --- Constants ---
    const size_t MAX_VERTEX_COUNT = 10000;
    const size_t MAX_INDEX_COUNT = 30000;

    // Window dimensions
    const int WIDTH = 1280;
    const int HEIGHT = 720;

    // --- Globals ---
    // Window
    WindowContext window{WIDTH, HEIGHT, "VulkanCAD"};

    // Device
    std::unique_ptr<VulkanDevice> vulkanDevice;

    // Pipeline
    std::unique_ptr<VulkanPipeline> vulkanPipeline;

    // Swapchain
    std::unique_ptr<VulkanSwapchain> vulkanSwapchain;

    // Render pass
    VkRenderPass renderPass;

    // Buffers
    std::unique_ptr<VulkanBuffer> vertexBuffer;
    std::unique_ptr<VulkanBuffer> indexBuffer;
    std::unique_ptr<VulkanBuffer> uboBuffer;

    // Object vectors
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Flags
    bool dirtyGeo = true;
    bool framebufferResized = false;

    // --- Functions ---
    // Core Lifecycle
    void initVulkan();
    void mainLoop();
    void cleanup();

    // Render pass
    void createRenderPass();

    // GPU resources
    void createGeoBuffers();
    void createUniformBuffers();

    // Frame execution
    void drawFrame();
    void updateUniformBuffer();
    void updateGpuBuffers();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
};