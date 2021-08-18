#version 460 core

in vec3 v_FragPos;
in vec3 v_Normal;
in vec3 v_LightPos;

out vec4 color;

uniform vec3 u_ObjectColor;
uniform vec3 u_LightColor;
uniform vec3 u_ViewPos;
uniform float u_Shininess;

void main()
{
      float ambientStrength = 0.1f;
      vec3 ambient = ambientStrength * u_LightColor;
      
      vec3 norm = normalize(v_Normal);
      vec3 lightDir = normalize(v_LightPos - v_FragPos);
      float diff = max(dot(norm, lightDir), 0.0);
      vec3 diffuse = diff * u_LightColor;

      float specularStrength = 0.6f;
      vec3 viewDir = normalize(-v_FragPos);
      vec3 reflectDir = reflect(-lightDir, norm);
      float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Shininess);
      vec3 specular = specularStrength * spec * u_LightColor;

      vec3 result = (diffuse + ambient + specular) * u_ObjectColor;
      color = vec4(result, 1.0f);
}