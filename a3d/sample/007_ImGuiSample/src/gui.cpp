//-------------------------------------------------------------------------------------------------
// File : gui.cpp
// Desc : ImGui Helper.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "gui.h"
#include <string>
#include <SampleUtil.h>

#if A3D_IS_WIN
    #include <Windows.h>
#elif A3D_IS_LINUX

#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
// GuiMgr class
///////////////////////////////////////////////////////////////////////////////////////////////////
GuiMgr GuiMgr::s_Instance;

//-------------------------------------------------------------------------------------------------
//      �R���X�g���N�^�ł�.
//-------------------------------------------------------------------------------------------------
GuiMgr::GuiMgr()
: m_pDevice             (nullptr)
, m_pSampler            (nullptr)
, m_pTexture            (nullptr)
, m_pTextureView        (nullptr)
, m_pDescriptorSetLayout(nullptr)
, m_pPipelineState      (nullptr)
, m_pCommandList        (nullptr)
, m_BufferIndex         (0)
{
    for(auto i=0; i<2; ++i)
    {
        m_pVB[i]    = nullptr;
        m_pIB[i]    = nullptr;
        m_SizeVB[i] = 0;
        m_SizeIB[i] = 0;

        m_pCB[i]            = nullptr;
        m_pCBV[i]           = nullptr;
        m_pDescriptorSet[i] = nullptr;
    }
}

//-------------------------------------------------------------------------------------------------
//      �f�X�g���N�^�ł�.
//-------------------------------------------------------------------------------------------------
GuiMgr::~GuiMgr()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      �V���O���g���C���X�^���X���擾���܂�.
//-------------------------------------------------------------------------------------------------
GuiMgr& GuiMgr::GetInstance()
{ return s_Instance; }

