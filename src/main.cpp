#include <iostream>
#include <stdexcept>

#include "renderer.hpp"

int main() {
    Renderer app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "CRITICAL ERROR: "<< e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}