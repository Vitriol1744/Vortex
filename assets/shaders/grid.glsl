#version 450
#pragma shader_stage(vertex)

// An infinite grid on the XZ plane (y = 0): a full-screen triangle whose
// pixels intersect their view ray with the plane. Writes the plane's depth so
// scene geometry hides it.

layout(set = 0, binding = 0) uniform Camera {
    mat4 ViewProjection;
    mat4 InverseViewProjection;
    vec4 Position;
    vec4 LightDirection;
    vec4 LightColor;
    vec4 Environment;
} camera;

layout(location = 0) out vec3 outNear;
layout(location = 1) out vec3 outFar;

vec3 Unproject(vec2 clip, float depth)
{
    vec4 world = camera.InverseViewProjection * vec4(clip, depth, 1.0);
    return world.xyz / world.w;
}

void main()
{
    vec2 uv     = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    vec2 clip   = uv * 2.0 - 1.0;
    outNear     = Unproject(clip, 0.0);
    outFar      = Unproject(clip, 1.0);
    gl_Position = vec4(clip, 0.0, 1.0);
}

#version 450
#pragma shader_stage(fragment)

layout(set = 0, binding = 0) uniform Camera {
    mat4 ViewProjection;
    mat4 InverseViewProjection;
    vec4 Position;
    vec4 LightDirection;
    vec4 LightColor;
    vec4 Environment;
} camera;

layout(location = 0) in vec3 inNear;
layout(location = 1) in vec3 inFar;
layout(location = 0) out vec4 outColor;

const float kCellSize     = 1.0;  // minor lines, world units
const float kMajorEvery   = 10.0; // a major line every N cells
const float kFadeDistance = 60.0;

/// Coverage of the lines every `spacing` units, anti-aliased by the
/// screen-space derivative.
float Lines(vec2 position, float spacing, float width)
{
    vec2 coord      = position / spacing;
    vec2 derivative = fwidth(coord);
    vec2 grid       = abs(fract(coord - 0.5) - 0.5) / (derivative * width);
    return 1.0 - min(min(grid.x, grid.y), 1.0);
}

void main()
{
    float t = -inNear.y / (inFar.y - inNear.y);
    if (!(t > 0.0) || t > 1.0) discard;

    vec3 position = inNear + t * (inFar - inNear);
    vec4 clip     = camera.ViewProjection * vec4(position, 1.0);
    gl_FragDepth  = clip.z / clip.w;

    vec2  xz    = position.xz;
    float minor = Lines(xz, kCellSize, 1.0);
    float major = Lines(xz, kCellSize * kMajorEvery, 1.5);

    vec3  color = vec3(0.45);
    float alpha = max(minor * 0.35, major * 0.7);

    // The X axis (z = 0) in red, the Z axis (x = 0) in blue.
    vec2 axisWidth = fwidth(xz) * 1.5;
    if (abs(position.z) < axisWidth.y) { color = vec3(0.85, 0.2, 0.2); alpha = 0.9; }
    if (abs(position.x) < axisWidth.x) { color = vec3(0.2, 0.35, 0.9); alpha = 0.9; }

    // Fade out with distance and at grazing angles, where lines alias.
    float distance = length(position - camera.Position.xyz);
    alpha *= 1.0 - smoothstep(kFadeDistance * 0.5, kFadeDistance, distance);
    if (alpha <= 0.002) discard;

    outColor = vec4(color, alpha);
}
