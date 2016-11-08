//-------------------------------------------------------------------------------------------------
// File : main.cpp
// Desc : Main Entry Point.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <a3d.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <cfloat>
#include "helpers/SampleApp.h"
#include "helpers/SampleUtil.h"
#include "helpers/SampleMath.h"
#include "helpers/SampleTarga.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    Vec3 Position;  //!< �ʒu���W�ł�.
    Vec2 TexCoord;  //!< ���_�J���[�ł�.
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Transform structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Transform
{
    Mat4 World;     //!< ���[���h�s��ł�.
    Mat4 View;      //!< �r���[�s��ł�.
    Mat4 Proj;      //!< �ˉe�s��ł�.
};


//-------------------------------------------------------------------------------------------------
// Forward declarations.
//-------------------------------------------------------------------------------------------------
bool InitA3D();
void TermA3D();
void DrawA3D();


//-------------------------------------------------------------------------------------------------
// Global Varaibles.
//-------------------------------------------------------------------------------------------------
IApp*                       g_pApp                  = nullptr;  //!< �E�B���h�E�̐������s���w���p�[�N���X�ł�.
a3d::IDevice*               g_pDevice               = nullptr;  //!< �f�o�C�X�ł�.
a3d::ISwapChain*            g_pSwapChain            = nullptr;  //!< �X���b�v�`�F�C���ł�.
a3d::IQueue*                g_pGraphicsQueue        = nullptr;  //!< �R�}���h�L���[�ł�.
a3d::IFence*                g_pFence                = nullptr;  //!< �t�F���X�ł�.
a3d::IDescriptorSetLayout*  g_pDescriptorSetLayout  = nullptr;  //!< �f�B�X�N���v�^�Z�b�g���C�A�E�g�ł�.
a3d::IPipelineState*        g_pPipelineState        = nullptr;  //!< �p�C�v���C���X�e�[�g�ł�.
a3d::IBuffer*               g_pVertexBuffer         = nullptr;  //!< ���_�o�b�t�@�ł�.
a3d::IBuffer*               g_pIndexBuffer          = nullptr;  //!< �C���f�b�N�X�o�b�t�@�ł�.
a3d::ITexture*              g_pDepthBuffer          = nullptr;  //!< �[�x�o�b�t�@�ł�.
a3d::ITexture*              g_pTexture              = nullptr;  //!< �e�N�X�`���ł�.
a3d::ISampler*              g_pSampler              = nullptr;  //!< �T���v���[�ł�.
a3d::ITexture*              g_pColorBuffer[2]       = {};       //!< �J���[�o�b�t�@�ł�.
a3d::ICommandList*          g_pCommandList[2]       = {};       //!< �R�}���h���X�g�ł�.
a3d::IFrameBuffer*          g_pFrameBuffer[2]       = {};       //!< �t���[���o�b�t�@�ł�.
a3d::IBuffer*               g_pConstantBuffer[2]    = {};       //!< �萔�o�b�t�@�ł�.
a3d::Viewport               g_Viewport              = {};       //!< �r���[�|�[�g�ł�.
a3d::Rect                   g_Scissor               = {};       //!< �V�U�[��`�ł�.
a3d::IDescriptorSet*        g_pDescriptorSet[2]     = {};       //!< �f�B�X�N���v�^�Z�b�g�ł�.
Transform                   g_Transform             = {};       //!< �ϊ��s��ł�.
float                       g_RotateAngle           = 0.0f;     //!< ��]�p�ł�.
void*                       g_pCbHead[2]            = {};       //!< �萔�o�b�t�@�̐擪�|�C���^�ł�.

//-------------------------------------------------------------------------------------------------
//      ���C���G���g���[�|�C���g�ł�.
//-------------------------------------------------------------------------------------------------
void main()
{
    // �E�B���h�E����.
    if (!CreateApp(960, 540, &g_pApp))
    { return; }

    // A3D������.
    if (!InitA3D())
    {
        g_pApp->Release();
        return;
    }

    // ���C�����[�v.
    while( g_pApp->IsLoop() )
    {
        // �`�揈��.
        DrawA3D();
    }

    // ��n��.
    TermA3D();
    g_pApp->Release();
}

