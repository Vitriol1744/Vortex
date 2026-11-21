#pragma compute CSMain

// Diffuse irradiance: the cosine-weighted average of the environment over the
// hemisphere around each direction (Lambert convolution).

#include "../include/IBL.hlsli"

[[vk::binding(0, 0)]] TextureCube Radiance;
[[vk::binding(1, 0)]] [[spv::format_rgba16f]] RWTexture2DArray<float4> Irradiance;

[numthreads(8, 8, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    uint width, height, faces;
    Irradiance.GetDimensions(width, height, faces);
    if (id.x >= width || id.y >= height) return;

    float3 N     = CubeTexelDirection(id, uint2(width, height));
    float3 up    = abs(N.y) < 0.999 ? float3(0.0, 1.0, 0.0) : float3(1.0, 0.0, 0.0);
    float3 right = normalize(cross(up, N));
    up           = cross(N, right);

    // A blurred mip keeps the coarse sampling free of fireflies.
    const float lod        = 4.0;
    const float delta      = 0.025;
    float3      irradiance = (float3)0.0;
    float       samples    = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += delta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += delta)
        {
            float3 tangent = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float3 dir     = tangent.x * right + tangent.y * up + tangent.z * N;
            irradiance += Radiance.SampleLevel(LinearSampler, dir, lod).rgb * cos(theta)
                        * sin(theta);
            samples += 1.0;
        }
    }
    Irradiance[id] = float4(PI * irradiance / samples, 1.0);
}
