//-------------------------------------------------------------------------------------------------
// File : a3dDepthStencilView.cpp
// Desc : DepthStencilView Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      深度ステンシルビューの構成設定に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeDSVDesc(const a3d::TargetViewDesc* pDesc, D3D11_DEPTH_STENCIL_VIEW_DESC& dsvDesc)
{
    dsvDesc.Format = a3d::ToNativeFormat(pDesc->Format);

    switch(pDesc->Dimension)
    {
    case a3d::VIEW_DIMENSION_TEXTURE1D:
        {
            dsvDesc.ViewDimension       = D3D11_DSV_DIMENSION_TEXTURE1D;
            dsvDesc.Texture1D.MipSlice  = pDesc->MipSlice;
        }
        break;

    case a3d::VIEW_DIMENSION_TEXTURE1D_ARRAY:
        {
            dsvDesc.ViewDimension                   = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
            dsvDesc.Texture1DArray.ArraySize        = pDesc->ArraySize;
            dsvDesc.Texture1DArray.FirstArraySlice  = pDesc->FirstArraySlice;
            dsvDesc.Texture1DArray.MipSlice         = pDesc->MipSlice;
        }
        break;

    case a3d::VIEW_DIMENSION_TEXTURE2D:
        {
            if (pDesc->MipLevels > 1)
            {
                dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
            }
            else
            {
                dsvDesc.ViewDimension       = D3D11_DSV_DIMENSION_TEXTURE2D;
                dsvDesc.Texture2D.MipSlice  = pDesc->MipSlice;
            }
        }
        break;

    case a3d::VIEW_DIMENSION_CUBEMAP:
    case a3d::VIEW_DIMENSION_CUBEMAP_ARRAY:
    case a3d::VIEW_DIMENSION_TEXTURE2D_ARRAY:
        {
            if (pDesc->MipLevels > 1 )
            {
                dsvDesc.ViewDimension                       = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
                dsvDesc.Texture2DMSArray.ArraySize          = pDesc->ArraySize;
                dsvDesc.Texture2DMSArray.FirstArraySlice    = pDesc->FirstArraySlice;
            }
            else
            {
                dsvDesc.ViewDimension                   = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                dsvDesc.Texture2DArray.ArraySize        = pDesc->ArraySize;
                dsvDesc.Texture2DArray.FirstArraySlice  = pDesc->FirstArraySlice;
                dsvDesc.Texture2DArray.MipSlice         = pDesc->MipSlice;
            }
        }
        break;

    case a3d::VIEW_DIMENSION_TEXTURE3D:
        { /* DO_NOTHING */ }
        break;
    }
}

} // namespace /* anonymous */

namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// DepthStencilView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DepthStencilView::DepthStencilView()
: m_RefCount(1)
, m_pDevice (nullptr)
, m_pTexture(nullptr)
, m_pDSV    (nullptr)
{ memset(&m_Desc, 0, sizeof(m_Desc)); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DepthStencilView::~DepthStencilView()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool DepthStencilView::Init(IDevice* pDevice, ITexture* pTexture, const TargetViewDesc* pDesc)
{
    if (pDevice == nullptr || pTexture == nullptr || pDesc == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pWrapTexture = static_cast<Texture*>(pTexture);
    A3D_ASSERT( pWrapTexture != nullptr );

    m_pTexture = pWrapTexture;
    m_pTexture->AddRef();

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    auto pD3D11Device = m_pDevice->GetD3D11Device();
    A3D_ASSERT(pD3D11Device != nullptr);
    auto textureDesc = m_pTexture->GetDesc();
 
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        ToNativeDSVDesc(pDesc, dsvDesc);

        auto hr = pD3D11Device->CreateDepthStencilView(
            pWrapTexture->GetD3D11Resource(),
            &dsvDesc,
            &m_pDSV);
        if ( FAILED(hr) )
        {
            A3D_LOG("Error : ID3D11Device::CreateDepthStencilView() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DepthStencilView::Term()
{
    SafeRelease( m_pDSV );
    SafeRelease( m_pTexture );
    SafeRelease( m_pDevice );
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void DepthStencilView::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void DepthStencilView::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DepthStencilView::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void DepthStencilView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
TargetViewDesc DepthStencilView::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      深度ステンシルビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11DepthStencilView* DepthStencilView::GetD3D11DepthStencilView() const
{ return m_pDSV; }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ITexture* DepthStencilView::GetResource() const
{ return m_pTexture; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool DepthStencilView::Create
(
    IDevice*                pDevice,
    ITexture*               pTexture,
    const TargetViewDesc*   pDesc,
    IDepthStencilView**     ppDepthStencilView
)
{
    if (pDevice == nullptr || pTexture == nullptr || pDesc == nullptr || ppDepthStencilView == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new DepthStencilView();
    if ( instance == nullptr )
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if ( !instance->Init(pDevice, pTexture, pDesc) )
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppDepthStencilView = instance;
    return true;
}

} // namespace a3d
