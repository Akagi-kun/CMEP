#version 450

layout(binding = 0) uniform MAT {
    mat4 data;
} mvp;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = mvp.data * vec4(inPosition, 1.0);
    fragColor = inColor;
}