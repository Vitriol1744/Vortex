#version 450
#pragma shader_stage(vertex)

// The environment behind everything: a full-screen triangle at the far plane
// that looks up the radiance cube along each pixel's view ray.

layout(location = 0) out vec2 outClip;

void main()
{
    // 3 vertices covering the screen, no vertex buffer.
    vec2 uv     = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    outClip     = uv * 2.0 - 1.0;
    gl_Position = vec4(outClip, 1.0, 1.0);
}

#version 450
#pragma shader_stage(fragment)

layout(set = 0, binding = 0) uniform Camera {
    mat4 ViewProjection;
    mat4 InverseViewProjection;
    vec4 Position;
    vec4 LightDirection;
    vec4 LightColor;
    vec4 Environment;
} camera;
layout(set = 0, binding = 4) uniform samplerCube Radiance;

layout(location = 0) in vec2 inClip;
layout(location = 0) out vec4 outColor;

vec3 Tonemap(vec3 color)
{
    return clamp((color * (2.51 * color + 0.03)) / (color * (2.43 * color + 0.59) + 0.14),
                 0.0, 1.0);
}

void main()
{
    vec4 far       = camera.InverseViewProjection * vec4(inClip, 1.0, 1.0);
    vec3 direction = normalize(far.xyz / far.w - camera.Position.xyz);
    vec3 color     = textureLod(Radiance, direction, camera.Environment.z).rgb;
    outColor       = vec4(Tonemap(color * camera.Environment.x * camera.Environment.y), 1.0);
}
