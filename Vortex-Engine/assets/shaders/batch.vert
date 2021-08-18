#version 460 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;

out vec4 v_Color;
out vec2 v_TexCoord;
out float v_TexIndex;

uniform mat4 u_ViewProjection = mat4(1.0f);
uniform mat4 u_Model          = mat4(1.0f);

void main()
{
    v_Color    = a_Color;
    v_TexCoord = a_TexCoord;
    v_TexIndex = a_TexIndex;

    gl_Position = u_ViewProjection * u_Model * vec4(a_Pos, 1.0f);
}