//-------------------------------------------------------------------------------------------------
//      �������������s���܂�.
//-------------------------------------------------------------------------------------------------
bool GuiMgr::Init(a3d::IDevice* pDevice, a3d::IFrameBuffer* pFrameBuffer, IApp* pApp)
{
    if (pDevice == nullptr || pApp == nullptr)
    { return false; }

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    m_pApp = pApp;

    // ���_�o�b�t�@�𐶐�.
    {
        a3d::BufferDesc desc = {};
        desc.Size                           = MaxPrimitiveCount * sizeof(ImDrawVert) * 4;
        desc.Stride                         = sizeof(ImDrawVert);
        desc.InitState                      = a3d::RESOURCE_STATE_GENERAL;
        desc.Usage                          = a3d::RESOURCE_USAGE_VERTEX_BUFFER;
        desc.HeapProperty.Type              = a3d::HEAP_TYPE_UPLOAD;
        desc.HeapProperty.CpuPageProperty   = a3d::CPU_PAGE_PROPERTY_DEFAULT;

        for(auto i=0; i<2; ++i)
        {
            if ( !m_pDevice->CreateBuffer(&desc, &m_pVB[i]) )
            { return false; }
        }
    }

    // �C���f�b�N�X�o�b�t�@�𐶐�.
    {
        a3d::BufferDesc desc = {};
        desc.Size                           = MaxPrimitiveCount * sizeof(ImDrawIdx) * 6;
        desc.Stride                         = sizeof(ImDrawIdx);
        desc.InitState                      = a3d::RESOURCE_STATE_GENERAL;
        desc.Usage                          = a3d::RESOURCE_USAGE_INDEX_BUFFER;
        desc.HeapProperty.Type              = a3d::HEAP_TYPE_UPLOAD;
        desc.HeapProperty.CpuPageProperty   = a3d::CPU_PAGE_PROPERTY_DEFAULT;

        for(auto i=0; i<2; ++i)
        {
            if ( !m_pDevice->CreateBuffer(&desc, &m_pIB[i]))
            { return false; }
        }
    }

    // �萔�o�b�t�@�𐶐�.
    {
        auto info = m_pDevice->GetInfo();

        a3d::BufferDesc desc = {};
        desc.Size                           = a3d::RoundUp<uint64_t>( sizeof(Mat4), info.ConstantBufferMemoryAlignment );
        desc.Stride                         = a3d::RoundUp<uint32_t>( sizeof(Mat4), info.ConstantBufferMemoryAlignment );
        desc.InitState                      = a3d::RESOURCE_STATE_GENERAL;
        desc.Usage                          = a3d::RESOURCE_USAGE_CONSTANT_BUFFER;
        desc.HeapProperty.Type              = a3d::HEAP_TYPE_UPLOAD;
        desc.HeapProperty.CpuPageProperty   = a3d::CPU_PAGE_PROPERTY_DEFAULT;

        for(auto i=0; i<2; ++i)
        {
            if ( !m_pDevice->CreateBuffer(&desc, &m_pCB[i]) )
            { return false; }

            a3d::BufferViewDesc viewDesc = {};
            viewDesc.Offset = 0;
            viewDesc.Range  = desc.Stride;

            if ( !m_pDevice->CreateBufferView(m_pCB[i], &viewDesc, &m_pCBV[i]) )
            { return false; }

            m_pProjection[i] = static_cast<Mat4*>(m_pCB[i]->Map());
        }
    }

    // �t�H���g�e�N�X�`���𐶐�.
    {
        uint8_t* pPixels;
        int width;
        int height;
        int bytePerPixel;

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->GetTexDataAsRGBA32(&pPixels, &width, &height, &bytePerPixel);

        auto rowPitch = width * bytePerPixel;
        auto size = rowPitch * height;

        a3d::BufferDesc bufDesc = {};
        bufDesc.Size                            = size;
        bufDesc.InitState                       = a3d::RESOURCE_STATE_GENERAL;
        bufDesc.Usage                           = a3d::RESOURCE_USAGE_COPY_SRC;
        bufDesc.HeapProperty.Type               = a3d::HEAP_TYPE_UPLOAD;
        bufDesc.HeapProperty.CpuPageProperty    = a3d::CPU_PAGE_PROPERTY_DEFAULT;

        a3d::IBuffer* pImmediate = nullptr;
        if (!m_pDevice->CreateBuffer(&bufDesc, &pImmediate))
        { return false; }

        a3d::TextureDesc desc = {};
        desc.Dimension                      = a3d::RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width                          = width;
        desc.Height                         = height;
        desc.DepthOrArraySize               = 1;
        desc.MipLevels                      = 1;
        desc.Format                         = a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM;
        desc.SampleCount                    = 1;
        desc.Layout                         = a3d::RESOURCE_LAYOUT_OPTIMAL;
        desc.InitState                      = a3d::RESOURCE_STATE_GENERAL;
        desc.Usage                          = a3d::RESOURCE_USAGE_SHADER_RESOURCE | a3d::RESOURCE_USAGE_COPY_DST;
        desc.HeapProperty.Type              = a3d::HEAP_TYPE_DEFAULT;
        desc.HeapProperty.CpuPageProperty   = a3d::CPU_PAGE_PROPERTY_DEFAULT;

        if (!m_pDevice->CreateTexture(&desc, &m_pTexture))
        {
            a3d::SafeRelease(pImmediate);
            return false;
        }

        a3d::TextureViewDesc viewDesc = {};
        viewDesc.Dimension          = a3d::VIEW_DIMENSION_TEXTURE2D;
        viewDesc.Format             = desc.Format;
        viewDesc.TextureAspect      = a3d::TEXTURE_ASPECT_COLOR;
        viewDesc.MipSlice           = 0;
        viewDesc.MipLevels          = desc.MipLevels;
        viewDesc.FirstArraySlice    = 0;
        viewDesc.ArraySize          = desc.DepthOrArraySize;
        viewDesc.ComponentMapping.R = a3d::TEXTURE_SWIZZLE_R;
        viewDesc.ComponentMapping.G = a3d::TEXTURE_SWIZZLE_G;
        viewDesc.ComponentMapping.B = a3d::TEXTURE_SWIZZLE_B;
        viewDesc.ComponentMapping.A = a3d::TEXTURE_SWIZZLE_A;

        if (!m_pDevice->CreateTextureView(m_pTexture, &viewDesc, &m_pTextureView))
        {
            a3d::SafeRelease(pImmediate);
            return false;
        }

        auto layout = m_pTexture->GetSubresourceLayout(0);

        auto dstPtr = static_cast<uint8_t*>(pImmediate->Map());
        assert( dstPtr != nullptr );

        auto srcPtr = pPixels;
        assert( srcPtr != nullptr );

        for(auto i=0; i<layout.RowCount; ++i)
        {
            memcpy(dstPtr, srcPtr, rowPitch);
            srcPtr += rowPitch;
            dstPtr += layout.RowPitch;
        }
        pImmediate->Unmap();

        a3d::Offset3D offset = {0, 0, 0};

        a3d::ICommandList* pCommandList;
        if (!m_pDevice->CreateCommandList(a3d::COMMANDLIST_TYPE_DIRECT, &pCommandList))
        { return false; }

        a3d::IQueue* pGraphicsQueue;
        m_pDevice->GetGraphicsQueue(&pGraphicsQueue);

        pCommandList->Begin();
        pCommandList->TextureBarrier(
            m_pTexture,
            a3d::RESOURCE_STATE_GENERAL,
            a3d::RESOURCE_STATE_COPY_DST);
        pCommandList->CopyBufferToTexture(
            m_pTexture,
            0,
            offset,
            pImmediate,
            0);
        pCommandList->TextureBarrier(
            m_pTexture,
            a3d::RESOURCE_STATE_COPY_DST,
            a3d::RESOURCE_STATE_SHADER_READ);
        pCommandList->End();
        pGraphicsQueue->Submit(pCommandList);
        pGraphicsQueue->Execute(nullptr);
        pGraphicsQueue->WaitIdle();

        a3d::SafeRelease(pCommandList);
        a3d::SafeRelease(pImmediate);
        a3d::SafeRelease(pGraphicsQueue);
    }

    // �T���v���[�𐶐�.
    {
        a3d::SamplerDesc desc = {};
        desc.AddressU           = a3d::TEXTURE_ADDRESS_MODE_CLAMP;
        desc.AddressV           = a3d::TEXTURE_ADDRESS_MODE_CLAMP;
        desc.AddressW           = a3d::TEXTURE_ADDRESS_MODE_CLAMP;
        desc.MinFilter          = a3d::FILTER_MODE_LINEAR;
        desc.MagFilter          = a3d::FILTER_MODE_LINEAR;
        desc.MipMapMode         = a3d::MIPMAP_MODE_LINEAR;
        desc.MinLod             = 0.0f;
        desc.AnisotropyEnable   = false;
        desc.MaxAnisotropy      = 1;
        desc.CompareEnable      = false;
        desc.CompareOp          = a3d::COMPARE_OP_NEVER;
        desc.MinLod             = 0.0f;
        desc.MaxLod             = FLT_MAX;
        desc.BorderColor        = a3d::BORDER_COLOR_TRANSPARENT_BLACK;

        if (!m_pDevice->CreateSampler(&desc, &m_pSampler))
        { return false; }
    }

    // �f�B�X�N���v�^�Z�b�g���C�A�E�g�𐶐�.
    {
    #if SAMPLE_IS_VULKAN || SAMPLE_IS_D3D12 || SAMPLE_IS_D3D11
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

        if (!m_pDevice->CreateDescriptorSetLayout(&desc, &m_pDescriptorSetLayout))
        { return false; }

        for(auto i=0; i<2; ++i)
        {
            if (!m_pDescriptorSetLayout->CreateDescriptorSet(&m_pDescriptorSet[i]))
            { return false; }

            m_pDescriptorSet[i]->SetView   (0, m_pCBV[i]);
            m_pDescriptorSet[i]->SetSampler(1, m_pSampler);
            m_pDescriptorSet[i]->SetView   (2, m_pTextureView);

        #if 1
            // DescriptorSet::Update()�͍폜�����\��ł�.
            m_pDescriptorSet[i]->Update();
        #endif
        }
    #else
        a3d::DescriptorSetLayoutDesc desc = {};
        desc.MaxSetCount               = 2;
        desc.EntryCount                = 2;
        
        desc.Entries[0].ShaderMask     = a3d::SHADER_MASK_VERTEX;
        desc.Entries[0].ShaderRegister = 0;
        desc.Entries[0].BindLocation   = 0;
        desc.Entries[0].Type           = a3d::DESCRIPTOR_TYPE_CBV;

        desc.Entries[1].ShaderMask     = a3d::SHADER_MASK_PIXEL;
        desc.Entries[1].ShaderRegister = 0;
        desc.Entries[1].BindLocation   = 0;
        desc.Entries[1].Type           = a3d::DESCRIPTOR_TYPE_SRV;

        if (!m_pDevice->CreateDescriptorSetLayout(&desc, &m_pDescriptorSetLayout))
        { return false; }

        for(auto i=0; i<2; ++i)
        {
            if (!m_pDescriptorSetLayout->CreateDescriptorSet(&m_pDescriptorSet[i]))
            { return false; }

            m_pDescriptorSet[i]->SetBuffer (0, m_pCBV[i]);
            m_pDescriptorSet[i]->SetSampler(1, m_pSampler);
            m_pDescriptorSet[i]->SetTexture(1, m_pTextureView, a3d::RESOURCE_STATE_SHADER_READ);
        }
    #endif
    }

    // �V�F�[�_�o�C�i����ǂݍ��݂܂�.
    a3d::ShaderBinary vs = {};
    a3d::ShaderBinary ps = {};
    {
        std::string dir = GetShaderDirectoryForSampleProgram();

    #if SAMPLE_IS_VULKAN
        const auto vsFilePath = dir + "imguiVS.spv";
        const auto psFilePath = dir + "imguiPS.spv";
    #elif SAMPLE_IS_D3D12 || SAMPLE_IS_D3D11
        const auto vsFilePath = dir + "imguiVS.cso";
        const auto psFilePath = dir + "imguiPS.cso";
    #else
        const auto vsFilePath = dir + "imguiVS.bin";
        const auto psFilePath = dir + "imguiPS.bin";
    #endif

        if (!LoadShaderBinary(vsFilePath.c_str(), vs))
        { return false; }

        if (!LoadShaderBinary(psFilePath.c_str(), ps))
        {
            DisposeShaderBinary(vs);
            return false;
        }
    }

    // �p�C�v���C���X�e�[�g�̐���.
    {
        // ���͗v�f�ł�.
        a3d::InputElementDesc inputElements[] = {
            { a3d::SEMANTICS_POSITION,  a3d::RESOURCE_FORMAT_R32G32_FLOAT   , 0,  0, a3d::INPUT_CLASSIFICATION_PER_VERTEX },
            { a3d::SEMANTICS_TEXCOORD0, a3d::RESOURCE_FORMAT_R32G32_FLOAT   , 0,  8, a3d::INPUT_CLASSIFICATION_PER_VERTEX },
            { a3d::SEMANTICS_COLOR0,    a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM , 0, 16, a3d::INPUT_CLASSIFICATION_PER_VERTEX }
        };

        // ���̓��C�A�E�g�ł�.
        a3d::InputLayoutDesc inputLayout = {};
        inputLayout.ElementCount = 3;
        inputLayout.pElements    = inputElements;

        // �X�e���V���X�e�[�g�ł�.
        a3d::StencilTestDesc stencilTest = {};
        stencilTest.StencilFailOp      = a3d::STENCIL_OP_KEEP;
        stencilTest.StencilDepthFailOp = a3d::STENCIL_OP_KEEP;
        stencilTest.StencilFailOp      = a3d::STENCIL_OP_KEEP;
        stencilTest.StencilCompareOp   = a3d::COMPARE_OP_NEVER;

        // �O���t�B�b�N�X�p�C�v���C���X�e�[�g��ݒ肵�܂�.
        a3d::GraphicsPipelineStateDesc desc = {};

        // �V�F�[�_�̐ݒ�.
        desc.VS = vs;
        desc.PS = ps;

        // �u�����h�X�e�[�g�̐ݒ�.
        desc.BlendState.IndependentBlendEnable          = false;
        desc.BlendState.LogicOpEnable                   = false;
        desc.BlendState.LogicOp                         = a3d::LOGIC_OP_NOOP;
        desc.BlendState.ColorTarget[0].BlendEnable      = true;
        desc.BlendState.ColorTarget[0].SrcBlend         = a3d::BLEND_FACTOR_SRC_ALPHA;
        desc.BlendState.ColorTarget[0].DstBlend         = a3d::BLEND_FACTOR_INV_SRC_ALPHA;
        desc.BlendState.ColorTarget[0].BlendOp          = a3d::BLEND_OP_ADD;
        desc.BlendState.ColorTarget[0].SrcBlendAlpha    = a3d::BLEND_FACTOR_INV_SRC_ALPHA;
        desc.BlendState.ColorTarget[0].DstBlendAlpha    = a3d::BLEND_FACTOR_ZERO;
        desc.BlendState.ColorTarget[0].BlendOpAlpha     = a3d::BLEND_OP_ADD;
        desc.BlendState.ColorTarget[0].EnableWriteR     = true;
        desc.BlendState.ColorTarget[0].EnableWriteG     = true;
        desc.BlendState.ColorTarget[0].EnableWriteB     = true;
        desc.BlendState.ColorTarget[0].EnableWriteA     = true;
        for(auto i=1; i<8; ++i)
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
        desc.RasterizerState.SlopeScaledDepthBias       = 0;
        desc.RasterizerState.DepthClipEnable            = false;
        desc.RasterizerState.EnableConservativeRaster   = false;
        
        // �}���`�T���v���X�e�[�g�̐ݒ�.
        desc.MultiSampleState.EnableAlphaToCoverage = false;
        desc.MultiSampleState.EnableMultiSample     = false;
        desc.MultiSampleState.SampleCount           = 1;

        // �[�x�X�e�[�g�̐ݒ�.
        desc.DepthState.DepthTestEnable      = false;
        desc.DepthState.DepthWriteEnable     = false;
        desc.DepthState.DepthCompareOp       = a3d::COMPARE_OP_NEVER;

        // �X�e���V���X�e�[�g�̐ݒ�.
        desc.StencilState.StencilTestEnable    = false;
        desc.StencilState.StencllReadMask      = 0;
        desc.StencilState.StencilWriteMask     = 0;
        desc.StencilState.FrontFace            = stencilTest;
        desc.StencilState.BackFace             = stencilTest;

        // �e�b�Z���[�V�����X�e�[�g�̐ݒ�.
        desc.TessellationState.PatchControlCount = 0;

        // ���̓A�E�g�̐ݒ�.
        desc.InputLayout = inputLayout;

        // �f�B�X�N���v�^�Z�b�g���C�A�E�g�̐ݒ�.
        desc.pLayout = m_pDescriptorSetLayout;
        
        // �v���~�e�B�u�g�|���W�[�̐ݒ�.
        desc.PrimitiveTopology = a3d::PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        // �t�H�[�}�b�g�̐ݒ�.
        desc.ColorCount = pFrameBuffer->GetDesc().ColorCount;
        desc.ColorTarget[0].Format      = pFrameBuffer->GetDesc().pColorTargets[0]->GetResource()->GetDesc().Format;
        desc.ColorTarget[0].SampleCount = pFrameBuffer->GetDesc().pColorTargets[0]->GetResource()->GetDesc().SampleCount;
        if (pFrameBuffer->GetDesc().pDepthTarget != nullptr)
        {
            desc.DepthTarget.Format      = pFrameBuffer->GetDesc().pDepthTarget->GetResource()->GetDesc().Format;
            desc.DepthTarget.SampleCount = pFrameBuffer->GetDesc().pDepthTarget->GetResource()->GetDesc().SampleCount;
        }
 
        // �L���b�V���ς݃p�C�v���C���X�e�[�g�̐ݒ�.
        desc.pCachedPSO = nullptr;

        // �O���t�B�b�N�X�p�C�v���C���X�e�[�g�̐���.
        if (!m_pDevice->CreateGraphicsPipeline(&desc, &m_pPipelineState))
        { return false; }
    }
    // �s�v�ɂȂ����̂Ŕj�����܂�.
    DisposeShaderBinary(vs);
    DisposeShaderBinary(ps);

    // �R�[���o�b�N�̓o�^.
    {
        static auto mouse = [](int x, int y, int wheelDelta, bool isDownL, bool isDownM, bool isDownR, void* pUser)
        {
            auto instance = static_cast<GuiMgr*>(pUser);
            assert(instance != nullptr);

            instance->OnMouse(x, y, wheelDelta, isDownL, isDownM, isDownR);
        };

        static auto keyboard = [](bool isKeyDown, bool isAltDown, uint32_t keyCode, void* pUser)
        {
            auto instance = static_cast<GuiMgr*>(pUser);
            assert(instance != nullptr);

            instance->OnKeyboard(keyCode, isKeyDown, isAltDown);
        };

        static auto character = [](uint32_t keyCode, void* pUser)
        {
            auto instance = static_cast<GuiMgr*>(pUser);
            assert(instance != nullptr);

            instance->OnChar(keyCode);
        };

        pApp->SetMouseCallback(mouse, this);
        pApp->SetKeyboardCallback(keyboard, this);
        pApp->SetCharCallback(character, this);
    }

    // ImGui�̏�����.
    {
        auto& io = ImGui::GetIO();
    #if A3D_IS_WIN
        io.KeyMap[ImGuiKey_Tab]         = VK_TAB;
        io.KeyMap[ImGuiKey_LeftArrow]   = VK_LEFT;
        io.KeyMap[ImGuiKey_RightArrow]  = VK_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow]     = VK_UP;
        io.KeyMap[ImGuiKey_DownArrow]   = VK_DOWN;
        io.KeyMap[ImGuiKey_PageUp]      = VK_PRIOR;
        io.KeyMap[ImGuiKey_PageDown]    = VK_NEXT;
        io.KeyMap[ImGuiKey_Home]        = VK_HOME;
        io.KeyMap[ImGuiKey_End]         = VK_END;
        io.KeyMap[ImGuiKey_Delete]      = VK_DELETE;
        io.KeyMap[ImGuiKey_Backspace]   = VK_BACK;
        io.KeyMap[ImGuiKey_Enter]       = VK_RETURN;
        io.KeyMap[ImGuiKey_Escape]      = VK_ESCAPE;
    #else
    #endif
        io.KeyMap[ImGuiKey_A] = 'A';
        io.KeyMap[ImGuiKey_C] = 'C';
        io.KeyMap[ImGuiKey_V] = 'V';
        io.KeyMap[ImGuiKey_X] = 'X';
        io.KeyMap[ImGuiKey_Y] = 'Y';
        io.KeyMap[ImGuiKey_Z] = 'Z';

        io.RenderDrawListsFn  = Draw;
        io.SetClipboardTextFn = nullptr;
        io.GetClipboardTextFn = nullptr;
        io.ImeWindowHandle    = pApp->GetWindowHandle();
        io.DisplaySize.x      = float(pApp->GetWidth());
        io.DisplaySize.y      = float(pApp->GetHeight());

        io.Fonts->TexID = reinterpret_cast<void*>(m_pTextureView);
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
        io.DeltaTime   = 1.0f / 60.0f;
        io.Framerate   = 0.5f;

        auto& style = ImGui::GetStyle();
        style.WindowRounding      = 2.0f;

    #if !(SAMPLE_IS_VULKAN || SAMPLE_IS_D3D12 || SAMPLE_IS_D3D11)
        io.MouseDrawCursor = true;
    #endif

        style.Colors[ImGuiCol_Text]                 = ImVec4(1.000000f, 1.000000f, 1.000000f, 1.000000f);
        style.Colors[ImGuiCol_TextDisabled]         = ImVec4(0.400000f, 0.400000f, 0.400000f, 1.000000f);
        style.Colors[ImGuiCol_WindowBg]             = ImVec4(0.060000f, 0.060000f, 0.060000f, 0.752000f);
        style.Colors[ImGuiCol_ChildWindowBg]        = ImVec4(1.000000f, 1.000000f, 1.000000f, 0.000000f);
        style.Colors[ImGuiCol_PopupBg]              = ImVec4(0.000000f, 0.000000f, 0.000000f, 0.752000f);
        style.Colors[ImGuiCol_Border]               = ImVec4(1.000000f, 1.000000f, 1.000000f, 0.312000f);
        style.Colors[ImGuiCol_BorderShadow]         = ImVec4(0.000000f, 0.000000f, 0.000000f, 0.080000f);
        style.Colors[ImGuiCol_FrameBg]              = ImVec4(0.800000f, 0.800000f, 0.800000f, 0.300000f);
        style.Colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.260000f, 0.590000f, 0.980000f, 0.320000f);
        style.Colors[ImGuiCol_FrameBgActive]        = ImVec4(0.260000f, 0.590000f, 0.980000f, 0.536000f);
        style.Colors[ImGuiCol_TitleBg]              = ImVec4(0.000000f, 0.250000f, 0.500000f, 0.500000f);
        style.Colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.000000f, 0.000000f, 0.500000f, 0.500000f);
        style.Colors[ImGuiCol_TitleBgActive]        = ImVec4(0.000000f, 0.500000f, 1.000000f, 0.800000f);
        style.Colors[ImGuiCol_MenuBarBg]            = ImVec4(0.140000f, 0.140000f, 0.140000f, 1.000000f);
        style.Colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.020000f, 0.020000f, 0.020000f, 0.424000f);
        style.Colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.310000f, 0.310000f, 0.310000f, 1.000000f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.410000f, 0.410000f, 0.410000f, 1.000000f);
        style.Colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.510000f, 0.510000f, 0.510000f, 1.000000f);
        style.Colors[ImGuiCol_CheckMark]            = ImVec4(0.260000f, 0.590000f, 0.980000f, 1.000000f);
        style.Colors[ImGuiCol_SliderGrab]           = ImVec4(0.240000f, 0.520000f, 0.880000f, 1.000000f);
        style.Colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.260000f, 0.590000f, 0.980000f, 1.000000f);
        style.Colors[ImGuiCol_Button]               = ImVec4(0.260000f, 0.590000f, 0.980000f, 0.320000f);
        style.Colors[ImGuiCol_ButtonHovered]        = ImVec4(0.260000f, 0.590000f, 0.980000f, 1.000000f);
        style.Colors[ImGuiCol_ButtonActive]         = ImVec4(0.060000f, 0.530000f, 0.980000f, 1.000000f);
        style.Colors[ImGuiCol_Header]               = ImVec4(0.260000f, 0.590000f, 0.980000f, 0.248000f);
        style.Colors[ImGuiCol_HeaderHovered]        = ImVec4(0.260000f, 0.590000f, 0.980000f, 0.640000f);
        style.Colors[ImGuiCol_HeaderActive]         = ImVec4(0.260000f, 0.590000f, 0.980000f, 1.000000f);
        style.Colors[ImGuiCol_ResizeGrip]           = ImVec4(0.000000f, 0.000000f, 0.000000f, 0.400000f);
        style.Colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.260000f, 0.590000f, 0.980000f, 0.536000f);
        style.Colors[ImGuiCol_ResizeGripActive]     = ImVec4(0.260000f, 0.590000f, 0.980000f, 0.760000f);
        style.Colors[ImGuiCol_PlotLines]            = ImVec4(0.610000f, 0.610000f, 0.610000f, 1.000000f);
        style.Colors[ImGuiCol_PlotLinesHovered]     = ImVec4(1.000000f, 0.430000f, 0.350000f, 1.000000f);
        style.Colors[ImGuiCol_PlotHistogram]        = ImVec4(0.900000f, 0.700000f, 0.000000f, 1.000000f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.000000f, 0.600000f, 0.000000f, 1.000000f);
        style.Colors[ImGuiCol_TextSelectedBg]       = ImVec4(0.260000f, 0.590000f, 0.980000f, 0.280000f);
        style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.800000f, 0.800000f, 0.800000f, 0.280000f);
    }

    m_LastTime = std::chrono::system_clock::now();

    return true;
}

