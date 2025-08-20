#include <metal_stdlib>
using namespace metal;

struct Uniforms {
    float4x4 modelMatrix;
};

struct VertexIn {
    float3 position [[attribute(0)]];
    float4 color    [[attribute(1)]];
    float2 tex_cords [[attribute(2)]];
};

struct VertexOut {
    float4 position [[position]];
    float4 color;
    float2 uv;
};

vertex VertexOut vertex_main(
        VertexIn in [[stage_in]],
        constant Uniforms& uniforms [[buffer(1)]]
) {
    VertexOut out;
    out.position = uniforms.modelMatrix * float4(in.position, 1.0);
    out.color = in.color;
    out.uv = in.tex_cords;
    return out;
}

fragment float4 fragment_main(
        VertexOut in [[stage_in]],
        texture2d<float> texture [[texture(0)]],
        texture2d<float> texture1 [[texture(1)]],
        sampler textureSampler [[sampler(0)]]
) {
    float4 texColor = texture.sample(textureSampler, in.uv);
    float4 texColor1 = texture1.sample(textureSampler, in.uv);
    return mix(texColor, texColor1, 0.2);
}
