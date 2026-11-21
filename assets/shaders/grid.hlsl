#pragma vertex VSMain
#pragma fragment PSMain

// An infinite grid on the XZ plane (y = 0): a full-screen triangle whose
// pixels intersect their view ray with the plane. Writes the plane's depth so
// scene geometry hides it.

#include "include/Scene.hlsli"

static const float kCellSize     = 1.0;  // minor lines, world units
static const float kMajorEvery   = 10.0; // a major line every N cells
static const float kFadeDistance = 60.0;

struct VSOutput
{
    float4                     Position : SV_Position;
    [[vk::location(0)]] float3 Near : TEXCOORD0;
    [[vk::location(1)]] float3 Far : TEXCOORD1;
};

float3 Unproject(float2 clip, float depth)
{
    float4 world = mul(Camera.InverseViewProjection, float4(clip, depth, 1.0));
    return world.xyz / world.w;
}

VSOutput VSMain(uint vertexID : SV_VertexID)
{
    float2   clip = FullscreenTriangle(vertexID);
    VSOutput output;
    output.Near     = Unproject(clip, 0.0);
    output.Far      = Unproject(clip, 1.0);
    output.Position = float4(clip, 0.0, 1.0);
    return output;
}

/// Coverage of the lines every `spacing` units, anti-aliased by the
/// screen-space derivative.
float Lines(float2 position, float spacing, float width)
{
    float2 coord      = position / spacing;
    float2 derivative = fwidth(coord);
    float2 grid       = abs(frac(coord - 0.5) - 0.5) / (derivative * width);
    return 1.0 - min(min(grid.x, grid.y), 1.0);
}

struct PSOutput
{
    float4 Color : SV_Target0;
    float  Depth : SV_Depth;
};

PSOutput PSMain(VSOutput input)
{
    float t = -input.Near.y / (input.Far.y - input.Near.y);
    if (!(t > 0.0) || t > 1.0) discard;

    float3 position = input.Near + t * (input.Far - input.Near);
    float4 clip     = mul(Camera.ViewProjection, float4(position, 1.0));

    float2 xz    = position.xz;
    float  minor = Lines(xz, kCellSize, 1.0);
    float  major = Lines(xz, kCellSize * kMajorEvery, 1.5);

    float3 color = (float3)0.45;
    float  alpha = max(minor * 0.35, major * 0.7);

    // The X axis (z = 0) in red, the Z axis (x = 0) in blue.
    float2 axisWidth = fwidth(xz) * 1.5;
    if (abs(position.z) < axisWidth.y)
    {
        color = float3(0.85, 0.2, 0.2);
        alpha = 0.9;
    }
    if (abs(position.x) < axisWidth.x)
    {
        color = float3(0.2, 0.35, 0.9);
        alpha = 0.9;
    }

    // Fade out with distance and at grazing angles, where lines alias.
    float distance = length(position - Camera.Position.xyz);
    alpha *= 1.0 - smoothstep(kFadeDistance * 0.5, kFadeDistance, distance);
    if (alpha <= 0.002) discard;

    PSOutput output;
    output.Color = float4(color, alpha);
    output.Depth = clip.z / clip.w;
    return output;
}
