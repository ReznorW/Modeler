#include <algorithm>
#include <stdexcept>
#include <array>
#include <string>

#include "VulkanSwapchain.hpp"

VulkanSwapchain::VulkanSwapchain(VulkanDevice& device, VkSurfaceKHR surface, VkExtent2D extent, VkSwapchainKHR oldSwapchain) 
    : deviceRef(device) {
    
    // Get surface and device capabilities
    auto swapChainSupport = deviceRef.querySwapChainSupport(device.getPhysicalDevice(), surface);

    // Get specific surface options from supported options
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D actualExtent = chooseSwapExtent(swapChainSupport.capabilities, extent);

    // Get number of images in swap chain according to surface capabilities
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // Set parameters for new swap chain
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = actualExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.oldSwapchain = oldSwapchain;

    // Get queue families
    QueueFamilyIndices indices = deviceRef.getIndices();
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    // Set swap chain parameters based on queue family indices
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    // Attempt to create swap chain
    if (vkCreateSwapchainKHR(deviceRef.getLogicalDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    }

    // Get vector of images for swap chain
    vkGetSwapchainImagesKHR(deviceRef.getLogicalDevice(), swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(deviceRef.getLogicalDevice(), swapChain, &imageCount, swapChainImages.data());

    // Set color format and extent
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    createImageViews();
    createDepthResources();
}

VulkanSwapchain::~VulkanSwapchain() {
    VkDevice device = deviceRef.getLogicalDevice();

    // Clean up depth image
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);

    // Clean up swapchain resources
    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

// --- Public functions ---
void VulkanSwapchain::createFramebuffers(VkRenderPass renderPass) {
    // Ensure that swapChainFramebuffers is the correct size
    swapChainFramebuffers.resize(swapChainImageViews.size());

    // Set parameters and create a framebuffer for each swap chain image view
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        // Make array for attaching image views to framebuffer
        std::array<VkImageView, 2> attachments = {
            swapChainImageViews[i],
            depthImageView
        };

        // Set parameters for framebuffer
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        // Attempt to create framebuffer
        if (vkCreateFramebuffer(deviceRef.getLogicalDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer.");
        }
    }
}

void VulkanSwapchain::recreate(VkSurfaceKHR surface, VkExtent2D extent, VkRenderPass renderPass) {
    vkDeviceWaitIdle(deviceRef.getLogicalDevice());

    // Create temporary swapchain
    VulkanSwapchain tempSwapchain(deviceRef, surface, extent, swapChain);

    // Clean up dependencies
    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(deviceRef.getLogicalDevice(), framebuffer, nullptr);
    }

    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(deviceRef.getLogicalDevice(), imageView, nullptr);
    }
    
    vkDestroyImageView(deviceRef.getLogicalDevice(), depthImageView, nullptr);
    vkDestroyImage(deviceRef.getLogicalDevice(), depthImage, nullptr);
    vkFreeMemory(deviceRef.getLogicalDevice(), depthImageMemory, nullptr);

    // Move data from the temporary swapchain into new swapchain
    this->swapChain = tempSwapchain.swapChain;
    this->swapChainImages = std::move(tempSwapchain.swapChainImages);
    this->swapChainImageFormat = tempSwapchain.swapChainImageFormat;
    this->swapChainExtent = tempSwapchain.swapChainExtent;
    this->swapChainImageViews = std::move(tempSwapchain.swapChainImageViews);
    
    this->depthImage = tempSwapchain.depthImage;
    this->depthImageMemory = tempSwapchain.depthImageMemory;
    this->depthImageView = tempSwapchain.depthImageView;

    // Set the temporary swapchain handle to null
    tempSwapchain.swapChain = VK_NULL_HANDLE;
    tempSwapchain.swapChainImageViews.clear(); 
    tempSwapchain.depthImage = VK_NULL_HANDLE;
    tempSwapchain.depthImageView = VK_NULL_HANDLE;
    tempSwapchain.depthImageMemory = VK_NULL_HANDLE;

    // Make new framebuffers
    createFramebuffers(renderPass);
}

// --- Private functions ---
void VulkanSwapchain::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(deviceRef.getLogicalDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image.");
    }

    // Memory Allocation
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(deviceRef.getLogicalDevice(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = deviceRef.findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(deviceRef.getLogicalDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate image memory.");
    }

    vkBindImageMemory(deviceRef.getLogicalDevice(), image, imageMemory, 0);
}

VkImageView VulkanSwapchain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(deviceRef.getLogicalDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture image view.");
    }

    return imageView;
}

void VulkanSwapchain::createImageViews() {
    // Ensure that swapChainImageViews has correct size
    swapChainImageViews.resize(swapChainImages.size());

    // Set parameters and create each image view in the swap chain
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        // Set parameters for image view
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        // Attempt to create image view
        if (vkCreateImageView(deviceRef.getLogicalDevice(), &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image views.");
        }
    }
}

void VulkanSwapchain::createDepthResources() {
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

VkSurfaceFormatKHR VulkanSwapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    // Check for a specific format otherwise return the first format
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR VulkanSwapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    // Check for a specific present mode otherwise return basic v-sync
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D windowExtent) {
    // Get surface dimensions
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D extent = windowExtent;
        extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return extent;
    }
}

