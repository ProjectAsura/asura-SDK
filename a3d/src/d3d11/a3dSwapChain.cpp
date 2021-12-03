//-------------------------------------------------------------------------------------------------
// File : a3dSwapChain.cpp
// Desc : SwapChain Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


#if defined(A3D_FOR_WINDOWS10)
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
#endif//defined(A3D_FOR_WINDOWS10)


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
, m_pBuffers    (nullptr)
, m_pSwapChain  (nullptr)
#if defined(A3D_FOR_WINDOWS10)
, m_pSwapChain4 (nullptr)
#endif
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

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    auto pNativeDevice = m_pDevice->GetD3D11Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    auto pNativeFactory = m_pDevice->GetDXGIFactory();
    A3D_ASSERT(pNativeFactory != nullptr);

    m_hWnd = static_cast<HWND>(pDesc->WindowHandle);

    auto w = pDesc->Extent.Width;
    auto h = pDesc->Extent.Height;

    // スクリーンサイズを取得.
    m_FullScreenWidth  = static_cast<uint32_t>(GetSystemMetrics(SM_CXSCREEN));
    m_FullScreenHeight = static_cast<uint32_t>(GetSystemMetrics(SM_CYSCREEN));

    if (pDesc->EnableFullScreen)
    {
        m_Desc.Extent.Width  = m_FullScreenWidth;
        m_Desc.Extent.Height = m_FullScreenHeight;

        w = m_FullScreenWidth;
        h = m_FullScreenHeight;
    }

    // スワップチェイン生成.
    {
        DXGI_SWAP_CHAIN_DESC desc = {};
        desc.BufferDesc.Width   = w;
        desc.BufferDesc.Height  = h;
        desc.BufferDesc.Format  = ToNativeFormat(pDesc->Format);
        desc.SampleDesc.Count   = pDesc->SampleCount;
        desc.SampleDesc.Quality = 0;
        desc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
        desc.BufferCount        = pDesc->BufferCount;
        desc.Windowed           = (pDesc->EnableFullScreen) ? FALSE : TRUE;
        desc.OutputWindow       = m_hWnd;
        desc.Flags              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        desc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        IDXGISwapChain* pSwapChain = nullptr;
        auto hr = pNativeFactory->CreateSwapChain(pNativeDevice, &desc, &m_pSwapChain);
        if (FAILED(hr))
        { return false; }

    #if defined(A3D_FOR_WINDOWS10)
        hr = m_pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain4));
        if (FAILED(hr))
        { SafeRelease(m_pSwapChain4); }
    #endif
    }

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    {
        m_pBuffers = new Texture* [m_Desc.BufferCount];
        if (m_pBuffers == nullptr)
        { return false; }

        ComponentMapping componentMapping;
        componentMapping.R = TEXTURE_SWIZZLE_R;
        componentMapping.G = TEXTURE_SWIZZLE_G;
        componentMapping.B = TEXTURE_SWIZZLE_B;
        componentMapping.A = TEXTURE_SWIZZLE_A;

        ID3D11Texture2D* pBuffer;
        auto hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBuffer));
        if ( FAILED(hr) )
        { return false; }

        for(auto i=0u; i<pDesc->BufferCount; ++i)
        {
            if (!Texture::CreateFromNative(
                pDevice,
                pBuffer,
                RESOURCE_USAGE_RENDER_TARGET_VIEW,
                componentMapping,
                reinterpret_cast<ITexture**>(&m_pBuffers[i])))
            {
                SafeRelease(pBuffer);
                return false;
            }
        }

        SafeRelease(pBuffer);
    }

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

#if defined(A3D_FOR_WINDOWS10)
    SafeRelease(m_pSwapChain4);
#endif//defiend(A3D_FOW_WINDOWS10)
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
{
    m_pSwapChain->Present( m_Desc.SyncInterval, 0);
    m_BufferIndex = (m_BufferIndex + 1) % m_Desc.BufferCount;
}

