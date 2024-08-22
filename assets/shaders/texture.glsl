#version 450
#pragma shader_stage(vertex)

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec2 lightPos;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(location = 2) out vec4 outPosition;
layout(location = 3) out vec2 lightPos;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    outPosition = ubo.model * vec4(inPosition, 1.0);
    lightPos = ubo.lightPos;
}

#version 450
#pragma shader_stage(fragment)

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec4 inPosition;
layout(location = 3) in vec2 lightPos;

layout(location = 0) out vec4 outColor;

void main() {
    float intensity = 1.0 / length(inPosition.xy - lightPos);
    outColor = vec4(fragColor * texture(texSampler, fragTexCoord).rgb, 1.0) * intensity;
}
