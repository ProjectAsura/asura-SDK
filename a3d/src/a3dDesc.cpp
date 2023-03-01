//----------------------------------------------------------------------------
// File : a3dDescSettings.cpp
// Desc : Desc Structure Settings.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

namespace {
constexpr float A3D_FLT_MAX = 3.402823466e+38F;        // max value
} 

namespace a3d {

///////////////////////////////////////////////////////////////////////////////
// SamplerDesc structure
///////////////////////////////////////////////////////////////////////////////
SamplerDesc SamplerDesc::PointClamp()
{
    SamplerDesc result = {};
    result.MinFilter        = FILTER_MODE_NEAREST;
    result.MagFilter        = FILTER_MODE_NEAREST;
    result.MipMapMode       = MIPMAP_MODE_NEAREST;
    result.AddressU         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.AddressV         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.AddressW         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.MipLodBias       = 0;
    result.AnisotropyEnable = false;
    result.MaxAnisotropy    = 0;
    result.CompareEnable    = false;
    result.CompareOp        = COMPARE_OP_ALWAYS;
    result.MinLod           = 0;
    result.MaxLod           = A3D_FLT_MAX;
    result.BorderColor      = BORDER_COLOR_OPAQUE_BLACK;
    return result;
}

SamplerDesc SamplerDesc::PointRepeat()
{
    SamplerDesc result = {};
    result.MinFilter        = FILTER_MODE_NEAREST;
    result.MagFilter        = FILTER_MODE_NEAREST;
    result.MipMapMode       = MIPMAP_MODE_NEAREST;
    result.AddressU         = TEXTURE_ADDRESS_MODE_REPEAT;
    result.AddressV         = TEXTURE_ADDRESS_MODE_REPEAT;
    result.AddressW         = TEXTURE_ADDRESS_MODE_REPEAT;
    result.MipLodBias       = 0;
    result.AnisotropyEnable = false;
    result.MaxAnisotropy    = 0;
    result.CompareEnable    = false;
    result.CompareOp        = COMPARE_OP_ALWAYS;
    result.MinLod           = 0;
    result.MaxLod           = A3D_FLT_MAX;
    result.BorderColor      = BORDER_COLOR_OPAQUE_BLACK;
    return result;
}

SamplerDesc SamplerDesc::PointMirror()
{
    SamplerDesc result = {};
    result.MinFilter        = FILTER_MODE_NEAREST;
    result.MagFilter        = FILTER_MODE_NEAREST;
    result.MipMapMode       = MIPMAP_MODE_NEAREST;
    result.AddressU         = TEXTURE_ADDRESS_MODE_MIRROR;
    result.AddressV         = TEXTURE_ADDRESS_MODE_MIRROR;
    result.AddressW         = TEXTURE_ADDRESS_MODE_MIRROR;
    result.MipLodBias       = 0;
    result.AnisotropyEnable = false;
    result.MaxAnisotropy    = 0;
    result.CompareEnable    = false;
    result.CompareOp        = COMPARE_OP_ALWAYS;
    result.MinLod           = 0;
    result.MaxLod           = A3D_FLT_MAX;
    result.BorderColor      = BORDER_COLOR_OPAQUE_BLACK;
    return result;
}

SamplerDesc SamplerDesc::LinearClamp()
{
    SamplerDesc result = {};
    result.MinFilter        = FILTER_MODE_LINEAR;
    result.MagFilter        = FILTER_MODE_LINEAR;
    result.MipMapMode       = MIPMAP_MODE_LINEAR;
    result.AddressU         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.AddressV         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.AddressW         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.MipLodBias       = 0;
    result.AnisotropyEnable = false;
    result.MaxAnisotropy    = 0;
    result.CompareEnable    = false;
    result.CompareOp        = COMPARE_OP_ALWAYS;
    result.MinLod           = 0;
    result.MaxLod           = A3D_FLT_MAX;
    result.BorderColor      = BORDER_COLOR_OPAQUE_BLACK;
    return result;
}

SamplerDesc SamplerDesc::LinearRepeat()
{
    SamplerDesc result = {};
    result.MinFilter        = FILTER_MODE_LINEAR;
    result.MagFilter        = FILTER_MODE_LINEAR;
    result.MipMapMode       = MIPMAP_MODE_LINEAR;
    result.AddressU         = TEXTURE_ADDRESS_MODE_REPEAT;
    result.AddressV         = TEXTURE_ADDRESS_MODE_REPEAT;
    result.AddressW         = TEXTURE_ADDRESS_MODE_REPEAT;
    result.MipLodBias       = 0;
    result.AnisotropyEnable = false;
    result.MaxAnisotropy    = 0;
    result.CompareEnable    = false;
    result.CompareOp        = COMPARE_OP_ALWAYS;
    result.MinLod           = 0;
    result.MaxLod           = A3D_FLT_MAX;
    result.BorderColor      = BORDER_COLOR_OPAQUE_BLACK;
    return result;
}

SamplerDesc SamplerDesc::LinearMirror()
{
    SamplerDesc result = {};
    result.MinFilter        = FILTER_MODE_LINEAR;
    result.MagFilter        = FILTER_MODE_LINEAR;
    result.MipMapMode       = MIPMAP_MODE_LINEAR;
    result.AddressU         = TEXTURE_ADDRESS_MODE_MIRROR;
    result.AddressV         = TEXTURE_ADDRESS_MODE_MIRROR;
    result.AddressW         = TEXTURE_ADDRESS_MODE_MIRROR;
    result.MipLodBias       = 0;
    result.AnisotropyEnable = false;
    result.MaxAnisotropy    = 0;
    result.CompareEnable    = false;
    result.CompareOp        = COMPARE_OP_ALWAYS;
    result.MinLod           = 0;
    result.MaxLod           = A3D_FLT_MAX;
    result.BorderColor      = BORDER_COLOR_OPAQUE_BLACK;
    return result;
}

SamplerDesc SamplerDesc::AnisotropicClamp(uint32_t maxAnisotropy)
{
    SamplerDesc result = {};
    result.MinFilter        = FILTER_MODE_LINEAR;
    result.MagFilter        = FILTER_MODE_LINEAR;
    result.MipMapMode       = MIPMAP_MODE_LINEAR;
    result.AddressU         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.AddressV         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.AddressW         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.MipLodBias       = 0;
    result.AnisotropyEnable = true;
    result.MaxAnisotropy    = maxAnisotropy;
    result.CompareEnable    = false;
    result.CompareOp        = COMPARE_OP_ALWAYS;
    result.MinLod           = 0;
    result.MaxLod           = A3D_FLT_MAX;
    result.BorderColor      = BORDER_COLOR_OPAQUE_BLACK;
    return result;
}

SamplerDesc SamplerDesc::AnisotropicRepeat(uint32_t maxAnisotropy)
{
    SamplerDesc result = {};
    result.MinFilter        = FILTER_MODE_LINEAR;
    result.MagFilter        = FILTER_MODE_LINEAR;
    result.MipMapMode       = MIPMAP_MODE_LINEAR;
    result.AddressU         = TEXTURE_ADDRESS_MODE_REPEAT;
    result.AddressV         = TEXTURE_ADDRESS_MODE_REPEAT;
    result.AddressW         = TEXTURE_ADDRESS_MODE_REPEAT;
    result.MipLodBias       = 0;
    result.AnisotropyEnable = true;
    result.MaxAnisotropy    = maxAnisotropy;
    result.CompareEnable    = false;
    result.CompareOp        = COMPARE_OP_ALWAYS;
    result.MinLod           = 0;
    result.MaxLod           = A3D_FLT_MAX;
    result.BorderColor      = BORDER_COLOR_OPAQUE_BLACK;
    return result;
}

SamplerDesc SamplerDesc::AnisotropicMirror(uint32_t maxAnisotropy)
{
    SamplerDesc result = {};
    result.MinFilter        = FILTER_MODE_LINEAR;
    result.MagFilter        = FILTER_MODE_LINEAR;
    result.MipMapMode       = MIPMAP_MODE_LINEAR;
    result.AddressU         = TEXTURE_ADDRESS_MODE_MIRROR;
    result.AddressV         = TEXTURE_ADDRESS_MODE_MIRROR;
    result.AddressW         = TEXTURE_ADDRESS_MODE_MIRROR;
    result.MipLodBias       = 0;
    result.AnisotropyEnable = true;
    result.MaxAnisotropy    = maxAnisotropy;
    result.CompareEnable    = false;
    result.CompareOp        = COMPARE_OP_ALWAYS;
    result.MinLod           = 0;
    result.MaxLod           = A3D_FLT_MAX;
    result.BorderColor      = BORDER_COLOR_OPAQUE_BLACK;
    return result;
}

SamplerDesc SamplerDesc::PointClampCmp(COMPARE_OP op)
{
    SamplerDesc result = {};
    result.MinFilter        = FILTER_MODE_NEAREST;
    result.MagFilter        = FILTER_MODE_NEAREST;
    result.MipMapMode       = MIPMAP_MODE_NEAREST;
    result.AddressU         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.AddressV         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.AddressW         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.MipLodBias       = 0;
    result.AnisotropyEnable = false;
    result.MaxAnisotropy    = 0;
    result.CompareEnable    = true;
    result.CompareOp        = op;
    result.MinLod           = 0;
    result.MaxLod           = A3D_FLT_MAX;
    result.BorderColor      = BORDER_COLOR_OPAQUE_BLACK;
    return result;
}

SamplerDesc SamplerDesc::LinearClampCmp(COMPARE_OP op)
{
    SamplerDesc result = {};
    result.MinFilter        = FILTER_MODE_LINEAR;
    result.MagFilter        = FILTER_MODE_LINEAR;
    result.MipMapMode       = MIPMAP_MODE_LINEAR;
    result.AddressU         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.AddressV         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.AddressW         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.MipLodBias       = 0;
    result.AnisotropyEnable = false;
    result.MaxAnisotropy    = 0;
    result.CompareEnable    = true;
    result.CompareOp        = op;
    result.MinLod           = 0;
    result.MaxLod           = A3D_FLT_MAX;
    result.BorderColor      = BORDER_COLOR_OPAQUE_BLACK;
    return result;
}

SamplerDesc SamplerDesc::AnisotropicClampCmp(COMPARE_OP op, uint32_t maxAnisotropy)
{
    SamplerDesc result = {};
    result.MinFilter        = FILTER_MODE_NEAREST;
    result.MagFilter        = FILTER_MODE_NEAREST;
    result.MipMapMode       = MIPMAP_MODE_NEAREST;
    result.AddressU         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.AddressV         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.AddressW         = TEXTURE_ADDRESS_MODE_CLAMP;
    result.MipLodBias       = 0;
    result.AnisotropyEnable = true;
    result.MaxAnisotropy    = maxAnisotropy;
    result.CompareEnable    = true;
    result.CompareOp        = op;
    result.MinLod           = 0;
    result.MaxLod           = A3D_FLT_MAX;
    result.BorderColor      = BORDER_COLOR_OPAQUE_BLACK;
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// RasterizerStatec structure
///////////////////////////////////////////////////////////////////////////////
RasterizerState RasterizerState::CullNone()
{
    RasterizerState result = {};
    result.PolygonMode              = POLYGON_MODE_SOLID;
    result.CullMode                 = CULL_MODE_NONE;
    result.FrontCounterClockWise    = false;
    result.DepthBias                = 0;
    result.DepthBiasClamp           = 0.0f;
    result.SlopeScaledDepthBias     = 0.0f;
    result.DepthClipEnable          = true;
    result.EnableConservativeRaster = false;
    return result;
}

RasterizerState RasterizerState::CullFront()
{
    RasterizerState result = {};
    result.PolygonMode              = POLYGON_MODE_SOLID;
    result.CullMode                 = CULL_MODE_FRONT;
    result.FrontCounterClockWise    = false;
    result.DepthBias                = 0;
    result.DepthBiasClamp           = 0.0f;
    result.SlopeScaledDepthBias     = 0.0f;
    result.DepthClipEnable          = true;
    result.EnableConservativeRaster = false;
    return result;
}

RasterizerState RasterizerState::CullBack()
{
    RasterizerState result = {};
    result.PolygonMode              = POLYGON_MODE_SOLID;
    result.CullMode                 = CULL_MODE_BACK;
    result.FrontCounterClockWise    = false;
    result.DepthBias                = 0;
    result.DepthBiasClamp           = 0.0f;
    result.SlopeScaledDepthBias     = 0.0f;
    result.DepthClipEnable          = true;
    result.EnableConservativeRaster = false;
    return result;
}

RasterizerState RasterizerState::Wireframe()
{
    RasterizerState result = {};
    result.PolygonMode              = POLYGON_MODE_WIREFRAME;
    result.CullMode                 = CULL_MODE_NONE;
    result.FrontCounterClockWise    = false;
    result.DepthBias                = 0;
    result.DepthBiasClamp           = 0.0f;
    result.SlopeScaledDepthBias     = 0.0f;
    result.DepthClipEnable          = true;
    result.EnableConservativeRaster = false;
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// StencilTestDesc structure
///////////////////////////////////////////////////////////////////////////////
StencilTestDesc StencilTestDesc::Default()
{
    StencilTestDesc result = {};
    result.StencilFailOp       = STENCIL_OP_KEEP;
    result.StencilDepthFailOp  = STENCIL_OP_KEEP;
    result.StencilPassOp       = STENCIL_OP_KEEP;
    result.StencilCompareOp    = COMPARE_OP_ALWAYS;
    return result;
}

StencilState StencilState::Default()
{
    StencilState result = {};
    result.StencilTestEnable    = false;
    result.StencllReadMask      = 0xff;
    result.StencilWriteMask     = 0xff;
    result.FrontFace            = StencilTestDesc::Default();
    result.BackFace             = StencilTestDesc::Default();
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// DepthState structure
///////////////////////////////////////////////////////////////////////////////
DepthState DepthState::None()
{
    DepthState result = {};
    result.DepthTestEnable  = false;
    result.DepthWriteEnable = false;
    result.DepthCompareOp   = COMPARE_OP_ALWAYS;
    return result;
}

DepthState DepthState::Default(COMPARE_OP op)
{
    DepthState result = {};
    result.DepthTestEnable  = true;
    result.DepthWriteEnable = true;
    result.DepthCompareOp   = op;
    return result;
}


DepthState DepthState::ReadOnly(COMPARE_OP op)
{
    DepthState result = {};
    result.DepthTestEnable  = true;
    result.DepthWriteEnable = false;
    result.DepthCompareOp   = op;
    return result;
}

DepthState DepthState::WriteOnly(COMPARE_OP op)
{
    DepthState result = {};
    result.DepthTestEnable  = false;
    result.DepthWriteEnable = true;
    result.DepthCompareOp   = op;
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// ColorBlendState structure
///////////////////////////////////////////////////////////////////////////////
ColorBlendState ColorBlendState::Opaque()
{
    ColorBlendState result = {};
    result.BlendEnable      = false;
    result.SrcBlend         = BLEND_FACTOR_ONE;
    result.DstBlend         = BLEND_FACTOR_ZERO;
    result.BlendOp          = BLEND_OP_ADD;
    result.SrcBlendAlpha    = BLEND_FACTOR_ONE;
    result.DstBlendAlpha    = BLEND_FACTOR_ZERO;
    result.BlendOpAlpha     = BLEND_OP_ADD;
    result.EnableWriteR     = true;
    result.EnableWriteB     = true;
    result.EnableWriteG     = true;
    result.EnableWriteA     = true;
    return result;
}

ColorBlendState ColorBlendState::AlphaBlend()
{
    ColorBlendState result = {};
    result.BlendEnable      = true;
    result.SrcBlend         = BLEND_FACTOR_SRC_ALPHA;
    result.DstBlend         = BLEND_FACTOR_INV_SRC_ALPHA;
    result.BlendOp          = BLEND_OP_ADD;
    result.SrcBlendAlpha    = BLEND_FACTOR_SRC_ALPHA;
    result.DstBlendAlpha    = BLEND_FACTOR_INV_SRC_ALPHA;
    result.BlendOpAlpha     = BLEND_OP_ADD;
    result.EnableWriteR     = true;
    result.EnableWriteB     = true;
    result.EnableWriteG     = true;
    result.EnableWriteA     = true;
    return result;
}

ColorBlendState ColorBlendState::Additive()
{
    ColorBlendState result = {};
    result.BlendEnable      = true;
    result.SrcBlend         = BLEND_FACTOR_SRC_ALPHA;
    result.DstBlend         = BLEND_FACTOR_ONE;
    result.BlendOp          = BLEND_OP_ADD;
    result.SrcBlendAlpha    = BLEND_FACTOR_SRC_ALPHA;
    result.DstBlendAlpha    = BLEND_FACTOR_ONE;
    result.BlendOpAlpha     = BLEND_OP_ADD;
    result.EnableWriteR     = true;
    result.EnableWriteB     = true;
    result.EnableWriteG     = true;
    result.EnableWriteA     = true;
    return result;
}

ColorBlendState ColorBlendState::Substract()
{
    ColorBlendState result = {};
    result.BlendEnable      = true;
    result.SrcBlend         = BLEND_FACTOR_SRC_ALPHA;
    result.DstBlend         = BLEND_FACTOR_ONE;
    result.BlendOp          = BLEND_OP_REV_SUB;
    result.SrcBlendAlpha    = BLEND_FACTOR_SRC_ALPHA;
    result.DstBlendAlpha    = BLEND_FACTOR_ONE;
    result.BlendOpAlpha     = BLEND_OP_REV_SUB;
    result.EnableWriteR     = true;
    result.EnableWriteB     = true;
    result.EnableWriteG     = true;
    result.EnableWriteA     = true;
    return result;
}

ColorBlendState ColorBlendState::PremultipliedAlpha()
{
    ColorBlendState result = {};
    result.BlendEnable      = true;
    result.SrcBlend         = BLEND_FACTOR_ONE;
    result.DstBlend         = BLEND_FACTOR_INV_SRC_ALPHA;
    result.BlendOp          = BLEND_OP_ADD;
    result.SrcBlendAlpha    = BLEND_FACTOR_ONE;
    result.DstBlendAlpha    = BLEND_FACTOR_INV_SRC_ALPHA;
    result.BlendOpAlpha     = BLEND_OP_ADD;
    result.EnableWriteR     = true;
    result.EnableWriteB     = true;
    result.EnableWriteG     = true;
    result.EnableWriteA     = true;
    return result;
}

ColorBlendState ColorBlendState::Multiply()
{
    ColorBlendState result = {};
    result.BlendEnable      = true;
    result.SrcBlend         = BLEND_FACTOR_ZERO;
    result.DstBlend         = BLEND_FACTOR_SRC_COLOR;
    result.BlendOp          = BLEND_OP_ADD;
    result.SrcBlendAlpha    = BLEND_FACTOR_ZERO;
    result.DstBlendAlpha    = BLEND_FACTOR_SRC_ALPHA;
    result.BlendOpAlpha     = BLEND_OP_ADD;
    result.EnableWriteR     = true;
    result.EnableWriteB     = true;
    result.EnableWriteG     = true;
    result.EnableWriteA     = true;
    return result;
}

ColorBlendState ColorBlendState::Screen()
{
    ColorBlendState result = {};
    result.BlendEnable      = true;
    result.SrcBlend         = BLEND_FACTOR_DST_COLOR;
    result.DstBlend         = BLEND_FACTOR_ONE;
    result.BlendOp          = BLEND_OP_ADD;
    result.SrcBlendAlpha    = BLEND_FACTOR_DST_ALPHA;
    result.DstBlendAlpha    = BLEND_FACTOR_ONE;
    result.BlendOpAlpha     = BLEND_OP_ADD;
    result.EnableWriteR     = true;
    result.EnableWriteB     = true;
    result.EnableWriteG     = true;
    result.EnableWriteA     = true;
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// BlendState structure
///////////////////////////////////////////////////////////////////////////////
BlendState BlendState::Default()
{
    BlendState result = {};
    result.IndependentBlendEnable   = false;
    for(auto i=0; i<8; ++i)
    { result.RenderTarget[i] = ColorBlendState::Opaque(); }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// MultiSampleState structure
///////////////////////////////////////////////////////////////////////////////
MultiSampleState MultiSampleState::Default()
{
    MultiSampleState result = {};
    result.EnableAlphaToCoverage    = false;
    result.EnableMultiSample        = false;
    result.SampleCount              = 1;
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// TessellationState structure
///////////////////////////////////////////////////////////////////////////////
TessellationState TessellationState::Default()
{
    TessellationState result = {};
    result.PatchControlCount = 0;
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// GraphicsPipelineStateDesc structure
///////////////////////////////////////////////////////////////////////////////
GraphicsPipelineStateDesc GraphicsPipelineStateDesc::Default()
{
    GraphicsPipelineStateDesc result = {};
    result.pLayout                  = nullptr;
    result.VS                       = {};
    result.PS                       = {};
    result.DS                       = {};
    result.HS                       = {};
    result.BlendState               = BlendState::Default();
    result.RasterizerState          = RasterizerState::CullNone();
    result.MultiSampleState         = MultiSampleState::Default();
    result.DepthState               = DepthState::Default();
    result.StencilState             = StencilState::Default();
    result.TessellationState        = TessellationState::Default();
    result.InputLayout.ElementCount = 0;
    result.InputLayout.pElements    = nullptr;
    result.PrimitiveTopology        = PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    result.RenderTargetCount        = 0;
    for(auto i=0; i<8; ++i)
    { result.RenderTarget[i]        = RESOURCE_FORMAT_UNKNOWN; }
    result.DepthTarget              = RESOURCE_FORMAT_UNKNOWN;
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// ComputePipelineStateDesc structure
///////////////////////////////////////////////////////////////////////////////
ComputePipelineStateDesc ComputePipelineStateDesc::Default()
{
    ComputePipelineStateDesc result = {};
    result.pLayout      = nullptr;
    result.CS           = {};
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// MeshletPipelineStateDesc structure
///////////////////////////////////////////////////////////////////////////////
MeshletPipelineStateDesc MeshletPipelineStateDesc::Default()
{
    MeshletPipelineStateDesc result = {};
    result.pLayout              = nullptr;
    result.AS                   = {};
    result.MS                   = {};
    result.PS                   = {};
    result.BlendState           = BlendState::Default();
    result.RasterizerState      = RasterizerState::CullNone();
    result.MultiSampleState     = MultiSampleState::Default();
    result.DepthState           = DepthState::Default();
    result.StencilState         = StencilState::Default();
    result.RenderTargetCount    = 0;
    for(auto i=0; i<8; ++i)
    { result.RenderTarget[i]    = RESOURCE_FORMAT_UNKNOWN; }
    result.DepthTarget          = RESOURCE_FORMAT_UNKNOWN;
    return result;
}

} // namespace a3d