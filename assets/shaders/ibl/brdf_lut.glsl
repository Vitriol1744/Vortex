#version 450
#pragma shader_stage(compute)

// The environment BRDF of the split-sum approximation: scale (r) and bias (g)
// to F0, by N.V (u) and roughness (v). Environment-independent; built once.

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(set = 0, binding = 0, rgba16f) uniform writeonly image2D LUT;

const float PI = 3.14159265359;
const uint  SAMPLES = 1024u;

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
    vec3  up       = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3  tangent  = normalize(cross(up, N));
    vec3  bitangent = cross(N, tangent);
    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    // k for image-based lighting.
    float k = (roughness * roughness) / 2.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

void main()
{
    ivec2 size = imageSize(LUT);
    if (gl_GlobalInvocationID.x >= size.x || gl_GlobalInvocationID.y >= size.y)
        return;

    float NdotV     = (float(gl_GlobalInvocationID.x) + 0.5) / float(size.x);
    float roughness = (float(gl_GlobalInvocationID.y) + 0.5) / float(size.y);
    vec3  V = vec3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);
    vec3  N = vec3(0.0, 0.0, 1.0);

    float scale = 0.0;
    float bias  = 0.0;
    for (uint i = 0u; i < SAMPLES; ++i)
    {
        vec3  H     = ImportanceSampleGGX(Hammersley(i, SAMPLES), N, roughness);
        vec3  L     = normalize(2.0 * dot(V, H) * H - V);
        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);
        if (NdotL <= 0.0) continue;

        float G     = GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
        float GVis  = G * VdotH / (NdotH * NdotV);
        float Fc    = pow(1.0 - VdotH, 5.0);
        scale += (1.0 - Fc) * GVis;
        bias  += Fc * GVis;
    }
    imageStore(LUT, ivec2(gl_GlobalInvocationID.xy),
               vec4(scale / float(SAMPLES), bias / float(SAMPLES), 0.0, 1.0));
}
