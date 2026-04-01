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
    initScene();
    mainLoop();
    cleanup();
}

void Renderer::addCube(glm::vec3 center, float size, glm::vec3 color) {
    float s = size / 2.0f;
    uint32_t startIndex = static_cast<uint32_t>(vertices.size());

    // Initialize surface normals
    glm::vec3 nUp(0.0f, -1.0f, 0.0f);
    glm::vec3 nDown(0.0f, 1.0f, 0.0f);
    glm::vec3 nFront(0.0f, 0.0f, 1.0f);
    glm::vec3 nBack(0.0f, 0.0f, -1.0f);
    glm::vec3 nRight(1.0f, 0.0f, 0.0f);
    glm::vec3 nLeft(-1.0f, 0.0f, 0.0f);

    // Initialize cube vertices
    std::vector<Vertex> cubeVertices = {
        // Front Face (Z+)
        {{center.x - s, center.y - s, center.z + s}, color, nFront},
        {{center.x + s, center.y - s, center.z + s}, color, nFront},
        {{center.x + s, center.y + s, center.z + s}, color, nFront},
        {{center.x - s, center.y + s, center.z + s}, color, nFront},

        // Back Face (Z-)
        {{center.x - s, center.y - s, center.z - s}, color, nBack},
        {{center.x + s, center.y - s, center.z - s}, color, nBack},
        {{center.x + s, center.y + s, center.z - s}, color, nBack},
        {{center.x - s, center.y + s, center.z - s}, color, nBack},

        // Top Face (Y-)
        {{center.x - s, center.y - s, center.z - s}, color, nUp},
        {{center.x + s, center.y - s, center.z - s}, color, nUp},
        {{center.x + s, center.y - s, center.z + s}, color, nUp},
        {{center.x - s, center.y - s, center.z + s}, color, nUp},

        // Bottom Face (Y+)
        {{center.x - s, center.y + s, center.z - s}, color, nDown},
        {{center.x + s, center.y + s, center.z - s}, color, nDown},
        {{center.x + s, center.y + s, center.z + s}, color, nDown},
        {{center.x - s, center.y + s, center.z + s}, color, nDown},

        // Right Face (X+)
        {{center.x + s, center.y - s, center.z + s}, color, nRight},
        {{center.x + s, center.y - s, center.z - s}, color, nRight},
        {{center.x + s, center.y + s, center.z - s}, color, nRight},
        {{center.x + s, center.y + s, center.z + s}, color, nRight},

        // Left Face (X-)
        {{center.x - s, center.y - s, center.z + s}, color, nLeft},
        {{center.x - s, center.y - s, center.z - s}, color, nLeft},
        {{center.x - s, center.y + s, center.z - s}, color, nLeft},
        {{center.x - s, center.y + s, center.z + s}, color, nLeft}
    };

    // Initialize indices
    std::vector<uint32_t> cubeIndices;
    for (int f = 0; f < 6; f++) {
        uint32_t offset = f * 4;
        cubeIndices.push_back(offset + 0);
        cubeIndices.push_back(offset + 1);
        cubeIndices.push_back(offset + 2);
        cubeIndices.push_back(offset + 2);
        cubeIndices.push_back(offset + 3);
        cubeIndices.push_back(offset + 0);
    }

    // Append to global vectors
    vertices.insert(vertices.end(), cubeVertices.begin(), cubeVertices.end());
    for (auto i : cubeIndices) {
        indices.push_back(startIndex + i);
    }

    // Trigger a GPU upload
    dirtyGeo = true;
}

void Renderer::addGrid(float size) {
    float s = size / 2.0f;
    uint32_t startIndex = static_cast<uint32_t>(sceneVertices.size());
    
    glm::vec3 gridFlag = glm::vec3(-1.0f); 
    glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f);

    // Grid vertices
    sceneVertices.push_back({{-s, 0.0f, -s }, gridFlag, up});
    sceneVertices.push_back({{s, 0.0f, -s }, gridFlag, up});
    sceneVertices.push_back({{s, 0.0f, s }, gridFlag, up});
    sceneVertices.push_back({{-s, 0.0f, s }, gridFlag, up});

    // Grid indices
    sceneIndices.push_back(startIndex + 0);
    sceneIndices.push_back(startIndex + 1);
    sceneIndices.push_back(startIndex + 2);
    sceneIndices.push_back(startIndex + 2);
    sceneIndices.push_back(startIndex + 3);
    sceneIndices.push_back(startIndex + 0);

    // Trigger a GPU upload
    dirtyGeo = true;
}

void Renderer::clearGeometry() {
    vertices.clear();
    indices.clear();
    selectedVertex = -1;
    dirtyGeo = true;
}

