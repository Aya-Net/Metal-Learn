//
// Created by GUO XIUYI on 2025/8/20.
//

#ifndef MYMETALAPP_SHADER_H
#define MYMETALAPP_SHADER_H

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <AppKit/AppKit.hpp>

class Shader {
  private:
    MTL::Library*   lib_;
    NS::String*     source_code_;
    NS::String*     vertex_name_;
    NS::String*     fragment_name_;
  public:
    Shader(const char* shaderPath, const char* vertexName, const char* fragmentName);
    void Compile(MTL::Device* device);
    MTL::RenderPipelineState* createPipeline(MTL::Device* device, MTL::VertexDescriptor* vertexDescriptor);
    ~Shader();

};
#endif //MYMETALAPP_SHADER_H
