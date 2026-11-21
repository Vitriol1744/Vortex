// Helpers shared by every shader.
//
// Vortex shaders are HLSL, compiled for Vulkan by glslang:
//   - Stages are entry points named by `#pragma <stage> <Entry>` lines
//     (vertex, fragment, compute, geometry, tesscontrol, tesseval).
//   - Resources are placed with [[vk::binding(binding, set)]] and push
//     constants declared [[vk::push_constant]] ConstantBuffer<T>.
//   - Textures are bound with their sampler (combined image samplers): a
//     SamplerState only says how to call Sample(); the sampler that is used
//     is the one bound with the texture.
#ifndef VORTEX_COMMON_HLSLI
#define VORTEX_COMMON_HLSLI

static const float PI = 3.14159265359;

/// Any texture's Sample()/SampleLevel() takes this; see above.
SamplerState LinearSampler;

/// ACES filmic curve (Narkowicz's fit).
float3 Tonemap(float3 color)
{
    return saturate((color * (2.51 * color + 0.03))
                    / (color * (2.43 * color + 0.59) + 0.14));
}

#endif
