//
// Created by GUO XIUYI on 2025/8/20.
//

#include "shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char *shaderPath, const char *vertexName, const char *fragmentName) {
    std::ifstream file;
    file.open(shaderPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << shaderPath << std::endl;
        assert(0);
    }
    std::stringstream reader;
    reader << file.rdbuf();
    std::string raw_string = reader.str();
    source_code_ = NS::String::string(raw_string.c_str(), NS::StringEncoding::UTF8StringEncoding);
    vertex_name_ = NS::String::string(vertexName, NS::StringEncoding::UTF8StringEncoding);
    fragment_name_ = NS::String::string(fragmentName, NS::StringEncoding::UTF8StringEncoding);
    lib_ = nullptr;
}

void Shader::Compile(MTL::Device *device) {
    if (!lib_) {
        MTL::CompileOptions* compileOptions = MTL::CompileOptions::alloc()->init();
        compileOptions->setLanguageVersion(MTL::LanguageVersion1_1);
        NS::Error *error;
        lib_ = device->newLibrary(source_code_, compileOptions, &error);
        if (!lib_) {
            std::cout << error->localizedDescription()->utf8String() << std::endl;
            assert(0);
        }
        compileOptions->release();
        std::cout << "Shader compiled successfully." << std::endl;
    } else {
        std::cout << "Shader already compiled." << std::endl;
    }
}

MTL::RenderPipelineState *Shader::createPipeline(MTL::Device *device, MTL::VertexDescriptor* vertexDescriptor) {
    if (!lib_) {
        std::cout << "Shader not compiled yet. Call Compile() first." << std::endl;
        return nullptr;
    }

    MTL::Function* vert_func = lib_->newFunction(vertex_name_);
    MTL::Function* frag_func = lib_->newFunction(fragment_name_);

    MTL::RenderPipelineDescriptor* pipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pipelineDescriptor->setVertexFunction(vert_func);
    pipelineDescriptor->setFragmentFunction(frag_func);
    pipelineDescriptor->setVertexDescriptor(vertexDescriptor);
    pipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm);

    NS::Error* error;
    MTL::RenderPipelineState* pipelineState = device->newRenderPipelineState(pipelineDescriptor, &error);

    if (!pipelineState) {
        std::cout << "Error creating pipeline state: " << error->localizedDescription()->utf8String() << std::endl;
        assert(0);
    } else {
        std::cout << "Pipeline state created successfully." << std::endl;
    }

    vert_func->release();
    frag_func->release();
    pipelineDescriptor->release();

    return pipelineState;
}

Shader::~Shader() {
    this->lib_->release();
    this->source_code_->release();
    this->vertex_name_->release();
    this->fragment_name_->release();
}


