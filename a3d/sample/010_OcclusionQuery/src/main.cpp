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
#include <string>
#include <SampleApp.h>
#include <SampleUtil.h>
#include <SampleMath.h>
#include <SampleGui.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    Vec3 Position;  //!< 位置座標です.
    Vec4 Color;     //!< 頂点カラーです.
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Transform structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Transform
{
    Mat4 World;     //!< ワールド行列です.
    Mat4 View;      //!< ビュー行列です.
    Mat4 Proj;      //!< 射影行列です.
};


//-------------------------------------------------------------------------------------------------
// Forward declarations.
//-------------------------------------------------------------------------------------------------
bool InitA3D();
void TermA3D();
void DrawA3D();
void Resize(uint32_t w, uint32_t h, void* ptr);

//-------------------------------------------------------------------------------------------------
// Global Varaibles.
//-------------------------------------------------------------------------------------------------
IApp*                       g_pApp                  = nullptr;  //!< ウィンドウの生成を行うヘルパークラスです.
a3d::IDevice*               g_pDevice               = nullptr;  //!< デバイスです.
a3d::ISwapChain*            g_pSwapChain            = nullptr;  //!< スワップチェインです.
a3d::IQueue*                g_pGraphicsQueue        = nullptr;  //!< コマンドキューです.
a3d::IFence*                g_pFence                = nullptr;  //!< フェンスです.
a3d::IDescriptorSetLayout*  g_pDescriptorSetLayout  = nullptr;  //!< ディスクリプタセットレイアウトです.
a3d::IPipelineState*        g_pPipelineState        = nullptr;  //!< パイプラインステートです.
a3d::IBuffer*               g_pVertexBuffer         = nullptr;  //!< 頂点バッファです.
a3d::IBuffer*               g_pIndexBuffer          = nullptr;  //!< インデックスバッファです.
a3d::ITexture*              g_pColorBuffer[2]       = {};       //!< カラーバッファです.
a3d::ITextureView*          g_pColorView[2]         = {};       //!< カラービューです.
a3d::ICommandList*          g_pCommandList[2]       = {};       //!< コマンドリストです.
a3d::IFrameBuffer*          g_pFrameBuffer[2]       = {};       //!< フレームバッファです.
a3d::IBuffer*               g_pConstantBuffer[2]    = {};       //!< 定数バッファです.
a3d::IBufferView*           g_pConstantView[2]      = {};       //!< 定数バッファビューです.
a3d::IDescriptorSet*        g_pDescriptorSet[2]     = {};       //!< ディスクリプタセットです.
a3d::Viewport               g_Viewport              = {};       //!< ビューポートです.
a3d::Rect                   g_Scissor               = {};       //!< シザー矩形です.
a3d::IQueryPool*            g_pQueryPool[2]         = {};       //!< クエリプールです.
a3d::IBuffer*               g_pQueryResultBuffer[2] = {};       //!< クエリ結果を格納するバッファです.
Transform                   g_Transform             = {};       //!< 変換行列です.
void*                       g_pCbHead[2]            = {};       //!< 定数バッファの先頭ポインタです.
float                       g_RotateAngle           = 0.0f;     //!< 回転角です.
bool                        g_Prepare               = false;    //!< 準備が完了したらtrue.
uint32_t                    g_PrevBufferIndex       = 0;


///////////////////////////////////////////////////////////////////////////////////////////////////
// Allocator class
///////////////////////////////////////////////////////////////////////////////////////////////////
class Allocator : public a3d::IAllocator
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NONTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOHTING */

    //=============================================================================================
    // public methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //      メモリを確保します.
    //---------------------------------------------------------------------------------------------
    void* Alloc(size_t size, size_t alignment) noexcept override
    {
        #if A3D_IS_WIN
            return _aligned_malloc(size, alignment);
        #else
            return aligned_alloc(alignment, size);
        #endif
    }

    //---------------------------------------------------------------------------------------------
    //      メモリを再確保します.
    //---------------------------------------------------------------------------------------------
    void* Realloc(void* ptr, size_t size, size_t alignment) noexcept override
    {
        #if A3D_IS_WIN
            return _aligned_realloc(ptr, size, alignment);
        #else
            auto allocSize = a3d::RoundUp(size, alignment);
            return realloc(ptr, allocSize);
        #endif
    }

    //---------------------------------------------------------------------------------------------
    //      メモリを解放します.
    //---------------------------------------------------------------------------------------------
    void Free(void* ptr) noexcept override
    {
        #if A3D_IS_WIN
            _aligned_free(ptr);
        #else
            free(ptr);
        #endif
    }
} g_Allocator;


