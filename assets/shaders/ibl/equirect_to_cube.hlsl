#pragma compute CSMain

// Projects an equirectangular (latitude/longitude) HDR onto the six faces of
// a cube map, one invocation per output texel.

#include "../include/IBL.hlsli"

[[vk::binding(0, 0)]] Texture2D Equirect;
[[vk::binding(1, 0)]] [[spv::format_rgba16f]] RWTexture2DArray<float4> Cube;

[numthreads(16, 16, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    uint width, height, faces;
    Cube.GetDimensions(width, height, faces);
    if (id.x >= width || id.y >= height) return;

    float3 dir    = CubeTexelDirection(id, uint2(width, height));
    float2 lonLat = float2(atan2(dir.z, dir.x) / (2.0 * PI) + 0.5, acos(dir.y) / PI);
    float4 color  = Equirect.SampleLevel(LinearSampler, lonLat, 0.0);
    Cube[id]      = float4(color.rgb, 1.0);
}
