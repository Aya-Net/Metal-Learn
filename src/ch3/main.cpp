#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <AppKit/AppKit.hpp>
#include <glm.hpp>

#include "glfw_adapter.h"

#include "utils/shader.h"

#include <string>
#include <iostream>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
};

int main()
{
    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    MTL::CommandQueue* queue = device->newCommandQueue();
    CA::MetalLayer* swapchain = CA::MetalLayer::layer();
    swapchain->setDevice(device);
    swapchain->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm);


    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello osx!", nullptr, nullptr);
    NS::Window* nswindow = get_ns_window(window, swapchain);

    MTL::ClearColor clearColor{0.1f, 0.2f, 0.3f, 1.0f};

    Shader shader("shaders/shader.metal", "vertex_main", "fragment_main");
    shader.Compile(device);
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();

    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vertexDescriptor->attributes()->object(0)->setOffset(0);
    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);
    vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
    vertexDescriptor->attributes()->object(1)->setOffset(sizeof(glm::vec3));
    vertexDescriptor->attributes()->object(1)->setBufferIndex(0);
    vertexDescriptor->layouts()->object(0)->setStride(sizeof(Vertex));
    vertexDescriptor->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);

    MTL::RenderPipelineState* trianglePipeline = shader.createPipeline(device, vertexDescriptor);


    MTL::Buffer* buffer;
    {
        Vertex vertices[3] = {
                {{ 0.0,  0.5, 0}, {1.0, 0.0, 0.0}},
                {{ 0.5, -0.5, 0}, {0.0, 1.0, 0.0}},
                {{-0.5, -0.5, 0}, {0.0, 0.0, 1.0}},
        };
        buffer = device->newBuffer(3 * sizeof(Vertex), MTL::ResourceStorageModeShared);

        memcpy(buffer->contents(), vertices, 3*sizeof(Vertex));
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
                renderCommandEncoder->setVertexBuffer(buffer, 0, 0);
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

