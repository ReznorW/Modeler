#include <iostream>

#include "input.hpp"
#include "renderer.hpp"
#include "windowContext.hpp"

void Input::update(WindowContext& window, Camera& camera, Renderer& renderer) {
    const auto& input = window.getInput();

    // === Keyboard input ===
    // --- Camera ---
    // Rotation
    float rotSpeed = 0.001f;
    if (input.isKeyHeld(keys.moveLeft)) camera.updateOrbit(-rotSpeed, 0.0f);
    if (input.isKeyHeld(keys.moveRight)) camera.updateOrbit(rotSpeed, 0.0f);
    if (input.isKeyHeld(keys.moveUp)) camera.updateOrbit(0.0f, rotSpeed);
    if (input.isKeyHeld(keys.moveDown)) camera.updateOrbit(0.0f, -rotSpeed);

    // Zoom
    float zoomSpeed = 0.01f;
    if (input.isKeyHeld(keys.zoomIn)) camera.updateZoom(-zoomSpeed);
    if (input.isKeyHeld(keys.zoomOut)) camera.updateZoom(zoomSpeed);

    // --- Actions ---
    // Spawn random cube
    if (input.isKeyPressed(keys.spawnCube)) {
        glm::vec3 pos = {(rand() % 10 - 5) / 2.0f, (rand() % 10 - 5) / 2.0f, (rand() % 10 - 5) / 2.0f};
        glm::vec3 color = {static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX};
        renderer.addCube(pos, 0.5f, color);
        std::cout << "Added Cube!" << std::endl;
    }

    // Clear shapes
    if (input.isKeyPressed(keys.clearGeo)) {
        renderer.clearGeometry();
    }

    // Exit program
    if (input.isKeyPressed(keys.closeApp)) {
        glfwSetWindowShouldClose(window.getHandle(), true);
    }

    // === Mouse input ===
    // --- Camera ---
    // Rotation
    if (input.isRightClickHeld()) {
        float sensitivity = 0.005f;

        float dx = input.getMouseDX() * sensitivity;
        float dy = input.getMouseDY() * sensitivity;

        camera.updateOrbit(dx, dy);
    }

    // Zoom
    if (input.isScroll()) {
        float zoomSensitivity = 1.0f;
        camera.updateZoom(static_cast<float>(-input.scrollY) * zoomSensitivity);
    }

    // === Reset input state ===
    window.resetInput();
}