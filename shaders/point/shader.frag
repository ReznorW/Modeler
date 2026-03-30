#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

void main() {
    vec2 circleCoord = 2.0 * gl_PointCoord - 1.0;
    float distSquared = dot(circleCoord, circleCoord);
    if (distSquared > 1.0) {
        discard;
    }

    float z = sqrt(1.0 - distSquared);
    vec3 normal = vec3(circleCoord.x, circleCoord.y, z);

    vec3 lightDir = normalize(vec3(0.4, -1.0, 0.7));
    float diffuse = max(dot(normal, lightDir), 0.2);

    vec3 baseColor = fragColor;
    outColor = vec4(baseColor * diffuse, 0.8); 
}