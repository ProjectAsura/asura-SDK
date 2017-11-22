//-------------------------------------------------------------------------------------------------
// File : app_framework.h
// Desc : Framework
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <app_framework.h>
#include <app_allocator.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// Framework class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタ.
//-------------------------------------------------------------------------------------------------
Framework::Framework()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタ.
//-------------------------------------------------------------------------------------------------
Framework::~Framework()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      実行.
//-------------------------------------------------------------------------------------------------
void Framework::Run
(
    uint32_t                width,
    uint32_t                height,
    a3d::RESOURCE_FORMAT    colorFormat,
    a3d::RESOURCE_FORMAT    depthFormat
)
{
    if (Init(width, height, colorFormat, depthFormat))
    { MainLoop(); }

    Term();
}

//-------------------------------------------------------------------------------------------------
//      初期化時の処理.
//-------------------------------------------------------------------------------------------------
bool Framework::Init
(
    uint32_t                width,
    uint32_t                height,
    a3d::RESOURCE_FORMAT    colorFormat,
    a3d::RESOURCE_FORMAT    depthFormat
)
{
    m_Ready = false;

    // アプリの生成.
    {
        if (!a3d::CreateApp(Allocator::Instance(), width, height, this, this, this, this, &m_pApp))
        { return false; }
    }

    // グラフィックスシステムの初期化.
    {
        a3d::SystemDesc desc = {};
        desc.pAllocator = Allocator::Instance();
        desc.pOption    = m_pApp->GetWindowHandle();

        if (!a3d::InitSystem(&desc))
        { return false; }
    }

    // デバイスの生成.
    {
        a3d::DeviceDesc desc = {};

        desc.EnableDebug = true;

        // 最大ディスクリプタ数を設定.
        desc.MaxColorTargetCount            = 512;
        desc.MaxDepthTargetCount            = 512;
        desc.MaxShaderResourceCount         = 4096;

        // 最大サブミット数を設定.
        desc.MaxGraphicsQueueSubmitCount    = 256;
        desc.MaxCopyQueueSubmitCount        = 256;
        desc.MaxComputeQueueSubmitCount     = 256;

        // デバイスを生成.
        if (!a3d::CreateDevice(&desc, &m_pDevice))
        { return false; }

        // コマンドキューを取得.
        m_pDevice->GetGraphicsQueue(&m_pGraphicsQueue);
    }

    // スワップチェインの生成.
    {
        a3d::SwapChainDesc desc = {};
        desc.Extent.Width   = m_pApp->GetWidth();
        desc.Extent.Height  = m_pApp->GetHeight();
        desc.Format         = colorFormat;
        desc.MipLevels      = 1;
        desc.SampleCount    = 1;
        desc.BufferCount    = 2;
        desc.SyncInterval   = 1;
        desc.InstanceHandle = m_pApp->GetInstanceHandle();
        desc.WindowHandle   = m_pApp->GetWindowHandle();

        if (!m_pDevice->CreateSwapChain(&desc, &m_pSwapChain))
        { return false; }

        // スワップチェインからバッファを取得.
        m_pSwapChain->GetBuffer(0, &m_pColorBuffer[0]);
        m_pSwapChain->GetBuffer(1, &m_pColorBuffer[1]);

        a3d::TextureViewDesc viewDesc = {};
        viewDesc.Dimension          = a3d::VIEW_DIMENSION_TEXTURE2D;
        viewDesc.Format             = colorFormat;
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
            if (!m_pDevice->CreateTextureView(m_pColorBuffer[i], &viewDesc, &m_pColorView[i]))
            { return false; }
        }
    }

    // 深度バッファの生成.
    {
        a3d::TextureDesc desc = {};
        desc.Dimension                      = a3d::RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width                          = m_pApp->GetWidth();
        desc.Height                         = m_pApp->GetHeight();
        desc.DepthOrArraySize               = 1;
        desc.Format                         = depthFormat;
        desc.MipLevels                      = 1;
        desc.SampleCount                    = 1;
        desc.Layout                         = a3d::RESOURCE_LAYOUT_OPTIMAL;
        desc.Usage                          = a3d::RESOURCE_USAGE_DEPTH_TARGET;
        desc.InitState                      = a3d::RESOURCE_STATE_DEPTH_WRITE;
        desc.HeapProperty.Type              = a3d::HEAP_TYPE_DEFAULT;
        desc.HeapProperty.CpuPageProperty   = a3d::CPU_PAGE_PROPERTY_DEFAULT;

        if (!m_pDevice->CreateTexture(&desc, &m_pDepthBuffer))
        { return false; }

        a3d::TextureViewDesc viewDesc = {};
        viewDesc.Dimension          = a3d::VIEW_DIMENSION_TEXTURE2D;
        viewDesc.Format             = desc.Format;
        viewDesc.TextureAspect      = a3d::TEXTURE_ASPECT_DEPTH;
        viewDesc.MipSlice           = 0;
        viewDesc.MipLevels          = desc.MipLevels;
        viewDesc.FirstArraySlice    = 0;
        viewDesc.ArraySize          = desc.DepthOrArraySize;
        viewDesc.ComponentMapping.R = a3d::TEXTURE_SWIZZLE_R;
        viewDesc.ComponentMapping.G = a3d::TEXTURE_SWIZZLE_G;
        viewDesc.ComponentMapping.B = a3d::TEXTURE_SWIZZLE_B;
        viewDesc.ComponentMapping.A = a3d::TEXTURE_SWIZZLE_A;

        if (!m_pDevice->CreateTextureView(m_pDepthBuffer, &viewDesc, &m_pDepthView))
        { return false; }
    }

    // フレームバッファの生成
    {
        // フレームバッファの設定.
        a3d::FrameBufferDesc desc = {};
        desc.ColorCount         = 1;
        desc.pColorTargets[0]   = m_pColorView[0];
        desc.pDepthTarget       = m_pDepthView;

        // 1枚目のフレームバッファを生成.
        if (!m_pDevice->CreateFrameBuffer(&desc, &m_pFrameBuffer[0]))
        { return false; }

        // 2枚目のフレームバッファを生成.
        desc.pColorTargets[0] = m_pColorView[1];
        if (!m_pDevice->CreateFrameBuffer(&desc, &m_pFrameBuffer[1]))
        { return false; }
    }

    // コマンドリストを生成.
    {
        for(auto i=0; i<2; ++i)
        {
            if (!m_pDevice->CreateCommandList(a3d::COMMANDLIST_TYPE_DIRECT, &m_pCommandList[i]))
            { return false; }
        }
    }

    // フェンスを生成.
    {
        if (!m_pDevice->CreateFence(&m_pFence))
        { return false; }
    }

    // GUIレンダラー初期化.
    {
        a3d::TargetFormat rtvFormat;
        rtvFormat.Format        = colorFormat;
        rtvFormat.SampleCount   = 1;

        a3d::TargetFormat dsvFormat;
        dsvFormat.Format        = depthFormat;
        dsvFormat.SampleCount   = 1;

        if (!a3d::GuiRenderer::Instance().Init(
            m_pDevice,
            m_pApp->GetWindowHandle(),
            m_pApp->GetWidth(),
            m_pApp->GetHeight(),
            rtvFormat,
            dsvFormat,
            nullptr))
        { return false; }
    }

    m_Ready = true;

    // ユーザー定義の初期化処理を呼び出し.
    if (!OnInit())
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      メインループ.
//-------------------------------------------------------------------------------------------------
void Framework::MainLoop()
{
    while(m_pApp->IsLoop())
    {
        // 描画処理.
        Draw();

        // いらないリソースを破棄.
        FrameDispose();
    }
}

