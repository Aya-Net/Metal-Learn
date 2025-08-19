// src/shaders/basic.metal
#include <metal_stdlib>
using namespace metal;

struct VSOut {
    float4 position [[position]];
    float4 color;
};

vertex VSOut vs_main(uint vid [[vertex_id]]) {
    constexpr float2 verts[3] = {
        float2( 0.0,  0.8),
        float2(-0.8, -0.8),
        float2( 0.8, -0.8)
    };
    constexpr float3 cols[3] = {
        float3(1, 0, 0),
        float3(0, 1, 0),
        float3(0, 0, 1)
    };
    VSOut o;
    o.position = float4(verts[vid], 0, 1);
    o.color = float4(cols[vid], 1.0);
    return o;
}

fragment float4 ps_main(VSOut in [[stage_in]]) {
    return in.color;
}
