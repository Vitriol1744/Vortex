#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;

out data
{
    vec4 pos;
    vec4 col;
} vertexOut;

uniform mat4 uView = mat4(1.0f);
uniform mat4 uModel = mat4(1.0f);
uniform mat4 uProjection = mat4(1.0f);

void main()
{
    gl_Position = uProjection * uView * uModel * vec4(pos, 1.0f);

    vertexOut.pos = uView * uModel * vec4( 1.0f);
    vertexOut.col = vec4(color, 1.0f);
}