//-------------------------------------------------------------------------------------------------
//      終了時の処理.
//-------------------------------------------------------------------------------------------------
void Framework::Term()
{
    if (m_pGraphicsQueue != nullptr)
    { m_pGraphicsQueue->WaitIdle(); }

    m_Ready = false;

    OnTerm();

    a3d::GuiRenderer::Instance().Term();

    {
        auto itr = std::begin(m_Disposer);
        while(itr != std::end(m_Disposer))
        {
            a3d::SafeRelease(itr->pItem);
            itr++;
        }
        m_Disposer.clear();
    }

    for(auto i=0; i<2; ++i)
    {
        a3d::SafeRelease(m_pFrameBuffer[i]);
        a3d::SafeRelease(m_pColorView[i]);
        a3d::SafeRelease(m_pColorBuffer[i]);
        a3d::SafeRelease(m_pCommandList[i]);
    }

    a3d::SafeRelease(m_pDepthView);
    a3d::SafeRelease(m_pDepthBuffer);
    a3d::SafeRelease(m_pFence);
    a3d::SafeRelease(m_pSwapChain);
    a3d::SafeRelease(m_pGraphicsQueue);
    a3d::SafeRelease(m_pDevice);
    a3d::TermSystem();
}

//-------------------------------------------------------------------------------------------------
//      描画処理.
//-------------------------------------------------------------------------------------------------
void Framework::Draw()
{
    if (!m_Ready || m_pSwapChain == nullptr)
    { return; }

    OnDraw();

    if (m_ChangeMode)
    {
        // アイドル状態になるまで待つ.
        m_pGraphicsQueue->WaitIdle();
        m_pDevice->WaitIdle();

        // フルスクリーン切り替え.
        m_pSwapChain->SetFullScreenMode(m_FullScreen);
        m_ChangeMode = false;
    }
}

