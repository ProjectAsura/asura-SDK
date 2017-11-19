//-------------------------------------------------------------------------------------------------
// File : a3dSwapChain.cpp
// Desc : SwapChain Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

template<typename T>
inline T Clamp(T value, T mini, T maxi)
{ return (value < mini) ? mini : ((value > maxi) ? maxi : value); }

UINT16 GetCoord(float value)
{
    // 正規化値を求めるためには 50000で割る必要があるが，A3Dでは正規化値を保持する設計なので,
    // DXGI側に渡すためには，50000倍する. 
    // https://msdn.microsoft.com/en-us/library/windows/desktop/mt732700(v=vs.85).aspx を参照
    return static_cast<UINT16>(Clamp(value, 0.0f, 1.0f) * 50000);
}

} // namespace /* anonymous */

namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// SwapChain class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
SwapChain::SwapChain()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pSwapChain  (nullptr)
, m_pSwapChain4 (nullptr)
, m_pBuffers    (nullptr)
, m_PresentFlag (0)
{ memset(&m_Desc, 0, sizeof(m_Desc)); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
SwapChain::~SwapChain()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool SwapChain::Init(IDevice* pDevice, IQueue* pQueue, const SwapChainDesc* pDesc)
{
    if (pDevice == nullptr || pQueue == nullptr || pDesc == nullptr)
    { return false; }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pWrapQueue = static_cast<Queue*>(pQueue);
    A3D_ASSERT(pWrapQueue != nullptr);

    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    auto pNativeFactory = m_pDevice->GetDXGIFactory();
    A3D_ASSERT(pNativeFactory != nullptr);

    auto pNativeQueue = pWrapQueue->GetD3D12Queue();
    A3D_ASSERT(pNativeQueue != nullptr);

    m_hWnd = static_cast<HWND>(pDesc->WindowHandle);
    m_IsTearingSupport = m_pDevice->IsTearingSupport();

    GetWindowRect(m_hWnd, &m_Rect);
    m_WindowStyle = GetWindowLong(m_hWnd, GWL_STYLE);

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    auto w = pDesc->Extent.Width;
    auto h = pDesc->Extent.Height;

    if ( pDesc->EnableFullScreen )
    {
        // スクリーンサイズを設定
        w = GetSystemMetrics(SM_CXSCREEN);
        h = GetSystemMetrics(SM_CYSCREEN);

        m_Desc.Extent.Width  = w;
        m_Desc.Extent.Height = h;
    }

    // スワップチェインの生成.
    {
        DXGI_SWAP_CHAIN_DESC1 desc = {};
        desc.Width              = w;
        desc.Height             = h;
        desc.Format             = ToNativeFormat(pDesc->Format);
        desc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
        desc.BufferCount        = pDesc->BufferCount;
        desc.SampleDesc.Count   = pDesc->SampleCount;
        desc.SampleDesc.Quality = 0;
        desc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.Flags              = (m_IsTearingSupport) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

        IDXGISwapChain1* pSwapChain = nullptr;
        auto hr = pNativeFactory->CreateSwapChainForHwnd(pNativeQueue, m_hWnd, &desc, nullptr, nullptr, &pSwapChain);
        if (FAILED(hr))
        { return false; }

        if (m_IsTearingSupport)
        { pNativeFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER); }

        hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
        SafeRelease(pSwapChain);
        if (FAILED(hr))
        { return false; }

        hr = m_pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain4));
        if (FAILED(hr))
        { SafeRelease(m_pSwapChain4); }

        if (pDesc->ColorSpace != COLOR_SPACE_SRGB)
        {
            auto color_space = ToNativeColorSpace(pDesc->ColorSpace);
            auto hr = m_pSwapChain4->SetColorSpace1(color_space);
            if (FAILED(hr))
            { return false; }
        }
    }

    {
        m_pBuffers = new Texture* [m_Desc.BufferCount];
        if (m_pBuffers == nullptr)
        { return false; }

        ComponentMapping componentMapping;
        componentMapping.R = TEXTURE_SWIZZLE_R;
        componentMapping.G = TEXTURE_SWIZZLE_G;
        componentMapping.B = TEXTURE_SWIZZLE_B;
        componentMapping.A = TEXTURE_SWIZZLE_A;

        for(auto i=0u; i<m_Desc.BufferCount; ++i)
        {
            ID3D12Resource* pBuffer;

            auto hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBuffer));
            if (SUCCEEDED(hr))
            {
                if (!Texture::CreateFromNative(
                    pDevice,
                    pBuffer,
                    RESOURCE_USAGE_COLOR_TARGET,
                    componentMapping,
                    reinterpret_cast<ITexture**>(&m_pBuffers[i])))
                {
                    SafeRelease(pBuffer);
                    return false;
                }
            }

            SafeRelease(pBuffer);
        }
    }

    // フルスクリーン処理.
    if ( pDesc->EnableFullScreen )
    {
        // フルスクリーンモードを設定.
        SetFullScreenMode(pDesc->EnableFullScreen);

        if (!m_IsTearingSupport)
        { ResizeBuffers( w, h ); }
    }

    if (m_Desc.SyncInterval == 0 && m_IsTearingSupport && !m_IsFullScreen)
    { m_PresentFlag = DXGI_PRESENT_ALLOW_TEARING; }
    else
    { m_PresentFlag = 0;}

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void SwapChain::Term()
{
    if (m_pBuffers != nullptr)
    {
        for(auto i=0u; i<m_Desc.BufferCount; ++i)
        { SafeRelease(m_pBuffers[i]); }

         delete[] m_pBuffers;
         m_pBuffers = nullptr;
    }

    // フルスクリーンモードのままだと例外が発生するので，ウィンドウモードに変更する.
    if (m_pSwapChain != nullptr)
    {
        auto isFullScreen = IsFullScreenMode();
        if (isFullScreen)
        { SetFullScreenMode(false); }
    }

    SafeRelease(m_pSwapChain4);
    SafeRelease(m_pSwapChain);
    SafeRelease(m_pDevice);

    memset(&m_Desc, 0, sizeof(m_Desc));
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void SwapChain::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解法処理を行います.
//-------------------------------------------------------------------------------------------------
void SwapChain::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t SwapChain::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void SwapChain::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
SwapChainDesc SwapChain::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      画面に表示します.
//-------------------------------------------------------------------------------------------------
void SwapChain::Present()
{ m_pSwapChain->Present( m_Desc.SyncInterval, m_PresentFlag ); }

//-------------------------------------------------------------------------------------------------
//      現在のバッファ番号を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t SwapChain::GetCurrentBufferIndex()
{ return m_pSwapChain->GetCurrentBackBufferIndex(); }

//-------------------------------------------------------------------------------------------------
//      指定バッファを取得します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::GetBuffer(uint32_t index, ITexture** ppResource)
{
    if (m_pSwapChain == nullptr || m_pBuffers == nullptr)
    { return false; }

    if (index >= m_Desc.BufferCount )
    { return false; }

    if (m_pBuffers[index] == nullptr)
    { return false; }

    *ppResource = m_pBuffers[index];
    m_pBuffers[index]->AddRef();

    return true;
}

//-------------------------------------------------------------------------------------------------
//      バッファをリサイズします.
//-------------------------------------------------------------------------------------------------
bool SwapChain::ResizeBuffers(uint32_t width, uint32_t height)
{
    if (m_pSwapChain == nullptr)
    { return false; }

    // 一旦テクスチャを破棄する.
    if (m_pBuffers != nullptr)
    {
        for(auto i=0u; i<m_Desc.BufferCount; ++i)
        { SafeRelease(m_pBuffers[i]); }
    }

    DXGI_MODE_DESC desc = {};
    desc.Width                      = width;
    desc.Height                     = height;
    desc.Format                     = ToNativeFormat(m_Desc.Format);
    desc.RefreshRate.Numerator      = 60;
    desc.RefreshRate.Denominator    = 1;

    auto hr = m_pSwapChain->ResizeTarget(&desc);
    if (FAILED(hr))
    { return false; }

    DXGI_SWAP_CHAIN_DESC sd = {};
    m_pSwapChain->GetDesc(&sd);

    hr = m_pSwapChain->ResizeBuffers(
        m_Desc.BufferCount,
        width,
        height,
        desc.Format,
        sd.Flags);
    if (FAILED(hr))
    { return false; }

    m_Desc.Extent.Width  = width;
    m_Desc.Extent.Height = height;

    // 再作成.
    {
        ComponentMapping componentMapping;
        componentMapping.R = TEXTURE_SWIZZLE_R;
        componentMapping.G = TEXTURE_SWIZZLE_G;
        componentMapping.B = TEXTURE_SWIZZLE_B;
        componentMapping.A = TEXTURE_SWIZZLE_A;

        for(auto i=0u; i<m_Desc.BufferCount; ++i)
        {
            ID3D12Resource* pBuffer;

            auto hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBuffer));
            if (SUCCEEDED(hr))
            {
                if (!Texture::CreateFromNative(
                    m_pDevice,
                    pBuffer,
                    RESOURCE_USAGE_COLOR_TARGET,
                    componentMapping,
                    reinterpret_cast<ITexture**>(&m_pBuffers[i])))
                {
                    SafeRelease(pBuffer);
                    return false;
                }
            }

            SafeRelease(pBuffer);
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      メタデータを設定します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::SetMetaData(META_DATA_TYPE type, void* pMetaData)
{
    if (m_pSwapChain4 == nullptr)
    { return false; }

    if (pMetaData == nullptr)
    { return false; }

    switch(type)
    {
    case META_DATA_HDR10:
        {
            auto pData = static_cast<MetaDataHDR10*>(pMetaData);
            A3D_ASSERT(pData != nullptr);

            DXGI_HDR_METADATA_HDR10 meta = {};
            meta.RedPrimary[0]              = GetCoord(pData->PrimaryR[0]);
            meta.RedPrimary[1]              = GetCoord(pData->PrimaryR[1]);
            meta.BluePrimary[0]             = GetCoord(pData->PrimaryB[0]);
            meta.BluePrimary[1]             = GetCoord(pData->PrimaryB[1]);
            meta.GreenPrimary[0]            = GetCoord(pData->PrimaryG[0]);
            meta.GreenPrimary[1]            = GetCoord(pData->PrimaryG[1]);
            meta.WhitePoint[0]              = GetCoord(pData->WhitePoint[0]);
            meta.WhitePoint[1]              = GetCoord(pData->WhitePoint[1]);
            meta.MaxMasteringLuminance      = static_cast<UINT>(pData->MaxMasteringLuminance / 10000.0);
            meta.MinMasteringLuminance      = static_cast<UINT>(pData->MinMasteringLuminance / 100000.0);
            meta.MaxContentLightLevel       = static_cast<UINT16>(pData->MaxContentLightLevel / 100000.0);
            meta.MaxFrameAverageLightLevel  = static_cast<UINT16>(pData->MaxFrameAverageLightLevel / 100000.0);

            auto hr = m_pSwapChain4->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(meta), &meta);
            if (FAILED(hr))
            { return false; }
        }
        break;

    default:
        { /* DO_NOTHING */}
        break;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      フルスクリーンモードかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool SwapChain::IsFullScreenMode() const
{
    if (m_IsTearingSupport)
    { return m_IsFullScreen; }

    BOOL isFullScreen;
    auto hr = m_pSwapChain->GetFullscreenState(&isFullScreen, nullptr);
    A3D_ASSERT(hr == S_OK);
    A3D_UNUSED(hr);
    return (isFullScreen == TRUE);
}

//-------------------------------------------------------------------------------------------------
//      フルスクリーンモードを設定します。
//-------------------------------------------------------------------------------------------------
bool SwapChain::SetFullScreenMode(bool enable)
{
    if (m_IsTearingSupport)
    {
        if (enable)
        {
            // ウィンドウサイズを保存しておく.
            GetWindowRect(m_hWnd, &m_Rect);

            // 余計なものを外す.
            auto style =  m_WindowStyle & 
                          ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME);

            // ウィンドウのスタイルを変更.
            SetWindowLong(m_hWnd, GWL_STYLE, style);

            DEVMODE devMode = {};
            devMode.dmSize = sizeof(DEVMODE);
            EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

            SetWindowPos(
                m_hWnd,
                HWND_TOPMOST,
                devMode.dmPosition.x,
                devMode.dmPosition.y,
                devMode.dmPosition.x + devMode.dmPelsWidth,
                devMode.dmPosition.y + devMode.dmPelsHeight,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);

            // 最大化.
            ShowWindow(m_hWnd, SW_MAXIMIZE);
        }
        else
        {
            // ウィンドウスタイルを元に戻す.
            SetWindowLong(m_hWnd, GWL_STYLE, m_WindowStyle);

            SetWindowPos(
                m_hWnd,
                HWND_NOTOPMOST,
                m_Rect.left,
                m_Rect.top,
                m_Rect.right - m_Rect.left,
                m_Rect.bottom - m_Rect.top,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ShowWindow(m_hWnd, SW_NORMAL);
        }

        m_IsFullScreen = enable;
    }
    else
    {
        auto hr = m_pSwapChain->SetFullscreenState(enable, nullptr);
        if (FAILED(hr))
        { return false; }

        m_IsFullScreen = enable;
    }

    if (m_Desc.SyncInterval == 0 && m_IsTearingSupport && !m_IsFullScreen)
    { m_PresentFlag = DXGI_PRESENT_ALLOW_TEARING; }
    else
    { m_PresentFlag = 0;}

    return true;
}

//-------------------------------------------------------------------------------------------------
//      色空間がサポートされているかチェックします.
//-------------------------------------------------------------------------------------------------
bool SwapChain::CheckColorSpaceSupport(COLOR_SPACE_TYPE type)
{
    uint32_t flags;
    auto hr = m_pSwapChain->CheckColorSpaceSupport(ToNativeColorSpace(type), &flags);
    if (FAILED(hr))
    { return false; }

    return flags & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT;
}

//-------------------------------------------------------------------------------------------------
//      スワップチェインを取得します.
//-------------------------------------------------------------------------------------------------
IDXGISwapChain3* SwapChain::GetDXGISwapChain() const
{ return m_pSwapChain; }

//-------------------------------------------------------------------------------------------------
//      スワップチェイン4を取得します.
//-------------------------------------------------------------------------------------------------
IDXGISwapChain4* SwapChain::GetDXGISwapChain4() const
{ return m_pSwapChain4; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool SwapChain::Create
(
    IDevice*             pDevice,
    IQueue*              pQueue,
    const SwapChainDesc* pDesc,
    ISwapChain**         ppSwapChain
)
{
    if (pDevice == nullptr || pQueue == nullptr || pDesc == nullptr || ppSwapChain == nullptr)
    { return false; }

    auto instance = new SwapChain();
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, pQueue, pDesc))
    {
        instance->Release();
        return false;
    }

    *ppSwapChain = instance;
    return true;
}

} // namespace a3d
