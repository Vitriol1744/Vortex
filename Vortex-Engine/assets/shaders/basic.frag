#version 460 core

out vec4 color;

uniform vec4 uColor;
uniform vec2 lightPos;

in data
{
    vec4 pos;
    vec4 col;
} fragmentIN;

void main()
{
    float intensity = 1.0 / length(fragmentIN.pos.xy - lightPos);
    color = fragmentIN.col * intensity;
}