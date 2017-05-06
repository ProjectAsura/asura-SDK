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
#include <cassert>
#include <SampleApp.h>


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
IApp*               g_pApp              = nullptr;  //!< ウィンドウの生成を行うヘルパークラスです.
a3d::IDevice*       g_pDevice           = nullptr;  //!< デバイスです.
a3d::ISwapChain*    g_pSwapChain        = nullptr;  //!< スワップチェインです.
a3d::IQueue*        g_pGraphicsQueue    = nullptr;  //!< コマンドキューです.
a3d::IFence*        g_pFence            = nullptr;  //!< フェンスです.
a3d::ITexture*      g_pColorBuffer[2]   = {};       //!< カラーバッファです.
a3d::ITextureView*  g_pColorView[2]     = {};       //!< カラービューです.
a3d::ICommandList*  g_pCommandList[2]   = {};       //!< コマンドリストです.
a3d::IFrameBuffer*  g_pFrameBuffer[2]   = {};       //!< フレームバッファです.
bool                g_Prepare           = false;    //!< 準備出来たらtrue.


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
            A3D_UNUSED(alignment);
            return realloc(ptr, size);
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

    // リサイズ時のコールバック関数を設定.
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
        desc.pOption    = g_pApp->GetWindowHandle();

        if (!a3d::InitSystem(&desc))
        { return false; }
    }

    // デバイスの生成.
    {
        a3d::DeviceDesc desc = {};

        desc.EnableDebug = true;

        // 最大ディスクリプタ数を設定.
        desc.MaxColorTargetCount            = 2;
        desc.MaxDepthTargetCount            = 1;
        desc.MaxShaderResourceCount         = 1;

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

    // スワップチェインの生成.
    {
    #if SAMPLE_IS_VULKAN && TARGET_PC
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

    g_Prepare = true;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      A3Dの終了処理を行います.
//-------------------------------------------------------------------------------------------------
void TermA3D()
{
    g_Prepare = false;

    // ダブルバッファリソースの破棄.
    for(auto i=0; i<2; ++i)
    {
        // フレームバッファの破棄.
        a3d::SafeRelease(g_pFrameBuffer[i]);

        // カラービューの破棄.
        a3d::SafeRelease(g_pColorView[i]);

        // カラーバッファの破棄.
        a3d::SafeRelease(g_pColorBuffer[i]);

        // コマンドリストの破棄.
        a3d::SafeRelease(g_pCommandList[i]);
    }

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

    // コマンドの記録を開始します.
    auto pCmd = g_pCommandList[idx];
    pCmd->Begin();

    // 書き込み用のバリアを設定します.
    pCmd->TextureBarrier(
        g_pColorBuffer[idx],
        a3d::RESOURCE_STATE_PRESENT,
        a3d::RESOURCE_STATE_COLOR_WRITE);

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
        /* TODO */
    }

    pCmd->SetFrameBuffer(nullptr);

    // 表示用にバリアを設定します.
    pCmd->TextureBarrier(
        g_pColorBuffer[idx],
        a3d::RESOURCE_STATE_COLOR_WRITE,
        a3d::RESOURCE_STATE_PRESENT);

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

    g_Prepare = true;
}