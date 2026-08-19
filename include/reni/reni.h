#ifndef RENI_H
#define RENI_H

#include <GLFW/glfw3.h>

#include "../marrow/marrow.h"
#include "../marrow/alloc.h"
#include "../marrow/genarr.h"

#ifdef MARROW_IMPLEMENTATION
#define RENI_IMPLEMENTATION
#endif

#define RENI_SHADER_MAX_BINDINGS 8
#define RENI_SHADER_BUFFER_MAX_ATTRIBUTES 16
#define RENI_SHADER_MAX_BUFFERS 8
#define RENI_SHADER_MAX_TARGETS 8
#define RENI_BINDING_MAX_ENTRIES 8

#ifdef __EMSCRIPTEN__
#define RENI_BACKEND_WINDOW cstr
#else
#define RENI_BACKEND_WINDOW GLFWwindow*
#endif

typedef enum ReniBufferUsage {
    ReniBufferUsage_Undefined = 0,
    ReniBufferUsage_MapRead = BIT(0),
    ReniBufferUsage_MapWrite = BIT(1),
    ReniBufferUsage_CopySrc = BIT(2),
    ReniBufferUsage_CopyDst = BIT(3),
    ReniBufferUsage_Index = BIT(4),
    ReniBufferUsage_Vertex = BIT(5),
    ReniBufferUsage_Uniform = BIT(6),
    ReniBufferUsage_Storage = BIT(7),
    ReniBufferUsage_Indirect = BIT(8),
} ReniBufferUsage;

typedef enum ReniTextureFormat {
    ReniTextureFormat_Undefined = 0,
    ReniTextureFormat_R8Unorm,
    ReniTextureFormat_R8Snorm,
    ReniTextureFormat_R8Uint,
    ReniTextureFormat_R8Sint,
    ReniTextureFormat_RG8Unorm,
    ReniTextureFormat_RG8Snorm,
    ReniTextureFormat_RGBA8Unorm,
    ReniTextureFormat_RGBA8UnormSrgb,
    ReniTextureFormat_RGBA8Snorm,
    ReniTextureFormat_RGBA8Uint,
    ReniTextureFormat_RGBA8Sint,
    ReniTextureFormat_BGRA8Unorm,
    ReniTextureFormat_BGRA8UnormSrgb,
    ReniTextureFormat_R16Uint,
    ReniTextureFormat_R16Sint,
    ReniTextureFormat_R16Float,
    ReniTextureFormat_RG16Uint,
    ReniTextureFormat_RG16Sint,
    ReniTextureFormat_RG16Float,
    ReniTextureFormat_RGBA16Uint,
    ReniTextureFormat_RGBA16Sint,
    ReniTextureFormat_RGBA16Float,
    ReniTextureFormat_R32Uint,
    ReniTextureFormat_R32Sint,
    ReniTextureFormat_R32Float,
    ReniTextureFormat_RG32Uint,
    ReniTextureFormat_RG32Sint,
    ReniTextureFormat_RG32Float,
    ReniTextureFormat_RGBA32Uint,
    ReniTextureFormat_RGBA32Sint,
    ReniTextureFormat_RGBA32Float,
    ReniTextureFormat_RGB10A2Unorm,
    ReniTextureFormat_RG11B10Ufloat,

    ReniTextureFormat_Depth16Unorm,
    ReniTextureFormat_Depth24Plus,
} ReniTextureFormat;

typedef enum ReniTextureUsage {
    ReniTextureUsage_Undefined = 0,
    ReniTextureUsage_CopySrc = BIT(0),
    ReniTextureUsage_CopyDst = BIT(1),
    ReniTextureUsage_TextureBinding = BIT(2),
    ReniTextureUsage_StorageBinding = BIT(3),
    ReniTextureUsage_RenderAttachment = BIT(4),
    ReniTextureUsage_TransientAttachment = BIT(5),
    ReniTextureUsage_StorageAttachment = BIT(6),
} ReniTextureUsage;

typedef enum ReniPresentMode {
    ReniPresentMode_Undefined = 0,
    ReniPresentMode_Fifo,
    ReniPresentMode_FifoRelaxed,
    ReniPresentMode_Immediate,
    ReniPresentMode_Mailbox,
} ReniPresentMode;

