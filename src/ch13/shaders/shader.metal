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

float3 CalcDirLight(
    thread float2 &texCoords, 
    constant DirLight &light, 
    thread float3 &normal, 
    thread float3 &viewDir,
    thread texture2d<float> &materialDiffuse,
    thread texture2d<float> &materialSpecular,
    constant float &materialShininess,
    thread sampler &s
) {
    float3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
    float3 ambient = light.ambient * float3(materialDiffuse.sample(s, texCoords));
    float3 diffuse = light.diffuse * diff * float3(materialDiffuse.sample(s, texCoords));
    float3 specular = light.specular * spec * float3(materialSpecular.sample(s, texCoords));
    return (ambient + diffuse + specular);
}

float3 CalcPosLight(
    thread float2 &texCoords, 
    thread float3 &fragPos,
    constant PosLight &light,
    thread float3 &normal, 
    thread float3 &viewDir,
    thread texture2d<float> &materialDiffuse,
    thread texture2d<float> &materialSpecular,
    constant float &materialShininess,
    thread sampler &s
) {
    float3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constantTerm + light.linearTerm * distance + 
                 light.quadraticTerm * (distance * distance));    

    float3 ambient = light.ambient * float3(materialDiffuse.sample(s, texCoords));
    float3 diffuse = light.diffuse * diff * float3(materialDiffuse.sample(s, texCoords));
    float3 specular = light.specular * spec * float3(materialSpecular.sample(s, texCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

float3 CalcSpotLight(
    thread float2 &texCoords, 
    thread float3 &fragPos,
    constant SpotLight &light,
    thread float3 &normal, 
    thread float3 &viewDir,
    thread texture2d<float> &materialDiffuse,
    thread texture2d<float> &materialSpecular,
    constant float &materialShininess,
    thread sampler &s
) {
    float3 lightDir = normalize(light.position - fragPos);
    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    float3 ambient = light.ambient * float3(materialDiffuse.sample(s, texCoords));
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constantTerm + light.linearTerm * distance + light.quadraticTerm * (distance * distance));

    float diff = max(dot(normal, lightDir), 0.0);
    float3 diffuse = light.diffuse * diff * float3(materialDiffuse.sample(s, texCoords));

    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);

    float3 specular = light.specular * spec * float3(materialSpecular.sample(s, texCoords));
    
    ambient *= attenuation; 
    diffuse *= attenuation;
    specular *= attenuation;

    diffuse  *= intensity;
    specular *= intensity;
    return (ambient + diffuse + specular);
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
        constant float& materialShininess [[buffer(0)]],
        constant float3& viewPos [[buffer(1)]],
        constant DirLight &dirLight [[buffer(2)]],
        constant PosLight *posLight [[buffer(3)]],
        constant int &posLightCount [[buffer(4)]],
        constant SpotLight &spotLight [[buffer(5)]],
        sampler s [[sampler(0)]]
) {
    float3 norm = normalize(in.norm);
    float3 viewDir = normalize(viewPos - in.fragPos);
    float3 result;
    result = CalcDirLight(in.texCoords, dirLight, norm, viewDir, materialDiffuse, materialSpecular, materialShininess, s);
    // for (int i = 0; i < posLightCount; i++) {
    //     result += CalcPosLight(in.texCoords, in.fragPos, posLight[i], norm, viewDir, materialDiffuse, materialSpecular, materialShininess, s);
    // }
    // result += CalcSpotLight(in.texCoords, in.fragPos, spotLight, norm, viewDir, materialDiffuse, materialSpecular, materialShininess, s);
    return float4(result, 1.0);
}



fragment float4 light_fragment(
        VertexOut in [[stage_in]],
        constant PosLight *posLight [[buffer(3)]],
        constant int &index [[buffer(4)]]

) {
    return float4(posLight[index].specular, 1.0);
}
