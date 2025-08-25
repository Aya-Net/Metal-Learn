#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <AppKit/AppKit.hpp>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <stb_image.h>

#include "glfw_adapter.h"
#include "utils/shader.h"
#include "utils/camera.h"
#include "utils/material.h"
#include "utils/model.h"

#include <string>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

struct DirLight {
    alignas(16) glm::vec3 direction;
    alignas(16) glm::vec3 ambient;
    alignas(16) glm::vec3 diffuse;
    alignas(16) glm::vec3 specular;
};

struct PosLight {
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 ambient;
    alignas(16) glm::vec3 diffuse;
    alignas(16) glm::vec3 specular;
    alignas(16) float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 direction;
    alignas(16) glm::vec3 ambient;
    alignas(16) glm::vec3 diffuse;
    alignas(16) glm::vec3 specular;
    alignas(16) float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
};

int screenWidth = 800;
int screenHeight = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = screenWidth;
float lastY = screenHeight;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


MTL::Texture* depthTexture;

int main() {
    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    MTL::CommandQueue* queue = device->newCommandQueue();
    CA::MetalLayer* swapChain = CA::MetalLayer::layer();
    swapChain->setDevice(device);
    swapChain->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm);
//    stbi_set_flip_vertically_on_load(true);

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello Metal!", nullptr, nullptr);
    NS::Window* nswindow = get_ns_window(window, swapChain);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    MTL::ClearColor clearColor{0.05f, 0.05f, 0.05f, 1.0f};



    float cubeVertices[] = {
            // positions          // texture Coords
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    float planeVertices[] = {
            // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
            5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
            -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

            5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
            5.0f, -0.5f, -5.0f,  2.0f, 2.0f
    };

    float transparentVertices[] = {
            // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    std::vector<glm::vec3> vegetation;
    vegetation.emplace_back(-1.5f,  0.0f, -0.48f);
    vegetation.emplace_back( 1.5f,  0.0f,  0.51f);
    vegetation.emplace_back( 0.0f,  0.0f,  0.7f);
    vegetation.emplace_back(-0.3f,  0.0f, -2.3f);
    vegetation.emplace_back( 0.5f,  0.0f, -0.6f);

    MTL::Buffer *cubeBuffer = device->newBuffer(cubeVertices, sizeof(cubeVertices), MTL::ResourceStorageModeShared);
    MTL::Buffer *planeBuffer = device->newBuffer(planeVertices, sizeof(planeVertices), MTL::ResourceStorageModeShared);
    MTL::Buffer *transparentBuffer = device->newBuffer(transparentVertices, sizeof(transparentVertices), MTL::ResourceStorageModeShared);

    TexturePool texturePool;

    texturePool.loadTexture("../../assets/marble.jpg", device, true);
    texturePool.loadTexture("../../assets/metal.png", device, true);
    texturePool.loadTexture("../../assets/blending_transparent_window.png", device, false);

    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();

    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vertexDescriptor->attributes()->object(0)->setOffset(0);
    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);

    vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(1)->setOffset(3 * sizeof(float));
    vertexDescriptor->attributes()->object(1)->setBufferIndex(0);

    vertexDescriptor->layouts()->object(0)->setStride(5 * sizeof(float));
    vertexDescriptor->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);

    Shader shader("shaders/shader.metal", "vertex_main", "fragment_main");
    shader.Compile(device);

    auto renderPipelineColorAttachmentDesc = MTL::RenderPipelineColorAttachmentDescriptor::alloc()->init();
    renderPipelineColorAttachmentDesc->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    renderPipelineColorAttachmentDesc->setBlendingEnabled(true);
    renderPipelineColorAttachmentDesc->setRgbBlendOperation(MTL::BlendOperationAdd);
    renderPipelineColorAttachmentDesc->setAlphaBlendOperation(MTL::BlendOperationAdd);
    renderPipelineColorAttachmentDesc->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    renderPipelineColorAttachmentDesc->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
    renderPipelineColorAttachmentDesc->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    renderPipelineColorAttachmentDesc->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    MTL::RenderPipelineState* renderPipeline = shader.createPipeline(device, vertexDescriptor, renderPipelineColorAttachmentDesc);
    renderPipelineColorAttachmentDesc->release();
    vertexDescriptor->release();

    MTL::TextureDescriptor* depthDesc = MTL::TextureDescriptor::texture2DDescriptor(
            MTL::PixelFormatDepth32Float,
            screenWidth * 2,
            screenHeight * 2,
            false
    );
    depthDesc->setUsage(MTL::TextureUsageRenderTarget);
    depthDesc->setStorageMode(MTL::StorageModePrivate);
    depthTexture = device->newTexture(depthDesc);
    depthDesc->release();

    MTL::SamplerDescriptor *planeSamplerDesc = MTL::SamplerDescriptor::alloc()->init();
    planeSamplerDesc->setMinFilter(MTL::SamplerMinMagFilterLinear);
    planeSamplerDesc->setMagFilter(MTL::SamplerMinMagFilterLinear);
    planeSamplerDesc->setMipFilter(MTL::SamplerMipFilterLinear);
    planeSamplerDesc->setSAddressMode(MTL::SamplerAddressModeRepeat);
    planeSamplerDesc->setTAddressMode(MTL::SamplerAddressModeRepeat);

    MTL::SamplerState *planeSamplerState = device->newSamplerState(planeSamplerDesc);
    planeSamplerDesc->release();

    MTL::SamplerDescriptor *vegetationSamplerDesc = MTL::SamplerDescriptor::alloc()->init();
    planeSamplerDesc->setMinFilter(MTL::SamplerMinMagFilterNearest);
    planeSamplerDesc->setMagFilter(MTL::SamplerMinMagFilterNearest);
    planeSamplerDesc->setMipFilter(MTL::SamplerMipFilterLinear);
    planeSamplerDesc->setSAddressMode(MTL::SamplerAddressModeClampToEdge);
    planeSamplerDesc->setTAddressMode(MTL::SamplerAddressModeClampToEdge);

    MTL::SamplerState *vegetationSamplerState = device->newSamplerState(planeSamplerDesc);
    vegetationSamplerDesc->release();


    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glfwPollEvents();

        //NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
        {

            CA::MetalDrawable* surface = swapChain->nextDrawable();
            MTL::CommandBuffer* cmd = queue->commandBuffer();
            MTL::RenderPassDescriptor* renderPassDescriptor = MTL::RenderPassDescriptor::alloc()->init();
            MTL::RenderPassColorAttachmentDescriptor* colorAttachment = renderPassDescriptor->colorAttachments()->object(0);
            colorAttachment->setTexture(surface->texture());
            colorAttachment->setLoadAction(MTL::LoadActionClear);
            colorAttachment->setStoreAction(MTL::StoreActionStore);
            colorAttachment->setClearColor(clearColor);

            MTL::RenderPassDepthAttachmentDescriptor* depthAttachment = renderPassDescriptor->depthAttachment();
            depthAttachment->setTexture(depthTexture);
            depthAttachment->setLoadAction(MTL::LoadActionClear);
            depthAttachment->setStoreAction(MTL::StoreActionDontCare);
            depthAttachment->setClearDepth(1.0);
            MTL::DepthStencilDescriptor* depthStateDesc = MTL::DepthStencilDescriptor::alloc()->init();
            depthStateDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
            depthStateDesc->setDepthWriteEnabled(true);
            MTL::DepthStencilState* depthState = device->newDepthStencilState(depthStateDesc);
            renderPassDescriptor->setDepthAttachment(depthAttachment);
            glm::mat4 modelMatrix(1.0f);
            glm::mat4 matrices[] = {
                    modelMatrix,
                    camera.GetViewMatrix(),
                    glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f),
                    glm::transpose(glm::inverse(modelMatrix)) // Normal matrix
            };
            float shininess = 32.0f;

            MTL::RenderCommandEncoder* renderCommandEncoder = cmd->renderCommandEncoder(renderPassDescriptor);
            {
                renderCommandEncoder->setRenderPipelineState(renderPipeline);
                renderCommandEncoder->setDepthStencilState(depthState);
                renderCommandEncoder->setFragmentSamplerState(planeSamplerState, 0);

                renderCommandEncoder->setVertexBuffer(cubeBuffer, 0, 0);
                renderCommandEncoder->setFragmentTexture(texturePool.getTexture(0), 0);

                matrices[0] = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, -1.0f));
                renderCommandEncoder->setVertexBytes(matrices, sizeof(matrices), 1);
                renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), 36);

                matrices[0] = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
                renderCommandEncoder->setVertexBytes(matrices, sizeof(matrices), 1);
                renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), 36);

                renderCommandEncoder->setVertexBuffer(planeBuffer, 0, 0);
                renderCommandEncoder->setFragmentTexture(texturePool.getTexture(1), 0);

                matrices[0] = glm::mat4(1.0f);
                renderCommandEncoder->setVertexBytes(matrices, sizeof(matrices), 1);
                renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), 6);

                renderCommandEncoder->setVertexBuffer(transparentBuffer, 0, 0);
                renderCommandEncoder->setFragmentTexture(texturePool.getTexture(2), 0);
                renderCommandEncoder->setFragmentSamplerState(vegetationSamplerState, 0);
                std::sort(vegetation.begin(), vegetation.end(),
                          [](const glm::vec3 &a, const glm::vec3 &b) {
                              return glm::length(camera.Position - a) > glm::length(camera.Position - b);
                          });
                for (auto i : vegetation) {
                    matrices[0] = glm::mat4(1.0f);
                    matrices[0] = glm::translate(matrices[0], i);
                    renderCommandEncoder->setVertexBytes(matrices, sizeof(matrices), 1);
                    renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), 6);
                }

//                model.Draw(renderCommandEncoder, objectPipeline);
            }
            renderCommandEncoder->endEncoding();

            cmd->presentDrawable(surface);
            cmd->commit();
            cmd->waitUntilCompleted();

            depthStateDesc->release();
            depthState->release();
            renderCommandEncoder->release();
            renderPassDescriptor->release();
            cmd->release();
            surface->release();

        }
        //pool->release();
    }

    depthTexture->release();
    planeSamplerState->release();
    vegetationSamplerState->release();

    renderPipeline->release();

    planeBuffer->release();
    cubeBuffer->release();
    transparentBuffer->release();

    queue->release();
    device->release();
    nswindow->release();

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    screenWidth = width;
    screenHeight = height;
}

void processInput(GLFWwindow *window) {
    deltaTime /= 2;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    // 如果没按下鼠标左键，直接返回，不处理视角
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS) {
        firstMouse = true; // 松开后下次点击时重新校正起始位置
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = (xpos - lastX) * 2;
    float yoffset = (lastY - ypos) * 2; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}
// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