void Renderer::translateVertex(glm::vec3 translation) {
    if (!showVertices || selectedVertex == -1) return;

    glm::vec3 targetPos = vertices[selectedVertex].pos;

    for (int i = 0; i < vertices.size(); i++) {
        if (glm::distance(vertices[i].pos, targetPos) < 0.0001f) {
            vertices[i].pos += translation;
        }
    }
    
    dirtyGeo = true;
}

int Renderer::findClosestVertex(float threshold = 0.5f) {
    glm::vec3 rayDirWorld = window.getInput().getMouseRay(window.getExtent().width, window.getExtent().height, camera.getView(), camera.getProjection());
    glm::vec3 rayOriginWorld = camera.getPosition();

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 invModel = glm::inverse(modelMatrix);

    glm::vec3 rayOrigin = glm::vec3(invModel * glm::vec4(rayOriginWorld, 1.0f));
    glm::vec3 rayDir = glm::normalize(glm::vec3(invModel * glm::vec4(rayDirWorld, 0.0f)));

    int closestIndex = -1;
    float minDistance = threshold;

    for (int i = 0; i < vertices.size(); i++) {
        // Get vertex position
        glm::vec3 vPos = vertices[i].pos;

        // Get vector from ray origin to vertex
        glm::vec3 toVertex = vPos - rayOrigin;

        float t = glm::dot(toVertex, rayDir);
        if (t < 0) continue;

        // Get the point along on the ray
        glm::vec3 pointOnRay = rayOrigin + rayDir * t;

        // Find distance from point to vertex
        float dist = glm::distance(pointOnRay, vPos);

        if (dist < minDistance) {
            minDistance = dist;
            closestIndex = i;
        }
    }

    return closestIndex;
}

// === Private functions ===
// --- Core Lifecycle ---
void Renderer::initVulkan() {
    // Initialize hardware
    vulkanDevice = std::make_unique<VulkanDevice>(window);

    // Initialize swapchain
    vulkanSwapchain = std::make_unique<VulkanSwapchain>(*vulkanDevice, vulkanDevice->getSurface(), window.getExtent());

    createRenderPass();

    // Create main pipeline
    PipelineConfig mainConfig{};
    mainPipeline = std::make_unique<VulkanPipeline>(*vulkanDevice, renderPass, vulkanSwapchain->getDescriptorSetLayout(), mainConfig, "../shaders/main");

    // Create point pipeline
    PipelineConfig pointConfig{};
    pointConfig.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    pointPipeline = std::make_unique<VulkanPipeline>(*vulkanDevice, renderPass, vulkanSwapchain->getDescriptorSetLayout(), pointConfig, "../shaders/point");
    vulkanSwapchain->createFramebuffers(renderPass);
    createGeoBuffers();
    createUniformBuffers();
    vulkanSwapchain->createDescriptorSets(uboBuffers);
}

void Renderer::initScene() {
    addGrid(200.0f);
}

