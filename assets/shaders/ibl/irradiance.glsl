#version 450
#pragma shader_stage(compute)

// Diffuse irradiance: the cosine-weighted average of the environment over the
// hemisphere around each direction (Lambert convolution).

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(set = 0, binding = 0) uniform samplerCube Radiance;
layout(set = 0, binding = 1, rgba16f) uniform writeonly image2DArray Irradiance;

const float PI = 3.14159265359;

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
    ivec3 size = imageSize(Irradiance);
    if (gl_GlobalInvocationID.x >= size.x || gl_GlobalInvocationID.y >= size.y)
        return;

    vec2 uv = (vec2(gl_GlobalInvocationID.xy) + 0.5) / vec2(size.xy) * 2.0 - 1.0;
    vec3 N  = CubeDirection(uv, gl_GlobalInvocationID.z);

    vec3 up    = abs(N.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = cross(N, right);

    // A blurred mip keeps the coarse sampling free of fireflies.
    float lod        = 4.0;
    vec3  irradiance = vec3(0.0);
    float samples    = 0.0;
    const float delta = 0.025;
    for (float phi = 0.0; phi < 2.0 * PI; phi += delta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += delta)
        {
            vec3 tangent = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 dir     = tangent.x * right + tangent.y * up + tangent.z * N;
            irradiance  += textureLod(Radiance, dir, lod).rgb * cos(theta) * sin(theta);
            samples     += 1.0;
        }
    }
    irradiance = PI * irradiance / samples;
    imageStore(Irradiance, ivec3(gl_GlobalInvocationID), vec4(irradiance, 1.0));
}
