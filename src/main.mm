// src/main.mm
#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

@interface Renderer : NSObject<MTKViewDelegate>
@property(nonatomic, strong) id<MTLDevice> device;
@property(nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property(nonatomic, strong) id<MTLRenderPipelineState> pipelineState;
@end

@implementation Renderer

- (instancetype)initWithDevice:(id<MTLDevice>)device view:(MTKView*)view {
    if ((self = [super init])) {
        _device = device;
        _commandQueue = [_device newCommandQueue];

        // 加载 metallib
        NSString* execDir = [[NSBundle mainBundle] executablePath].stringByDeletingLastPathComponent;
        NSString* libPath = [execDir stringByAppendingPathComponent:@"default.metallib"];
        NSError* error = nil;
        NSURL* libURL = [NSURL fileURLWithPath:libPath];
        id<MTLLibrary> lib = [_device newLibraryWithURL:libURL error:&error];
        if (!lib) { NSLog(@"Failed to load metallib: %@", error); return nil; }

        id<MTLFunction> vs = [lib newFunctionWithName:@"vs_main"];
        id<MTLFunction> ps = [lib newFunctionWithName:@"ps_main"];

        MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
        desc.vertexFunction = vs;
        desc.fragmentFunction = ps;
        desc.colorAttachments[0].pixelFormat = view.colorPixelFormat;

        _pipelineState = [_device newRenderPipelineStateWithDescriptor:desc error:&error];
        if (!_pipelineState) { NSLog(@"Failed to create pipeline: %@", error); return nil; }
    }
    return self;
}

- (void)drawInMTKView:(MTKView *)view {
    MTLRenderPassDescriptor* pass = view.currentRenderPassDescriptor;
    if (!pass) return;
    id<CAMetalDrawable> drawable = view.currentDrawable;

    id<MTLCommandBuffer> cmd = [self.commandQueue commandBuffer];
    id<MTLRenderCommandEncoder> enc = [cmd renderCommandEncoderWithDescriptor:pass];
    [enc setRenderPipelineState:self.pipelineState];

    // 绘制 3 个顶点，不需要 VBO，直接用 vertex_id
    [enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];

    [enc endEncoding];
    [cmd presentDrawable:drawable];
    [cmd commit];
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size { }

@end

@interface WindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation WindowDelegate
- (void)windowWillClose:(NSNotification *)notification {
    [NSApp terminate:nil];
}
@end


int main(int argc, const char * argv[]) {
    @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        NSRect frame = NSMakeRect(100, 100, 800, 600);
        NSWindow* window = [[NSWindow alloc]
            initWithContentRect:frame
                      styleMask:(NSWindowStyleMaskTitled |
                                 NSWindowStyleMaskClosable |
                                 NSWindowStyleMaskResizable)
                        backing:NSBackingStoreBuffered
                          defer:NO];
        [window setTitle:@"Metal Triangle"];
        [window makeKeyAndOrderFront:nil];

        WindowDelegate* winDelegate = [[WindowDelegate alloc] init];
        [window setDelegate:winDelegate];
        
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        MTKView* mtkView = [[MTKView alloc] initWithFrame:frame device:device];
        mtkView.clearColor = MTLClearColorMake(0, 0, 0, 1);
        [window setContentView:mtkView];

        Renderer* renderer = [[Renderer alloc] initWithDevice:device view:mtkView];
        mtkView.delegate = renderer;

        [NSApp activateIgnoringOtherApps:YES];
        [NSApp run];
    }
    return 0;
}
