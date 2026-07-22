#include <webgpu/webgpu.h>
#include <marrow/marrow.h>

#ifndef __EMSCRIPTEN__
    #include <GLFW/glfw3.h>
    #if defined(_WIN32)
        #define GLFW_EXPOSE_NATIVE_WIN32
    #elif defined(__linux__)
        #define GLFW_EXPOSE_NATIVE_X11
        #define GLFW_EXPOSE_NATIVE_WAYLAND
    #endif
    #include <GLFW/glfw3native.h>
#endif

#define WEBGPU_STR_EXACT(str) (WGPUStringView) { .data = str, .length = sizeof(str) - 1 }
#define WEBGPU_STR(str) (WGPUStringView) { .data = str, .length = WGPU_STRLEN }

STRUCT(RequestAdapterUserData) {
    WGPUAdapter adapter;
    int request_done;
};

STRUCT(RequestDeviceUserData) {
    WGPUDevice device;
    int request_done;
};

static void request_adapter_callback(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userdata1, void* userdata2)
{
    if (status != WGPURequestAdapterStatus_Success)
        mrw_error("Failed to get WebGPU adapter");

    RequestAdapterUserData* callback_user_data = userdata1;
    callback_user_data->adapter = adapter;
    callback_user_data->request_done = 1;
}

static WGPUAdapter get_adapter(WGPUInstance instance, WGPURequestAdapterOptions request_adapter_options)
{
    RequestAdapterUserData request_adapter_user_data = { 0 };
    wgpuInstanceRequestAdapter(instance, &request_adapter_options, (WGPURequestAdapterCallbackInfo) {
        .mode = WGPUCallbackMode_AllowSpontaneous ,
        .callback = request_adapter_callback,
        .userdata1 = &request_adapter_user_data
    });
    #ifdef __EMSCRIPTEN__
        while(!request_adapter_user_data.request_done) emscripten_sleep(100);
    #endif
    return request_adapter_user_data.adapter;
}

static void request_device_callback(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void* userdata1, void* userdata2)
{
    if (status != WGPURequestDeviceStatus_Success)
    {
        mrw_debug("Failed to get WebGPU device");
    }

    RequestDeviceUserData* callback_user_data = userdata1;
    callback_user_data->device = device;
    callback_user_data->request_done = 1;
}

static void device_uncaptured_error_callback(WGPUDevice const* device, WGPUErrorType type, WGPUStringView message, void* userdata1, void* userdata2)
{
    if (message.length == WGPU_STRLEN)
    {
        mrw_error("Uncaptured device error ({}): {}", (u32)type, message.data);
    }
    else
    {
        char data[message.length + 1];
        buf_copy(data, message.data, message.length + 1);
        data[message.length] = 0;
        mrw_error("Uncaptured device error ({}): {}", (u32)type, data);
    }
}

static WGPUDevice get_device(WGPUAdapter adapter)
{
    WGPUDeviceDescriptor device_descriptor = {
        .label = WEBGPU_STR("Device :D"),
        .defaultQueue.label = WEBGPU_STR("Default queue"),
        .uncapturedErrorCallbackInfo = (WGPUUncapturedErrorCallbackInfo) {
            .callback = device_uncaptured_error_callback,
        }
    };
    RequestDeviceUserData request_device_user_data = { 0 };
    wgpuAdapterRequestDevice(adapter, &device_descriptor,
        (WGPURequestDeviceCallbackInfo){
            .mode = WGPUCallbackMode_AllowSpontaneous,
            .callback = request_device_callback,
            .userdata1 = &request_device_user_data
        }
    );
    #ifdef __EMSCRIPTEN__
        while(!request_device_user_data.request_done) emscripten_sleep(100);
    #endif
    return request_device_user_data.device;
}

