#pragma once

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
    double scrollY = 0.0;
    bool rightMouseDown = false;

    void update(GLFWwindow* window) {
        // Keyboard input
        previous = current;

        static const std::vector<int> keysToTrack = {
            GLFW_KEY_A, 
            GLFW_KEY_C, 
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

        // Mouse input
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

    // Mouse input types
    bool isRightClickHeld() const {
        return rightMouseDown;
    }

    bool isRightClickPressed() const {
        // TODO: Add logic for getting single click
        return rightMouseDown; 
    }

    bool isScroll() const {
        return scrollY != 0.0;
    }
};