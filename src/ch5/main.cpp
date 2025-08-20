#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <AppKit/AppKit.hpp>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <stb_image.h>

#include "glfw_adapter.h"
#include "utils/shader.h"

#include <string>
#include <iostream>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
};

int main()
{
    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    MTL::CommandQueue* queue = device->newCommandQueue();
    CA::MetalLayer* swapChain = CA::MetalLayer::layer();
    swapChain->setDevice(device);
    swapChain->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm);
    stbi_set_flip_vertically_on_load(true);

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello osx!", nullptr, nullptr);
    NS::Window* nswindow = get_ns_window(window, swapChain);
    MTL::ClearColor clearColor{0.2f, 0.3f, 0.3f, 1.0f};

    Shader shader("shaders/shader.metal", "vertex_main", "fragment_main");
    shader.Compile(device);
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();

    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vertexDescriptor->attributes()->object(0)->setOffset(0);
    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);
    vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
    vertexDescriptor->attributes()->object(1)->setOffset(sizeof(glm::vec3));
    vertexDescriptor->attributes()->object(1)->setBufferIndex(0);
    vertexDescriptor->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(2)->setOffset(sizeof(glm::vec3) + sizeof(glm::vec3));
    vertexDescriptor->attributes()->object(2)->setBufferIndex(0);

    vertexDescriptor->layouts()->object(0)->setStride(sizeof(Vertex));
    vertexDescriptor->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);

    MTL::RenderPipelineState* trianglePipeline = shader.createPipeline(device, vertexDescriptor);

    uint16_t indices[][3] = {
            {0, 1, 3},
            {1, 2, 3}
    };

    MTL::Buffer* indexBuffer = device->newBuffer(
            indices,
            sizeof(indices),
            MTL::ResourceStorageModeShared
    );

    Vertex vertices[] = {
//              ---- 位置 ----         ---- 颜色 ----       - 纹理坐标 -
            {{0.5f,  0.5f, 0.0f},   {1.0f, 0.0f, 0.0f},   {1.0f, 1.0f}},   // 右上
            {{0.5f, -0.5f, 0.0f},   {0.0f, 1.0f, 0.0f},   {1.0f, 0.0f}},   // 右下
            {{-0.5f, -0.5f, 0.0f},  {0.0f, 0.0f, 1.0f},   {0.0f, 0.0f}},   // 左下
            {{-0.5f,  0.5f, 0.0f},  {1.0f, 1.0f, 0.0f},   {0.0f, 1.0f}}    // 左上
    };
    MTL::Buffer* buffer = device->newBuffer(vertices, sizeof(vertices), MTL::ResourceStorageModeShared);

    // Create texture
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("../../assets/container.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::texture2DDescriptor(
            MTL::PixelFormatRGBA8Unorm,
            texWidth,
            texHeight,
            false
    );
    texDesc->setUsage(MTL::TextureUsageShaderRead);
    MTL::Texture* texture = device->newTexture(texDesc);
    texDesc->release();
    MTL::Region region(0, 0, 0, texWidth, texHeight, 1);
    texture->replaceRegion(region, 0, pixels, texWidth * 4);

    stbi_image_free(pixels);

    pixels = stbi_load("../../assets/awesomeface.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    MTL::TextureDescriptor* texDesc2 = MTL::TextureDescriptor::texture2DDescriptor(
            MTL::PixelFormatRGBA8Unorm,
            texWidth,
            texHeight,
            false
    );
    texDesc2->setUsage(MTL::TextureUsageShaderRead);
    MTL::Texture* texture2 = device->newTexture(texDesc2);
    texDesc2->release();

    MTL::Region region2 = MTL::Region(0, 0, 0, texWidth, texHeight, 1);
    texture2->replaceRegion(region2, 0, pixels, texWidth * 4);

    stbi_image_free(pixels);

    glm::mat4 modelMatrix(1.0f);

    MTL::Buffer* model_buffer = device->newBuffer(sizeof(glm::mat4), MTL::ResourceStorageModeShared);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
        {
            CA::MetalDrawable* surface = swapChain->nextDrawable();
            MTL::CommandBuffer* cmd = queue->commandBuffer();
            MTL::RenderPassDescriptor* renderPassDescriptor = MTL::RenderPassDescriptor::alloc()->init();
            MTL::RenderPassColorAttachmentDescriptor* colorAttachment = renderPassDescriptor->colorAttachments()->object(0);
            colorAttachment->setTexture(surface->texture());
            colorAttachment->setLoadAction(MTL::LoadActionClear);
            colorAttachment->setStoreAction(MTL::StoreActionStore);
            colorAttachment->setClearColor(clearColor);
            modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.5f, 0.0f));
            modelMatrix = glm::rotate(modelMatrix, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
            memcpy(model_buffer->contents(), &modelMatrix, sizeof(glm::mat4));
            MTL::RenderCommandEncoder* renderCommandEncoder = cmd->renderCommandEncoder(renderPassDescriptor);
            {
                renderCommandEncoder->setRenderPipelineState(trianglePipeline);
                renderCommandEncoder->setVertexBuffer(buffer, 0, 0);
                renderCommandEncoder->setVertexBuffer(model_buffer, 0, 1);
                renderCommandEncoder->setFragmentTexture(texture, 0);
                renderCommandEncoder->setFragmentTexture(texture2, 1);
                renderCommandEncoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, 6, MTL::IndexTypeUInt16, indexBuffer, 0);
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
    texture->release();
    indexBuffer->release();
    buffer->release();

    glfwTerminate();
    return 0;
}

