#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <stdexcept>
#include <set>
#include <fstream>
#include <string>
#include <cstdint>
#include <cstddef>
#include <chrono>
#include <array>

#include "renderer.hpp"

// === Public functions ===
void Renderer::run() {
    initVulkan();
    mainLoop();
    cleanup();
}

void Renderer::addCube(glm::vec3 center, float size, glm::vec3 color) {
    float s = size / 2.0f;
    uint32_t startIndex = static_cast<uint32_t>(vertices.size());

    // Initialize cube vertices
    std::vector<Vertex> cubeVertices = {
        {{center.x - s, center.y - s, center.z + s}, color},
        {{center.x + s, center.y - s, center.z + s}, color},
        {{center.x + s, center.y + s, center.z + s}, color},
        {{center.x - s, center.y + s, center.z + s}, color},
        {{center.x - s, center.y - s, center.z - s}, color},
        {{center.x + s, center.y - s, center.z - s}, color},
        {{center.x + s, center.y + s, center.z - s}, color},
        {{center.x - s, center.y + s, center.z - s}, color}
    };

    // Initialize indices
    std::vector<uint32_t> cubeIndices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        4, 0, 3, 3, 7, 4,
        1, 5, 6, 6, 2, 1,
        3, 2, 6, 6, 7, 3,
        4, 5, 1, 1, 0, 4
    };

    // Append to global vectors
    vertices.insert(vertices.end(), cubeVertices.begin(), cubeVertices.end());
    for (auto i : cubeIndices) {
        indices.push_back(startIndex + i);
    }

    // Trigger a GPU upload
    dirtyGeo = true;
}

void Renderer::clearGeometry() {
    vertices.clear();
    indices.clear();
    dirtyGeo = true;
}

// === Private functions ===
// --- Core Lifecycle ---
void Renderer::initVulkan() {
    // Initialize input
    window.initInput(this);

    // Create vulkan instance and surface
    createInstance();
    window.createSurface(instance, &surface);

    // Initialize hardware
    vulkanDevice = std::make_unique<VulkanDevice>(instance, surface);

    // Initialize swapchain
    vulkanSwapchain = std::make_unique<VulkanSwapchain>(*vulkanDevice, surface, window.getExtent());

    createRenderPass();
    vulkanPipeline = std::make_unique<VulkanPipeline>(*vulkanDevice, renderPass, vulkanSwapchain->getDescriptorSetLayout(), "shaders/vert.spv", "shaders/frag.spv");
    vulkanSwapchain->createFramebuffers(renderPass);
    createGeoBuffers();
    createUniformBuffers();
    vulkanSwapchain->createDescriptorSets(uboBuffer->getHandle());
    createSyncObjects();
}

void Renderer::mainLoop() {
    while (!window.shouldClose()) {
        // Poll input
        window.pollEvents();

        // Update cube position
        updateUniformBuffer();

        // Draw frame
        drawFrame();
    }

    // Set device to idle
    vkDeviceWaitIdle(vulkanDevice->getLogicalDevice());
}

void Renderer::cleanup() {
    // Get device
    VkDevice device = vulkanDevice->getLogicalDevice();

    // Wait for device to finish
    vkDeviceWaitIdle(device);

    // Clean up render pass
    vkDestroyRenderPass(device, renderPass, nullptr);

    // Clean up sync objects
    vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
    vkDestroyFence(device, inFlightFence, nullptr);

    // Clean up surface and instance
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

// --- Infrastructure set up ---
void Renderer::createInstance() {
    // Set up application information/background
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Renderer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Vector";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    // Get the required Vulkan extensions for GLFW
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // Set up parameters for a new Vulkan instance
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount = 0;

    // Attempt to create the new Vulkan instance
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance.");
    }
}

void Renderer::createSyncObjects() {
    // Get device
    VkDevice device = vulkanDevice->getLogicalDevice();

    // Set parameters for semaphore
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // Set parameters for fence
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    // Attempt to create semaphores and fence
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create synchronization objects for a frame.");
    }
}

