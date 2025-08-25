//
// Created by GUO XIUYI on 2025/8/22.
//

#ifndef MYMETALAPP_MESH_H
#define MYMETALAPP_MESH_H

#include <Metal/Metal.hpp>
#include <glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "texture_pool.h"

#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

struct Texture {
    unsigned int id;
    std::string type;
    aiString path;
};

class Mesh {
  public:

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    MTL::Buffer* vertexBuffer = nullptr;
    MTL::Buffer* indexBuffer = nullptr;

    Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, std::vector<Texture> &textures, TexturePool *texturePool, MTL::Device* device);

    void createBuffers(MTL::Device* device) {
        setupMesh(device);
    }

    void Draw(MTL::RenderCommandEncoder* encoder, MTL::RenderPipelineState *pipelineState);

//    ~Mesh() {
//        if (vertexBuffer) vertexBuffer->release();
//        if (indexBuffer) indexBuffer->release();
//    }
  private:

    TexturePool* texturePool_;

    void setupMesh(MTL::Device* device);
};

#endif //MYMETALAPP_MESH_H
