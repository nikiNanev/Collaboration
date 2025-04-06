#version 450

layout(location = 0) in vec2 inPosition;        // Vertex position (local square vertices)
layout(location = 1) in vec2 inOffset;          // Per-instance offset
layout(location = 2) in vec3 inColor;           // Per-instance color

layout(location = 0) out vec3 fragColor;

layout(binding = 0) uniform UniformBufferObject {
    float time;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    // Add movement using sin/cos over time, unique per instance
    float angle = ubo.time + inOffset.x * 10.0;
    vec2 animatedOffset = inOffset + vec2(sin(angle), cos(angle)) * 0.2;

    vec2 pos = inPosition + animatedOffset;

    gl_Position = ubo.proj * ubo.view * vec4(pos, 0.0, 1.0);
    fragColor = inColor;
}
