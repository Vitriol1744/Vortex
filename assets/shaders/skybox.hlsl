#pragma vertex VSMain
#pragma fragment PSMain

// The environment behind everything: a full-screen triangle at the far plane
// that looks up the radiance cube along each pixel's view ray.

#include "include/Scene.hlsli"

[[vk::binding(4, 0)]] TextureCube Radiance;

struct VSOutput
{
    float4                     Position : SV_Position;
    [[vk::location(0)]] float2 Clip : TEXCOORD0;
};

VSOutput VSMain(uint vertexID : SV_VertexID)
{
    VSOutput output;
    output.Clip     = FullscreenTriangle(vertexID);
    output.Position = float4(output.Clip, 1.0, 1.0);
    return output;
}

float4 PSMain(VSOutput input) : SV_Target0
{
    float4 far       = mul(Camera.InverseViewProjection, float4(input.Clip, 1.0, 1.0));
    float3 direction = normalize(far.xyz / far.w - Camera.Position.xyz);
    float3 color     = Radiance.SampleLevel(LinearSampler, direction, Camera.Environment.z).rgb;
    return float4(Tonemap(color * Camera.Environment.x * Camera.Environment.y), 1.0);
}