typedef enum ReniSamplerAddressMode {
    ReniSamplerAddressMode_Undefined = 0,
    ReniSamplerAddressMode_ClampToEdge,
    ReniSamplerAddressMode_Repeat,
    ReniSamplerAddressMode_MirrorRepeat,
} ReniSamplerAddressMode;

typedef enum ReniSamplerFilterMode {
    ReniSamplerFilterMode_Undefined = 0,
    ReniSamplerFilterMode_Nearest,
    ReniSamplerFilterMode_Linear,
} ReniSamplerFilterMode;

typedef enum ReniCompareFunction {
    ReniCompareFunction_Undefined = 0,
    ReniCompareFunction_Less,
    ReniCompareFunction_Equal,
    ReniCompareFunction_LessEqual,
    ReniCompareFunction_Greater,
    ReniCompareFunction_NotEqual,
    ReniCompareFunction_GreaterEqual,
    ReniCompareFunction_Always,
} ReniCompareFunction;

typedef enum ReniShaderStage {
    ReniShaderStage_Undefined = 0,
    ReniShaderStage_Vertex = BIT(0),
    ReniShaderStage_Fragment = BIT(1),
} ReniShaderStage;

typedef enum ReniBufferBindingType {
    ReniBufferBindingType_Undefined = 0,
    ReniBufferBindingType_Uniform,
    ReniBufferBindingType_Storage,
    ReniBufferBindingType_ReadOnlyStorage,
} ReniBufferBindingType;

typedef enum ReniSamplerType {
    ReniSamplerType_Undefined = 0,
    ReniSamplerType_NonFiltering,
    ReniSamplerType_Filtering,
    ReniSamplerType_Comparison,
} ReniSamplerType;

typedef enum ReniSampleType {
    ReniSampleType_Undefined = 0,
    ReniSampleType_Float,
    ReniSampleType_UnfilterableFloat,
    ReniSampleType_Depth,
    ReniSampleType_Sint,
    ReniSampleType_Uint,
} ReniSampleType;

typedef enum ReniVertexFormat {
    ReniVertexFormat_Undefined = 0,
    ReniVertexFormat_Uint8, ReniVertexFormat_Uint8x2, ReniVertexFormat_Uint8x4,
    ReniVertexFormat_Sint8, ReniVertexFormat_Sint8x2, ReniVertexFormat_Sint8x4,
    ReniVertexFormat_Unorm8, ReniVertexFormat_Unorm8x2, ReniVertexFormat_Unorm8x4,
    ReniVertexFormat_Snorm8, ReniVertexFormat_Snorm8x2, ReniVertexFormat_Snorm8x4,
    ReniVertexFormat_Uint16, ReniVertexFormat_Uint16x2, ReniVertexFormat_Uint16x4,
    ReniVertexFormat_Sint16, ReniVertexFormat_Sint16x2, ReniVertexFormat_Sint16x4,
    ReniVertexFormat_Unorm16, ReniVertexFormat_Unorm16x2, ReniVertexFormat_Unorm16x4,
    ReniVertexFormat_Snorm16, ReniVertexFormat_Snorm16x2, ReniVertexFormat_Snorm16x4,
    ReniVertexFormat_Float16, ReniVertexFormat_Float16x2, ReniVertexFormat_Float16x4,
    ReniVertexFormat_Float32, ReniVertexFormat_Float32x2, ReniVertexFormat_Float32x3, ReniVertexFormat_Float32x4,
    ReniVertexFormat_Uint32, ReniVertexFormat_Uint32x2, ReniVertexFormat_Uint32x3, ReniVertexFormat_Uint32x4,
    ReniVertexFormat_Sint32, ReniVertexFormat_Sint32x2, ReniVertexFormat_Sint32x3, ReniVertexFormat_Sint32x4,
} ReniVertexFormat;

typedef enum ReniBlendFactor {
    ReniBlendFactor_Undefined = 0,
    ReniBlendFactor_Zero,
    ReniBlendFactor_One,
    ReniBlendFactor_SrcAlpha,
    ReniBlendFactor_DstAlpha,
    ReniBlendFactor_OneMinusSrcAlpha,
} ReniBlendFactor;

typedef enum ReniBlendOperation {
    ReniBlendOperation_Undefined = 0,
    ReniBlendOperation_Add,
    ReniBlendOperation_Subtract,
    ReniBlendOperation_ReverseSubtract,
    ReniBlendOperation_Min,
    ReniBlendOperation_Max,
} ReniBlendOperation;

