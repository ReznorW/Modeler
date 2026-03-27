#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <array>
#include <optional>
#include <memory>

#include "windowContext.hpp"
#include "vulkanDevice.hpp"
#include "vulkanSwapchain.hpp"
#include "vulkanBuffer.hpp"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

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
    
    // Instance
    VkInstance instance;
    VkSurfaceKHR surface;

    // Device
    std::unique_ptr<VulkanDevice> vulkanDevice;

    // Queues
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    // Sync objects
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    // Buffers
    std::unique_ptr<VulkanBuffer> vertexBuffer;
    std::unique_ptr<VulkanBuffer> indexBuffer;
    std::unique_ptr<VulkanBuffer> uboBuffer;

    // Pipeline
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    // Render pass
    VkRenderPass renderPass;

    // Swapchain
    std::unique_ptr<VulkanSwapchain> vulkanSwapchain;

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

    // Infrastructure set up
    void createInstance();
    void createSyncObjects();

    // Render pass
    void createRenderPass();

    // Graphics pipeline
    void createGraphicsPipeline();

    // GPU resources
    void createGeoBuffers();
    void createUniformBuffers();

    // Frame execution
    void drawFrame();
    void updateUniformBuffer();
    void updateGpuBuffers();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    // Utils
    VkShaderModule createShaderModule(const std::vector<char>& code);
    std::vector<char> readFile(const std::string& filename);
};