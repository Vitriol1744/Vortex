#pragma vertex VSMain
#pragma fragment PSMain

// Renderer2D's textured, vertex-colored quads.

#include "include/Common.hlsli"

// Set 0: per-view data, bound by the renderer.
struct CameraData
{
    float4x4 ViewProjection;
};
[[vk::binding(0, 0)]] ConstantBuffer<CameraData> Camera;

// Per draw: the object's transform.
struct ObjectData
{
    float4x4 Model;
};
[[vk::push_constant]] ConstantBuffer<ObjectData> Object;

// Set 1: the material.
[[vk::binding(0, 1)]] Texture2D texSampler;

struct VSInput
{
    [[vk::location(0)]] float3 Position : POSITION;
    [[vk::location(1)]] float3 Color : COLOR0;
    [[vk::location(2)]] float2 TexCoord : TEXCOORD0;
};

struct VSOutput
{
    float4                     Position : SV_Position;
    [[vk::location(0)]] float3 Color : COLOR0;
    [[vk::location(1)]] float2 TexCoord : TEXCOORD0;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.Position = mul(Camera.ViewProjection, mul(Object.Model, float4(input.Position, 1.0)));
    output.Color    = input.Color;
    output.TexCoord = input.TexCoord;
    return output;
}

float4 PSMain(VSOutput input) : SV_Target0
{
    float3 sampled = texSampler.Sample(LinearSampler, input.TexCoord).rgb;
    return float4(input.Color * sampled, 1.0);
}
