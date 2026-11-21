#pragma compute CSMain

// The environment BRDF of the split-sum approximation: scale (r) and bias (g)
// to F0, by N.V (u) and roughness (v). Environment-independent; built once.

#include "../include/IBL.hlsli"

[[vk::binding(0, 0)]] [[spv::format_rgba16f]] RWTexture2D<float4> LUT;

static const uint SAMPLES = 1024;

float GeometrySchlickGGX(float NdotV, float roughness)
{
    // k for image-based lighting.
    float k = (roughness * roughness) / 2.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

[numthreads(16, 16, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    uint width, height;
    LUT.GetDimensions(width, height);
    if (id.x >= width || id.y >= height) return;

    float  NdotV     = (float(id.x) + 0.5) / float(width);
    float  roughness = (float(id.y) + 0.5) / float(height);
    float3 V         = float3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);
    float3 N         = float3(0.0, 0.0, 1.0);

    float scale = 0.0;
    float bias  = 0.0;
    for (uint i = 0; i < SAMPLES; ++i)
    {
        float3 H     = ImportanceSampleGGX(Hammersley(i, SAMPLES), N, roughness);
        float3 L     = normalize(2.0 * dot(V, H) * H - V);
        float  NdotL = max(L.z, 0.0);
        float  NdotH = max(H.z, 0.0);
        float  VdotH = max(dot(V, H), 0.0);
        if (NdotL <= 0.0) continue;

        float G    = GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
        float GVis = G * VdotH / (NdotH * NdotV);
        float Fc   = pow(1.0 - VdotH, 5.0);
        scale += (1.0 - Fc) * GVis;
        bias  += Fc * GVis;
    }
    LUT[id.xy] = float4(scale / float(SAMPLES), bias / float(SAMPLES), 0.0, 1.0);
}
