#version 460 core

out vec4 color;

uniform float uTime;
uniform vec2 uMouse;

in data
{
    vec4 pos;
    vec4 col;
} fragmentIN;

void main()
{
      vec3 light = vec3(1.0f, 1.0f, 1.0f) * 0.4;
      color = vec4(fragmentIN.col.xyz * light, 1.0f);
}