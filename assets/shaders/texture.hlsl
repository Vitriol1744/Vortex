#pragma shader_stage(vertex)

// HLSL structures representing the constant data layout
struct UniformBufferObject {
    matrix model;
    matrix viewproj;
};

// Bind to descriptor set 0, binding 0 (b0)
// NOTE: shaderc requires layout(row_major) or explicit transpose 
// if your C++ math library (like GLM) sends data in column-major format.
ConstantBuffer<UniformBufferObject> ubo : register(b0);

// Vertex Input Attributes
struct VSInput {
    float3 inPosition : POSITION; // semantic maps to layout(location = 0)
    float3 inColor    : COLOR;    // semantic maps to layout(location = 1)
    float2 inTexCoord : TEXCOORD; // semantic maps to layout(location = 2)
};

// Vertex Output / Fragment Input Stage Interface
struct VSOutput {
    float4 gl_Position : SV_Position; // System Value for clip-space coordinates
    float3 fragColor    : COLOR0;      // User-defined interpolator
    float2 fragTexCoord : TEXCOORD0;   // User-defined interpolator
};

VSOutput main(VSInput input) {
    VSOutput output;
    
    // GLSL matrix multiplication is: viewproj * model * position
    // HLSL defaults to row-major math, but since shaderc converts HLSL 
    // down to GLSL AST rules, keep the multiplication order matching your C++ matrix setup.
    output.gl_Position = mul(ubo.viewproj, mul(ubo.model, float4(input.inPosition, 1.0)));
    
    output.fragColor = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    
    return output;
}

#pragma shader_stage(fragment)

// In Vulkan-flavored HLSL, combined image samplers are split into 
// a Texture2D object and a SamplerState companion object.
// shaderc maps register(t1) to binding 1.
Texture2D texSampler     : register(t1);
SamplerState samState    : register(s1);

struct FSInput {
    float4 gl_Position : SV_Position;
    float3 fragColor    : COLOR0;
    float2 fragTexCoord : TEXCOORD0;
};

// Fragment shader output targets layout(location = 0) via SV_Target0
float4 main(FSInput input) : SV_Target0 {
    // texture() is replaced by Sample()
    float3 sampledColor = texSampler.Sample(samState, input.fragTexCoord).rgb;
    
    return float4(input.fragColor * sampledColor, 1.0);
}
