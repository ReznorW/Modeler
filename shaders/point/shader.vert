#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
    int selectedIndex;
} push;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPos, 1.0);
    gl_PointSize = 10.0;

    // Check if this vertex is selected
    if (gl_VertexIndex == push.selectedIndex) {
        fragColor = vec3(1.0, 1.0, 0.0);
    } else {
        fragColor = vec3(0.0, 1.0, 1.0);
    }
}