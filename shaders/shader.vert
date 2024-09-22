#version 450

layout (location = 0) in vec3 ipos;
layout (location = 1) in vec3 icolor;

layout (location = 0) out vec3 vcolor;

void main() {
    gl_Position = vec4(ipos, 1.0);
    vcolor = icolor;
}