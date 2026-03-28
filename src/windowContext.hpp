#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

#include "inputState.hpp"

class Renderer;

class WindowContext {
public:
    WindowContext(int width, int height, const std::string& name);
    ~WindowContext();

    // Prevent duplicates
    WindowContext(const WindowContext&) = delete;
    WindowContext& operator=(const WindowContext&) = delete;

    bool shouldClose() { return glfwWindowShouldClose(window); }
    void pollEvents() { glfwPollEvents(); }
    void waitEvents() { glfwWaitEvents(); }
    
    // Getters
    GLFWwindow* getHandle() { return window; }
    void getFramebufferSize(int* width, int* height);
    VkExtent2D getExtent();

    // Setters
    void setShouldClose(bool value) { glfwSetWindowShouldClose(window, value ? GLFW_TRUE : GLFW_FALSE); }
    
    // Surface creation
    VkResult createSurface(VkInstance instance, VkSurfaceKHR* surface);

    // Input
    void updateInput() { inputState.update(window); }
    void resetInput() { inputState.reset(); }
    InputState& getInput() { return inputState; }

    // Resize flag
    bool wasResized() { return framebufferResized; }
    void resetResizeFlag() { framebufferResized = false; }

private:
    GLFWwindow* window;
    InputState inputState;
    int width;
    int height;
    std::string windowName;
    bool framebufferResized = false;

    void initWindow();
    
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};