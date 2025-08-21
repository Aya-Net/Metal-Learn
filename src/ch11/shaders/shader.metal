#include <metal_stdlib>
using namespace metal;

struct Uniforms {
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 normMatrix;
};

struct DirLight {
    float3 direction;
    float3 ambient;
    float3 diffuse;
    float3 specular;
};

struct PosLight {
    float3 position;
    float3 ambient;
    float3 diffuse;
    float3 specular;

    float constantTerm;
    float linearTerm;
    float quadraticTerm;
};

struct SpotLight {
    float3 position;
    float3 direction;
    float3 ambient;
    float3 diffuse;
    float3 specular;

    float cutOff;
    float outerCutOff;
    float constantTerm;
    float linearTerm;
    float quadraticTerm;
};

struct VertexIn {
    float3 position [[attribute(0)]];
    float3 norm [[attribute(1)]];
    float2 texCoords [[attribute(2)]];
};

struct VertexOut {
    float4 position [[position]];
    float3 fragPos;
    float3 norm;
    float2 texCoords [[attribute(2)]];

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
    out.texCoords = in.texCoords;
    return out;
}

/*
vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));  
vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
FragColor = vec4(ambient + diffuse + specular, 1.0);
*/
fragment float4 object_fragment(
        VertexOut in [[stage_in]],
        texture2d<float> materialDiffuse [[texture(0)]],
        texture2d<float> materialSpecular [[texture(1)]],
        constant float& shininess [[buffer(0)]],
        constant DirLight& dirLight [[buffer(1)]],
        constant float3& viewPos [[buffer(2)]],
        sampler s [[sampler(0)]]
) {
    // float3 ambient = light.ambient * material.ambient;
    float3 ambient = dirLight.ambient * float3(materialDiffuse.sample(s, in.texCoords));

    // diffuse.sample(s, in.texCoords)

    float3 norm = normalize(in.norm);
    float3 lightDir = normalize(-dirLight.direction);
    float diff = max(dot(norm, lightDir), 0.0);

    float3 diffuse = dirLight.diffuse * diff * float3(materialDiffuse.sample(s, in.texCoords));

    // float3 diffuse = light.diffuse * (diff * material.diffuse);

    float3 viewDir = normalize(viewPos - in.fragPos);
    float3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    float3 specular = dirLight.specular * spec * float3(materialSpecular.sample(s, in.texCoords));
    // float3 specular = light.specular * (spec);

    float3 result = ambient + diffuse + specular;
    return float4(result, 1.0);
}

fragment float4 object_fragment_pos(
        VertexOut in [[stage_in]],
        texture2d<float> materialDiffuse [[texture(0)]],
        texture2d<float> materialSpecular [[texture(1)]],
        constant float& shininess [[buffer(0)]],
        constant PosLight& light [[buffer(1)]],
        constant float3& viewPos [[buffer(2)]],
        sampler s [[sampler(0)]]
) {
    // float3 ambient = light.ambient * material.ambient;
    float3 ambient = light.ambient * float3(materialDiffuse.sample(s, in.texCoords));

    // diffuse.sample(s, in.texCoords)

    float3 norm = normalize(in.norm);
    float3 lightDir = normalize(light.position - in.fragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    float3 diffuse = light.diffuse * diff * float3(materialDiffuse.sample(s, in.texCoords));

    // float3 diffuse = light.diffuse * (diff * material.diffuse);

    float3 viewDir = normalize(viewPos - in.fragPos);
    float3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    float3 specular = light.specular * spec * float3(materialSpecular.sample(s, in.texCoords));
    // float3 specular = light.specular * (spec);
    float distance = length(light.position - in.fragPos);
    float attenuation = 1.0 / (light.constantTerm + light.linearTerm * distance + light.quadraticTerm * (distance * distance));

    ambient *= attenuation; 
    diffuse *= attenuation;
    specular *= attenuation;
    float3 result = ambient + diffuse + specular;

    return float4(result, 1.0);
}

fragment float4 object_fragment_spot(
        VertexOut in [[stage_in]],
        texture2d<float> materialDiffuse [[texture(0)]],
        texture2d<float> materialSpecular [[texture(1)]],
        constant float& shininess [[buffer(0)]],
        constant SpotLight& light [[buffer(1)]],
        constant float3& viewPos [[buffer(2)]],
        sampler s [[sampler(0)]]
) {
    float3 lightDir = normalize(light.position - in.fragPos);
    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    float3 result;
    float3 ambient = light.ambient * float3(materialDiffuse.sample(s, in.texCoords));
    float distance = length(light.position - in.fragPos);
    float attenuation = 1.0 / (light.constantTerm + light.linearTerm * distance + light.quadraticTerm * (distance * distance));

    float3 norm = normalize(in.norm);
        
    float diff = max(dot(norm, lightDir), 0.0);

    float3 diffuse = light.diffuse * diff * float3(materialDiffuse.sample(s, in.texCoords));

    float3 viewDir = normalize(viewPos - in.fragPos);
    float3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    float3 specular = light.specular * spec * float3(materialSpecular.sample(s, in.texCoords));
    
    ambient *= attenuation; 
    diffuse *= attenuation;
    specular *= attenuation;

    diffuse  *= intensity;
    specular *= intensity;
    result = ambient + diffuse + specular;
    return float4(result, 1.0);
}

fragment float4 light_fragment(
        VertexOut in [[stage_in]],
        constant PosLight& light [[buffer(1)]]

) {
    return float4(light.specular, 1.0);
}
