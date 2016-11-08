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
    Vec3 Position;  //!< 位置座標です.
    Vec2 TexCoord;  //!< 頂点カラーです.
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
a3d::ITexture*              g_pDepthBuffer          = nullptr;  //!< 深度バッファです.
a3d::ITexture*              g_pTexture              = nullptr;  //!< テクスチャです.
a3d::ISampler*              g_pSampler              = nullptr;  //!< サンプラーです.
a3d::ITexture*              g_pColorBuffer[2]       = {};       //!< カラーバッファです.
a3d::ICommandList*          g_pCommandList[2]       = {};       //!< コマンドリストです.
a3d::IFrameBuffer*          g_pFrameBuffer[2]       = {};       //!< フレームバッファです.
a3d::IBuffer*               g_pConstantBuffer[2]    = {};       //!< 定数バッファです.
a3d::Viewport               g_Viewport              = {};       //!< ビューポートです.
a3d::Rect                   g_Scissor               = {};       //!< シザー矩形です.
a3d::IDescriptorSet*        g_pDescriptorSet[2]     = {};       //!< ディスクリプタセットです.
Transform                   g_Transform             = {};       //!< 変換行列です.
float                       g_RotateAngle           = 0.0f;     //!< 回転角です.
void*                       g_pCbHead[2]            = {};       //!< 定数バッファの先頭ポインタです.

//-------------------------------------------------------------------------------------------------
//      メインエントリーポイントです.
//-------------------------------------------------------------------------------------------------
void main()
{
    // ウィンドウ生成.
    if (!CreateApp(960, 540, &g_pApp))
    { return; }

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
    // デバイスの生成.
    {
        a3d::DeviceDesc desc = {};

        desc.EnableDebug = true;

        // 最大ディスクリプタ数を設定.
        desc.MaxColorTargetCount            = 2;
        desc.MaxDepthTargetCount            = 2;
        desc.MaxShaderResourceCount         = 4;
        desc.MaxSamplerCount                = 2;

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
    }

    // 深度バッファの生成.
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

    // フレームバッファの生成
    {
        // スワップチェインからバッファを取得.
        g_pSwapChain->GetBuffer(0, &g_pColorBuffer[0]);
        g_pSwapChain->GetBuffer(1, &g_pColorBuffer[1]);

        // フレームバッファの設定.
        a3d::FrameBufferDesc desc = {};
        desc.ColorCount         = 1;
        desc.pColorTargets[0]   = g_pColorBuffer[0];
        desc.pDepthTarget       = g_pDepthBuffer;

        // 1枚目のフレームバッファを生成.
        if (!g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[0]))
        { return false; }

        // 2枚目のフレームバッファを生成.
        desc.pColorTargets[0] = g_pColorBuffer[1];
        if (!g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[1]))
        { return false; }
    }

    // コマンドリストを生成.
    {
        for(auto i=0; i<2; ++i)
        {
            if (!g_pDevice->CreateCommandList(a3d::COMMANDLIST_TYPE_DIRECT, nullptr, &g_pCommandList[i]))
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
        desc.EnableRow                      = false;

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

    // シェーダバイナリを読み込みます.
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

    // ディスクリプタセットレイアウトを生成します.
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

    // グラフィックスパイプラインステートを生成します.
    {
        // 入力要素です.
        a3d::InputElementDesc inputElements[] = {
            { "POSITION", 0, 0, a3d::RESOURCE_FORMAT_R32G32B32_FLOAT , 0  },
            { "TEXCOORD", 0, 1, a3d::RESOURCE_FORMAT_R32G32_FLOAT    , 12 },
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
        desc.RasterizerState.SlopeScaledDepthBais       = 0;
        desc.RasterizerState.DepthClipEnable            = true;
        desc.RasterizerState.EnableConservativeRaster   = false;
        
        // マルチサンプルステートの設定.
        desc.MultiSampleState.EnableAlphaToCoverage = false;
        desc.MultiSampleState.EnableMultiSample     = false;
        desc.MultiSampleState.SampleCount           = 1;

        // 深度ステンシルステートの設定.
        desc.DepthStencilState.DepthTestEnable      = true;
        desc.DepthStencilState.DepthWriteEnable     = true;
        desc.DepthStencilState.DepthCompareOp       = a3d::COMPARE_OP_LESS;
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
        { return false; }
    }
     // 不要になったので破棄します.
    DisposeShaderBinary(vs);
    DisposeShaderBinary(ps);

    // テクスチャの生成.
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

    // サンプラーの生成.
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

    return true;
}

//-------------------------------------------------------------------------------------------------
//      A3Dの終了処理を行います.
//-------------------------------------------------------------------------------------------------
void TermA3D()
{
    // ダブルバッファリソースの破棄.
    for(auto i=0; i<2; ++i)
    {
        // フレームバッファの破棄.
        a3d::SafeRelease(g_pFrameBuffer[i]);

        // コマンドリストの破棄.
        a3d::SafeRelease(g_pCommandList[i]);

        // カラーバッファの破棄.
        a3d::SafeRelease(g_pColorBuffer[i]);

        // 定数バッファの破棄.
        a3d::SafeRelease(g_pConstantBuffer[i]);

        // ディスクリプタセットの破棄.
        a3d::SafeRelease(g_pDescriptorSet[i]);
    }

    // サンプラーの破棄.
    a3d::SafeRelease(g_pSampler);

    // テクスチャの破棄.
    a3d::SafeRelease(g_pTexture);

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

    // 深度バッファの破棄.
    a3d::SafeRelease(g_pDepthBuffer);

    // スワップチェインの破棄.
    a3d::SafeRelease(g_pSwapChain);

    // グラフィックスキューの破棄.
    a3d::SafeRelease(g_pGraphicsQueue);

    // デバイスの破棄.
    a3d::SafeRelease(g_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      A3Dによる描画処理を行います.
//-------------------------------------------------------------------------------------------------
void DrawA3D()
{
    // バッファ番号を取得します.
    auto idx = g_pSwapChain->GetCurrentBufferIndex();

    // 定数バッファを更新.
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

    a3d::ClearDepthStencilValue clearDepth = {};
    clearDepth.Depth                = 1.0f;
    clearDepth.Stencil              = 0;
    clearDepth.EnableClearDepth     = true;
    clearDepth.EnableClearStencil   = false;

    pCmd->ClearFrameBuffer(1, &clearColor, &clearDepth);

    {
        // ディスクリプタセットレイアウトを設定します.
        pCmd->SetDescriptorSetLayout(g_pDescriptorSetLayout);

        // パイプラインステートを設定します.
        pCmd->SetPipelineState(g_pPipelineState);

        // ビューポートとシザー矩形を設定します.
        // NOTE : ビューポートとシザー矩形の設定は，必ずSetPipelineState() の後である必要があります.
        pCmd->SetViewports(1, &g_Viewport);
        pCmd->SetScissors (1, &g_Scissor);

        // 頂点バッファを設定します.
        pCmd->SetVertexBuffers(0, 1, &g_pVertexBuffer, nullptr);
        pCmd->SetIndexBuffer(g_pIndexBuffer, 0);

        // 矩形を描画.
        pCmd->SetDescriptorSet(g_pDescriptorSet[idx]);
        pCmd->DrawIndexedInstanced(6, 1, 0, 0, 0);
    }
    
    // 表示用のバリアを設定する前に，フレームバッファの設定を解除する必要があります.
    pCmd->SetFrameBuffer(nullptr);

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

    // 画面に表示します.
    g_pSwapChain->Present();
}
