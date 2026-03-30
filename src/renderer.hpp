#pragma once

#include "windowContext.hpp"
#include "vulkanDevice.hpp"
#include "vulkanSwapchain.hpp"
#include "vulkanBuffer.hpp"
#include "vulkanPipeline.hpp"
#include "camera.hpp"
#include "input.hpp"

class Renderer {
public: 
    void run();
    void addCube(glm::vec3 center, float size, glm::vec3 color);
    void addGrid(float size);
    void clearGeometry();
    void toggleVertices() { showVertices = !showVertices; }
    int findClosestVertex(float threshold);
    void selectVertex(int index) { selectedVertex = index; }
    void deselectAll() { selectedVertex = -1; }
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

    // Input
    Input input;

    // Device
    std::unique_ptr<VulkanDevice> vulkanDevice;

    // Pipelines
    std::unique_ptr<VulkanPipeline> mainPipeline;
    std::unique_ptr<VulkanPipeline> pointPipeline;

    // Swapchain
    std::unique_ptr<VulkanSwapchain> vulkanSwapchain;

    // Render pass
    VkRenderPass renderPass;

    // Buffers
    std::unique_ptr<VulkanBuffer> vertexBuffer;
    std::unique_ptr<VulkanBuffer> indexBuffer;
    std::vector<std::unique_ptr<VulkanBuffer>> uboBuffers;

    // Object vectors
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Selection
    int selectedVertex = -1;

    // Camera
    Camera camera;

    // Flags
    bool dirtyGeo = true;
    bool framebufferResized = false;
    bool showVertices = false;

    // --- Functions ---
    // Core Lifecycle
    void initVulkan();
    void initScene();
    void mainLoop();
    void cleanup();

    // Render pass
    void createRenderPass();

    // GPU resources
    void createGeoBuffers();
    void createUniformBuffers();

    // Frame execution
    void drawFrame();
    void updateUniformBuffer(uint32_t currentImage);
    void updateGpuBuffers();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
};