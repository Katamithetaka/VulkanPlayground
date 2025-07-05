#version 450

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec3 a_Color;
layout (location = 2) in vec2 a_UV;
layout(location = 0) out vec3 v_fragColor;
layout(location = 1) out vec2 v_UV;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;


void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(a_Position, 0.0, 1.0);
    v_fragColor = a_Color;
    v_UV = a_UV;
}