#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <AppKit/AppKit.hpp>

#include "glfw_adapter.h"

#include <string>
#include <iostream>

std::string metal_shader_src = R"(
#include <metal_stdlib>
using namespace metal;

struct VertexPayload {
    float4 position [[position]];
    float3 color;
};

constant float4 pos[] = {
    float4( 0.0,  0.5, 0.0, 1.0),
    float4(-0.5, -0.5, 0.0, 1.0),
    float4( 0.5, -0.5, 0.0, 1.0),
};

constant float3 color[] = {
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0),
};

vertex VertexPayload vert_main(
    uint vid [[vertex_id]]
)
{
    VertexPayload payload;
    payload.position = pos[vid];
    payload.color = color[vid];
    return payload;
}

fragment float4 frag_main(
    VertexPayload in [[stage_in]]
)
{
    return float4(in.color, 1.0);
}
)";

int main()
{
    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    MTL::CommandQueue* queue = device->newCommandQueue();
    CA::MetalLayer* swapchain = CA::MetalLayer::layer();
    swapchain->setDevice(device);
    swapchain->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm);

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello osx!", nullptr, nullptr);
    NS::Window* nswindow = get_ns_window(window, swapchain);

    MTL::ClearColor clearColor{0.1f, 0.2f, 0.3f, 1.0f};

    MTL::RenderPipelineState* trianglePipeline;
    {
        MTL::CompileOptions* compileOptions = MTL::CompileOptions::alloc()->init();
        compileOptions->setLanguageVersion(MTL::LanguageVersion1_1);
        NS::Error* error;

        NS::String* ns_shader_src = NS::String::string(metal_shader_src.c_str(), NS::StringEncoding::UTF8StringEncoding);
        MTL::Library* lib = device->newLibrary(ns_shader_src, compileOptions, &error);
        if (!lib) {
            std::cout << error->localizedDescription()->utf8String() << std::endl;
            assert(0);
        }

        NS::String* vs_name = NS::String::string("vert_main", NS::StringEncoding::UTF8StringEncoding);
        MTL::Function* vert_func = lib->newFunction(vs_name);

        NS::String* fs_name = NS::String::string("frag_main", NS::StringEncoding::UTF8StringEncoding);
        MTL::Function* frag_func = lib->newFunction(fs_name);

        MTL::RenderPipelineDescriptor* pipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
        pipelineDescriptor->setVertexFunction(vert_func);
        pipelineDescriptor->setFragmentFunction(frag_func);
        pipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm);

        trianglePipeline = device->newRenderPipelineState(pipelineDescriptor, &error);

        vert_func->release();
        frag_func->release();
        pipelineDescriptor->release();
        lib->release();
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
        {
            CA::MetalDrawable* surface = swapchain->nextDrawable();
            MTL::CommandBuffer* cmd = queue->commandBuffer();
            MTL::RenderPassDescriptor* renderPassDescriptor = MTL::RenderPassDescriptor::alloc()->init();
            MTL::RenderPassColorAttachmentDescriptor* colorAttachment = renderPassDescriptor->colorAttachments()->object(0);
            colorAttachment->setTexture(surface->texture());
            colorAttachment->setLoadAction(MTL::LoadActionClear);
            colorAttachment->setStoreAction(MTL::StoreActionStore);
            colorAttachment->setClearColor(clearColor);

            MTL::RenderCommandEncoder* renderCommandEncoder = cmd->renderCommandEncoder(renderPassDescriptor);
            {
                renderCommandEncoder->setRenderPipelineState(trianglePipeline);
                renderCommandEncoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangleStrip, NS::UInteger(0), NS::UInteger(3));
            }
            renderCommandEncoder->endEncoding();

            cmd->presentDrawable(surface);
            cmd->commit();
            cmd->waitUntilCompleted();
        }
        pool->release();
    }

    queue->release();
    device->release();
    nswindow->release();

    glfwTerminate();
    return 0;
}

