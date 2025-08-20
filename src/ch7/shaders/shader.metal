#include <metal_stdlib>
using namespace metal;

struct Uniforms {
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

struct Colors {
    float4 objectColor;
    float4 lightColor;
};

struct VertexIn {
    float3 position [[attribute(0)]];
    float2 tex_cords [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]];
};

vertex VertexOut vertex_main(
        VertexIn in [[stage_in]],
        constant Uniforms& uniforms [[buffer(1)]]
) {
    VertexOut out;
    out.position = uniforms.projectionMatrix * uniforms.viewMatrix * uniforms.modelMatrix * float4(in.position, 1.0);

    return out;
}

fragment float4 object_fragment(
        VertexOut in [[stage_in]],
        constant Colors& colors [[buffer(0)]]
) {
    return colors.objectColor * colors.lightColor;
}

fragment float4 light_fragment(
        VertexOut in [[stage_in]],
        constant Colors& colors [[buffer(0)]]
) {
    return colors.lightColor;
}