//-------------------------------------------------------------------------------------------------
//      A3D�̏��������s���܂�.
//-------------------------------------------------------------------------------------------------
bool InitA3D()
{
    // �f�o�C�X�̐���.
    {
        a3d::DeviceDesc desc = {};

        desc.EnableDebug = true;

        // �ő�f�B�X�N���v�^����ݒ�.
        desc.MaxColorTargetCount            = 2;
        desc.MaxDepthTargetCount            = 2;
        desc.MaxShaderResourceCount         = 4;
        desc.MaxSamplerCount                = 2;

        // �ő�T�u�~�b�g����ݒ�.
        desc.MaxGraphicsQueueSubmitCount    = 256;
        desc.MaxCopyQueueSubmitCount        = 256;
        desc.MaxComputeQueueSubmitCount     = 256;

        // �f�o�C�X�𐶐�.
        if (!a3d::CreateDevice(&desc, &g_pDevice))
        { return false; }

        // �R�}���h�L���[���擾.
        g_pDevice->GetGraphicsQueue(&g_pGraphicsQueue);
    }

    auto info = g_pDevice->GetInfo();

    // �X���b�v�`�F�C���̐���.
    {
    #if SAMPLE_IS_VULKAN
        auto format = a3d::RESOURCE_FORMAT_B8G8R8A8_UNORM;
    #else
        auto format = a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM;
    #endif

        a3d::SwapChainDesc desc = {};
        desc.Extent.Width   = g_pApp->GetWidth();
        desc.Extent.Height  = g_pApp->GetHeight();
        desc.Format         = format;
        desc.MipLevels      = 1;
        desc.SampleCount    = 1;
        desc.BufferCount    = 2;
        desc.SyncInterval   = 1;
        desc.InstanceHandle = g_pApp->GetInstanceHandle();
        desc.WindowHandle   = g_pApp->GetWindowHandle();

        if (!g_pDevice->CreateSwapChain(&desc, &g_pSwapChain))
        { return false; }
    }

    // �[�x�o�b�t�@�̐���.
    {
        a3d::TextureDesc desc = {};
        desc.Dimension                      = a3d::RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width                          = g_pApp->GetWidth();
        desc.Height                         = g_pApp->GetHeight();
        desc.DepthOrArraySize               = 1;
        desc.Format                         = a3d::RESOURCE_FORMAT_D32_FLOAT;
        desc.MipLevels                      = 1;
        desc.SampleCount                    = 1;
        desc.Layout                         = a3d::RESOURCE_LAYOUT_OPTIMAL;
        desc.Usage                          = a3d::RESOURCE_USAGE_DEPTH_TARGET;
        desc.InitState                      = a3d::RESOURCE_STATE_DEPTH_WRITE;
        desc.HeapProperty.Type              = a3d::HEAP_TYPE_DEFAULT;
        desc.HeapProperty.CpuPageProperty   = a3d::CPU_PAGE_PROPERTY_DEFAULT;
        desc.ComponentMapping.R             = a3d::TEXTURE_SWIZZLE_R;
        desc.ComponentMapping.G             = a3d::TEXTURE_SWIZZLE_G;
        desc.ComponentMapping.B             = a3d::TEXTURE_SWIZZLE_B;
        desc.ComponentMapping.A             = a3d::TEXTURE_SWIZZLE_A;

        if (!g_pDevice->CreateTexture(&desc, &g_pDepthBuffer))
        { return false; }
    }

    // �t���[���o�b�t�@�̐���
    {
        // �X���b�v�`�F�C������o�b�t�@���擾.
        g_pSwapChain->GetBuffer(0, &g_pColorBuffer[0]);
        g_pSwapChain->GetBuffer(1, &g_pColorBuffer[1]);

        // �t���[���o�b�t�@�̐ݒ�.
        a3d::FrameBufferDesc desc = {};
        desc.ColorCount         = 1;
        desc.pColorTargets[0]   = g_pColorBuffer[0];
        desc.pDepthTarget       = g_pDepthBuffer;

        // 1���ڂ̃t���[���o�b�t�@�𐶐�.
        if (!g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[0]))
        { return false; }

        // 2���ڂ̃t���[���o�b�t�@�𐶐�.
        desc.pColorTargets[0] = g_pColorBuffer[1];
        if (!g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[1]))
        { return false; }
    }

    // �R�}���h���X�g�𐶐�.
    {
        for(auto i=0; i<2; ++i)
        {
            if (!g_pDevice->CreateCommandList(a3d::COMMANDLIST_TYPE_DIRECT, nullptr, &g_pCommandList[i]))
            { return false; }
        }
    }

    // �t�F���X�𐶐�.
    {
        if (!g_pDevice->CreateFence(&g_pFence))
        { return false; }
    }

    // ���_�o�b�t�@�𐶐�.
    {
        Vertex vertices[] = {
            { Vec3( 1.0f, -1.0f, 0.0f), Vec2(1.0f, 0.0f) },
            { Vec3(-1.0f, -1.0f, 0.0f), Vec2(0.0f, 0.0f) },
            { Vec3(-1.0f,  1.0f, 0.0f), Vec2(0.0f, 1.0f) },
            { Vec3( 1.0f,  1.0f, 0.0f), Vec2(1.0f, 1.0f) },
        };

        a3d::BufferDesc desc = {};
        desc.Size                           = sizeof(vertices);
        desc.Stride                         = sizeof(Vertex);
        desc.InitState                      = a3d::RESOURCE_STATE_GENERAL;
        desc.Usage                          = a3d::RESOURCE_USAGE_VERTEX_BUFFER;
        desc.HeapProperty.Type              = a3d::HEAP_TYPE_UPLOAD;
        desc.HeapProperty.CpuPageProperty   = a3d::CPU_PAGE_PROPERTY_DEFAULT;
        desc.EnableRow                      = false;

        if ( !g_pDevice->CreateBuffer(&desc, &g_pVertexBuffer) )
        { return false; }

        auto ptr = g_pVertexBuffer->Map();
        if ( ptr == nullptr )
        { return false; }

        memcpy( ptr, vertices, sizeof(vertices) );
        
        g_pVertexBuffer->Unmap();
    }

    // �C���f�b�N�X�o�b�t�@�𐶐�.
    {
        uint32_t indices[] = {
            0, 1, 2,
            2, 0, 3,
        };

        a3d::BufferDesc desc = {};
        desc.Size                           = sizeof(indices);
        desc.Stride                         = sizeof(uint32_t);
        desc.InitState                      = a3d::RESOURCE_STATE_GENERAL;
        desc.Usage                          = a3d::RESOURCE_USAGE_INDEX_BUFFER;
        desc.HeapProperty.Type              = a3d::HEAP_TYPE_UPLOAD;
        desc.HeapProperty.CpuPageProperty   = a3d::CPU_PAGE_PROPERTY_DEFAULT;
        desc.EnableRow                      = false;

        if ( !g_pDevice->CreateBuffer(&desc, &g_pIndexBuffer) )
        { return false; }

        auto ptr = g_pIndexBuffer->Map();
        if ( ptr == nullptr )
        { return false; }

        memcpy( ptr, indices, sizeof(indices) );

        g_pIndexBuffer->Unmap();
    }

    // �萔�o�b�t�@�𐶐�.
    {
        a3d::BufferDesc desc = {};
        desc.Size                           = a3d::RoundUp<uint64_t>( sizeof(Transform), info.ConstantBufferMemoryAlignment );
        desc.Stride                         = a3d::RoundUp<uint32_t>( sizeof(Transform), info.ConstantBufferMemoryAlignment );
        desc.InitState                      = a3d::RESOURCE_STATE_GENERAL;
        desc.Usage                          = a3d::RESOURCE_USAGE_CONSTANT_BUFFER;
        desc.HeapProperty.Type              = a3d::HEAP_TYPE_UPLOAD;
        desc.HeapProperty.CpuPageProperty   = a3d::CPU_PAGE_PROPERTY_DEFAULT;
        desc.EnableRow                      = false;

        for(auto i=0; i<2; ++i)
        {
            if (!g_pDevice->CreateBuffer(&desc, &g_pConstantBuffer[i]))
            { return false; }

            g_pCbHead[i] = g_pConstantBuffer[i]->Map();
        }

        Vec3 pos = Vec3(0.0f, 0.0f, 5.0f);
        Vec3 at  = Vec3(0.0f, 0.0f, 0.0f);
        Vec3 up  = Vec3(0.0f, 1.0f, 0.0f);

        auto aspect = static_cast<float>(g_pApp->GetWidth()) / static_cast<float>(g_pApp->GetHeight());

        g_Transform.World = Mat4::Identity();
        g_Transform.View  = Mat4::LookAt(pos, at, up);
        g_Transform.Proj  = Mat4::PersFov(ToRadian(45.0f), aspect, 0.1f, 1000.0f);
    }

    // �V�F�[�_�o�C�i����ǂݍ��݂܂�.
    a3d::ShaderBinary vs = {};
    a3d::ShaderBinary ps = {};
    {
    #if SAMPLE_IS_VULKAN
        const auto vsFilePath = "../res/glsl/simpleVS.spv";
        const auto psFilePath = "../res/glsl/simplePS.spv";
    #else
        const auto vsFilePath = "../res/hlsl/simpleVS.cso";
        const auto psFilePath = "../res/hlsl/simplePS.cso";
    #endif

        if (!LoadShaderBinary(vsFilePath, vs))
        { return false; }

        if (!LoadShaderBinary(psFilePath, ps))
        {
            DisposeShaderBinary(vs);
            return false;
        }
    }

    // �f�B�X�N���v�^�Z�b�g���C�A�E�g�𐶐����܂�.
    {
        a3d::DescriptorSetLayoutDesc desc = {};
        desc.MaxSetCount               = 2;
        desc.EntryCount                = 3;
        
        desc.Entries[0].ShaderMask     = a3d::SHADER_MASK_VERTEX;
        desc.Entries[0].ShaderRegister = 0;
        desc.Entries[0].BindLocation   = 0;
        desc.Entries[0].Type           = a3d::DESCRIPTOR_TYPE_CBV;

        desc.Entries[1].ShaderMask     = a3d::SHADER_MASK_PIXEL;
        desc.Entries[1].ShaderRegister = 0;
        desc.Entries[1].BindLocation   = 1;
        desc.Entries[1].Type           = a3d::DESCRIPTOR_TYPE_SMP;

        desc.Entries[2].ShaderMask     = a3d::SHADER_MASK_PIXEL;
        desc.Entries[2].ShaderRegister = 0;
        desc.Entries[2].BindLocation   = 2;
        desc.Entries[2].Type           = a3d::DESCRIPTOR_TYPE_SRV;

        if (!g_pDevice->CreateDescriptorSetLayout(&desc, &g_pDescriptorSetLayout))
        { return false; }

        for(auto i=0; i<2; ++i)
        {
            if (!g_pDescriptorSetLayout->CreateDescriptorSet(&g_pDescriptorSet[i]))
            { return false; }
        }
    }

    // �O���t�B�b�N�X�p�C�v���C���X�e�[�g�𐶐����܂�.
    {
        // ���͗v�f�ł�.
        a3d::InputElementDesc inputElements[] = {
            { "POSITION", 0, 0, a3d::RESOURCE_FORMAT_R32G32B32_FLOAT , 0  },
            { "TEXCOORD", 0, 1, a3d::RESOURCE_FORMAT_R32G32_FLOAT    , 12 },
        };

        // ���̓X�g���[���ł�.
        a3d::InputStreamDesc inputStream = {};
        inputStream.ElementCount    = 2;
        inputStream.pElements       = inputElements;
        inputStream.StreamIndex     = 0;
        inputStream.StrideInBytes   = sizeof(Vertex);
        inputStream.InputClass      = a3d::INPUT_CLASSIFICATION_PER_VERTEX;

        // ���̓��C�A�E�g�ł�.
        a3d::InputLayoutDesc inputLayout = {};
        inputLayout.StreamCount = 1;
        inputLayout.pStreams    = &inputStream;

        // �X�e���V���X�e�[�g�ł�.
        a3d::StencilState stencilState = {};
        stencilState.StencilFailOp      = a3d::STENCIL_OP_KEEP;
        stencilState.StencilDepthFailOp = a3d::STENCIL_OP_KEEP;
        stencilState.StencilFailOp      = a3d::STENCIL_OP_KEEP;
        stencilState.StencilCompareOp   = a3d::COMPARE_OP_NEVER;

        // �O���t�B�b�N�X�p�C�v���C���X�e�[�g��ݒ肵�܂�.
        a3d::GraphicsPipelineStateDesc desc = {};

        // �V�F�[�_�̐ݒ�.
        desc.VertexShader = vs;
        desc.PixelShader  = ps;

        // �u�����h�X�e�[�g�̐ݒ�.
        desc.BlendState.IndependentBlendEnable          = false;
        desc.BlendState.LogicOpEnable                   = false;
        desc.BlendState.LogicOp                         = a3d::LOGIC_OP_NOOP;
        for(auto i=0; i<8; ++i)
        {
            desc.BlendState.ColorTarget[i].BlendEnable      = false;
            desc.BlendState.ColorTarget[i].SrcBlend         = a3d::BLEND_FACTOR_ONE;
            desc.BlendState.ColorTarget[i].DstBlend         = a3d::BLEND_FACTOR_ZERO;
            desc.BlendState.ColorTarget[i].BlendOp          = a3d::BLEND_OP_ADD;
            desc.BlendState.ColorTarget[i].SrcBlendAlpha    = a3d::BLEND_FACTOR_ONE;
            desc.BlendState.ColorTarget[i].DstBlendAlpha    = a3d::BLEND_FACTOR_ZERO;
            desc.BlendState.ColorTarget[i].BlendOpAlpha     = a3d::BLEND_OP_ADD;
            desc.BlendState.ColorTarget[i].EnableWriteR     = true;
            desc.BlendState.ColorTarget[i].EnableWriteG     = true;
            desc.BlendState.ColorTarget[i].EnableWriteB     = true;
            desc.BlendState.ColorTarget[i].EnableWriteA     = true;
        }

        // ���X�^���C�U�\�X�e�[�g�̐ݒ�.
        desc.RasterizerState.PolygonMode                = a3d::POLYGON_MODE_SOLID;
        desc.RasterizerState.CullMode                   = a3d::CULL_MODE_NONE;
        desc.RasterizerState.FrontCounterClockWise      = false;
        desc.RasterizerState.DepthBias                  = 0;
        desc.RasterizerState.DepthBiasClamp             = 0.0f;
        desc.RasterizerState.SlopeScaledDepthBais       = 0;
        desc.RasterizerState.DepthClipEnable            = true;
        desc.RasterizerState.EnableConservativeRaster   = false;
        
        // �}���`�T���v���X�e�[�g�̐ݒ�.
        desc.MultiSampleState.EnableAlphaToCoverage = false;
        desc.MultiSampleState.EnableMultiSample     = false;
        desc.MultiSampleState.SampleCount           = 1;

        // �[�x�X�e���V���X�e�[�g�̐ݒ�.
        desc.DepthStencilState.DepthTestEnable      = true;
        desc.DepthStencilState.DepthWriteEnable     = true;
        desc.DepthStencilState.DepthCompareOp       = a3d::COMPARE_OP_LESS;
        desc.DepthStencilState.StencilTestEnable    = false;
        desc.DepthStencilState.StencllReadMask      = 0;
        desc.DepthStencilState.StencilWriteMask     = 0;
        desc.DepthStencilState.FrontFace            = stencilState;
        desc.DepthStencilState.BackFace             = stencilState;

        // �e�b�Z���[�V�����X�e�[�g�̐ݒ�.
        desc.TessellationState.PatchControlCount = 0;

        // ���̓A�E�g�̐ݒ�.
        desc.InputLayout = inputLayout;

        // �f�B�X�N���v�^�Z�b�g���C�A�E�g�̐ݒ�.
        desc.pLayout = g_pDescriptorSetLayout;
        
        // �v���~�e�B�u�g�|���W�[�̐ݒ�.
        desc.PrimitiveTopology = a3d::PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        // �t���[���o�b�t�@�̐ݒ�.
        desc.pFrameBuffer = g_pFrameBuffer[0];

        // �L���b�V���ς݃p�C�v���C���X�e�[�g�̐ݒ�.
        desc.pCachedPSO = nullptr;

        // �O���t�B�b�N�X�p�C�v���C���X�e�[�g�̐���.
        if (!g_pDevice->CreateGraphicsPipeline(&desc, &g_pPipelineState))
        { return false; }
    }
     // �s�v�ɂȂ����̂Ŕj�����܂�.
    DisposeShaderBinary(vs);
    DisposeShaderBinary(ps);

    // �e�N�X�`���̐���.
    {
        Targa targa;
        if (!targa.Load(L"../res/texture/sample32bitRLE.tga"))
        { return false; }

        auto rowPitch = targa.GetWidth() * targa.GetBytePerPixel();
        auto size     = rowPitch * targa.GetHeight() ;

        a3d::BufferDesc bufDesc = {};
        bufDesc.Size                            = size;
        bufDesc.InitState                       = a3d::RESOURCE_STATE_GENERAL;
        bufDesc.Usage                           = a3d::RESOURCE_USAGE_COPY_SRC;
        bufDesc.HeapProperty.Type               = a3d::HEAP_TYPE_UPLOAD;
        bufDesc.HeapProperty.CpuPageProperty    = a3d::CPU_PAGE_PROPERTY_DEFAULT;

        a3d::IBuffer* pImmediate = nullptr;
        if (!g_pDevice->CreateBuffer(&bufDesc, &pImmediate))
        { return false; }

        a3d::TextureDesc desc = {};
        desc.Dimension                      = a3d::RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width                          = targa.GetWidth();
        desc.Height                         = targa.GetHeight();
        desc.DepthOrArraySize               = 1;
        desc.MipLevels                      = 1;
        desc.Format                         = a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM;
        desc.SampleCount                    = 1;
        desc.Layout                         = a3d::RESOURCE_LAYOUT_OPTIMAL;
        desc.InitState                      = a3d::RESOURCE_STATE_GENERAL;
        desc.Usage                          = a3d::RESOURCE_USAGE_SHADER_RESOURCE | a3d::RESOURCE_USAGE_COPY_DST;
        desc.HeapProperty.Type              = a3d::HEAP_TYPE_DEFAULT;
        desc.HeapProperty.CpuPageProperty   = a3d::CPU_PAGE_PROPERTY_DEFAULT;
        desc.ComponentMapping.R             = a3d::TEXTURE_SWIZZLE_R;
        desc.ComponentMapping.G             = a3d::TEXTURE_SWIZZLE_G;
        desc.ComponentMapping.B             = a3d::TEXTURE_SWIZZLE_B;
        desc.ComponentMapping.A             = a3d::TEXTURE_SWIZZLE_A;

        if (!g_pDevice->CreateTexture(&desc, &g_pTexture))
        {
            a3d::SafeRelease(pImmediate);
            return false;
        }

        auto layout = g_pTexture->GetSubresourceLayout(0);

        auto dstPtr = static_cast<uint8_t*>(pImmediate->Map());
        assert( dstPtr != nullptr );

        auto srcPtr = targa.GetPixels();
        assert( srcPtr != nullptr );

        for(auto i=0; i<layout.RowCount; ++i)
        {
            memcpy(dstPtr, srcPtr, rowPitch);
            srcPtr += rowPitch;
            dstPtr += layout.RowPitch;
        }
        pImmediate->Unmap();

        a3d::Offset3D offset = {0, 0, 0};

        g_pCommandList[0]->Begin();
        g_pCommandList[0]->TextureBarrier(g_pTexture, a3d::RESOURCE_STATE_COPY_DST);
        g_pCommandList[0]->CopyBufferToTexture(g_pTexture, 0, offset, pImmediate, 0);
        g_pCommandList[0]->TextureBarrier(g_pTexture, a3d::RESOURCE_STATE_SHADER_READ);
        g_pCommandList[0]->End();
        g_pGraphicsQueue->Submit(g_pCommandList[0]);
        g_pGraphicsQueue->Execute(nullptr);
        g_pGraphicsQueue->WaitIdle();

        a3d::SafeRelease(pImmediate);
    }

    // �T���v���[�̐���.
    {
        a3d::SamplerDesc desc = {};
        desc.MinFilter          = a3d::FILTER_MODE_LINEAR;
        desc.MagFilter          = a3d::FILTER_MODE_LINEAR;
        desc.MipMapMode         = a3d::MIPMAP_MODE_LINEAR;
        desc.AddressU           = a3d::TEXTURE_ADDRESS_MODE_CLAMP;
        desc.AddressV           = a3d::TEXTURE_ADDRESS_MODE_CLAMP;
        desc.AddressW           = a3d::TEXTURE_ADDRESS_MODE_CLAMP;
        desc.MinLod             = 0.0f;
        desc.AnisotropyEnable   = false;
        desc.MaxAnisotropy      = 16;
        desc.CompareEnable      = false;
        desc.CompareOp          = a3d::COMPARE_OP_NEVER;
        desc.MinLod             = 0.0f;
        desc.MaxLod             = FLT_MAX;
        desc.BorderColor        = a3d::BORDER_COLOR_OPAQUE_WHITE;

        if (!g_pDevice->CreateSampler(&desc, &g_pSampler))
        { return false; }
    }

    // �r���[�|�[�g�̐ݒ�.
    g_Viewport.X        = 0.0f;
    g_Viewport.Y        = 0.0f;
    g_Viewport.Width    = static_cast<float>(g_pApp->GetWidth());
    g_Viewport.Height   = static_cast<float>(g_pApp->GetHeight());
    g_Viewport.MinDepth = 0.0f;
    g_Viewport.MaxDepth = 1.0f;

    // �V�U�[��`�̐ݒ�.
    g_Scissor.Offset.X      = 0;
    g_Scissor.Offset.Y      = 0;
    g_Scissor.Extent.Width  = g_pApp->GetWidth();
    g_Scissor.Extent.Height = g_pApp->GetHeight();

    return true;
}

