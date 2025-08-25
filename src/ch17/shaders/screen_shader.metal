#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float3 position [[attribute(0)]];
    // float3 norm [[attribute(1)]];
    float2 texCoords [[attribute(1)]];
};

struct ScreenBufferOut {
    float4 position [[position]];
    float2 texCoords;
};

constant const float offset = 1.0 / 600.0;  
constant const float2 offsets[9] = {
        float2(-offset,  offset), // 左上
        float2( 0.0f,    offset), // 正上
        float2( offset,  offset), // 右上
        float2(-offset,  0.0f),   // 左
        float2( 0.0f,    0.0f),   // 中
        float2( offset,  0.0f),   // 右
        float2(-offset, -offset), // 左下
        float2( 0.0f,   -offset), // 正下
        float2( offset, -offset)  // 右下
};
constant const float sampleKernel[9] = {
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16  
};

vertex ScreenBufferOut vertex_screen(
        VertexIn in [[stage_in]]
) {
    ScreenBufferOut out;
    out.position = float4(in.position.x, -in.position.y, 0.0, 1.0);
    out.texCoords = in.texCoords;
    return out;
}

fragment float4 fragment_screen(
        ScreenBufferOut in [[stage_in]],
        texture2d<float> tex [[texture(0)]],
        sampler s [[sampler(0)]]
) {

    float3 sampleTex[9];
    for(int i = 0; i < 9; i++) {
        sampleTex[i] = float3(tex.sample(s, in.texCoords.xy + offsets[i]));
    }
    float3 col = float3(0.0, 0.0, 0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * sampleKernel[i];
    return float4(col, 1.0);
}