void Renderer::createRenderPass() {
    // Set parameters for color attachment
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = vulkanSwapchain->getFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; 
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; 
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Set parameters for color attachment reference
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Set parameters for depth attachment
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Set parameters for depth attachment reference
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Combine attachments
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

    // Set parameters for subpass and pass in attachment reference
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // Set parameters for subpass dependency
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Set parameters for render pass and pass in attachment, subpass, and subpass dependency
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    // Attempt to create render pass
    if (vkCreateRenderPass(vulkanDevice->getLogicalDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass.");
    }
}

// --- GPU resources ---
void Renderer::createGeoBuffers() {
    // Create Vertex Staging and Device Local
    VkDeviceSize vertexBufferSize = sizeof(Vertex) * MAX_VERTEX_COUNT;
    vertexBuffer = std::make_unique<VulkanBuffer>(*vulkanDevice, vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Create Index Staging and Device Local
    VkDeviceSize indexBufferSize = sizeof(uint32_t) * MAX_INDEX_COUNT;
    indexBuffer = std::make_unique<VulkanBuffer>(*vulkanDevice, indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void Renderer::createUniformBuffers() {
    // Create Uniform Buffer Object and pointer
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    uboBuffer = std::make_unique<VulkanBuffer>(*vulkanDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uboBuffer->map();
}

// --- Frame execution ---
void Renderer::drawFrame() {
    // Handle window minimization
    int width = 0, height = 0;
    window.getFramebufferSize(&width, &height);
    while (width == 0 || height == 0) {
        return;
    }

    // Get device
    VkDevice device = vulkanDevice->getLogicalDevice();

    // Wait for sync objects
    vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

    // Update buffers if geometry is dirty
    if (dirtyGeo) {
        updateGpuBuffers();
        dirtyGeo = false;
    }

    // Reset sync objects
    vkResetFences(device, 1, &inFlightFence);

    // Get next image to write to
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, vulkanSwapchain->getHandle(), UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vulkanSwapchain->recreate(surface, window.getExtent(), renderPass);
        return;
    }

    // Get commandbuffer
    VkCommandBuffer commandBuffer = vulkanDevice->getCommandBuffer();

    // Reset and begin to record command buffer
    vkResetCommandBuffer(commandBuffer, 0);
    recordCommandBuffer(commandBuffer, imageIndex);

    // Send command buffer to queue
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Attempt to submit command buffer
    if (vkQueueSubmit(vulkanDevice->getGraphicsQueue(), 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer.");
    }

    // Set parameters for presenting image
    VkSwapchainKHR swapChains[] = { vulkanSwapchain->getHandle() };
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(vulkanDevice->getPresentQueue(), &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || window.wasResized()) {
        window.resetResizeFlag();
        vulkanSwapchain->recreate(surface, window.getExtent(), renderPass);
    }
}

void Renderer::updateUniformBuffer() {
    float width = static_cast<float>(vulkanSwapchain->getExtent().width);
    float height = static_cast<float>(vulkanSwapchain->getExtent().height);
    if (width <= 0.0f || height <= 0.0f) {
        return; 
    }
    float aspect = width / height;

    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Z
    model = glm::rotate(model, time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Y
    model = glm::rotate(model, time * glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // X
    ubo.model = model;
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1; // Flip Y axis bc Vulkan

    uboBuffer->writeToBuffer(&ubo);
}

void Renderer::updateGpuBuffers() {
    if (vertices.empty()) return;

    // Update vertex buffer
    VkDeviceSize vertexSize = sizeof(Vertex) * vertices.size();
    VulkanBuffer vertexStaging{*vulkanDevice, vertexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
    vertexStaging.map();
    vertexStaging.writeToBuffer(vertices.data());
    vulkanDevice->copyBuffer(vertexStaging.getHandle(), vertexBuffer->getHandle(), vertexSize);

    // Update index buffer
    VkDeviceSize indexSize = sizeof(Vertex) * vertices.size();
    VulkanBuffer indexStaging{*vulkanDevice, indexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
    indexStaging.map();
    indexStaging.writeToBuffer(indices.data());
    vulkanDevice->copyBuffer(indexStaging.getHandle(), indexBuffer->getHandle(), indexSize);
}

void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    // Set parameters for a command buffer to begin
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    // Attempt to begin command buffer
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer.");
    }

    // Set parameters for a render pass to begin
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = vulkanSwapchain->getFramebuffer(imageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = vulkanSwapchain->getExtent();

    // Set background color
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.15f, 0.15f, 0.15f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    // Begin render pass
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Set parameters for viewport
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) vulkanSwapchain->getExtent().width;
    viewport.height = (float) vulkanSwapchain->getExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Set viewport
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    // Set parameters for scissor
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = vulkanSwapchain->getExtent();

    // Set scissor
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // Bind pipeline to command buffer
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->getPipeline());

    // Bind descriptor sets to command buffer
    VkDescriptorSet currentDescriptorSet = vulkanSwapchain->getDescriptorSet(imageIndex);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->getLayout(), 0, 1, &currentDescriptorSet, 0, nullptr);

    // Bind geometry buffers to command buffer
    VkBuffer vertexBuffers[] = { vertexBuffer->getHandle() };
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT32);

    // Pass in indices for draw order
    if (!indices.empty()) {
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    }

    // End render pass
    vkCmdEndRenderPass(commandBuffer);

    // Attempt to end command buffer recording
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to finish recording command buffer.");
    }
}