typedef enum ReniSurfaceStatus {
    ReniSurfaceStatus_SuccessOptimal,
    ReniSurfaceStatus_SuccessSuboptimal,
    ReniSurfaceStatus_Timeout,
    ReniSurfaceStatus_Outdated,
    ReniSurfaceStatus_Lost,
    ReniSurfaceStatus_Error,
} ReniSurfaceStatus;

#define RENI_HANDLE(name) STRUCT(name) { GenarrHandle h; }
#define RENI_STRUCT(name) RENI_HANDLE(name); STRUCT(name ## Config)

RENI_STRUCT(ReniBuffer) {
    str name;
    ReniBufferUsage usage;
    u8Slice data;
};

RENI_STRUCT(ReniTexture) {
    str name;
    u32 width;
    u32 height;
    bool multisampled;
    ReniTextureFormat format;
    ReniTextureUsage usage;
};
RENI_STRUCT(ReniSampler) {
    str name;
    ReniSamplerAddressMode address; // u, v, w
    ReniSamplerFilterMode filter; // mag, min, mipmap
    ReniCompareFunction compare;
};

STRUCT(ReniBindingLayoutEntry)
{
    ReniShaderStage visibility;
    struct {
        ReniBufferBindingType type;
        bool dynamic_offset;
    } buffer;
    struct {
        ReniSampleType type;
        bool multisampled;
    } texture;
    struct {
        ReniSamplerType type;
    } sampler;
};

RENI_STRUCT(ReniBindingLayout)
{
    str name;
    ReniBindingLayoutEntry entries[RENI_BINDING_MAX_ENTRIES];
};

STRUCT(ReniBindingEntry)
{
    struct {
        ReniBuffer buffer;
        usize offset;
        usize size; // 0 means whole buffer
    } buffer;
    ReniTexture texture;
    ReniSampler sampler;

    u32 _gen;
};

RENI_STRUCT(ReniBinding) {
    str name;
    ReniBindingLayout layout;
    ReniBindingEntry entries[RENI_BINDING_MAX_ENTRIES]; // layout hold n_entries
};

STRUCT(ReniBlendComponent) {
    ReniBlendFactor src;
    ReniBlendFactor dst;
    ReniBlendOperation op;
};

STRUCT(ReniBlendState) {
    ReniBlendComponent color;
    ReniBlendComponent alpha;
};

STRUCT(ReniTarget) {
    ReniTextureFormat format;
    ReniBlendState blend_state;
};

STRUCT(ReniVertexAttribute)
{
    u32 location;
    usize offset;
    ReniVertexFormat format;
};

STRUCT(ReniVertexBufferLayout)
{
    bool instance;
    usize stride;
    ReniVertexAttribute attributes[RENI_SHADER_BUFFER_MAX_ATTRIBUTES];
};

RENI_STRUCT(ReniShader) {
    str name;
    struct {
        struct {
            cstr path;
            cstrSlice includes;
        } file;
        str string;
    } source;

    ReniBindingLayout layouts[RENI_SHADER_MAX_BINDINGS];

    struct {
        str entry;
        ReniVertexBufferLayout buffers[RENI_SHADER_MAX_BUFFERS];
    } vertex;

    struct {
        str entry;
        bool multisample; // true = 4
        ReniTextureFormat depth_format;
        bool dont_write_depth;
        ReniTarget targets[RENI_SHADER_MAX_TARGETS];
    } fragment;

    bool culling; // always CCW
    bool wireframe;
};

RENI_STRUCT(ReniSurface) {
    RENI_BACKEND_WINDOW window;
    ReniPresentMode mode;
};

STRUCT(ReniSurfaceState) {
    u32 width;
    u32 height;
};

STRUCT(ReniRenderpassTarget)
{
    ReniTexture texture;
    ReniTexture resolve_texture;
    bool clear;
    f32 clear_value[4];
};

RENI_STRUCT(ReniRenderpass) {
  ReniRenderpassTarget targets[RENI_SHADER_MAX_TARGETS];
  struct {
      ReniTexture target;
      bool clear;
      f32 clear_value;
  } depth;
};

typedef void (*ReniErrorCallback)(str message);
STRUCT(ReniConfig) {
    str name;

    ReniErrorCallback error_callback;

    Allocator* allocator;
    Allocator* frame_allocator;
};

