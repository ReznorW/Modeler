#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <array>
#include <optional>
#include <memory>

#include "windowContext.hpp"
#include "vulkanDevice.hpp"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
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
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    // Queues
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    // Command pool
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    // Sync objects
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    // Buffers and memory
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer stagingVertexBuffer;
    VkDeviceMemory stagingVertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkBuffer stagingIndexBuffer;
    VkDeviceMemory stagingIndexBufferMemory;
    VkBuffer uboBuffer;
    void* uboMapped;
    VkDeviceMemory uboBufferMemory;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    // Pipeline
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    // Render Pass
    VkRenderPass renderPass;

    // Swapchain
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkSwapchainKHR swapChain;
    VkExtent2D swapChainExtent;
    VkFormat swapChainImageFormat;

    // Depth image
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

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
    void createSurface();
    void createCommandPool();
    void createSyncObjects();
    void createDescriptorSetLayout();

    // Swapchain
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createDepthResources();
    void createFramebuffers();
    void recreateSwapChain();
    void cleanupSwapChain();

    // SwapChain helpers
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    // GPU resources
    void createGeoBuffers();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();

    // GPU resource helpers
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer destBuffer, VkDeviceSize size);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    // Frame execution
    void drawFrame();
    void updateUniformBuffer();
    void updateGpuBuffers();
    void createCommandBuffer();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    // Utils
    VkShaderModule createShaderModule(const std::vector<char>& code);
    std::vector<char> readFile(const std::string& filename);
};