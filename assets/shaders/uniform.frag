#version 450
#pragma shader_stage(fragment)

layout(location = 0) in vec4 FragColor;
layout(location = 1) in vec2 pos;
layout(location = 2) in vec2 lightPos;

layout(location = 0) out vec4 outColor;

void main() 
{
    vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    float intensity = 1.0f / length(pos - lightPos);
    
    vec3 result = (ambient) * FragColor.xyz;
    outColor = vec4(result, 1.0f);
    //outColor = fragColor * intensity;
}
