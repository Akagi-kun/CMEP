#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in int axis;

out int fragAxis;

uniform mat4 matMVP;

void main() 
{ 
    gl_Position = matMVP * vec4(pos, 1.0f);
    fragAxis = axis;
}