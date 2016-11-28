﻿//-------------------------------------------------------------------------------------------------
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

    auto pWrapDevice = reinterpret_cast<Device*>(pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pWrapQueue = reinterpret_cast<Queue*>(pQueue);
    A3D_ASSERT(pWrapQueue != nullptr);

    auto pNativeDevice = pWrapDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    auto pNativeFactory = pWrapDevice->GetDXGIFactory();
    A3D_ASSERT(pNativeFactory != nullptr);

    auto pNativeQueue = pWrapQueue->GetD3D12Queue();
    A3D_ASSERT(pNativeQueue != nullptr);

    {
        DXGI_SWAP_CHAIN_DESC desc = {};
        desc.BufferDesc.Width   = pDesc->Extent.Width;
        desc.BufferDesc.Height  = pDesc->Extent.Height;
        desc.BufferDesc.Format  = ToNativeFormat(pDesc->Format);
        desc.SampleDesc.Count   = pDesc->SampleCount;
        desc.SampleDesc.Quality = 0;
        desc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
        desc.BufferCount        = pDesc->BufferCount;
        desc.Windowed           = TRUE;
        desc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.OutputWindow       = static_cast<HWND>(pDesc->WindowHandle);

        IDXGISwapChain* pSwapChain = nullptr;
        auto hr = pNativeFactory->CreateSwapChain(pNativeQueue, &desc, &pSwapChain);
        if (FAILED(hr))
        { return false; }

        hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
        SafeRelease(pSwapChain);

        if (FAILED(hr))
        { return false; }

        // メタデータがある場合は設定する.
        if (pDesc->pMetaData != nullptr)
        {
            switch(pDesc->MetaDataType)
            {
            case META_DATA_HDR10:
                {
                    auto pData = static_cast<MetaDataHDR10*>(pDesc->pMetaData);
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

                    hr = m_pSwapChain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(meta), &meta);
                    if (FAILED(hr))
                    { return false; }
                }
                break;

            default:
                break;
            }
        }
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

        for(auto i=0u; i<pDesc->BufferCount; ++i)
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
                    &m_pBuffers[i]))
                {
                    SafeRelease(pBuffer);
                    return false;
                }
            }

            SafeRelease(pBuffer);

            auto pWrapResource = reinterpret_cast<Texture*>(m_pBuffers[i]);
            A3D_ASSERT(pWrapResource != nullptr);

            pWrapResource->SetState(a3d::RESOURCE_STATE_PRESENT);
        }
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
{ m_pSwapChain->Present( m_Desc.SyncInterval, 0); }

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
    if (index >= m_Desc.BufferCount )
    { return false; }

    if (m_pBuffers[index] == nullptr)
    { return false; }

    *ppResource = m_pBuffers[index];
    m_pBuffers[index]->AddRef();

    return true;
}

//-------------------------------------------------------------------------------------------------
//      スワップチェインを取得します.
//-------------------------------------------------------------------------------------------------
IDXGISwapChain4* SwapChain::GetDXGISwapChain() const
{ return m_pSwapChain; }

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
