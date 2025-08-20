#import "glfw_adapter.h"
#include <iostream>
NS::Window* get_ns_window(GLFWwindow* glfwWindow, CA::MetalLayer* layer) {
    CALayer* obj_layer = (__bridge CALayer*) layer;
    NSWindow* obj_window = glfwGetCocoaWindow(glfwWindow);
    obj_layer.contentsScale = obj_window.backingScaleFactor;
    obj_window.contentView.layer = obj_layer;
    obj_window.contentView.wantsLayer = YES;
    return (__bridge NS::Window*)obj_window;
}