//-------------------------------------------------------------------------------------------------
//      メインエントリーポイントです.
//-------------------------------------------------------------------------------------------------
void Main()
{
    // ウィンドウ生成.
    if (!CreateApp(960, 540, &g_pApp))
    { return; }

    g_pApp->SetResizeCallback(Resize, nullptr);

    // A3D初期化.
    if (!InitA3D())
    {
        g_pApp->Release();
        return;
    }

    // メインループ.
    while( g_pApp->IsLoop() )
    {
        // 描画処理.
        DrawA3D();
    }

    // 後始末.
    TermA3D();
    g_pApp->Release();
}

//-------------------------------------------------------------------------------------------------
//      A3Dの初期化を行います.
//-------------------------------------------------------------------------------------------------
bool InitA3D()
{
    g_Prepare = false;

    // グラフィックスシステムの初期化.
    {
        a3d::SystemDesc desc = {};
        desc.pAllocator = &g_Allocator;

        if (!a3d::InitSystem(&desc))
        { return false; }
    }

    // デバイスの生成.
    {
        a3d::DeviceDesc desc = {};

        desc.EnableDebug = true;

        // 最大ディスクリプタ数を設定.
        desc.MaxColorTargetCount            = 256;
        desc.MaxDepthTargetCount            = 256;
        desc.MaxShaderResourceCount         = 256;
        desc.MaxSamplerCount                = 256;

        // 最大サブミット数を設定.
        desc.MaxGraphicsQueueSubmitCount    = 256;
        desc.MaxCopyQueueSubmitCount        = 256;
        desc.MaxComputeQueueSubmitCount     = 256;

        // デバイスを生成.
        if (!a3d::CreateDevice(&desc, &g_pDevice))
        { return false; }

        // コマンドキューを取得.
        g_pDevice->GetGraphicsQueue(&g_pGraphicsQueue);
    }

    auto info = g_pDevice->GetInfo();

    // スワップチェインの生成.
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

        // スワップチェインからバッファを取得.
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

    // フレームバッファの生成
    {
        // フレームバッファの設定.
        a3d::FrameBufferDesc desc = {};
        desc.ColorCount         = 1;
        desc.pColorTargets[0]   = g_pColorView[0];
        desc.pDepthTarget       = nullptr;

        // 1枚目のフレームバッファを生成.
        if (!g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[0]))
        { return false; }

        // 2枚目のフレームバッファを生成.
        desc.pColorTargets[0] = g_pColorView[1];
        if (!g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[1]))
        { return false; }
    }

    // コマンドリストを生成.
    {
        for(auto i=0; i<2; ++i)
        {
            if (!g_pDevice->CreateCommandList(a3d::COMMANDLIST_TYPE_DIRECT, &g_pCommandList[i]))
            { return false; }
        }
    }

    // フェンスを生成.
    {
        if (!g_pDevice->CreateFence(&g_pFence))
        { return false; }
    }

    // 頂点バッファを生成.
    {
        Vertex vertices[] = {
            { Vec3( 1.0f, -1.0f, 0.0f), Vec4(1.0f, 0.0f, 0.0f, 1.0f) },
            { Vec3(-1.0f, -1.0f, 0.0f), Vec4(0.0f, 1.0f, 0.0f, 1.0f) },
            { Vec3(-1.0f,  1.0f, 0.0f), Vec4(0.0f, 0.0f, 1.0f, 1.0f) },
            { Vec3( 1.0f,  1.0f, 0.0f), Vec4(1.0f, 0.0f, 1.0f, 1.0f) },
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

    // インデックスバッファを生成.
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

        if ( !g_pDevice->CreateBuffer(&desc, &g_pIndexBuffer) )
        { return false; }

        auto ptr = g_pIndexBuffer->Map();
        if ( ptr == nullptr )
        { return false; }

        memcpy( ptr, indices, sizeof(indices) );

        g_pIndexBuffer->Unmap();
    }

    // 定数バッファを生成.
    {
        auto stride = a3d::RoundUp<uint32_t>( sizeof(Transform), info.ConstantBufferMemoryAlignment );

        a3d::BufferDesc desc = {};
        desc.Size                           = stride;
        desc.Stride                         = stride;
        desc.InitState                      = a3d::RESOURCE_STATE_GENERAL;
        desc.Usage                          = a3d::RESOURCE_USAGE_CONSTANT_BUFFER;
        desc.HeapProperty.Type              = a3d::HEAP_TYPE_UPLOAD;
        desc.HeapProperty.CpuPageProperty   = a3d::CPU_PAGE_PROPERTY_DEFAULT;

        a3d::BufferViewDesc viewDesc = {};
        viewDesc.Offset = 0;
        viewDesc.Range  = stride;

        for(auto i=0; i<2; ++i)
        {
            if (!g_pDevice->CreateBuffer(&desc, &g_pConstantBuffer[i]))
            { return false; }

            if (!g_pDevice->CreateBufferView(g_pConstantBuffer[i], &viewDesc, &g_pConstantView[i]))
            { return false; }

            g_pCbHead[i] = g_pConstantBuffer[i]->Map();
        }

        Vec3 pos = Vec3(0.0f, 0.0f, -5.0f);
        Vec3 at  = Vec3(0.0f, 0.0f, 0.0f);
        Vec3 up  = Vec3(0.0f, 1.0f, 0.0f);

        auto aspect = static_cast<float>(g_pApp->GetWidth()) / static_cast<float>(g_pApp->GetHeight());

        g_Transform.World = Mat4::Identity();
        g_Transform.View  = Mat4::LookAt(pos, at, up);
        g_Transform.Proj  = Mat4::PersFov(ToRadian(45.0f), aspect, 0.1f, 1000.0f);
    }

    // シェーダバイナリを読み込みます.
    a3d::ShaderBinary vs = {};
    a3d::ShaderBinary ps = {};
    {
        std::string dir = GetShaderDirectoryForSampleProgram();;

    #if SAMPLE_IS_VULKAN
        const auto vsFilePath = dir + "simpleVS.spv";
        const auto psFilePath = dir + "simplePS.spv";
    #elif SAMPLE_IS_D3D12 || SAMPLE_IS_D3D11
        const auto vsFilePath = dir + "simpleVS.cso";
        const auto psFilePath = dir + "simplePS.cso";
    #else
        const auto vsFilePath = dir + "simpleVS.bin";
        const auto psFilePath = dir + "simplePS.bin";
    #endif

        if (!LoadShaderBinary(vsFilePath.c_str(), vs))
        { return false; }

        if (!LoadShaderBinary(psFilePath.c_str(), ps))
        {
            DisposeShaderBinary(vs);
            return false;
        }
    }

    // ディスクリプタセットレイアウトを生成します.
    {
        a3d::DescriptorSetLayoutDesc desc = {};
        desc.MaxSetCount               = 2;
        desc.EntryCount                = 1;
        desc.Entries[0].ShaderMask     = a3d::SHADER_MASK_VERTEX;
        desc.Entries[0].ShaderRegister = 0;
        desc.Entries[0].Type           = a3d::DESCRIPTOR_TYPE_CBV;

        if (!g_pDevice->CreateDescriptorSetLayout(&desc, &g_pDescriptorSetLayout))
        { return false; }

        for(auto i=0; i<2; ++i)
        {
            if (!g_pDescriptorSetLayout->CreateDescriptorSet(&g_pDescriptorSet[i]))
            { return false; }

            g_pDescriptorSet[i]->SetBuffer(0, g_pConstantView[i]);
            g_pDescriptorSet[i]->Update();
        }
    }

    // グラフィックスパイプラインステートを生成します.
    {
        // 入力要素です.
        a3d::InputElementDesc inputElements[] = {
            { "POSITION", 0, 0, a3d::RESOURCE_FORMAT_R32G32B32_FLOAT   , 0  },
            { "COLOR"   , 0, 1, a3d::RESOURCE_FORMAT_R32G32B32A32_FLOAT, 12 },
        };

        // 入力ストリームです.
        a3d::InputStreamDesc inputStream = {};
        inputStream.ElementCount    = 2;
        inputStream.pElements       = inputElements;
        inputStream.StreamIndex     = 0;
        inputStream.StrideInBytes   = sizeof(Vertex);
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

        // ラスタライザ―ステートの設定.
        desc.RasterizerState.PolygonMode                = a3d::POLYGON_MODE_SOLID;
        desc.RasterizerState.CullMode                   = a3d::CULL_MODE_NONE;
        desc.RasterizerState.FrontCounterClockWise      = false;
        desc.RasterizerState.DepthBias                  = 0;
        desc.RasterizerState.DepthBiasClamp             = 0.0f;
        desc.RasterizerState.SlopeScaledDepthBias       = 0;
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
        desc.pLayout = g_pDescriptorSetLayout;
        
        // プリミティブトポロジーの設定.
        desc.PrimitiveTopology = a3d::PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        // フレームバッファの設定.
        desc.pFrameBuffer = g_pFrameBuffer[0];

        // キャッシュ済みパイプラインステートの設定.
        desc.pCachedPSO = nullptr;

        // グラフィックスパイプラインステートの生成.
        if (!g_pDevice->CreateGraphicsPipeline(&desc, &g_pPipelineState))
        {
            DisposeShaderBinary(vs);
            DisposeShaderBinary(ps);
            return false;
        }
    }
     // 不要になったので破棄します.
    DisposeShaderBinary(vs);
    DisposeShaderBinary(ps);


    // ビューポートの設定.
    g_Viewport.X        = 0.0f;
    g_Viewport.Y        = 0.0f;
    g_Viewport.Width    = static_cast<float>(g_pApp->GetWidth());
    g_Viewport.Height   = static_cast<float>(g_pApp->GetHeight());
    g_Viewport.MinDepth = 0.0f;
    g_Viewport.MaxDepth = 1.0f;

    // シザー矩形の設定.
    g_Scissor.Offset.X      = 0;
    g_Scissor.Offset.Y      = 0;
    g_Scissor.Extent.Width  = g_pApp->GetWidth();
    g_Scissor.Extent.Height = g_pApp->GetHeight();

    // クエリプールの生成.
    {
        a3d::QueryPoolDesc desc = {};
        desc.Count = 1;
        desc.Type  = a3d::QUERY_TYPE_OCCLUSION;

        for(auto i=0; i<2; ++i)
        {
            if(!g_pDevice->CreateQueryPool(&desc, &g_pQueryPool[i]))
            { return false; }
        }
    }

    // クエリ結果格納バッファの生成.
    {
        a3d::BufferDesc desc = {};
        desc.Size                           = sizeof(uint64_t);
        desc.Stride                         = sizeof(uint64_t);
        desc.InitState                      = a3d::RESOURCE_STATE_COPY_DST;
        desc.Usage                          = a3d::RESOURCE_USAGE_QUERY_BUFFER;
        desc.HeapProperty.Type              = a3d::HEAP_TYPE_READBACK;
        desc.HeapProperty.CpuPageProperty   = a3d::CPU_PAGE_PROPERTY_DEFAULT;

        for(auto i=0; i<2; ++i)
        {
            if (!g_pDevice->CreateBuffer(&desc, &g_pQueryResultBuffer[i]))
            { return false; }
        }
    }

    // GUIマネージャの初期化.
    if (!GuiMgr::GetInstance().Init(g_pDevice, g_pFrameBuffer[0], g_pApp))
    { return false; }

    g_Prepare = true;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      A3Dの終了処理を行います.
//-------------------------------------------------------------------------------------------------
void TermA3D()
{
    g_Prepare = false;

    GuiMgr::GetInstance().Term();

    // ダブルバッファリソースの破棄.
    for(auto i=0; i<2; ++i)
    {

        a3d::SafeRelease(g_pQueryResultBuffer[i]);
        a3d::SafeRelease(g_pQueryPool[i]);

        // フレームバッファの破棄.
        a3d::SafeRelease(g_pFrameBuffer[i]);

        // カラービューの破棄.
        a3d::SafeRelease(g_pColorView[i]);

        // カラーバッファの破棄.
        a3d::SafeRelease(g_pColorBuffer[i]);
        
        // コマンドリストの破棄.
        a3d::SafeRelease(g_pCommandList[i]);

        // 定数バッファビューの破棄.
        a3d::SafeRelease(g_pConstantView[i]);

        // 定数バッファの破棄.
        a3d::SafeRelease(g_pConstantBuffer[i]);

        // ディスクリプタセットの破棄.
        a3d::SafeRelease(g_pDescriptorSet[i]);
    }

    // パイプラインステートの破棄.
    a3d::SafeRelease(g_pPipelineState);

    // 頂点バッファの破棄.
    a3d::SafeRelease(g_pVertexBuffer);

    // インデックスバッファの破棄.
    a3d::SafeRelease(g_pIndexBuffer);

    // ディスクリプタセットレイアウトの破棄.
    a3d::SafeRelease(g_pDescriptorSetLayout);

    // フェンスの破棄.
    a3d::SafeRelease(g_pFence);

    // スワップチェインの破棄.
    a3d::SafeRelease(g_pSwapChain);

    // グラフィックスキューの破棄.
    a3d::SafeRelease(g_pGraphicsQueue);

    // デバイスの破棄.
    a3d::SafeRelease(g_pDevice);

    // グラフィックスシステムの終了処理.
    a3d::TermSystem();
}

//-------------------------------------------------------------------------------------------------
//      A3Dによる描画処理を行います.
//-------------------------------------------------------------------------------------------------
void DrawA3D()
{
    if (!g_Prepare)
    { return; }

    // バッファ番号を取得します.
    auto idx = g_pSwapChain->GetCurrentBufferIndex();

    // 定数バッファを更新.
    {
        g_RotateAngle += 0.01f;
        g_Transform.World = Mat4::RotateY(g_RotateAngle);

        memcpy(g_pCbHead[idx], &g_Transform, sizeof(g_Transform));
    }

    // コマンドの記録を開始します.
    auto pCmd = g_pCommandList[idx];
    pCmd->Begin();

    // 書き込み用のバリアを設定します.
    pCmd->TextureBarrier(g_pColorBuffer[idx], a3d::RESOURCE_STATE_COLOR_WRITE);

    // フレームバッファを設定します.
    pCmd->SetFrameBuffer(g_pFrameBuffer[idx]);

    // フレームバッファをクリアします.
    a3d::ClearColorValue clearColor = {};
    clearColor.Float[0] = 0.25f;
    clearColor.Float[1] = 0.25f;
    clearColor.Float[2] = 0.25f;
    clearColor.Float[3] = 1.0f;
    pCmd->ClearFrameBuffer(1, &clearColor, nullptr);

    {
        // クエリ開始.
        pCmd->BeginQuery(g_pQueryPool[idx], 0);

        // パイプラインステートを設定します.
        pCmd->SetPipelineState(g_pPipelineState);

        // ディスクリプタセットを設定します.
        pCmd->SetDescriptorSet(g_pDescriptorSet[idx]);

        // ビューポートとシザー矩形を設定します.
        // NOTE : ビューポートとシザー矩形の設定は，必ずSetPipelineState() の後である必要があります.
        pCmd->SetViewports(1, &g_Viewport);
        pCmd->SetScissors (1, &g_Scissor);

        // 頂点バッファを設定します.
        pCmd->SetVertexBuffers(0, 1, &g_pVertexBuffer, nullptr);
        pCmd->SetIndexBuffer(g_pIndexBuffer, 0);

        // 三角形を描画します.
        pCmd->DrawIndexedInstanced(6, 1, 0, 0, 0);

        // クエリ終了.
        pCmd->EndQuery(g_pQueryPool[idx], 0);
    }

    // Debug GUIの描画.
    {
        // 描画開始.
        GuiMgr::GetInstance().SwapBuffers();

        {
            uint64_t sampleCount = 0;
            auto ptr = g_pQueryResultBuffer[g_PrevBufferIndex]->Map();
            memcpy(&sampleCount, ptr, sizeof(uint64_t));
            g_pQueryResultBuffer[g_PrevBufferIndex]->Unmap();

            ImGuiWindowFlags flag = 0;
            flag |= ImGuiWindowFlags_NoResize;
            flag |= ImGuiWindowFlags_NoMove;
            flag |= ImGuiWindowFlags_NoTitleBar;

            ImGui::Begin("Info", nullptr, flag);
            ImGui::SetWindowPos(ImVec2(10, 10));
            ImGui::SetWindowSize(ImVec2(160, 50));
            ImGui::Text("FPS : %.3f", ImGui::GetIO().Framerate);
            ImGui::Text("Query Sample : %lu", sampleCount);
            ImGui::End();
        }

        // 描画終了.
        GuiMgr::GetInstance().Issue(pCmd);
    }
    
    // 表示用のバリアを設定する前に，フレームバッファの設定を解除する必要があります.
    pCmd->SetFrameBuffer(nullptr);

    // クエリ結果を取得します.(ResolveQuery()はフレームバッファがバインドされていない状態で呼ぶ必要があります).
    pCmd->ResolveQuery(g_pQueryPool[idx], 0, 1, g_pQueryResultBuffer[idx], 0);

    // 表示用にバリアを設定します.
    pCmd->TextureBarrier(g_pColorBuffer[idx], a3d::RESOURCE_STATE_PRESENT);

    // コマンドリストへの記録を終了します.
    pCmd->End();

    // コマンドキューに登録します.
    g_pGraphicsQueue->Submit(pCmd);

    // コマンドを実行します.
    g_pGraphicsQueue->Execute(g_pFence);

    // コマンドを実行完了を待機します.
    if (!g_pFence->IsSignaled())
    { g_pFence->Wait(UINT32_MAX); }

    g_PrevBufferIndex = idx;

    // 画面に表示します.
    g_pSwapChain->Present();

}

//-------------------------------------------------------------------------------------------------
//      リサイズ処理です.
//-------------------------------------------------------------------------------------------------
void Resize( uint32_t w, uint32_t h, void* pUser )
{
    A3D_UNUSED( pUser );

    // 準備が完了してない状態だったら，処理できないので即終了.
    if (!g_Prepare || g_pSwapChain == nullptr)
    { return; }

    // サンプル数以下の場合はクラッシュする原因となるので処理させない.
    {
        auto desc = g_pSwapChain->GetDesc();
        if ( w < desc.SampleCount || h < desc.SampleCount )
        { return; }
    }

    g_Prepare = false;

    // アイドル状態になるまで待つ.
    g_pGraphicsQueue->WaitIdle();
    g_pDevice->WaitIdle();


    for(auto i=0; i<2; ++i)
    {
        // フレームバッファの破棄.
        a3d::SafeRelease(g_pFrameBuffer[i]);

        // カラービューの破棄.
        a3d::SafeRelease(g_pColorView[i]);

        // カラーバッファの破棄.
        a3d::SafeRelease(g_pColorBuffer[i]);
    }

    // スワップチェインのリサイズ処理です.
    g_pSwapChain->ResizeBuffers( w, h );

    // テクスチャビューを生成.
    {
        auto desc = g_pSwapChain->GetDesc();

        // スワップチェインからバッファを取得.
        g_pSwapChain->GetBuffer(0, &g_pColorBuffer[0]);
        g_pSwapChain->GetBuffer(1, &g_pColorBuffer[1]);

        a3d::TextureViewDesc viewDesc = {};
        viewDesc.Dimension          = a3d::VIEW_DIMENSION_TEXTURE2D;
        viewDesc.Format             = desc.Format;
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
            auto ret = g_pDevice->CreateTextureView(g_pColorBuffer[i], &viewDesc, &g_pColorView[i]);
            assert(ret == true);
        }
    }

    // フレームバッファの生成
    {
        // フレームバッファの設定.
        a3d::FrameBufferDesc desc = {};
        desc.ColorCount         = 1;
        desc.pColorTargets[0]   = g_pColorView[0];
        desc.pDepthTarget       = nullptr;

        // 1枚目のフレームバッファを生成.
        auto ret = g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[0]);
        assert(ret == true);

        // 2枚目のフレームバッファを生成.
        desc.pColorTargets[0] = g_pColorView[1];
        ret = g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[1]);
        assert(ret == true);
    }

    // ビューポートの設定.
    g_Viewport.X        = 0;
    g_Viewport.Y        = 0;
    g_Viewport.Width    = float(w);
    g_Viewport.Height   = float(h);

    // シザー矩形の設定.
    g_Scissor.Offset.X      = 0;
    g_Scissor.Offset.Y      = 0;
    g_Scissor.Extent.Width  = w;
    g_Scissor.Extent.Height = h;

    g_Prepare = true;
}