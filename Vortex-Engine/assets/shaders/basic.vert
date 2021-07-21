#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in float texIndex;

out vec4 vColor;

uniform mat4 uView = mat4(1.0f);
uniform mat4 uProjection = mat4(1.0f);

void main()
{
    vColor = color;
    gl_Position = vec4(pos, 1.0f);
}