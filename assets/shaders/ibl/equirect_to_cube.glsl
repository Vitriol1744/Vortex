#version 450
#pragma shader_stage(compute)

// Projects an equirectangular (latitude/longitude) HDR onto the six faces of
// a cube map, one invocation per output texel.

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(set = 0, binding = 0) uniform sampler2D Equirect;
layout(set = 0, binding = 1, rgba16f) uniform writeonly image2DArray Cube;

const float PI = 3.14159265359;

// Direction through texel `uv` (in [-1, 1]) of cube face `face`, in Vulkan's
// face order: +X, -X, +Y, -Y, +Z, -Z.
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

void main()
{
    ivec3 size = imageSize(Cube);
    if (gl_GlobalInvocationID.x >= size.x || gl_GlobalInvocationID.y >= size.y)
        return;

    vec2 uv  = (vec2(gl_GlobalInvocationID.xy) + 0.5) / vec2(size.xy) * 2.0 - 1.0;
    vec3 dir = CubeDirection(uv, gl_GlobalInvocationID.z);

    vec2 lonLat = vec2(atan(dir.z, dir.x) / (2.0 * PI) + 0.5, acos(dir.y) / PI);
    vec4 color  = textureLod(Equirect, lonLat, 0.0);
    imageStore(Cube, ivec3(gl_GlobalInvocationID), vec4(color.rgb, 1.0));
}