STRUCT(ReniDrawConfig)
{
    ReniBuffer vertices;
    usize n_vertices; // only required if drawing without index buffer
    ReniBuffer indices;
    bool u32_indices; // default is u16
    ReniBuffer instances;
    usize n_instances;
};

STRUCT(ReniSurfaceAcquired) {
    ReniSurfaceStatus status;
    ReniTexture texture;
};

#undef RENI_HANDLE
#undef RENI_STRUCT

typedef struct Reni Reni;

Reni* reni_create_reni(ReniConfig config);
void reni_destroy_reni(Reni* reni);

ReniSurface reni_create_surface(Reni* reni, ReniSurfaceConfig config);
void reni_release_surface(Reni* reni, ReniSurface surface);

ReniTextureFormat reni_surface_get_format(Reni* reni, ReniSurface surface);
void reni_surface_update(Reni* reni, ReniSurface surface, ReniSurfaceState state);
ReniSurfaceAcquired reni_surface_acquire(Reni* reni, ReniSurface surface);

ReniBuffer reni_create_buffer(Reni* reni, ReniBufferConfig config);
void reni_release_buffer(Reni* reni, ReniBuffer buffer);
void reni_buffer_write(Reni* reni, ReniBuffer buffer, u8Slice data, usize offset);

ReniTexture reni_create_texture(Reni* reni, ReniTextureConfig config);
ReniTexture reni_recreate_texture(Reni* reni, ReniTexture texture, ReniTextureConfig config);
void reni_release_texture(Reni* reni, ReniTexture texture);
const ReniTextureConfig* reni_get_texture_config(Reni* reni, ReniTexture texture);
void reni_texture_resize(Reni* reni, ReniTexture texture, u32 w, u32 h);
ReniTextureFormat reni_texture_get_format(Reni* reni, ReniTexture texture);

ReniSampler reni_create_sampler(Reni* reni, ReniSamplerConfig config);
ReniSampler reni_recreate_sampler(Reni* reni, ReniSampler sampler, ReniSamplerConfig config);
void reni_release_sampler(Reni* reni, ReniSampler sampler);

ReniBindingLayout reni_create_binding_layout(Reni* reni, ReniBindingLayoutConfig config);
void reni_release_binding_layout(Reni* reni, ReniBindingLayout layout);

ReniBinding reni_create_binding(Reni* reni, ReniBindingConfig config);
ReniBinding reni_recreate_binding(Reni* reni, ReniBinding binding, ReniBindingConfig config);
void reni_release_binding(Reni* reni, ReniBinding binding);
const ReniBindingConfig* reni_get_binding_config(Reni* reni, ReniBinding binding);

ReniShader reni_create_shader(Reni* reni, ReniShaderConfig config);
ReniShader reni_recreate_shader(Reni*, ReniShader, ReniShaderConfig);
void reni_release_shader(Reni* reni, ReniShader shader);
const ReniShaderConfig* reni_get_shader_config(Reni* reni, ReniShader shader);

void reni_begin(Reni* reni);

ReniRenderpass reni_create_renderpass(Reni* reni, ReniRenderpassConfig config);
void reni_renderpass_set_shader(Reni* reni, ReniRenderpass pass, ReniShader shader);
void reni_renderpass_set_binding(Reni* reni, ReniRenderpass pass, u32 index, ReniBinding binding);
void reni_renderpass_draw(Reni* reni, ReniRenderpass pass, ReniDrawConfig config);
void reni_submit_renderpass(Reni* reni, ReniRenderpass renderpass);

void reni_end(Reni* reni);

#define RENI_BLEND_STATE_BLEND (ReniBlendComponent){ .src = ReniBlendFactor_SrcAlpha, .dst = ReniBlendFactor_OneMinusSrcAlpha, .op = ReniBlendOperation_Add }
#define RENI_BLEND_STATE_ADD (ReniBlendComponent){ .src = ReniBlendFactor_One, .dst = ReniBlendFactor_One, .op = ReniBlendOperation_Add }
#define RENI_BLEND_STATE_OVERWRITE (ReniBlendComponent){ .src = ReniBlendFactor_Zero, .dst = ReniBlendFactor_One, .op = ReniBlendOperation_Add }

#ifdef RENI_IMPLEMENTATION
#include "reni_wgpu.h"
#endif // RENI_IMPLEMENTATION

#undef RENI_BACKEND_WINDOW

#endif // RENI_H
