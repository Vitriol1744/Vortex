#version 450
#pragma shader_stage(vertex)

layout(set = 0, binding = 0) uniform UniformBufferObject 
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec2 lightPos;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 pos;
layout(location = 2) out vec2 lightPos;
//layout(location = 1) out float intensity;

void main() 
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    pos = inPosition;
    lightPos = ubo.lightPos;
    //intensity = 1.0f / length(inPosition - ubo.lightPos);
    fragColor = vec4(inColor, 1.0f);
}


#version 450
#pragma shader_stage(fragment)

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec4 fragColor;
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
    
    vec3 result = (ambient) * fragColor.xyz;
    outColor = vec4(result, 1.0f);
    //outColor = fragColor * intensity;
}
