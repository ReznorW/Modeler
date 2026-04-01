#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

#include <unordered_map>
#include <vector>

struct InputState {
    // Keyboard tracking
    std::unordered_map<int, bool> current;
    std::unordered_map<int, bool> previous;

    // Mouse tracking
    double mouseX, mouseY;
    double lastX, lastY;
    bool rightMouseDown = false;
    bool previousRightMouseDown = false;
    bool leftMouseDown = false;
    bool previousLeftMouseDown = false;
    double scrollY = 0.0;

    void update(GLFWwindow* window) {
        // Keyboard input
        previous = current;

        static const std::vector<int> keysToTrack = {
            GLFW_KEY_W,
            GLFW_KEY_A, 
            GLFW_KEY_S,
            GLFW_KEY_D,
            GLFW_KEY_Q,
            GLFW_KEY_E,
            GLFW_KEY_X,
            GLFW_KEY_C, 
            GLFW_KEY_V,
            GLFW_KEY_ESCAPE, 
            GLFW_KEY_LEFT, 
            GLFW_KEY_RIGHT, 
            GLFW_KEY_UP, 
            GLFW_KEY_DOWN,
            GLFW_KEY_EQUAL, 
            GLFW_KEY_MINUS
        };

        for (int key : keysToTrack) {
            current[key] = (glfwGetKey(window, key) == GLFW_PRESS);
        }

        // === Mouse input ===
        // --- Right Mouse Button ---
        bool currentlyPressedRMB = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);

        if (currentlyPressedRMB && !rightMouseDown) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwGetCursorPos(window, &mouseX, &mouseY);
            lastX = mouseX;
            lastY = mouseY;
        } else if (!currentlyPressedRMB && rightMouseDown) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        if (currentlyPressedRMB) {
            lastX = mouseX;
            lastY = mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
        }

        rightMouseDown = currentlyPressedRMB;

        // --- Left Mouse Button ---
        previousLeftMouseDown = leftMouseDown;
        leftMouseDown = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
        if (!rightMouseDown) {
            glfwGetCursorPos(window, &mouseX, &mouseY);
        }

    }

    void reset() {
        scrollY = 0.0;
    }

    // Keyboard input types
    bool isKeyPressed(int key) const {
        auto curr = current.find(key);
        auto prev = previous.find(key);
        return (curr != current.end() && curr->second) && 
               (prev == previous.end() || !prev->second);
    }

    bool isKeyHeld(int key) const {
        auto curr = current.find(key);
        return curr != current.end() && curr->second;
    }

    // Mouse input helpers
    float getMouseDX() const { return static_cast<float>(mouseX - lastX); }
    float getMouseDY() const { return static_cast<float>(mouseY - lastY); }
    glm::vec3 getMouseRay(int windowWidth, int windowHeight, glm::mat4 view, glm::mat4 proj) {
        // Convert mouse coords to NDC
        float x = (2.0f * mouseX) / windowWidth - 1.0f;
        float y = 1.0f - (2.0f * mouseY) / windowHeight;

        // Unproject from clip space to view space
        glm::vec4 rayClip = glm::vec4(x, y, -1.0, 1.0);
        glm::vec4 rayView = glm::inverse(proj) * rayClip;
        rayView = glm::vec4(rayView.x, rayView.y, 1.0, 0.0);

        // Unproject from view space to world space
        glm::vec3 rayWorld = glm::vec3(glm::inverse(view) * rayView);
        return glm::normalize(rayWorld);
    }

    // Mouse input types
    bool isRightClickHeld() const {
        return rightMouseDown;
    }

    bool isRightClickPressed() const {
        // TODO: Add logic for getting single click
        return rightMouseDown; 
    }

    bool isLeftClickHeld() const {
        return leftMouseDown;
    }

    bool isLeftClickPressed() const {
        return leftMouseDown && !previousLeftMouseDown;
    }

    bool isScroll() const {
        return scrollY != 0.0;
    }
};