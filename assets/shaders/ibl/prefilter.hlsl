#pragma compute CSMain

// Specular pre-filtering (split-sum, Karis 2013): for one roughness, the
// environment convolved with the GGX lobe, importance sampled. One dispatch
// per mip; roughness grows with the mip level.

#include "../include/IBL.hlsli"

[[vk::binding(0, 0)]] TextureCube Radiance;
[[vk::binding(1, 0)]] [[spv::format_rgba16f]] RWTexture2DArray<float4> Prefiltered;

/// Mirrors the push constants in Environment.cpp.
struct Parameters
{
    float Roughness;
    // Width of the radiance cube's mip 0, for the PDF-based source mip.
    float RadianceSize;
};
[[vk::push_constant]] ConstantBuffer<Parameters> Params;

static const uint SAMPLES = 512;

[numthreads(8, 8, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    uint width, height, faces;
    Prefiltered.GetDimensions(width, height, faces);
    if (id.x >= width || id.y >= height) return;

    float3 N = CubeTexelDirection(id, uint2(width, height));
    float3 V = N;

    // Mip 0 is a mirror: copy the environment.
    if (Params.Roughness <= 0.0)
    {
        Prefiltered[id] = float4(Radiance.SampleLevel(LinearSampler, N, 0.0).rgb, 1.0);
        return;
    }

    float  texelSolidAngle = 4.0 * PI / (6.0 * Params.RadianceSize * Params.RadianceSize);
    float3 color           = (float3)0.0;
    float  weight          = 0.0;
    for (uint i = 0; i < SAMPLES; ++i)
    {
        float3 H     = ImportanceSampleGGX(Hammersley(i, SAMPLES), N, Params.Roughness);
        float3 L     = normalize(2.0 * dot(V, H) * H - V);
        float  NdotL = dot(N, L);
        if (NdotL <= 0.0) continue;

        // Sample a mip whose texels cover the sample's solid angle.
        float NdotH            = max(dot(N, H), 0.0);
        float pdf              = DistributionGGX(NdotH, Params.Roughness) * 0.25;
        float sampleSolidAngle = 1.0 / (float(SAMPLES) * pdf + 0.0001);
        float lod              = 0.5 * log2(sampleSolidAngle / texelSolidAngle) + 1.0;

        color += Radiance.SampleLevel(LinearSampler, L, max(lod, 0.0)).rgb * NdotL;
        weight += NdotL;
    }
    Prefiltered[id] = float4(color / weight, 1.0);
}