//-------------------------------------------------------------------------------------------------
//      不要リソースの破棄.
//-------------------------------------------------------------------------------------------------
void Framework::FrameDispose()
{
    auto itr = std::begin(m_Disposer);
    while(itr != std::end(m_Disposer))
    {
        if (itr->life == 0)
        {
            a3d::SafeRelease(itr->pItem);
            itr = m_Disposer.erase(itr);
        }
        else
        {
            itr->life--;
            itr++;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//      リサイズ処理.
//-------------------------------------------------------------------------------------------------
void Framework::OnEvent(const a3d::ResizeEventArg& arg)
{
    // 準備が出来ていない場合はそのまま終了.
    if (!m_Ready || m_pSwapChain == nullptr)
    { return; }

    // サンプル数を下回る場合はクラッシュするので処理させない.
    {
        auto desc = m_pSwapChain->GetDesc();
        if (arg.Width < desc.SampleCount || arg.Height < desc.SampleCount)
        { return; }
    }

    // 準備状態に移行.
    m_Ready = false;

    // アイドル状態になるまで待つ.
    m_pGraphicsQueue->WaitIdle();
    m_pDevice->WaitIdle();

    for(auto i=0; i<2; ++i)
    {
        // フレームバッファの破棄.
        a3d::SafeRelease(m_pFrameBuffer[i]);

        // カラービューの破棄.
        a3d::SafeRelease(m_pColorView[i]);

        // カラーバッファの破棄.
        a3d::SafeRelease(m_pColorBuffer[i]);
    }

    // スワップチェインのリサイズ処理です.
    m_pSwapChain->ResizeBuffers( arg.Width, arg.Height );

    // テクスチャビューを生成.
    {
        auto desc = m_pSwapChain->GetDesc();

        // スワップチェインからバッファを取得.
        m_pSwapChain->GetBuffer(0, &m_pColorBuffer[0]);
        m_pSwapChain->GetBuffer(1, &m_pColorBuffer[1]);

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
            auto ret = m_pDevice->CreateTextureView(m_pColorBuffer[i], &viewDesc, &m_pColorView[i]);
            assert(ret == true);
        }
    }

    // フレームバッファの生成
    {
        // フレームバッファの設定.
        a3d::FrameBufferDesc desc = {};
        desc.ColorCount         = 1;
        desc.pColorTargets[0]   = m_pColorView[0];
        desc.pDepthTarget       = nullptr;

        // 1枚目のフレームバッファを生成.
        auto ret = m_pDevice->CreateFrameBuffer(&desc, &m_pFrameBuffer[0]);
        assert(ret == true);

        // 2枚目のフレームバッファを生成.
        desc.pColorTargets[0] = m_pColorView[1];
        ret = m_pDevice->CreateFrameBuffer(&desc, &m_pFrameBuffer[1]);
        assert(ret == true);
    }

    // 準備完了.
    m_Ready = true;

    // ユーザー定義関数を呼び出す.
    OnResize(arg);
}

//-------------------------------------------------------------------------------------------------
//      マウスの処理.
//-------------------------------------------------------------------------------------------------
void Framework::OnEvent(const a3d::MouseEventArg& arg)
{
    OnMouse(arg);
}

//-------------------------------------------------------------------------------------------------
//      キーの処理.
//-------------------------------------------------------------------------------------------------
void Framework::OnEvent(const a3d::KeyEventArg& arg)
{
    OnKey(arg);
}

//-------------------------------------------------------------------------------------------------
//      タイピング処理.
//-------------------------------------------------------------------------------------------------
void Framework::OnEvent(uint32_t keyCode)
{
    OnTyping(keyCode);
}

a3d::ICommandList* Framework::GetCurrentCommandList() const
{
    auto idx = m_pSwapChain->GetCurrentBufferIndex();
    return m_pCommandList[idx];
}

a3d::IFrameBuffer* Framework::GetCurrentFrameBuffer() const
{
    auto idx = m_pSwapChain->GetCurrentBufferIndex();
    return m_pFrameBuffer[idx];
}

a3d::ITexture* Framework::GetCurrentColorBuffer() const
{
    auto idx = m_pSwapChain->GetCurrentBufferIndex();
    return m_pColorBuffer[idx];
}

a3d::ITextureView* Framework::GetCurrentColorView() const
{
    auto idx = m_pSwapChain->GetCurrentBufferIndex();
    return m_pColorView[idx];
}

a3d::IApp* Framework::GetApp() const
{ return m_pApp; }

bool Framework::IsReady() const
{ return m_Ready; }

void Framework::AddToDisposer(a3d::IReference* pItem, uint32_t life)
{
    DisposeItem entry;
    entry.pItem = pItem;
    entry.life  = life;
    m_Disposer.push_back(entry);
}

void Framework::ToggleFullScreen()
{
    m_FullScreen = true;
    m_ChangeMode = true;
}
