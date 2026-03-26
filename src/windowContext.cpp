#include "windowContext.hpp"
#include "renderer.hpp"
#include <stdexcept>
#include <iostream>

WindowContext::WindowContext(int w, int h, const std::string& name) 
    : width(w), height(h), windowName(name) {
    initWindow();
}

WindowContext::~WindowContext() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void WindowContext::initInput(Renderer* renderer) {
    glfwSetWindowUserPointer(window, renderer);

    glfwSetWindowCloseCallback(window, [](GLFWwindow* w) {
        glfwSetWindowShouldClose(w, GLFW_TRUE);
    });

    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
        auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(w));

        if (action == GLFW_PRESS) {
            // Spawn random cube
            if (key == GLFW_KEY_A) {
                float x = (rand() % 10 - 5) / 2.0f;
                float y = (rand() % 10 - 5) / 2.0f;
                float z = (rand() % 10 - 5) / 2.0f;

                float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                
                app->addCube(glm::vec3(x, y, z), 0.5f, glm::vec3(r, g, b));
                //std::cout << "Cube added! Total vertices: " << app->vertices.size() << std::endl;
            }

            // Clear screen
            if (key == GLFW_KEY_C) {
                app->clearGeometry();
            }

            // Close program
            if (key == GLFW_KEY_ESCAPE) {
                glfwSetWindowShouldClose(w, GLFW_TRUE);
            }
        }
    });
}

void WindowContext::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
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