#version 460 core

in vec2 v_TexCoords;
in vec4 vColor;

out vec4 color;

uniform sampler2D u_Texture;

void main()
{
      color = vColor;
      //color = texture(u_Texture, v_TexCoords);
}