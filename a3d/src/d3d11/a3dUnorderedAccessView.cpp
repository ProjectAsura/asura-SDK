//-------------------------------------------------------------------------------------------------
// File : a3dUnorderedAccessView.cpp
// Desc : Unordered Access View Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// UnorderedAccessView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
UnorderedAccessView::UnorderedAccessView()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pResource   (nullptr)
, m_pSRV        (nullptr)
{ memset(&m_Desc, 0, sizeof(m_Desc)); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
UnorderedAccessView::~UnorderedAccessView()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool UnorderedAccessView::Init(IDevice* pDevice, IResource* pResource, const UnorderedAccessViewDesc* pDesc)
{
    if (pDevice == nullptr || pResource == nullptr || pDesc == nullptr)
    { return false; }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    m_pResource = pResource;
    m_pResource->AddRef();

    auto pNativeDevice = m_pDevice->GetD3D11Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    if (pResource->GetKind() == RESOURCE_KIND_BUFFER)
    {
        auto pWrapBuffer = static_cast<Buffer*>(pResource);
        A3D_ASSERT(pWrapBuffer != nullptr);

        auto bufferDesc = pWrapBuffer->GetDesc();

        if ((bufferDesc.Usage & RESOURCE_USAGE_UNORDERED_ACCESS) != RESOURCE_USAGE_UNORDERED_ACCESS)
        { return false; }

        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
        uav_desc.Format                 = ToNativeFormat(pDesc->Format);
        uav_desc.ViewDimension          = D3D11_UAV_DIMENSION_BUFFER;
        uav_desc.Buffer.FirstElement    = UINT(pDesc->FirstElement);
        uav_desc.Buffer.NumElements     = pDesc->ElementCount;
        uav_desc.Buffer.Flags           = (pDesc->StructuredByteStride == 0) ? D3D11_BUFFER_UAV_FLAG_RAW : 0;

        auto hr = pNativeDevice->CreateUnorderedAccessView(
            pWrapBuffer->GetD3D11Buffer(), &uav_desc, &m_pUAV);
        if (FAILED(hr))
        { return false; }
    }
    else if (pResource->GetKind() == RESOURCE_KIND_TEXTURE)
    {
        auto pWrapTexture = static_cast<Texture*>(pResource);
        A3D_ASSERT(pWrapTexture != nullptr);

        auto textureDesc = pWrapTexture->GetDesc();

        if ((textureDesc.Usage & RESOURCE_USAGE_UNORDERED_ACCESS) != RESOURCE_USAGE_UNORDERED_ACCESS)
        { return false; }

        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
        uav_desc.Format         = ToNativeFormat(pDesc->Format);
        uav_desc.ViewDimension  = ToNativeUAVDimension(pDesc->Dimension);

        switch (uav_desc.ViewDimension)
        {
        case D3D11_UAV_DIMENSION_UNKNOWN:
            { return false; }

        case D3D11_UAV_DIMENSION_BUFFER:
            { return false; }

        case D3D11_UAV_DIMENSION_TEXTURE1D:
            {
                uav_desc.Texture1D.MipSlice = pDesc->MipSlice;
            }
            break;

        case D3D11_UAV_DIMENSION_TEXTURE1DARRAY:
            {
                uav_desc.Texture1DArray.ArraySize       = pDesc->ElementCount;
                uav_desc.Texture1DArray.FirstArraySlice = UINT(pDesc->FirstElement);
                uav_desc.Texture1DArray.MipSlice        = pDesc->MipSlice;
            }
            break;

        case D3D11_UAV_DIMENSION_TEXTURE2D:
            {
                uav_desc.Texture2D.MipSlice = pDesc->MipSlice;
            }
            break;

        case D3D11_UAV_DIMENSION_TEXTURE2DARRAY:
            {
                uav_desc.Texture2DArray.ArraySize       = pDesc->ElementCount;
                uav_desc.Texture2DArray.FirstArraySlice = UINT(pDesc->FirstElement);
                uav_desc.Texture2DArray.MipSlice        = pDesc->MipSlice;
            }
            break;

        case D3D11_UAV_DIMENSION_TEXTURE3D:
            {
                uav_desc.Texture3D.FirstWSlice      = UINT(pDesc->FirstElement);
                uav_desc.Texture3D.MipSlice         = pDesc->MipSlice;
                uav_desc.Texture3D.WSize            = pDesc->ElementCount;
            }
            break;
        }

        auto hr = pNativeDevice->CreateUnorderedAccessView(
            pWrapTexture->GetD3D11Resource(),
            &uav_desc,
            &m_pUAV);
        if ( FAILED(hr) )
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void UnorderedAccessView::Term()
{
    SafeRelease( m_pSRV );
    SafeRelease( m_pResource );
    SafeRelease( m_pDevice );
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void UnorderedAccessView::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void UnorderedAccessView::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t UnorderedAccessView::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void UnorderedAccessView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
UnorderedAccessViewDesc UnorderedAccessView::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
IResource* UnorderedAccessView::GetResource() const
{ return m_pResource; }

//-------------------------------------------------------------------------------------------------
//      アンオーダドアクセスビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11UnorderedAccessView* UnorderedAccessView::GetD3D11UnorderedAccessView() const
{ return m_pUAV; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool UnorderedAccessView::Create
(
    IDevice*                        pDevice,
    IResource*                      pResource,
    const UnorderedAccessViewDesc*  pDesc,
    IUnorderedAccessView**          ppStorageView
)
{
    if (pDevice == nullptr || pResource == nullptr || pDesc == nullptr || ppStorageView == nullptr)
    { return false; }

    auto instance = new UnorderedAccessView();
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, pResource, pDesc))
    {
        SafeRelease(instance);
        return false;
    }

    *ppStorageView = instance;
    return true;
}

} // namespace a3d
