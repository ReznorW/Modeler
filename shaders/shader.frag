#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} ubo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(vec3(0.4, -1.0, 0.7));
    vec3 viewDir = normalize(ubo.cameraPos - fragWorldPos);

    // Ambient
    float ambient = 0.2;

    // Diffuse
    float diffuse = max(dot(normal, lightDir), 0.0);

    // Specular
    vec3 halfwayDir = normalize(-lightDir + viewDir);
    float specularStength = 0.5;
    float specular = pow(max(dot(normal, halfwayDir), 0.0), 32.0) * specularStength;

    vec3 result = (ambient + diffuse + specular) * fragColor;

    outColor = vec4(result, 1.0);
}