void Renderer::mainLoop() {
    while (!window.shouldClose()) {
        // Poll input
        window.pollEvents();

        // Process input
        window.updateInput();
        input.update(window, camera, *this);

        // Update camera
        camera.updateMatrices(vulkanSwapchain->getExtentAspectRatio());

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
    // Create Scene Vertex Staging and Device Local
    VkDeviceSize sceneVertexBufferSize = sizeof(Vertex) * MAX_VERTEX_COUNT;
    sceneVertexBuffer = std::make_unique<VulkanBuffer>(*vulkanDevice, sceneVertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Create Scene Index Staging and Device Local
    VkDeviceSize sceneIndexBufferSize = sizeof(uint32_t) * MAX_INDEX_COUNT;
    sceneIndexBuffer = std::make_unique<VulkanBuffer>(*vulkanDevice, sceneIndexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); 

    // Create Vertex Staging and Device Local
    VkDeviceSize vertexBufferSize = sizeof(Vertex) * MAX_VERTEX_COUNT;
    vertexBuffer = std::make_unique<VulkanBuffer>(*vulkanDevice, vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Create Index Staging and Device Local
    VkDeviceSize indexBufferSize = sizeof(uint32_t) * MAX_INDEX_COUNT;
    indexBuffer = std::make_unique<VulkanBuffer>(*vulkanDevice, indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void Renderer::createUniformBuffers() {
    // Create Uniform Buffer Objects for each image
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    size_t imageCount = vulkanSwapchain->getImageCount();
    
    uboBuffers.resize(imageCount);

    for (size_t i = 0; i < imageCount; i++) {
        uboBuffers[i] = std::make_unique<VulkanBuffer>(*vulkanDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        uboBuffers[i]->map();
    }
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
    VkFence inFlightFence = vulkanDevice->getInFlightFence();
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
    VkResult result = vkAcquireNextImageKHR(device, vulkanSwapchain->getHandle(), UINT64_MAX, vulkanDevice->getImageAvailableSemaphore(), VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vulkanSwapchain->recreate(vulkanDevice->getSurface(), window.getExtent(), renderPass);
        return;
    }

    // Update UBO
    updateUniformBuffer(imageIndex);

    // Get commandbuffer
    VkCommandBuffer commandBuffer = vulkanDevice->getCommandBuffer();

    // Reset and begin to record command buffer
    vkResetCommandBuffer(commandBuffer, 0);
    recordCommandBuffer(commandBuffer, imageIndex);

    // Send command buffer to queue
    VkSemaphore waitSemaphores[] = { vulkanDevice->getImageAvailableSemaphore() };
    VkSemaphore signalSemaphores[] = { vulkanDevice->getRenderFinishedSemaphore() };
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
        vulkanSwapchain->recreate(vulkanDevice->getSurface(), window.getExtent(), renderPass);
    }
}

void Renderer::updateUniformBuffer(uint32_t currentImage) {
    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0f);
    ubo.view = camera.getView();
    ubo.proj = camera.getProjection();
    ubo.proj[1][1] *= -1; // Flip Y axis bc Vulkan
    ubo.cameraPos = glm::vec4(camera.getPosition(), 1.0f);

    uboBuffers[currentImage]->writeToBuffer(&ubo);
}

void Renderer::updateGpuBuffers() {
    if (!sceneVertices.empty()) {
        // Update scene vertex buffer
        VkDeviceSize sceneVertexSize = sizeof(Vertex) * sceneVertices.size();
        VulkanBuffer sceneVertexStaging{*vulkanDevice, sceneVertexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        sceneVertexStaging.map();
        sceneVertexStaging.writeToBuffer(sceneVertices.data());
        vulkanDevice->copyBuffer(sceneVertexStaging.getHandle(), sceneVertexBuffer->getHandle(), sceneVertexSize);

        // Update scene index buffer
        VkDeviceSize sceneIndexSize = sizeof(uint32_t) * sceneIndices.size();
        VulkanBuffer sceneIndexStaging{*vulkanDevice, sceneIndexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        sceneIndexStaging.map();
        sceneIndexStaging.writeToBuffer(sceneIndices.data());
        vulkanDevice->copyBuffer(sceneIndexStaging.getHandle(), sceneIndexBuffer->getHandle(), sceneIndexSize);
    }

    if (!vertices.empty()) {
        // Update vertex buffer
        VkDeviceSize vertexSize = sizeof(Vertex) * vertices.size();
        VulkanBuffer vertexStaging{*vulkanDevice, vertexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        vertexStaging.map();
        vertexStaging.writeToBuffer(vertices.data());
        vulkanDevice->copyBuffer(vertexStaging.getHandle(), vertexBuffer->getHandle(), vertexSize);

        // Update index buffer
        VkDeviceSize indexSize = sizeof(uint32_t) * indices.size();
        VulkanBuffer indexStaging{*vulkanDevice, indexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        indexStaging.map();
        indexStaging.writeToBuffer(indices.data());
        vulkanDevice->copyBuffer(indexStaging.getHandle(), indexBuffer->getHandle(), indexSize);
    }
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
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mainPipeline->getPipeline());

    // Bind descriptor sets to command buffer
    VkDescriptorSet currentDescriptorSet = vulkanSwapchain->getDescriptorSet(imageIndex);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mainPipeline->getLayout(), 0, 1, &currentDescriptorSet, 0, nullptr);

    // Bind scene geometry buffers to command buffer
    if (!sceneIndices.empty()) {
        VkBuffer sceneVertexBuffers[] = { sceneVertexBuffer->getHandle() };
        VkDeviceSize sceneOffsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, sceneVertexBuffers, sceneOffsets);
        vkCmdBindIndexBuffer(commandBuffer, sceneIndexBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(sceneIndices.size()), 1, 0, 0, 0);
    }

    // Bind geometry buffers to command buffer
    VkBuffer vertexBuffers[] = { vertexBuffer->getHandle() };
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT32);

    // Pass in indices for draw order
    if (!indices.empty()) {
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    }

    if (showVertices) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pointPipeline->getPipeline());
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pointPipeline->getLayout(), 0, 1, &currentDescriptorSet, 0, nullptr);
        SelectionPC push{};
        push.selectedIndex = selectedVertex;
        vkCmdPushConstants(commandBuffer, pointPipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SelectionPC), &push);
        vkCmdDraw(commandBuffer, vertices.size(), 1, 0, 0);
    }

    // End render pass
    vkCmdEndRenderPass(commandBuffer);

    // Attempt to end command buffer recording
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to finish recording command buffer.");
    }
}