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

int main()
{
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

    MTL::Buffer *cubeBuffer = device->newBuffer(cubeVertices, sizeof(cubeVertices), MTL::ResourceStorageModeShared);
    MTL::Buffer *planeBuffer = device->newBuffer(planeVertices, sizeof(planeVertices), MTL::ResourceStorageModeShared);

    TexturePool texturePool;

    texturePool.loadTexture("../../assets/marble.jpg", device, true);
    texturePool.loadTexture("../../assets/metal.png", device, true);

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
    Shader frameShader("shaders/shader.metal", "vertex_main", "fragment_frame");
    frameShader.Compile(device);

    MTL::RenderPipelineState *renderPipeline = shader.createPipeline(device, vertexDescriptor);
    MTL::RenderPipelineState *framePipeline = frameShader.createPipeline(device, vertexDescriptor);
    vertexDescriptor->release();

    MTL::TextureDescriptor* depthDesc = MTL::TextureDescriptor::texture2DDescriptor(
            MTL::PixelFormat::PixelFormatDepth32Float_Stencil8,
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
            renderPassDescriptor->setDepthAttachment(depthAttachment);

            MTL::DepthStencilDescriptor *objectDepthStateDesc = MTL::DepthStencilDescriptor::alloc()->init();
            objectDepthStateDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
            objectDepthStateDesc->setDepthWriteEnabled(true);
            MTL::StencilDescriptor *objectStencilDesc = MTL::StencilDescriptor::alloc()->init();
            objectStencilDesc->setStencilCompareFunction(MTL::CompareFunctionAlways);
            objectStencilDesc->setDepthStencilPassOperation(MTL::StencilOperationReplace);
            objectStencilDesc->setWriteMask(0xFF);
            objectDepthStateDesc->setFrontFaceStencil(objectStencilDesc);
            objectDepthStateDesc->setBackFaceStencil(objectStencilDesc);
            MTL::DepthStencilState *objectDepthStencilState = device->newDepthStencilState(objectDepthStateDesc);

            objectStencilDesc->release();
            objectDepthStateDesc->release();

            MTL::DepthStencilDescriptor *frameDepthStateDesc = MTL::DepthStencilDescriptor::alloc()->init();
            frameDepthStateDesc->setDepthCompareFunction(MTL::CompareFunctionAlways);
            frameDepthStateDesc->setDepthWriteEnabled(false);
            MTL::StencilDescriptor *frameStencilDesc = MTL::StencilDescriptor::alloc()->init();
            frameStencilDesc->setStencilCompareFunction(MTL::CompareFunctionNotEqual);
            frameStencilDesc->setStencilFailureOperation(MTL::StencilOperationKeep);
            frameStencilDesc->setDepthFailureOperation(MTL::StencilOperationKeep);
            frameStencilDesc->setDepthStencilPassOperation(MTL::StencilOperationKeep);
            frameStencilDesc->setReadMask(0xFF);
            frameDepthStateDesc->setFrontFaceStencil(frameStencilDesc);
            frameDepthStateDesc->setBackFaceStencil(frameStencilDesc);
            MTL::DepthStencilState *frameDepthStencilState = device->newDepthStencilState(frameDepthStateDesc);

            frameStencilDesc->release();
            frameDepthStateDesc->release();


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
                renderCommandEncoder->setFragmentSamplerState(planeSamplerState, 0);

                renderCommandEncoder->setRenderPipelineState(renderPipeline);
                renderCommandEncoder->setDepthStencilState(objectDepthStencilState);

                renderCommandEncoder->setStencilReferenceValue(0x00);
                renderCommandEncoder->setVertexBuffer(planeBuffer, 0, 0);
                renderCommandEncoder->setFragmentTexture(texturePool.getTexture(1), 0);
                matrices[0] = glm::mat4(1.0f);
                renderCommandEncoder->setVertexBytes(matrices, sizeof(matrices), 1);
                renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), 6);

                renderCommandEncoder->setStencilReferenceValue(0xFF);
                renderCommandEncoder->setVertexBuffer(cubeBuffer, 0, 0);
                renderCommandEncoder->setFragmentTexture(texturePool.getTexture(0), 0);
                matrices[0] = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, -1.0f));
                renderCommandEncoder->setVertexBytes(matrices, sizeof(matrices), 1);
                renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), 36);
                matrices[0] = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
                renderCommandEncoder->setVertexBytes(matrices, sizeof(matrices), 1);
                renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), 36);

                renderCommandEncoder->setDepthStencilState(frameDepthStencilState);
                renderCommandEncoder->setRenderPipelineState(framePipeline);
                float scale = 1.1f;
                matrices[0] = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, -1.0f)), glm::vec3(scale));
                renderCommandEncoder->setVertexBytes(matrices, sizeof(matrices), 1);
                renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), 36);
                matrices[0] = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f)), glm::vec3(scale));
                renderCommandEncoder->setVertexBytes(matrices, sizeof(matrices), 1);
                renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), 36);



//                model.Draw(renderCommandEncoder, objectPipeline);
            }
            renderCommandEncoder->endEncoding();

            cmd->presentDrawable(surface);
            cmd->commit();
            cmd->waitUntilCompleted();

            objectDepthStencilState->release();
            frameDepthStencilState->release();
            renderCommandEncoder->release();
            renderPassDescriptor->release();
            cmd->release();
            surface->release();

        }
        //pool->release();
    }

    depthTexture->release();
    planeSamplerState->release();

    renderPipeline->release();
    framePipeline->release();
    planeBuffer->release();
    cubeBuffer->release();

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
