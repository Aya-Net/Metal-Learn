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

#include <string>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
};

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
    stbi_set_flip_vertically_on_load(true);

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello Metal!", nullptr, nullptr);
    NS::Window* nswindow = get_ns_window(window, swapChain);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    MTL::ClearColor clearColor{0.1f, 0.1f, 0.1f, 1.0f};

    Shader objectShader("shaders/shader.metal", "vertex_main", "object_fragment");
    Shader lightShader("shaders/shader.metal", "vertex_main", "light_fragment");
    Shader posLightShader("shaders/shader.metal", "vertex_main", "object_fragment_pos");
    Shader spotLightShader("shaders/shader.metal", "vertex_main", "object_fragment_spot");
    objectShader.Compile(device);
    lightShader.Compile(device);
    posLightShader.Compile(device);
    spotLightShader.Compile(device);

    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();

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

    MTL::RenderPipelineState* objectPipeline = objectShader.createPipeline(device, vertexDescriptor);
    MTL::RenderPipelineState* lightPipeline = lightShader.createPipeline(device, vertexDescriptor);
    MTL::RenderPipelineState* posLightPipeline = posLightShader.createPipeline(device, vertexDescriptor);
    MTL::RenderPipelineState* spotLightPipeline = spotLightShader.createPipeline(device, vertexDescriptor);

    float vertices[] = {
            // positions          // normals           // texture coords
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    glm::vec3 cubePositions[] = {
            glm::vec3( 0.0f,  0.0f,  0.0f),
            glm::vec3( 2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3( 2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3( 1.3f, -2.0f, -2.5f),
            glm::vec3( 1.5f,  2.0f, -2.5f),
            glm::vec3( 1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    glm::vec3 pointLightPositions[] = {
            glm::vec3( 0.7f,  0.2f,  2.0f),
            glm::vec3( 2.3f, -3.3f, -4.0f),
            glm::vec3(-4.0f,  2.0f, -12.0f),
            glm::vec3( 0.0f,  0.0f, -3.0f)
    };

    MTL::Buffer* buffer = device->newBuffer(vertices, sizeof(vertices), MTL::ResourceStorageModeShared);

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

    Material material(device, "../../assets/container2.png", "../../assets/container2_specular.png", 32);

    DirLight dirLight{glm::vec3(-0.2f, -1.0f, -0.3f),
                glm::vec3(0.05f),
                glm::vec3(0.4f),
                glm::vec3(0.5f)};

//    PosLight posLight{glm::vec3(1.2f, 1.0f, 2.0f),
//                   glm::vec3(0.2f),
//                   glm::vec3(0.5f),
//                   glm::vec3(1.0f),
//                   1.0f,
//                   0.09f,
//                   0.032f};

    PosLight posLight[4] = {
        {pointLightPositions[0], glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f), 1.0f, 0.09f, 0.032f},
        {pointLightPositions[1], glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f), 1.0f, 0.09f, 0.032f},
        {pointLightPositions[2], glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f), 1.0f, 0.09f, 0.032f},
        {pointLightPositions[3], glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f), 1.0f, 0.09f, 0.032f}
    };

    MTL::Buffer *posLightBuffer = device->newBuffer(&posLight, sizeof(posLight), MTL::ResourceStorageModeShared);
    const int posLightCount = 4;

    SpotLight spotLight{camera.Position,
                        camera.Front,
                        glm::vec3(0.0f),
                        glm::vec3(1.0f),
                        glm::vec3(1.0f),
                        glm::cos(glm::radians(12.5f)),
                        glm::cos(glm::radians(15.0f)),
                        1.0f,
                        0.09f,
                        0.032f};


    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        spotLight.position = camera.Position;
        spotLight.direction = camera.Front;


        processInput(window);

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
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            glm::mat4 matrices[] = {
                    modelMatrix,
                    camera.GetViewMatrix(),
                    glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f),
                    glm::transpose(glm::inverse(modelMatrix)) // Normal matrix
            };


            MTL::RenderCommandEncoder* renderCommandEncoder = cmd->renderCommandEncoder(renderPassDescriptor);
            {
                renderCommandEncoder->setDepthStencilState(depthState);
//                renderCommandEncoder->setRenderPipelineState(posLightPipeline);

                renderCommandEncoder->setVertexBuffer(buffer, 0, 0);

                renderCommandEncoder->setVertexBytes(matrices, sizeof(matrices), 1);

                renderCommandEncoder->setFragmentTexture(material.diffuseTexture, 0);
                renderCommandEncoder->setFragmentTexture(material.specularTexture, 1);
                renderCommandEncoder->setFragmentBytes(&material.shininess, sizeof(float), 0);
                renderCommandEncoder->setFragmentBytes(&camera.Position, sizeof(camera.Position), 1);
                renderCommandEncoder->setFragmentBytes(&dirLight, sizeof(dirLight), 2);
                renderCommandEncoder->setFragmentBytes(&posLight, sizeof(posLight), 3);
                renderCommandEncoder->setFragmentBytes(&posLightCount, sizeof(posLightCount), 4);
                renderCommandEncoder->setFragmentBytes(&spotLight, sizeof(spotLight), 5);

//                renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(36));
                renderCommandEncoder->setRenderPipelineState(objectPipeline); // draw cubes
                {
                    for (int i = 0; i < 10; ++i) {
                        modelMatrix = glm::translate(glm::mat4(1.0f), cubePositions[i]);
                        float angle = 20.0f * i;
                        modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                        matrices[0] = modelMatrix;
                        matrices[3] = glm::transpose(glm::inverse(modelMatrix)); // Normal matrix
                        renderCommandEncoder->setVertexBytes(matrices, sizeof(matrices), 1);
                        renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0),
                                                             NS::UInteger(36));
                    }
                }
                renderCommandEncoder->setRenderPipelineState(lightPipeline); // draw light cubes
                {
                    for (int i = 0; i < posLightCount; ++i) {
                        matrices[0] = glm::translate(glm::mat4(1.0f), pointLightPositions[i]);
                        matrices[0] = glm::scale(matrices[0], glm::vec3(0.2f));
                        renderCommandEncoder->setVertexBytes(matrices, sizeof(matrices), 1);
                        renderCommandEncoder->setFragmentBytes(&i, sizeof(i), 4);
                        renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(36));
                    }
                }


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
    buffer->release();
    posLightBuffer->release();

    objectPipeline->release();
    lightPipeline->release();
    posLightPipeline->release();
    spotLightPipeline->release();

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    screenWidth = width;
    screenHeight = height;
}

void processInput(GLFWwindow *window) {
    deltaTime *= 2;
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
