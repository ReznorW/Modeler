cmake_minimum_required(VERSION 3.15)
project(VulkanCAD)

set(CMAKE_CXX_STANDARD 17)

# 1. Find Vulkan (Required SDK must be installed on your OS)
find_package(Vulkan REQUIRED)

# 2. Add GLFW (Build from source in external folder)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# 3. Include GLM (Header-only)
set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glm)

# 4. Setup ImGui Sources
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

# 5. Create the Executable (Include IMGUI_SOURCES here!)
add_executable(VulkanCAD 
    src/main.cpp 
    ${IMGUI_SOURCES}
)

# 6. Link Everything
target_link_libraries(VulkanCAD 
    PRIVATE 
    Vulkan::Vulkan 
    glfw 
)

# Include directories for headers
target_include_directories(VulkanCAD PRIVATE 
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${Vulkan_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIR}
    external/glfw/include
)