#version 460 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;

out vec3 v_FragPos;
out vec3 v_Normal;
out vec3 v_LightPos;

uniform mat4 u_ViewProjection   = mat4(1.0f);
uniform mat4 u_View             = mat4(1.0f);
uniform mat4 u_Projection       = mat4(1.0f);
uniform mat4 u_Model            = mat4(1.0f);

uniform vec3 u_LightPos;

void main()
{
    v_LightPos = vec3(u_View * vec4(u_LightPos, 1.0f));
    v_FragPos = vec3(u_View * u_Model * vec4(a_Pos, 1.0f));
    //TODO: Inverse u_Model on CPU
    v_Normal = mat3(transpose(inverse(u_View * u_Model))) * a_Normal;

    gl_Position = u_Projection * vec4(v_FragPos, 1.0f);
}