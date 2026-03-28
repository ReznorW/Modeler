#include "windowContext.hpp"
#include "renderer.hpp"
#include <stdexcept>
#include <iostream>

// --- Callbacks ---
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto w = reinterpret_cast<WindowContext*>(glfwGetWindowUserPointer(window));
    if (w) {
        w->getInput().scrollY = yoffset;
    }
}

WindowContext::WindowContext(int w, int h, const std::string& name) 
    : width(w), height(h), windowName(name) {
    initWindow();
}

WindowContext::~WindowContext() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

VkExtent2D WindowContext::getExtent() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D extent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    return extent;
}

void WindowContext::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetScrollCallback(window, scrollCallback);
}

void WindowContext::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<WindowContext*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
    app->width = width;
    app->height = height;
}

void WindowContext::getFramebufferSize(int* w, int* h) {
    glfwGetFramebufferSize(window, w, h);
}

VkResult WindowContext::createSurface(VkInstance instance, VkSurfaceKHR* surface) {
    // Create a Vulkan surface on the GLFW window
    return glfwCreateWindowSurface(instance, window, nullptr, surface);
}