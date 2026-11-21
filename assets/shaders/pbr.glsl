#version 450
#pragma shader_stage(vertex)

// Metal/roughness PBR with image-based lighting. Compiled twice: as is for
// static meshes, and with SKINNED defined for skeletal meshes.

layout(set = 0, binding = 0) uniform Camera {
    mat4 ViewProjection;
    mat4 InverseViewProjection;
    vec4 Position;
    // xyz: direction the light travels, w: unused.
    vec4 LightDirection;
    // rgb: color, a: intensity.
    vec4 LightColor;
    // x: exposure, y: environment intensity, z: skybox blur (mip), w: unused.
    vec4 Environment;
} camera;

layout(push_constant) uniform Object {
    mat4 Model;
    vec4 Albedo;
    // x: metallic, y: roughness.
    vec4 Surface;
} object;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
#ifdef SKINNED
layout(location = 3) in uvec4 inBoneIndices;
layout(location = 4) in vec4 inBoneWeights;

// Set 2: this instance's pose, bone-space to model-space matrices.
layout(std430, set = 2, binding = 0) readonly buffer Bones {
    mat4 Transforms[];
} bones;
#endif

layout(location = 0) out vec3 outWorldPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTexCoord;

void main()
{
    mat4 model = object.Model;
#ifdef SKINNED
    mat4 skin = bones.Transforms[inBoneIndices.x] * inBoneWeights.x
              + bones.Transforms[inBoneIndices.y] * inBoneWeights.y
              + bones.Transforms[inBoneIndices.z] * inBoneWeights.z
              + bones.Transforms[inBoneIndices.w] * inBoneWeights.w;
    model = model * skin;
#endif
    vec4 world       = model * vec4(inPosition, 1.0);
    outWorldPosition = world.xyz;
    outNormal        = normalize(mat3(transpose(inverse(model))) * inNormal);
    outTexCoord      = inTexCoord;
    gl_Position      = camera.ViewProjection * world;
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
layout(set = 0, binding = 1) uniform samplerCube Irradiance;
layout(set = 0, binding = 2) uniform samplerCube Prefiltered;
layout(set = 0, binding = 3) uniform sampler2D   BRDFLut;

// Set 1: the material.
layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform Object {
    mat4 Model;
    vec4 Albedo;
    vec4 Surface;
} object;

layout(location = 0) in vec3 inWorldPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

float DistributionGGX(float NdotH, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float d  = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}
float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return (NdotV / (NdotV * (1.0 - k) + k)) * (NdotL / (NdotL * (1.0 - k) + k));
}
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0)
                    * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ACES filmic curve (Narkowicz's fit).
vec3 Tonemap(vec3 color)
{
    return clamp((color * (2.51 * color + 0.03)) / (color * (2.43 * color + 0.59) + 0.14),
                 0.0, 1.0);
}

void main()
{
    vec4  base      = texture(texSampler, inTexCoord) * object.Albedo;
    // Alpha-tested cutouts (leaves, chains, fences).
    if (base.a < 0.5) discard;
    vec3  albedo    = base.rgb;
    float metallic  = clamp(object.Surface.x, 0.0, 1.0);
    float roughness = clamp(object.Surface.y, 0.04, 1.0);

    vec3  N     = normalize(inNormal);
    vec3  V     = normalize(camera.Position.xyz - inWorldPosition);
    float NdotV = max(dot(N, V), 0.0001);
    vec3  F0    = mix(vec3(0.04), albedo, metallic);

    // Direct light: one directional light.
    vec3  L        = normalize(-camera.LightDirection.xyz);
    vec3  H        = normalize(V + L);
    float NdotL    = max(dot(N, L), 0.0);
    float NdotH    = max(dot(N, H), 0.0);
    vec3  F        = FresnelSchlick(max(dot(H, V), 0.0), F0);
    vec3  specular = DistributionGGX(NdotH, roughness)
                   * GeometrySmith(NdotV, NdotL, roughness) * F
                   / (4.0 * NdotV * max(NdotL, 0.0001));
    vec3  kD       = (1.0 - F) * (1.0 - metallic);
    vec3  direct   = (kD * albedo / PI + specular) * camera.LightColor.rgb
                   * camera.LightColor.a * NdotL;

    // Image-based light (split sum).
    vec3  Fr       = FresnelSchlickRoughness(NdotV, F0, roughness);
    vec3  kDIbl    = (1.0 - Fr) * (1.0 - metallic);
    vec3  diffuse  = texture(Irradiance, N).rgb * albedo;
    float maxLod   = float(textureQueryLevels(Prefiltered) - 1);
    vec3  R        = reflect(-V, N);
    vec3  prefiltered = textureLod(Prefiltered, R, roughness * maxLod).rgb;
    vec2  brdf     = texture(BRDFLut, vec2(NdotV, roughness)).rg;
    vec3  ambient  = (kDIbl * diffuse + prefiltered * (Fr * brdf.x + brdf.y))
                   * camera.Environment.y;

    vec3 color = (direct + ambient) * camera.Environment.x;
    outColor   = vec4(Tonemap(color), base.a);
}
