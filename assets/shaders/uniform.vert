#version 450
#pragma shader_stage(vertex)

layout(set = 0, binding = 0) uniform UniformBufferObject {
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
