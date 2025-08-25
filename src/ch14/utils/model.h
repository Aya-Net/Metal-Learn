//
// Created by GUO XIUYI on 2025/8/22.
//

#ifndef MYMETALAPP_MODEL_H
#define MYMETALAPP_MODEL_H

#include <Metal/Metal.hpp>

#include "mesh.h"

#include <vector>
class Model
{
  public:
    MTL::VertexDescriptor *vertexDescriptor;
    /*  函数   */
    Model(const char *path, MTL::Device *device) : directory_(path) {
        texturePool_ = new TexturePool();
        loadModel(path, device);
        vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
        vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
        vertexDescriptor->attributes()->object(0)->setOffset(0);
        vertexDescriptor->attributes()->object(0)->setBufferIndex(0);

        vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
        vertexDescriptor->attributes()->object(1)->setOffset(3 * sizeof(float));
        vertexDescriptor->attributes()->object(1)->setBufferIndex(0);

        vertexDescriptor->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
        vertexDescriptor->attributes()->object(2)->setOffset(6 * sizeof(float));
        vertexDescriptor->attributes()->object(2)->setBufferIndex(0);

        vertexDescriptor->layouts()->object(0)->setStride(8 * sizeof(float));
        vertexDescriptor->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
    }
    void Draw(MTL::RenderCommandEncoder *encoder, MTL::RenderPipelineState *pipelineState);

    ~Model() {
        for (auto &mesh : meshes_) {
            mesh.vertexBuffer->release();
            mesh.indexBuffer->release();
        }
        delete texturePool_;
        vertexDescriptor->release();
    }
  private:
    /*  模型数据  */
    std::vector<Mesh> meshes_;
    std::string directory_;
    TexturePool *texturePool_;
    /*  函数   */
    void loadModel(const std::string &path, MTL::Device *device);
    void processNode(aiNode *node, const aiScene *scene, MTL::Device *device);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene, MTL::Device *device);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                         std::string typeName, MTL::Device *device);
};

#endif //MYMETALAPP_MODEL_H
