cmake_minimum_required(VERSION 3.15)
project(VulkanCAD)

set(CMAKE_CXX_STANDARD 17)

# Find Vulkan
find_package(Vulkan REQUIRED)

# Add GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# Include GLM
set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glm)

# Set up ImGui sources
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
set(IMGUI_SOURCES 
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
)

# Create the executable
add_executable(VulkanCAD 
    src/main.cpp 
    src/renderer.cpp
    src/renderer.hpp
    src/windowContext.cpp
    src/windowContext.hpp
    ${IMGUI_SOURCES}
)

# Link everything together
target_link_libraries(VulkanCAD 
    PRIVATE 
    Vulkan::Vulkan 
    glfw 
)

# Include directories for headers
target_include_directories(VulkanCAD PRIVATE 
    src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${Vulkan_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIR}
    external/glfw/include
)