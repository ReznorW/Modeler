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
        int moveVertexLeft = GLFW_KEY_A;
        int moveVertexRight = GLFW_KEY_D;
        int moveVertexUp = GLFW_KEY_W;
        int moveVertexDown = GLFW_KEY_S;
        int moveVertexForward = GLFW_KEY_Q;
        int moveVertexBackward = GLFW_KEY_E;
        int spawnCube = GLFW_KEY_X;
        int clearGeo = GLFW_KEY_C;
        int closeApp = GLFW_KEY_ESCAPE;
        int showVerts = GLFW_KEY_V;
        int addSelected = GLFW_KEY_LEFT_CONTROL;
    };

    void update(WindowContext& window, Camera& camera, Renderer& renderer);

private:
    KeyMappings keys{};
};