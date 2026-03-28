#pragma once

#include "windowContext.hpp"
#include "camera.hpp"

class Renderer; 

class Input {
public:
    struct KeyMappings {
        int moveLeft = GLFW_KEY_LEFT;
        int moveRight = GLFW_KEY_RIGHT;
        int moveUp = GLFW_KEY_UP;
        int moveDown = GLFW_KEY_DOWN;
        int zoomIn = GLFW_KEY_EQUAL;
        int zoomOut = GLFW_KEY_MINUS;
        int spawnCube = GLFW_KEY_A;
        int clearGeo = GLFW_KEY_C;
        int closeApp = GLFW_KEY_ESCAPE;
    };

    void update(WindowContext& window, Camera& camera, Renderer& renderer);

private:
    KeyMappings keys{};
};