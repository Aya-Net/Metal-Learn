//
// Created by GUO XIUYI on 2025/8/22.
//

#include "mesh.h"

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, std::vector<Texture> &textures, TexturePool *texturePool, MTL::Device *device)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    this->texturePool_ = texturePool;
}

void Mesh::setupMesh(MTL::Device *device) {
    // Create vertex buffer
    vertexBuffer = device->newBuffer(
            vertices.data(),
            sizeof(Vertex) * vertices.size(),
            MTL::ResourceStorageModeShared
    );

    // Create index buffer
    indexBuffer = device->newBuffer(
            indices.data(),
            sizeof(unsigned int) * indices.size(),
            MTL::ResourceStorageModeShared
    );
}

void Mesh::Draw(MTL::RenderCommandEncoder *encoder, MTL::RenderPipelineState *pipelineState) {
    encoder->setRenderPipelineState(pipelineState);
    // Bind textures
    int diffuseId = 0;
    int specularId = 1;
    for (auto & texture : textures) {
        if (texture.type == "texture_diffuse") {
            encoder->setFragmentTexture(texturePool_->getTexture(texture.id), diffuseId);
            diffuseId += 2;
        } else if (texture.type == "texture_specular") {
            encoder->setFragmentTexture(texturePool_->getTexture(texture.id), specularId);
            specularId += 2;
        }
    }

    // Set vertex buffer
    encoder->setVertexBuffer(vertexBuffer, 0, 0);

    // Draw indexed primitives
    encoder->drawIndexedPrimitives(
            MTL::PrimitiveTypeTriangle,
            NS::UInteger(indices.size()),
            MTL::IndexTypeUInt32,
            indexBuffer,
            0
    );
}
