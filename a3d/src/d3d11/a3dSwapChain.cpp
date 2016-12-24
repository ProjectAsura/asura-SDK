//-------------------------------------------------------------------------------------------------
// File : a3dSwapChain.cpp
// Desc : SwapChain Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


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

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    auto pWrapDevice = reinterpret_cast<Device*>(pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetD3D11Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    auto pNativeFactory = pWrapDevice->GetDXGIFactory();
    A3D_ASSERT(pNativeFactory != nullptr);

    m_hWnd = static_cast<HWND>(pDesc->WindowHandle);

    {
        DXGI_SWAP_CHAIN_DESC desc = {};
        desc.BufferDesc.Width   = pDesc->Extent.Width;
        desc.BufferDesc.Height  = pDesc->Extent.Height;
        desc.BufferDesc.Format  = ToNativeFormat(pDesc->Format);
        desc.SampleDesc.Count   = pDesc->SampleCount;
        desc.SampleDesc.Quality = 0;
        desc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
        desc.BufferCount        = pDesc->BufferCount;
        desc.Windowed           = (pDesc->EnableFullScreen) ? FALSE : TRUE;
        desc.OutputWindow       = m_hWnd;

        IDXGISwapChain* pSwapChain = nullptr;
        auto hr = pNativeFactory->CreateSwapChain(pNativeDevice, &desc, &pSwapChain);
        if (FAILED(hr))
        { return false; }

        hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
        SafeRelease(pSwapChain);

        if (FAILED(hr))
        { return false; }
    }

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    {
        m_pBuffers = new ITexture* [m_Desc.BufferCount];
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
                RESOURCE_USAGE_COLOR_TARGET,
                componentMapping,
                &m_pBuffers[i]))
            {
                SafeRelease(pBuffer);
                return false;
            }

            auto pWrapResource = reinterpret_cast<Texture*>(m_pBuffers[i]);
            A3D_ASSERT(pWrapResource != nullptr);

            pWrapResource->SetState(a3d::RESOURCE_STATE_PRESENT);
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
    for(auto i=0u; i<m_Desc.BufferCount; ++i)
    { SafeRelease(m_pBuffers[i]); }

    if (m_pBuffers)
    { delete[] m_pBuffers; }

    // フルスクリーンモードのままだと例外が発生するので，ウィンドウモードに変更する.
    if (m_pSwapChain != nullptr)
    {
        auto isFullScreen = IsFullScreenMode();
        if (isFullScreen)
        { SetFullScreenMode(false); }
    }

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
    if (index >= m_Desc.BufferCount )
    { return false; }

    if (m_pBuffers[index] == nullptr)
    { return false; }

    *ppResource = m_pBuffers[index];
    m_pBuffers[index]->AddRef();

    return true;
}

//-------------------------------------------------------------------------------------------------
//      メタデータを設定します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::SetMetaData(META_DATA_TYPE type, void* pData)
{
    /* NOT SUPPORT */
    A3D_UNUSED(type);
    A3D_UNUSED(pData);
    return false;
}

//-------------------------------------------------------------------------------------------------
//      色空間がサポートされているかチェックします.
//-------------------------------------------------------------------------------------------------
bool SwapChain::CheckColorSpaceSupport(COLOR_SPACE_TYPE type, uint32_t* pFlags)
{
    /* NOT SUPPORT */
    A3D_UNUSED(type);
    A3D_UNUSED(pFlags);
    return false;
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
    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pDisplay = pWrapDevice->GetDXGIOutput();
    A3D_ASSERT(pDisplay != nullptr);

    auto hr = m_pSwapChain->SetFullscreenState(enable, pDisplay);
    if (FAILED(hr))
    { return false; }

    return true;
}

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
