// Image-based lighting helpers of the environment precomputation passes.
#ifndef VORTEX_IBL_HLSLI
#define VORTEX_IBL_HLSLI

#include "Common.hlsli"

/// Direction through texel `uv` (in [-1, 1]) of cube face `face`, in Vulkan's
/// face order: +X, -X, +Y, -Y, +Z, -Z.
float3 CubeDirection(float2 uv, uint face)
{
    switch (face)
    {
        case 0: return normalize(float3( 1.0, -uv.y, -uv.x));
        case 1: return normalize(float3(-1.0, -uv.y,  uv.x));
        case 2: return normalize(float3( uv.x,  1.0,  uv.y));
        case 3: return normalize(float3( uv.x, -1.0, -uv.y));
        case 4: return normalize(float3( uv.x, -uv.y,  1.0));
        default: return normalize(float3(-uv.x, -uv.y, -1.0));
    }
}

/// Texel `id` of a cube map `size` texels wide as a direction.
float3 CubeTexelDirection(uint3 id, uint2 size)
{
    float2 uv = (float2(id.xy) + 0.5) / float2(size) * 2.0 - 1.0;
    return CubeDirection(uv, id.z);
}

float2 Hammersley(uint i, uint n)
{
    uint bits = reversebits(i);
    return float2(float(i) / float(n), float(bits) * 2.3283064365386963e-10);
}

float3 ImportanceSampleGGX(float2 xi, float3 N, float roughness)
{
    float  a        = roughness * roughness;
    float  phi      = 2.0 * PI * xi.x;
    float  cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
    float  sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float3 H        = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    float3 up        = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent   = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

float DistributionGGX(float NdotH, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float d  = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

#endif
