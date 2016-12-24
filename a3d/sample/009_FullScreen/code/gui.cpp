//-------------------------------------------------------------------------------------------------
// File : gui.cpp
// Desc : ImGui Helper.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "gui.h"
#include <SampleUtil.h>

#if A3D_IS_WIN
    #include <Windows.h>
#elif A3D_IS_LINUX

#endif

namespace /* anonymous */ {

void MyDraw(ImDrawData* pDrawData)
{ GuiMgr::GetInstance().Draw(pDrawData); }

} // namespace /* anonymous */

///////////////////////////////////////////////////////////////////////////////////////////////////
// GuiMgr class
///////////////////////////////////////////////////////////////////////////////////////////////////
GuiMgr GuiMgr::s_Instance;

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
GuiMgr::GuiMgr()
: m_pDevice             (nullptr)
, m_pConstantBuffer     (nullptr)
, m_pConstantView       (nullptr)
, m_pSampler            (nullptr)
, m_pTexture            (nullptr)
, m_pTextureView        (nullptr)
, m_pDescriptorSetLayout(nullptr)
, m_pDescriptorSet      (nullptr)
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
    }
}

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
GuiMgr::~GuiMgr()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-------------------------------------------------------------------------------------------------
GuiMgr& GuiMgr::GetInstance()
{ return s_Instance; }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool GuiMgr::Init(a3d::IDevice* pDevice, a3d::IFrameBuffer* pFrameBuffer, IApp* pApp)
{
    if (pDevice == nullptr || pApp == nullptr)
    { return false; }

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    // 頂点バッファを生成.
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

    // インデックスバッファを生成.
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

    // 定数バッファを生成.
    {
        auto info = m_pDevice->GetInfo();

        a3d::BufferDesc desc = {};
        desc.Size                           = a3d::RoundUp<uint64_t>( sizeof(Mat4), info.ConstantBufferMemoryAlignment );
        desc.Stride                         = a3d::RoundUp<uint32_t>( sizeof(Mat4), info.ConstantBufferMemoryAlignment );
        desc.InitState                      = a3d::RESOURCE_STATE_GENERAL;
        desc.Usage                          = a3d::RESOURCE_USAGE_CONSTANT_BUFFER;
        desc.HeapProperty.Type              = a3d::HEAP_TYPE_UPLOAD;
        desc.HeapProperty.CpuPageProperty   = a3d::CPU_PAGE_PROPERTY_DEFAULT;

        if ( !m_pDevice->CreateBuffer(&desc, &m_pConstantBuffer) )
        { return false; }

        a3d::BufferViewDesc viewDesc = {};
        viewDesc.Offset = 0;
        viewDesc.Range  = desc.Stride;

        if ( !m_pDevice->CreateBufferView(m_pConstantBuffer, &viewDesc, &m_pConstantView) )
        { return false; }

        m_pProjection = static_cast<Mat4*>(m_pConstantBuffer->Map());
    }

    // フォントテクスチャを生成.
    {
        uint8_t* pPixels;
        int width;
        int height;
        int bytePerPixel;
        
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
        if (!m_pDevice->CreateCommandList(a3d::COMMANDLIST_TYPE_DIRECT, nullptr, &pCommandList))
        { return false; }

        a3d::IQueue* pGraphicsQueue;
        m_pDevice->GetGraphicsQueue(&pGraphicsQueue);

        pCommandList->Begin();
        pCommandList->TextureBarrier(m_pTexture, a3d::RESOURCE_STATE_COPY_DST);
        pCommandList->CopyBufferToTexture(m_pTexture, 0, offset, pImmediate, 0);
        pCommandList->TextureBarrier(m_pTexture, a3d::RESOURCE_STATE_SHADER_READ);
        pCommandList->End();
        pGraphicsQueue->Submit(pCommandList);
        pGraphicsQueue->Execute(nullptr);
        pGraphicsQueue->WaitIdle();

        a3d::SafeRelease(pCommandList);
        a3d::SafeRelease(pImmediate);
        a3d::SafeRelease(pGraphicsQueue);
    }

    // サンプラーを生成.
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
        desc.MaxAnisotropy      = 16;
        desc.CompareEnable      = false;
        desc.CompareOp          = a3d::COMPARE_OP_NEVER;
        desc.MinLod             = 0.0f;
        desc.MaxLod             = FLT_MAX;
        desc.BorderColor        = a3d::BORDER_COLOR_TRANSPARENT_BLACK;

        if (!m_pDevice->CreateSampler(&desc, &m_pSampler))
        { return false; }
    }

    // ディスクリプタセットレイアウトを生成.
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

        if (!m_pDevice->CreateDescriptorSetLayout(&desc, &m_pDescriptorSetLayout))
        { return false; }

        if (!m_pDescriptorSetLayout->CreateDescriptorSet(&m_pDescriptorSet))
        { return false; }

        m_pDescriptorSet->SetBuffer (0, m_pConstantView);
        m_pDescriptorSet->SetSampler(1, m_pSampler);
        m_pDescriptorSet->SetTexture(2, m_pTextureView);
        m_pDescriptorSet->Update();
    }

    // シェーダバイナリを読み込みます.
    a3d::ShaderBinary vs = {};
    a3d::ShaderBinary ps = {};
    {
    #if SAMPLE_IS_VULKAN
        const auto vsFilePath = "../res/glsl/imguiVS.spv";
        const auto psFilePath = "../res/glsl/imguiPS.spv";
    #else
        const auto vsFilePath = "../res/hlsl/imguiVS.cso";
        const auto psFilePath = "../res/hlsl/imguiPS.cso";
    #endif

        if (!LoadShaderBinary(vsFilePath, vs))
        { return false; }

        if (!LoadShaderBinary(psFilePath, ps))
        {
            DisposeShaderBinary(vs);
            return false;
        }
    }

    // パイプラインステートの生成.
    {
        // 入力要素です.
        a3d::InputElementDesc inputElements[] = {
            { "POSITION", 0, 0, a3d::RESOURCE_FORMAT_R32G32_FLOAT   , 0  },
            { "TEXCOORD", 0, 1, a3d::RESOURCE_FORMAT_R32G32_FLOAT   , 8  },
            { "COLOR"   , 0, 2, a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM , 16 }
        };

        // 入力ストリームです.
        a3d::InputStreamDesc inputStream = {};
        inputStream.ElementCount    = 3;
        inputStream.pElements       = inputElements;
        inputStream.StreamIndex     = 0;
        inputStream.StrideInBytes   = sizeof(ImDrawVert);
        inputStream.InputClass      = a3d::INPUT_CLASSIFICATION_PER_VERTEX;

        // 入力レイアウトです.
        a3d::InputLayoutDesc inputLayout = {};
        inputLayout.StreamCount = 1;
        inputLayout.pStreams    = &inputStream;

        // ステンシルステートです.
        a3d::StencilState stencilState = {};
        stencilState.StencilFailOp      = a3d::STENCIL_OP_KEEP;
        stencilState.StencilDepthFailOp = a3d::STENCIL_OP_KEEP;
        stencilState.StencilFailOp      = a3d::STENCIL_OP_KEEP;
        stencilState.StencilCompareOp   = a3d::COMPARE_OP_NEVER;

        // グラフィックスパイプラインステートを設定します.
        a3d::GraphicsPipelineStateDesc desc = {};

        // シェーダの設定.
        desc.VertexShader = vs;
        desc.PixelShader  = ps;

        // ブレンドステートの設定.
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

        // ラスタライザ―ステートの設定.
        desc.RasterizerState.PolygonMode                = a3d::POLYGON_MODE_SOLID;
        desc.RasterizerState.CullMode                   = a3d::CULL_MODE_NONE;
        desc.RasterizerState.FrontCounterClockWise      = false;
        desc.RasterizerState.DepthBias                  = 0;
        desc.RasterizerState.DepthBiasClamp             = 0.0f;
        desc.RasterizerState.SlopeScaledDepthBais       = 0;
        desc.RasterizerState.DepthClipEnable            = true;
        desc.RasterizerState.EnableConservativeRaster   = false;
        
        // マルチサンプルステートの設定.
        desc.MultiSampleState.EnableAlphaToCoverage = false;
        desc.MultiSampleState.EnableMultiSample     = false;
        desc.MultiSampleState.SampleCount           = 1;

        // 深度ステンシルステートの設定.
        desc.DepthStencilState.DepthTestEnable      = false;
        desc.DepthStencilState.DepthWriteEnable     = false;
        desc.DepthStencilState.DepthCompareOp       = a3d::COMPARE_OP_NEVER;
        desc.DepthStencilState.StencilTestEnable    = false;
        desc.DepthStencilState.StencllReadMask      = 0;
        desc.DepthStencilState.StencilWriteMask     = 0;
        desc.DepthStencilState.FrontFace            = stencilState;
        desc.DepthStencilState.BackFace             = stencilState;

        // テッセレーションステートの設定.
        desc.TessellationState.PatchControlCount = 0;

        // 入力アウトの設定.
        desc.InputLayout = inputLayout;

        // ディスクリプタセットレイアウトの設定.
        desc.pLayout = m_pDescriptorSetLayout;
        
        // プリミティブトポロジーの設定.
        desc.PrimitiveTopology = a3d::PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        // フレームバッファの設定.
        desc.pFrameBuffer = pFrameBuffer;

        // キャッシュ済みパイプラインステートの設定.
        desc.pCachedPSO = nullptr;

        // グラフィックスパイプラインステートの生成.
        if (!m_pDevice->CreateGraphicsPipeline(&desc, &m_pPipelineState))
        { return false; }
    }
    // 不要になったので破棄します.
    DisposeShaderBinary(vs);
    DisposeShaderBinary(ps);

    // コールバックの登録.
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

    // ImGuiの初期化.
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

        io.RenderDrawListsFn  = MyDraw;
        io.SetClipboardTextFn = nullptr;
        io.GetClipboardTextFn = nullptr;
        io.ImeWindowHandle    = pApp->GetWindowHandle();
        io.DisplaySize.x      = float(pApp->GetWidth());
        io.DisplaySize.y      = float(pApp->GetHeight());

        io.Fonts->TexID = reinterpret_cast<void*>(m_pTextureView);

        ImGui::NewFrame();
    }

    m_LastTime = std::chrono::system_clock::now();

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void GuiMgr::Term()
{
    for(auto i=0; i<2; ++i)
    {
        a3d::SafeRelease(m_pVB[i]);
        a3d::SafeRelease(m_pIB[i]);
        m_SizeVB[i] = 0;
        m_SizeIB[i] = 0;
    }

    a3d::SafeRelease(m_pConstantView);
    a3d::SafeRelease(m_pConstantBuffer);
    a3d::SafeRelease(m_pSampler);
    a3d::SafeRelease(m_pTextureView);
    a3d::SafeRelease(m_pTexture);
    a3d::SafeRelease(m_pDescriptorSet);
    a3d::SafeRelease(m_pDescriptorSetLayout);
    a3d::SafeRelease(m_pPipelineState);
    m_pCommandList = nullptr;
    a3d::SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      バッファを入れ替えます.
//-------------------------------------------------------------------------------------------------
void GuiMgr::SwapBuffers()
{
    auto time = std::chrono::system_clock::now();
    auto elapsedMilliSec = std::chrono::duration_cast<std::chrono::milliseconds>(time - m_LastTime).count();
    auto elapsedSec = float(elapsedMilliSec / 1000.0);

    auto& io = ImGui::GetIO();
    io.DeltaTime = elapsedSec;
    
    m_BufferIndex = (m_BufferIndex + 1) % 2;
    ImGui::NewFrame();

    m_LastTime = time;
}

//-------------------------------------------------------------------------------------------------
//      コマンドを発行します.
//-------------------------------------------------------------------------------------------------
void GuiMgr::Issue(a3d::ICommandList* pCommandList)
{
    m_pCommandList = pCommandList;
    ImGui::Render();
}

//-------------------------------------------------------------------------------------------------
//      描画処理を行います.
//-------------------------------------------------------------------------------------------------
void GuiMgr::Draw(ImDrawData* pData)
{
    auto& io = ImGui::GetIO();
    auto width  = static_cast<int>(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    auto height = static_cast<int>(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if ( width == 0 || height == 0 )
    { return; }

    size_t vtxSize = pData->TotalVtxCount * sizeof(ImDrawVert);
    size_t idxSize = pData->TotalIdxCount * sizeof(ImDrawIdx);

    // 最大サイズを超える場合は描画せずに終了.
    if (vtxSize >= MaxPrimitiveCount * sizeof(ImDrawVert) * 4 ||
        idxSize >= MaxPrimitiveCount * sizeof(ImDrawIdx) * 6)
    { return; }

    // 頂点とインデックスの更新.
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

    // パイプラインステートとディスクリプタセット・ディスクリプタセットレイアウトを設定.
    {
        m_pCommandList->SetPipelineState(m_pPipelineState);
        m_pCommandList->SetDescriptorSetLayout(m_pDescriptorSetLayout);
        m_pCommandList->SetDescriptorSet(m_pDescriptorSet);
    }

    // 頂点バッファとインデックスバッファを設定.
    {
        uint64_t offset = 0;
        m_pCommandList->SetVertexBuffers(0, 1, &m_pVB[m_BufferIndex], &offset);
        m_pCommandList->SetIndexBuffer(m_pIB[m_BufferIndex], 0);
    }

    // ビューポートを設定.
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

    // 定数バッファを更新.
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
        memcpy( m_pProjection, mvp, sizeof(float) * 16 );
    }

    // 描画コマンドを生成.
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
                    a3d::Rect scissor = {};
                    scissor.Offset.X      = static_cast<int>(pCmd->ClipRect.x);
                    scissor.Offset.Y      = static_cast<int>(pCmd->ClipRect.y);
                    scissor.Extent.Width  = static_cast<uint32_t>(pCmd->ClipRect.z - pCmd->ClipRect.x);
                    scissor.Extent.Height = static_cast<uint32_t>(pCmd->ClipRect.w - pCmd->ClipRect.y);
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
//      マウスコールバックです.
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
//      キーボードコールバックです.
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
//      文字入力コールバックです.
//-------------------------------------------------------------------------------------------------
void GuiMgr::OnChar(uint32_t keyCode)
{
    if (keyCode > 0 && keyCode < 0x10000)
    {
        auto& io = ImGui::GetIO();
        io.AddInputCharacter(ImWchar(keyCode));
    }
}