//-------------------------------------------------------------------------------------------------
//      現在のバッファ番号を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t SwapChain::GetCurrentBufferIndex()
{ return m_BufferIndex; }

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
    for(auto i=0u; i<m_Desc.BufferCount; ++i)
    { SafeRelease(m_pBuffers[i]); }

    {
        DXGI_MODE_DESC desc = {};
        desc.Width                      = width;
        desc.Height                     = height;
        desc.RefreshRate.Numerator      = 60;
        desc.RefreshRate.Denominator    = 1;
        desc.Format                     = ToNativeFormat(m_Desc.Format);

        auto hr = m_pSwapChain->ResizeTarget(&desc);
        if ( FAILED(hr) )
        { return false; }

        hr = m_pSwapChain->ResizeBuffers(
            m_Desc.BufferCount,
            width,
            height,
            desc.Format,
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
        if (FAILED(hr))
        { return false; }
    }

    m_Desc.Extent.Width  = width;
    m_Desc.Extent.Height = height;

    // 作成し直す.
    {
        ComponentMapping componentMapping;
        componentMapping.R = TEXTURE_SWIZZLE_R;
        componentMapping.G = TEXTURE_SWIZZLE_G;
        componentMapping.B = TEXTURE_SWIZZLE_B;
        componentMapping.A = TEXTURE_SWIZZLE_A;

        ID3D11Texture2D* pBuffer;
        auto hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBuffer));
        if ( FAILED(hr) )
        { return false; }

        for(auto i=0u; i<m_Desc.BufferCount; ++i)
        {
            if (!Texture::CreateFromNative(
                m_pDevice,
                pBuffer,
                RESOURCE_USAGE_RENDER_TARGET_VIEW,
                componentMapping,
                reinterpret_cast<ITexture**>(&m_pBuffers[i])))
            {
                SafeRelease(pBuffer);
                return false;
            }
        }

        SafeRelease(pBuffer);
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      メタデータを設定します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::SetMetaData(META_DATA_TYPE type, void* pMetaData)
{
    #if defined(A3D_FOR_WINDOWS10)
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
    #else
    {
        /* NOT SUPPORT */
        A3D_UNUSED(type);
        A3D_UNUSED(pMetaData);
        return false;
    }
    #endif
}

//-------------------------------------------------------------------------------------------------
//      色空間がサポートされているかチェックします.
//-------------------------------------------------------------------------------------------------
bool SwapChain::CheckColorSpaceSupport(COLOR_SPACE_TYPE type)
{
    #if defined(A3D_FOR_WINDOWS10)
    {
        if (m_pSwapChain4 == nullptr)
        { return false; }

        // HDRディスプレイをサポートしているかどうかチェックする.
        if (type == COLOR_SPACE_BT2100_PQ || type == COLOR_SPACE_BT2100_HLG)
        {
            RECT region;
            GetWindowRect(m_hWnd, &region);

            if (!m_pDevice->CheckDisplayHDRSupport(region))
            { return false; }
        }

        uint32_t flags;
        auto hr = m_pSwapChain4->CheckColorSpaceSupport(ToNativeColorSpace(type), &flags);
        if (FAILED(hr))
        { return false; }

        return flags & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT;
    }
    #else
    {
        /* NOT SUPPORT */
        A3D_UNUSED(type);
        A3D_UNUSED(pFlags);
        return false;
    }
    #endif
}

//-------------------------------------------------------------------------------------------------
//      色空間を設定します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::SetColorSpace(COLOR_SPACE_TYPE type)
{
    #if defined(A3D_FOR_WINDOWS10)
    {
        if (type == COLOR_SPACE_BT2100_PQ || type == COLOR_SPACE_BT2100_HLG)
        {
            RECT region;
            GetWindowRect(m_hWnd, &region);

            if (!m_pDevice->CheckDisplayHDRSupport(region))
            { return false; }
        }

        auto color_space = ToNativeColorSpace(type);
        auto hr = m_pSwapChain4->SetColorSpace1(color_space);
        if (FAILED(hr))
        { return false; }

        return true;
    }
    #else
    {
        return false;
    }
    #endif
}

//-------------------------------------------------------------------------------------------------
//      フルスクリーンモードかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool SwapChain::IsFullScreenMode() const
{
    BOOL isFullScreen;
    auto hr = m_pSwapChain->GetFullscreenState(&isFullScreen, nullptr);
    A3D_ASSERT(hr == S_OK);
    A3D_UNUSED(hr);

    return isFullScreen == TRUE;
}

//-------------------------------------------------------------------------------------------------
//      フルスクリーンモードを設定します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::SetFullScreenMode(bool enable)
{
    auto hr = m_pSwapChain->SetFullscreenState(enable, nullptr);
    if (FAILED(hr))
    { return false; }

    if (enable)
    {
        ResizeBuffers(m_FullScreenWidth, m_FullScreenHeight); 
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      スワップチェインを取得します.
//-------------------------------------------------------------------------------------------------
IDXGISwapChain* SwapChain::GetDXGISwapChain() const
{ return m_pSwapChain; }

#if defined(A3D_FOR_WINDOWS10)
//-------------------------------------------------------------------------------------------------
//      スワップチェイン4を取得します.
//-------------------------------------------------------------------------------------------------
IDXGISwapChain4* SwapChain::GetDXGISwapChain4() const
{ return m_pSwapChain4; }

#endif // defined(A3D_FOR_WINDOWS10)

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