static WGPUSurface get_surface(WGPUInstance instance
#ifndef __EMSCRIPTEN__
    , GLFWwindow* window
#endif
) {
    i32 platform = glfwGetPlatform();
    mrw_debug_val(platform);
    return wgpuInstanceCreateSurface(instance, &(WGPUSurfaceDescriptor) {
        .label = (WGPUStringView){ NULL, WGPU_STRLEN },
        .nextInChain = (WGPUChainedStruct*)
    #if defined(__EMSCRIPTEN__)
        &(WGPUEmscriptenSurfaceSourceCanvasHTMLSelector) {
            .chain.sType = WGPUSType_EmscriptenSurfaceSourceCanvasHTMLSelector,
            .selector = (WGPUStringView){ "canvas", WGPU_STRLEN },
        },
    #elif defined(__linux__)
        (platform == GLFW_PLATFORM_X11 ?
        (void*)&(WGPUSurfaceSourceXlibWindow) {
            .chain.sType =  WGPUSType_SurfaceSourceXlibWindow,
            .display = glfwGetX11Display(),
            .window = glfwGetX11Window(window),
        }  :
        (void*)&(WGPUSurfaceSourceWaylandSurface) {
            .chain.sType = WGPUSType_SurfaceSourceWaylandSurface,
            .display = glfwGetWaylandDisplay(),
            .surface = glfwGetWaylandWindow(window),
        }),
    #else
        &(WGPUSurfaceSourceWindowsHWND) {
            .chain.sType = WGPUSType_SurfaceSourceWindowsHWND,
            .hinstance = GetModuleHandle(NULL),
            .hwnd = glfwGetWin32Window(window)
        },
    #endif
    });
}

typedef struct WGPUDynamicBuffer
{
    WGPUBuffer data;
    WGPUBufferUsage usage;
    u32 count;
    u32 element_size;
} WGPUDynamicBuffer;

void wgpuDeviceDynamicBufferEnsure(WGPUDevice device, WGPUDynamicBuffer* buffer, u32 count)
{
    if (buffer->count >= count) return;
    buffer->count = count;
    if (buffer->data) wgpuBufferRelease(buffer->data);
    buffer->data =  wgpuDeviceCreateBuffer(device, &(WGPUBufferDescriptor) {
        .size = buffer->count * buffer->element_size,
        .usage = buffer->usage
    });
}

WGPUDynamicBuffer wgpuDeviceCreateDynamicBuffer(WGPUDevice device, u32 count, u32 element_size, WGPUBufferUsage usage)
{
    WGPUDynamicBuffer buffer = { .element_size = element_size, .usage = usage };
    wgpuDeviceDynamicBufferEnsure(device, &buffer, count);
    return buffer;
}

WGPUBuffer wgpuDeviceCreateBufferWithData(WGPUDevice device, WGPUQueue queue, u8Slice data, WGPUBufferUsage usage) {
    WGPUBuffer ret = wgpuDeviceCreateBuffer(device, &(WGPUBufferDescriptor){
            .size = slice_size(data),
            .usage = usage
        }
    );
    wgpuQueueWriteBuffer(queue, ret, 0, data.start, slice_size(data));
    return ret;
}

void wgpuDynamicBufferRelease(WGPUDynamicBuffer* buffer)
{
    wgpuBufferRelease(buffer->data);
    *buffer = (WGPUDynamicBuffer) { 0 };
}

usize wgpuDynamicBufferGetCount(WGPUDynamicBuffer* buffer)
{
    return buffer->count;
}

WGPUShaderModule load_shader_module_from_file(WGPUDevice device, const char* path)
{
    FILE *fp = fopen(path, "rb"); fseek(fp, 0, SEEK_END);
    u64 len = ftell(fp); fseek(fp, 0, SEEK_SET);
    char shader_code[len + 1]; fread(shader_code, 1, len, fp);
    shader_code[len] = 0; fclose(fp);
    return wgpuDeviceCreateShaderModule(device, &(WGPUShaderModuleDescriptor) {
        .label = WEBGPU_STR("planet shader descriptor"),
        .nextInChain = (WGPUChainedStruct*)&(WGPUShaderSourceWGSL) {
            .chain.sType = WGPUSType_ShaderSourceWGSL,
            .code = WEBGPU_STR(shader_code)
        }
    });
}

#define wgpu_stencil_keep_always (WGPUStencilFaceState){\
    .compare = WGPUCompareFunction_Always,\
    .failOp = WGPUStencilOperation_Keep,\
    .depthFailOp = WGPUStencilOperation_Keep,\
    .passOp = WGPUStencilOperation_Keep\
}

#define wgpu_color_blend_state (WGPUBlendComponent){\
    .srcFactor = WGPUBlendFactor_SrcAlpha,\
    .dstFactor = WGPUBlendFactor_OneMinusSrcAlpha,\
    .operation = WGPUBlendOperation_Add\
}

#define wgpu_alpha_blend_state (WGPUBlendComponent){\
    .srcFactor = WGPUBlendFactor_Zero,\
    .dstFactor = WGPUBlendFactor_One,\
    .operation = WGPUBlendOperation_Add\
}

#define wgpu_normal_blend_state (WGPUBlendState){\
    .color = wgpu_color_blend_state,\
    .alpha = wgpu_alpha_blend_state\
}
