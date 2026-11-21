#pragma vertex VSMain
#pragma fragment PSMain

// Metal/roughness PBR with image-based lighting. Compiled twice: as is for
// static meshes, and with SKINNED defined for skeletal meshes.

#include "include/Scene.hlsli"

/// Per object. Mirrors `ObjectData` in SceneRenderer.cpp.
struct ObjectData
{
    float4x4 Model;
    float4   Albedo;
    // x: metallic, y: roughness.
    float4   Surface;
};

/// Per draw: where this draw's instances start in `Objects`. Objects drawn
/// alike (same mesh and material) are one instanced draw.
struct DrawData
{
    uint FirstObject;
};
[[vk::push_constant]] ConstantBuffer<DrawData> Draw;

// Set 0: the environment, next to the camera, and every object of the view.
[[vk::binding(1, 0)]] TextureCube Irradiance;
[[vk::binding(2, 0)]] TextureCube Prefiltered;
[[vk::binding(3, 0)]] Texture2D   BRDFLut;
[[vk::binding(5, 0)]] StructuredBuffer<ObjectData> Objects;

// Set 1: the material.
[[vk::binding(0, 1)]] Texture2D texSampler;

#ifdef SKINNED
// Set 2: this instance's pose, bone-space to model-space matrices.
[[vk::binding(0, 2)]] StructuredBuffer<float4x4> Bones;
#endif

struct VSInput
{
    [[vk::location(0)]] float3 Position : POSITION;
    [[vk::location(1)]] float3 Normal : NORMAL;
    [[vk::location(2)]] float2 TexCoord : TEXCOORD0;
#ifdef SKINNED
    [[vk::location(3)]] uint4  BoneIndices : BLENDINDICES;
    [[vk::location(4)]] float4 BoneWeights : BLENDWEIGHT;
#endif
};

struct VSOutput
{
    float4                     Position : SV_Position;
    [[vk::location(0)]] float3 WorldPosition : POSITION0;
    [[vk::location(1)]] float3 Normal : NORMAL0;
    [[vk::location(2)]] float2 TexCoord : TEXCOORD0;
    [[vk::location(3)]] nointerpolation float4 Albedo : COLOR0;
    [[vk::location(4)]] nointerpolation float4 Surface : COLOR1;
};

/// transpose(inverse(m)) up to scale, which is all a normal needs: the
/// cofactor matrix, with the determinant's sign so mirrored transforms keep
/// their normals pointing out.
float3x3 NormalMatrix(float3x3 m)
{
    float3x3 cofactor = float3x3(cross(m[1], m[2]), cross(m[2], m[0]), cross(m[0], m[1]));
    return dot(m[0], cofactor[0]) < 0.0 ? -cofactor : cofactor;
}

VSOutput VSMain(VSInput input, uint instance : SV_InstanceID)
{
    ObjectData object = Objects[Draw.FirstObject + instance];
    float4x4   model  = object.Model;
#ifdef SKINNED
    float4x4 skin = Bones[input.BoneIndices.x] * input.BoneWeights.x
                  + Bones[input.BoneIndices.y] * input.BoneWeights.y
                  + Bones[input.BoneIndices.z] * input.BoneWeights.z
                  + Bones[input.BoneIndices.w] * input.BoneWeights.w;
    model = mul(model, skin);
#endif
    float4 world = mul(model, float4(input.Position, 1.0));

    VSOutput output;
    output.WorldPosition = world.xyz;
    output.Normal        = normalize(mul(NormalMatrix((float3x3)model), input.Normal));
    output.TexCoord      = input.TexCoord;
    output.Albedo        = object.Albedo;
    output.Surface       = object.Surface;
    output.Position      = mul(Camera.ViewProjection, world);
    return output;
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return (NdotV / (NdotV * (1.0 - k) + k)) * (NdotL / (NdotL * (1.0 - k) + k));
}

float DistributionGGX(float NdotH, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float d  = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max((float3)(1.0 - roughness), F0) - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float4 PSMain(VSOutput input) : SV_Target0
{
    float4 base      = texSampler.Sample(LinearSampler, input.TexCoord) * input.Albedo;
    // Alpha-tested cutouts (leaves, chains, fences): textures without alpha
    // are opaque and never cut.
    if (base.a < 0.5) discard;
    float3 albedo    = base.rgb;
    float  metallic  = saturate(input.Surface.x);
    float  roughness = clamp(input.Surface.y, 0.04, 1.0);

    float3 N     = normalize(input.Normal);
    float3 V     = normalize(Camera.Position.xyz - input.WorldPosition);
    float  NdotV = max(dot(N, V), 0.0001);
    float3 F0    = lerp((float3)0.04, albedo, metallic);

    // Direct light: one directional light.
    float3 L        = normalize(-Camera.LightDirection.xyz);
    float3 H        = normalize(V + L);
    float  NdotL    = max(dot(N, L), 0.0);
    float  NdotH    = max(dot(N, H), 0.0);
    float3 F        = FresnelSchlick(max(dot(H, V), 0.0), F0);
    float3 specular = DistributionGGX(NdotH, roughness)
                    * GeometrySmith(NdotV, NdotL, roughness) * F
                    / (4.0 * NdotV * max(NdotL, 0.0001));
    float3 kD       = (1.0 - F) * (1.0 - metallic);
    float3 direct   = (kD * albedo / PI + specular) * Camera.LightColor.rgb
                    * Camera.LightColor.a * NdotL;

    // Image-based light (split sum).
    float3 Fr      = FresnelSchlickRoughness(NdotV, F0, roughness);
    float3 kDIbl   = (1.0 - Fr) * (1.0 - metallic);
    float3 diffuse = Irradiance.Sample(LinearSampler, N).rgb * albedo;

    uint width, height, levels;
    Prefiltered.GetDimensions(0, width, height, levels);
    float  maxLod      = float(levels - 1);
    float3 R           = reflect(-V, N);
    float3 prefiltered = Prefiltered.SampleLevel(LinearSampler, R, roughness * maxLod).rgb;
    float2 brdf        = BRDFLut.Sample(LinearSampler, float2(NdotV, roughness)).rg;
    float3 ambient     = (kDIbl * diffuse + prefiltered * (Fr * brdf.x + brdf.y))
                       * Camera.Environment.y;

    float3 color = (direct + ambient) * Camera.Environment.x;
    return float4(Tonemap(color), base.a);
}
