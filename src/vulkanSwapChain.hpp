#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

#include "VulkanDevice.hpp"

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class VulkanSwapchain {
public:
    VulkanSwapchain(VulkanDevice& device, VkSurfaceKHR surface, VkExtent2D extent, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
    ~VulkanSwapchain();

    // Prevent duplicates
    VulkanSwapchain(const VulkanSwapchain&) = delete;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

    VkSwapchainKHR getHandle() { return swapChain; }
    VkFormat getFormat() { return swapChainImageFormat; }
    VkExtent2D getExtent() { return swapChainExtent; }
    VkFramebuffer getFramebuffer(uint32_t index) { return swapChainFramebuffers[index]; }
    size_t getImageCount() { return swapChainImages.size(); }
    VkDescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; }
    VkDescriptorSet getDescriptorSet(uint32_t index) { return descriptorSets[index]; }

    void createFramebuffers(VkRenderPass renderPass);
    void createDescriptorSets(VkBuffer uboBuffer);
    void recreate(VkSurfaceKHR surface, VkExtent2D extent, VkRenderPass renderPass);

private:
    VulkanDevice& deviceRef;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    // Depth resources
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    // Descriptor resources
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void createImageViews();
    void createDepthResources();
    void createDescriptorSetLayout();
    void createDescriptorPool();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D windowExtent);
};