//-------------------------------------------------------------------------------------------------
//      A3D�̏I���������s���܂�.
//-------------------------------------------------------------------------------------------------
void TermA3D()
{
    // �_�u���o�b�t�@���\�[�X�̔j��.
    for(auto i=0; i<2; ++i)
    {
        // �t���[���o�b�t�@�̔j��.
        a3d::SafeRelease(g_pFrameBuffer[i]);

        // �R�}���h���X�g�̔j��.
        a3d::SafeRelease(g_pCommandList[i]);

        // �J���[�o�b�t�@�̔j��.
        a3d::SafeRelease(g_pColorBuffer[i]);

        // �萔�o�b�t�@�̔j��.
        a3d::SafeRelease(g_pConstantBuffer[i]);

        // �f�B�X�N���v�^�Z�b�g�̔j��.
        a3d::SafeRelease(g_pDescriptorSet[i]);
    }

    // �T���v���[�̔j��.
    a3d::SafeRelease(g_pSampler);

    // �e�N�X�`���̔j��.
    a3d::SafeRelease(g_pTexture);

    // �p�C�v���C���X�e�[�g�̔j��.
    a3d::SafeRelease(g_pPipelineState);

    // ���_�o�b�t�@�̔j��.
    a3d::SafeRelease(g_pVertexBuffer);

    // �C���f�b�N�X�o�b�t�@�̔j��.
    a3d::SafeRelease(g_pIndexBuffer);

    // �f�B�X�N���v�^�Z�b�g���C�A�E�g�̔j��.
    a3d::SafeRelease(g_pDescriptorSetLayout);

    // �t�F���X�̔j��.
    a3d::SafeRelease(g_pFence);

    // �[�x�o�b�t�@�̔j��.
    a3d::SafeRelease(g_pDepthBuffer);

    // �X���b�v�`�F�C���̔j��.
    a3d::SafeRelease(g_pSwapChain);

    // �O���t�B�b�N�X�L���[�̔j��.
    a3d::SafeRelease(g_pGraphicsQueue);

    // �f�o�C�X�̔j��.
    a3d::SafeRelease(g_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      A3D�ɂ��`�揈�����s���܂�.
//-------------------------------------------------------------------------------------------------
void DrawA3D()
{
    // �o�b�t�@�ԍ����擾���܂�.
    auto idx = g_pSwapChain->GetCurrentBufferIndex();

    // �萔�o�b�t�@���X�V.
    {
        g_RotateAngle += 0.025f;
        g_Transform.World = Mat4::RotateY(g_RotateAngle);

        auto ptr = static_cast<uint8_t*>(g_pCbHead[idx]);
        memcpy(ptr, &g_Transform, sizeof(g_Transform));

        auto stride = g_pConstantBuffer[idx]->GetDesc().Stride;

        g_pDescriptorSet[idx]->SetBuffer(0, g_pConstantBuffer[idx], stride, 0);
        g_pDescriptorSet[idx]->SetSampler(1, g_pSampler);
        g_pDescriptorSet[idx]->SetTexture(2, g_pTexture);
        g_pDescriptorSet[idx]->Update();
    }

    // �R�}���h�̋L�^���J�n���܂�.
    auto pCmd = g_pCommandList[idx];
    pCmd->Begin();

    // �������ݗp�̃o���A��ݒ肵�܂�.
    pCmd->TextureBarrier(g_pColorBuffer[idx], a3d::RESOURCE_STATE_COLOR_WRITE);

    // �t���[���o�b�t�@��ݒ肵�܂�.
    pCmd->SetFrameBuffer(g_pFrameBuffer[idx]);

    // �t���[���o�b�t�@���N���A���܂�.
    a3d::ClearColorValue clearColor = {};
    clearColor.Float[0] = 0.25f;
    clearColor.Float[1] = 0.25f;
    clearColor.Float[2] = 0.25f;
    clearColor.Float[3] = 1.0f;

    a3d::ClearDepthStencilValue clearDepth = {};
    clearDepth.Depth                = 1.0f;
    clearDepth.Stencil              = 0;
    clearDepth.EnableClearDepth     = true;
    clearDepth.EnableClearStencil   = false;

    pCmd->ClearFrameBuffer(1, &clearColor, &clearDepth);

    {
        // �f�B�X�N���v�^�Z�b�g���C�A�E�g��ݒ肵�܂�.
        pCmd->SetDescriptorSetLayout(g_pDescriptorSetLayout);

        // �p�C�v���C���X�e�[�g��ݒ肵�܂�.
        pCmd->SetPipelineState(g_pPipelineState);

        // �r���[�|�[�g�ƃV�U�[��`��ݒ肵�܂�.
        // NOTE : �r���[�|�[�g�ƃV�U�[��`�̐ݒ�́C�K��SetPipelineState() �̌�ł���K�v������܂�.
        pCmd->SetViewports(1, &g_Viewport);
        pCmd->SetScissors (1, &g_Scissor);

        // ���_�o�b�t�@��ݒ肵�܂�.
        pCmd->SetVertexBuffers(0, 1, &g_pVertexBuffer, nullptr);
        pCmd->SetIndexBuffer(g_pIndexBuffer, 0);

        // ��`��`��.
        pCmd->SetDescriptorSet(g_pDescriptorSet[idx]);
        pCmd->DrawIndexedInstanced(6, 1, 0, 0, 0);
    }
    
    // �\���p�̃o���A��ݒ肷��O�ɁC�t���[���o�b�t�@�̐ݒ����������K�v������܂�.
    pCmd->SetFrameBuffer(nullptr);

    // �\���p�Ƀo���A��ݒ肵�܂�.
    pCmd->TextureBarrier(g_pColorBuffer[idx], a3d::RESOURCE_STATE_PRESENT);

    // �R�}���h���X�g�ւ̋L�^���I�����܂�.
    pCmd->End();

    // �R�}���h�L���[�ɓo�^���܂�.
    g_pGraphicsQueue->Submit(pCmd);

    // �R�}���h�����s���܂�.
    g_pGraphicsQueue->Execute(g_pFence);

    // �R�}���h�����s������ҋ@���܂�.
    if (!g_pFence->IsSignaled())
    { g_pFence->Wait(UINT32_MAX); }

    // ��ʂɕ\�����܂�.
    g_pSwapChain->Present();
}
