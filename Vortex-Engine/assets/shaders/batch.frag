#version 460 core

in vec4 v_Color;
in vec2 v_TexCoord;
in float v_TexIndex;

out vec4 color;

void main()
{
    color = v_Color;
}