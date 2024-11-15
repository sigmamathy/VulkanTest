#version 450

layout (location = 0) in vec3 ipos;
layout (location = 1) in vec3 icolor;

layout (location = 0) out vec3 vcolor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(ipos, 1.0);
    vcolor = icolor;
}