//-------------------------------------------------------------------------------------------------
//      �I���������s���܂�.
//-------------------------------------------------------------------------------------------------
void GuiMgr::Term()
{
    for(auto i=0; i<2; ++i)
    {
        a3d::SafeRelease(m_pVB[i]);
        a3d::SafeRelease(m_pIB[i]);
        a3d::SafeRelease(m_pCB[i]);
        a3d::SafeRelease(m_pCBV[i]);
        a3d::SafeRelease(m_pDescriptorSet[i]);
        m_SizeVB[i] = 0;
        m_SizeIB[i] = 0;
    }

    a3d::SafeRelease(m_pSampler);
    a3d::SafeRelease(m_pTextureView);
    a3d::SafeRelease(m_pTexture);
    a3d::SafeRelease(m_pDescriptorSetLayout);
    a3d::SafeRelease(m_pPipelineState);
    m_pCommandList = nullptr;
    a3d::SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      �o�b�t�@�����ւ��܂�.
//-------------------------------------------------------------------------------------------------
void GuiMgr::SwapBuffers()
{
    auto time = std::chrono::system_clock::now();
    auto elapsedMilliSec = std::chrono::duration_cast<std::chrono::milliseconds>(time - m_LastTime).count();
    auto elapsedSec = float(elapsedMilliSec / 1000.0);

    auto& io = ImGui::GetIO();
    io.DeltaTime = elapsedSec;
    io.DisplaySize.x = float(m_pApp->GetWidth());
    io.DisplaySize.y = float(m_pApp->GetHeight());
    
    m_BufferIndex = (m_BufferIndex + 1) % 2;
    ImGui::NewFrame();

    m_LastTime = time;
}

//-------------------------------------------------------------------------------------------------
//      �R�}���h�𔭍s���܂�.
//-------------------------------------------------------------------------------------------------
void GuiMgr::Issue(a3d::ICommandList* pCommandList)
{
    m_pCommandList = pCommandList;
    ImGui::Render();
}

//-------------------------------------------------------------------------------------------------
//      �`�揈�����s���܂�.
//-------------------------------------------------------------------------------------------------
void GuiMgr::Draw(ImDrawData* pData)
{ s_Instance.OnDraw(pData); }

//-------------------------------------------------------------------------------------------------
//      �`�揈�����s���܂�.
//-------------------------------------------------------------------------------------------------
void GuiMgr::OnDraw(ImDrawData* pData)
{
    auto& io = ImGui::GetIO();
    auto width  = static_cast<int>(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    auto height = static_cast<int>(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if ( width == 0 || height == 0 )
    { return; }

    size_t vtxSize = pData->TotalVtxCount * sizeof(ImDrawVert);
    size_t idxSize = pData->TotalIdxCount * sizeof(ImDrawIdx);

    // �ő�T�C�Y�𒴂���ꍇ�͕`�悹���ɏI��.
    if (vtxSize >= MaxPrimitiveCount * sizeof(ImDrawVert) * 4 ||
        idxSize >= MaxPrimitiveCount * sizeof(ImDrawIdx) * 6)
    { return; }

    // ���_�ƃC���f�b�N�X�̍X�V.
    {
        auto pVtxDst = static_cast<ImDrawVert*>(m_pVB[m_BufferIndex]->Map());
        auto pIdxDst = static_cast<ImDrawIdx*> (m_pIB[m_BufferIndex]->Map());

        for(auto n=0; n<pData->CmdListsCount; ++n)
        {
            const auto pList = pData->CmdLists[n];
            memcpy(pVtxDst, &pList->VtxBuffer[0], pList->VtxBuffer.size() * sizeof(ImDrawVert));
            memcpy(pIdxDst, &pList->IdxBuffer[0], pList->IdxBuffer.size() * sizeof(ImDrawIdx));
            pVtxDst += pList->VtxBuffer.size();
            pIdxDst += pList->IdxBuffer.size();
        }

        m_pVB[m_BufferIndex]->Unmap();
        m_pIB[m_BufferIndex]->Unmap();
    }

    // �p�C�v���C���X�e�[�g�ƃf�B�X�N���v�^�Z�b�g��ݒ�.
    {
        m_pCommandList->SetPipelineState(m_pPipelineState);
        m_pCommandList->SetDescriptorSet(m_pDescriptorSet[m_BufferIndex]);
    }

    // ���_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@��ݒ�.
    {
        uint64_t offset = 0;
        m_pCommandList->SetVertexBuffers(0, 1, &m_pVB[m_BufferIndex], &offset);
        m_pCommandList->SetIndexBuffer(m_pIB[m_BufferIndex], 0);
    }

    // �r���[�|�[�g��ݒ�.
    {
        a3d::Viewport viewport = {};
        viewport.X          = 0;
        viewport.Y          = 0;
        viewport.Width      = io.DisplaySize.x;
        viewport.Height     = io.DisplaySize.y;
        viewport.MinDepth   = 0.0f;
        viewport.MaxDepth   = 1.0f;
        m_pCommandList->SetViewports(1, &viewport);
    }

    // �萔�o�b�t�@���X�V.
    {
        float L = 0.0f;
        float R = ImGui::GetIO().DisplaySize.x;
        float B = ImGui::GetIO().DisplaySize.y;
        float T = 0.0f;
        float mvp[4][4] =
        {
            { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
            { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
            { 0.0f,         0.0f,           0.5f,       0.0f },
            { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
        };
        memcpy( m_pProjection[m_BufferIndex], mvp, sizeof(float) * 16 );
    }

    // �`��R�}���h�𐶐�.
    {
        int vtxOffset = 0;
        int idxOffset = 0;
        for(auto n=0; n<pData->CmdListsCount; ++n)
        {
            const auto pList = pData->CmdLists[n];
            for(auto i =0; i<pList->CmdBuffer.size(); ++i)
            {
                auto pCmd = &pList->CmdBuffer[i];
                if (pCmd->UserCallback)
                { pCmd->UserCallback(pList, pCmd); }
                else
                {
                    auto x = static_cast<int>(pCmd->ClipRect.x);
                    auto y = static_cast<int>(pCmd->ClipRect.y);
                    auto w = static_cast<int>(pCmd->ClipRect.z - pCmd->ClipRect.x);
                    auto h = static_cast<int>(pCmd->ClipRect.w - pCmd->ClipRect.y);

                    if ( x < 0 ) { x = 0; }
                    if ( y < 0 ) { y = 0; }
                    if ( w < 1 ) { w = 1; }
                    if ( h < 1 ) { h = 1; }

                    a3d::Rect scissor = {};
                    scissor.Offset.X        = x;
                    scissor.Offset.Y        = y;
                    scissor.Extent.Width    = uint32_t(w);
                    scissor.Extent.Height   = uint32_t(h);

                    m_pCommandList->SetScissors(1, &scissor);
                    m_pCommandList->DrawIndexedInstanced(pCmd->ElemCount, 1, idxOffset, vtxOffset, 0);
                }
                idxOffset += pCmd->ElemCount;
            }
            vtxOffset += pList->VtxBuffer.size();
        }
    }
}

//-------------------------------------------------------------------------------------------------
//      �}�E�X�R�[���o�b�N�ł�.
//-------------------------------------------------------------------------------------------------
void GuiMgr::OnMouse(int x, int y, int wheelDelta, bool isDownL, bool isDownM, bool isDownR)
{
    auto& io = ImGui::GetIO();

    io.MousePosPrev = io.MousePos;
    io.MousePos = ImVec2( float(x), float(y) );
    io.MouseDown[0] = isDownL;
    io.MouseDown[1] = isDownR;
    io.MouseDown[2] = isDownM;
    io.MouseDown[3] = false;
    io.MouseDown[4] = false;
    io.MouseWheel   += (wheelDelta > 0) ? 1.0f : -1.0f;
}

//-------------------------------------------------------------------------------------------------
//      �L�[�{�[�h�R�[���o�b�N�ł�.
//-------------------------------------------------------------------------------------------------
void GuiMgr::OnKeyboard(uint32_t keyCode, bool isKeyDown, bool isAltDown)
{
    auto& io = ImGui::GetIO();

    io.KeysDown[keyCode] = isKeyDown;
    io.KeyAlt   = isAltDown;
    io.KeyCtrl  = false;
    io.KeyShift = false;
    io.KeySuper = false;
}

//-------------------------------------------------------------------------------------------------
//      �������̓R�[���o�b�N�ł�.
//-------------------------------------------------------------------------------------------------
void GuiMgr::OnChar(uint32_t keyCode)
{
    if (keyCode > 0 && keyCode < 0x10000)
    {
        auto& io = ImGui::GetIO();
        io.AddInputCharacter(ImWchar(keyCode));
    }
}
