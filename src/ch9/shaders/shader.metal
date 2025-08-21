#include <metal_stdlib>
using namespace metal;

struct Uniforms {
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 normMatrix;
};

struct Light {
    float3 position;
    float3 ambient;
    float3 diffuse;
    float3 specular;
};

struct Material {
    float3 ambient;
    float3 diffuse;
    float3 specular;
    float shininess;
};

struct VertexIn {
    float3 position [[attribute(0)]];
    float3 norm [[attribute(1)]];
    float3 texCoord [[attribute(2)]];
};

struct VertexOut {
    float4 position [[position]];
    float3 fragPos;
    float3 norm;
};

vertex VertexOut vertex_main(
        VertexIn in [[stage_in]],
        constant Uniforms& uniforms [[buffer(1)]]
) {
    VertexOut out;
    out.position = uniforms.projectionMatrix * uniforms.viewMatrix * uniforms.modelMatrix * float4(in.position, 1.0);
    out.fragPos = float3(uniforms.modelMatrix * float4(in.position, 1.0));
    float3x3 normalMatrix = float3x3(
        uniforms.normMatrix[0].xyz,
        uniforms.normMatrix[1].xyz,
        uniforms.normMatrix[2].xyz
    );
    out.norm = normalMatrix * in.norm;
    return out;
}

fragment float4 object_fragment(
        VertexOut in [[stage_in]],
        constant Light& light [[buffer(0)]],
        constant Material& material [[buffer(1)]],
        constant float3& viewPos [[buffer(2)]]
) {
    float3 ambient = light.ambient * material.ambient;
    
    float3 norm = normalize(in.norm);
    float3 lightDir = normalize(light.position - in.fragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    float3 diffuse = light.diffuse * (diff * material.diffuse);

    float3 viewDir = normalize(viewPos - in.fragPos);
    float3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float3 specular = light.specular * (spec * material.specular);

    float3 result = ambient + diffuse + specular;
    return float4(result, 1.0);
}

fragment float4 light_fragment(
        VertexOut in [[stage_in]],
        constant Light& light [[buffer(0)]]

) {
    return float4(light.specular, 1.0);
}
