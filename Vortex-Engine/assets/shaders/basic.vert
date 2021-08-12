#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in float texIndex;

out vec4 vColor;
out vec2 v_TexCoords;

uniform mat4 u_ViewProjection = mat4(1.0f);

void main()
{
    vColor = color;
    v_TexCoords = texCoords;
    gl_Position = u_ViewProjection * vec4(pos, 1.0f);
}