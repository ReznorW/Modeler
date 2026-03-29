#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 cameraPos;
} ubo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

void main() {
    // Check if grid
    if (fragColor.r < 0.0) {
        vec2 coord = fragWorldPos.xz;
        vec2 derivative = fwidth(coord);

        // Make grid lines
        vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
        float line = min(grid.x, grid.y);
        float gridAlpha = 1.0 - min(line, 1.0);

        // Make axis lines
        float axisThickness = 2.0;
        float isXAxis = step(abs(coord.y), derivative.y * axisThickness);
        float isZAxis = step(abs(coord.x), derivative.x * axisThickness);
        
        // Set color
        vec3 finalGridColor = vec3(0.3);

        if (isXAxis > 0.5) finalGridColor = vec3(1.0, 0.2, 0.2);
        if (isZAxis > 0.5) finalGridColor = vec3(0.2, 0.2, 1.0);

        // Linear fog
        float dist = length(ubo.cameraPos.xyz - fragWorldPos);
        float opacity = 1.0 - smoothstep(10.0, 100.0, dist);

        float alpha = max(gridAlpha, max(isXAxis, isZAxis));
        if (alpha < 0.1) discard;

        outColor = vec4(finalGridColor, alpha * opacity);
        return;
    }

    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(vec3(0.4, -1.0, 0.7));
    vec3 viewDir = normalize(ubo.cameraPos.xyz - fragWorldPos);

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