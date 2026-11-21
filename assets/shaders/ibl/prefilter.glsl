#version 450
#pragma shader_stage(compute)

// Specular pre-filtering (split-sum, Karis 2013): for one roughness, the
// environment convolved with the GGX lobe, importance sampled. One dispatch
// per mip; roughness grows with the mip level.

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(set = 0, binding = 0) uniform samplerCube Radiance;
layout(set = 0, binding = 1, rgba16f) uniform writeonly image2DArray Prefiltered;

layout(push_constant) uniform Parameters {
    float Roughness;
    // Width of the radiance cube's mip 0, for the PDF-based source mip.
    float RadianceSize;
} params;

const float PI = 3.14159265359;
const uint  SAMPLES = 512u;

vec3 CubeDirection(vec2 uv, uint face)
{
    switch (face)
    {
        case 0: return normalize(vec3( 1.0, -uv.y, -uv.x));
        case 1: return normalize(vec3(-1.0, -uv.y,  uv.x));
        case 2: return normalize(vec3( uv.x,  1.0,  uv.y));
        case 3: return normalize(vec3( uv.x, -1.0, -uv.y));
        case 4: return normalize(vec3( uv.x, -uv.y,  1.0));
        default: return normalize(vec3(-uv.x, -uv.y, -1.0));
    }
}

vec2 Hammersley(uint i, uint n)
{
    uint bits = i;
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return vec2(float(i) / float(n), float(bits) * 2.3283064365386963e-10);
}

vec3 ImportanceSampleGGX(vec2 xi, vec3 N, float roughness)
{
    float a        = roughness * roughness;
    float phi      = 2.0 * PI * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    vec3  H        = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    vec3 up      = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

float DistributionGGX(float NdotH, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float d  = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

void main()
{
    ivec3 size = imageSize(Prefiltered);
    if (gl_GlobalInvocationID.x >= size.x || gl_GlobalInvocationID.y >= size.y)
        return;

    vec2 uv = (vec2(gl_GlobalInvocationID.xy) + 0.5) / vec2(size.xy) * 2.0 - 1.0;
    vec3 N  = CubeDirection(uv, gl_GlobalInvocationID.z);
    vec3 V  = N;

    // Mip 0 is a mirror: copy the environment.
    if (params.Roughness <= 0.0)
    {
        imageStore(Prefiltered, ivec3(gl_GlobalInvocationID),
                   vec4(textureLod(Radiance, N, 0.0).rgb, 1.0));
        return;
    }

    float texelSolidAngle = 4.0 * PI / (6.0 * params.RadianceSize * params.RadianceSize);
    vec3  color  = vec3(0.0);
    float weight = 0.0;
    for (uint i = 0u; i < SAMPLES; ++i)
    {
        vec3  H     = ImportanceSampleGGX(Hammersley(i, SAMPLES), N, params.Roughness);
        vec3  L     = normalize(2.0 * dot(V, H) * H - V);
        float NdotL = dot(N, L);
        if (NdotL <= 0.0) continue;

        // Sample a mip whose texels cover the sample's solid angle.
        float NdotH = max(dot(N, H), 0.0);
        float pdf   = DistributionGGX(NdotH, params.Roughness) * 0.25;
        float sampleSolidAngle = 1.0 / (float(SAMPLES) * pdf + 0.0001);
        float lod = 0.5 * log2(sampleSolidAngle / texelSolidAngle) + 1.0;

        color  += textureLod(Radiance, L, max(lod, 0.0)).rgb * NdotL;
        weight += NdotL;
    }
    imageStore(Prefiltered, ivec3(gl_GlobalInvocationID), vec4(color / weight, 1.0));
}
