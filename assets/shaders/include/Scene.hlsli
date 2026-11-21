// Per-view data of the scene renderer (set 0).
#ifndef VORTEX_SCENE_HLSLI
#define VORTEX_SCENE_HLSLI

#include "Common.hlsli"

/// Mirrors `CameraData` in SceneRenderer.cpp.
struct CameraData
{
    float4x4 ViewProjection;
    float4x4 InverseViewProjection;
    float4   Position;
    // xyz: direction the light travels, w: unused.
    float4   LightDirection;
    // rgb: color, a: intensity.
    float4   LightColor;
    // x: exposure, y: environment intensity, z: skybox blur (mip), w: unused.
    float4   Environment;
};

[[vk::binding(0, 0)]] ConstantBuffer<CameraData> Camera;

/// A triangle covering the screen, from SV_VertexID alone (no vertex buffer):
/// clip-space xy in [-1, 3].
float2 FullscreenTriangle(uint vertexID)
{
    float2 uv = float2((vertexID << 1) & 2, vertexID & 2);
    return uv * 2.0 - 1.0;
}

#endif
