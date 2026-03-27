#pragma once

#include <vulkan/vulkan.h>

#include "vulkanDevice.hpp"

class VulkanBuffer {
public:
    VulkanBuffer(VulkanDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~VulkanBuffer();

    // Prevent duplicates
    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;

    void map();
    void unmap();
    void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE);

    VkBuffer getHandle() const { return buffer; }
    VkDeviceSize getSize() const { return bufferSize; }

private:
    VulkanDevice& deviceRef;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDeviceSize bufferSize;
    void* mapped = nullptr;
};