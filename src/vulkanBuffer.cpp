#include <cstring>

#include "vulkanBuffer.hpp"

VulkanBuffer::VulkanBuffer(VulkanDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    : deviceRef{device}, bufferSize{size} {
    deviceRef.createBuffer(bufferSize, usage, properties, buffer, memory);
}

VulkanBuffer::~VulkanBuffer() {
    unmap();
    vkDestroyBuffer(deviceRef.getLogicalDevice(), buffer, nullptr);
    vkFreeMemory(deviceRef.getLogicalDevice(), memory, nullptr);
}

void VulkanBuffer::map() {
    vkMapMemory(deviceRef.getLogicalDevice(), memory, 0, bufferSize, 0, &mapped);
}

void VulkanBuffer::unmap() {
    if (mapped) {
        vkUnmapMemory(deviceRef.getLogicalDevice(), memory);
        mapped = nullptr;
    }
}

void VulkanBuffer::writeToBuffer(void* data, VkDeviceSize size) {
    if (size == VK_WHOLE_SIZE) {
        memcpy(mapped, data, bufferSize);
    } else {
        memcpy(mapped, data, size);
    }
}