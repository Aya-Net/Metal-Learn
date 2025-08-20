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
    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello osx!", nullptr, nullptr);
    NS::Window* nswindow = get_ns_window(window, swapChain);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    MTL::ClearColor clearColor{0.0f, 0.0f, 0.0f, 0.0f};

    Shader objectShader("shaders/shader.metal", "vertex_main", "object_fragment");
    Shader lightShader("shaders/shader.metal", "vertex_main", "light_fragment");
    objectShader.Compile(device);
    lightShader.Compile(device);
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();

    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vertexDescriptor->attributes()->object(0)->setOffset(0);
    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);

    vertexDescriptor->layouts()->object(0)->setStride(5 * sizeof(float));
    vertexDescriptor->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);

    MTL::RenderPipelineState* objectPipeline = objectShader.createPipeline(device, vertexDescriptor);
    MTL::RenderPipelineState* lightPipeline = lightShader.createPipeline(device, vertexDescriptor);

    float vertices[] = {
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

    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

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

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

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

            glm::mat4 matrices[] = {
                    glm::mat4(1.0f),
                    camera.GetViewMatrix(),
                    glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f)
            };

            glm::vec4 colors[] = {
                    glm::vec4(1.0f, 0.5f, 0.31f, 1.0f),
                    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
            };

            MTL::RenderCommandEncoder* renderCommandEncoder = cmd->renderCommandEncoder(renderPassDescriptor);
            {
                renderCommandEncoder->setDepthStencilState(depthState);
                renderCommandEncoder->setRenderPipelineState(objectPipeline);
                renderCommandEncoder->setVertexBuffer(buffer, 0, 0);

                renderCommandEncoder->setVertexBytes(matrices, sizeof(matrices), 1);
                renderCommandEncoder->setFragmentBytes(colors, sizeof(colors), 0);

                renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(36));

                matrices[0] = glm::translate(matrices[0], lightPos);
                matrices[0] = glm::scale(matrices[0], glm::vec3(0.2f));
                renderCommandEncoder->setRenderPipelineState(lightPipeline);
                renderCommandEncoder->setVertexBytes(matrices, sizeof(matrices), 1);
                renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(36));
                // renderCommandEncoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, 6, MTL::IndexTypeUInt16, indexBuffer, 0);
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

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    screenWidth = width;
    screenHeight = height;
}

void processInput(GLFWwindow *window)
{
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

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
