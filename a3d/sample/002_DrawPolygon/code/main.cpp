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
#include "helpers/SampleApp.h"
#include "helpers/SampleUtil.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    float Position[3];      //!< �ʒu���W�ł�.
    float Color   [4];      //!< ���_�J���[�ł�.
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
a3d::ITexture*              g_pColorBuffer[2]       = {};       //!< �J���[�o�b�t�@�ł�.
a3d::ITextureView*          g_pColorView[2]         = {};       //!< �J���[�r���[�ł�.
a3d::ICommandList*          g_pCommandList[2]       = {};       //!< �R�}���h���X�g�ł�.
a3d::IFrameBuffer*          g_pFrameBuffer[2]       = {};       //!< �t���[���o�b�t�@�ł�.
a3d::Viewport               g_Viewport              = {};       //!< �r���[�|�[�g�ł�.
a3d::Rect                   g_Scissor               = {};       //!< �V�U�[��`�ł�.


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
        desc.MaxDepthTargetCount            = 1;
        desc.MaxShaderResourceCount         = 1;

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

        // �X���b�v�`�F�C������o�b�t�@���擾.
        g_pSwapChain->GetBuffer(0, &g_pColorBuffer[0]);
        g_pSwapChain->GetBuffer(1, &g_pColorBuffer[1]);

        a3d::TextureViewDesc viewDesc = {};
        viewDesc.Dimension          = a3d::VIEW_DIMENSION_TEXTURE2D;
        viewDesc.Format             = format;
        viewDesc.TextureAspect      = a3d::TEXTURE_ASPECT_COLOR;
        viewDesc.MipSlice           = 0;
        viewDesc.MipLevels          = desc.MipLevels;
        viewDesc.FirstArraySlice    = 0;
        viewDesc.ArraySize          = 1;
        viewDesc.ComponentMapping.R = a3d::TEXTURE_SWIZZLE_R;
        viewDesc.ComponentMapping.G = a3d::TEXTURE_SWIZZLE_G;
        viewDesc.ComponentMapping.B = a3d::TEXTURE_SWIZZLE_B;
        viewDesc.ComponentMapping.A = a3d::TEXTURE_SWIZZLE_A;

        for(auto i=0; i<2; ++i)
        {
            if (!g_pDevice->CreateTextureView(g_pColorBuffer[i], &viewDesc, &g_pColorView[i]))
            { return false; }
        }
    }

    // �t���[���o�b�t�@�̐���
    {
        // �t���[���o�b�t�@�̐ݒ�.
        a3d::FrameBufferDesc desc = {};
        desc.ColorCount         = 1;
        desc.pColorTargets[0]   = g_pColorView[0];
        desc.pDepthTarget       = nullptr;

        // 1���ڂ̃t���[���o�b�t�@�𐶐�.
        if (!g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[0]))
        { return false; }

        // 2���ڂ̃t���[���o�b�t�@�𐶐�.
        desc.pColorTargets[0] = g_pColorView[1];
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
            {  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f },
            { -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f },
            {  0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f },
        };

        a3d::BufferDesc desc = {};
        desc.Size                           = sizeof(vertices);
        desc.Stride                         = sizeof(Vertex);
        desc.InitState                      = a3d::RESOURCE_STATE_GENERAL;
        desc.Usage                          = a3d::RESOURCE_USAGE_VERTEX_BUFFER;
        desc.HeapProperty.Type              = a3d::HEAP_TYPE_UPLOAD;
        desc.HeapProperty.CpuPageProperty   = a3d::CPU_PAGE_PROPERTY_DEFAULT;

        if ( !g_pDevice->CreateBuffer(&desc, &g_pVertexBuffer) )
        { return false; }

        auto ptr = g_pVertexBuffer->Map();
        if ( ptr == nullptr )
        { return false; }

        memcpy( ptr, vertices, sizeof(vertices) );
        
        g_pVertexBuffer->Unmap();
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
        desc.EntryCount  = 0;
        desc.MaxSetCount = 0;

        if (!g_pDevice->CreateDescriptorSetLayout(&desc, &g_pDescriptorSetLayout))
        { return false; }
    }

    // �O���t�B�b�N�X�p�C�v���C���X�e�[�g�𐶐����܂�.
    {
        // ���͗v�f�ł�.
        a3d::InputElementDesc inputElements[] = {
            { "POSITION", 0, 0, a3d::RESOURCE_FORMAT_R32G32B32_FLOAT   , 0  },
            { "COLOR"   , 0, 1, a3d::RESOURCE_FORMAT_R32G32B32A32_FLOAT, 12 },
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
        desc.DepthStencilState.DepthTestEnable      = false;
        desc.DepthStencilState.DepthWriteEnable     = false;
        desc.DepthStencilState.DepthCompareOp       = a3d::COMPARE_OP_NEVER;
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

        // �J���[�r���[�̔j��.
        a3d::SafeRelease(g_pColorView[i]);

        // �J���[�o�b�t�@�̔j��.
        a3d::SafeRelease(g_pColorBuffer[i]);

        // �R�}���h���X�g�̔j��.
        a3d::SafeRelease(g_pCommandList[i]);
    }

    // �p�C�v���C���X�e�[�g�̔j��.
    a3d::SafeRelease(g_pPipelineState);

    // ���_�o�b�t�@�̔j��.
    a3d::SafeRelease(g_pVertexBuffer);

    // �f�B�X�N���v�^�Z�b�g���C�A�E�g�̔j��.
    a3d::SafeRelease(g_pDescriptorSetLayout);

    // �t�F���X�̔j��.
    a3d::SafeRelease(g_pFence);

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
    pCmd->ClearFrameBuffer(1, &clearColor, nullptr);

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

        // �O�p�`��`�悵�܂�.
        pCmd->DrawInstanced(3, 1, 0, 0);
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
