#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>
#include <cstdint>

// --- Structs ---
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanDevice {
public:
    VulkanDevice(VkInstance instance, VkSurfaceKHR surface);
    ~VulkanDevice();

    // Prevent duplicates
    VulkanDevice(const VulkanDevice&) = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;

    VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
    VkDevice getLogicalDevice() { return device; }
    VkQueue getGraphicsQueue() { return graphicsQueue; }
    VkQueue getPresentQueue() { return presentQueue; }
    QueueFamilyIndices getIndices() { return indices; }
    VkCommandBuffer getCommandBuffer() const { return commandBuffer; }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer destBuffer, VkDeviceSize size);

private:
    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    QueueFamilyIndices indices;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    void pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
    void createLogicalDevice(VkSurfaceKHR surface);
    void createCommandPool();
    void createCommandBuffer();
    
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
};