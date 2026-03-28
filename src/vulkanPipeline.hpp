#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <memory>
#include "vulkanDevice.hpp"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 normal;
    
    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};

class VulkanPipeline {
public:
    VulkanPipeline(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout, const std::string& vertPath, const std::string& fragPath);
    ~VulkanPipeline();

    // Prevent copying
    VulkanPipeline(const VulkanPipeline&) = delete;
    void operator=(const VulkanPipeline&) = delete;

    VkPipeline getPipeline() const { return graphicsPipeline; }
    VkPipelineLayout getLayout() const { return pipelineLayout; }

private:
    static VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);
    static std::vector<char> readFile(const std::string& filename);

    VulkanDevice& deviceRef;
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
};