//-------------------------------------------------------------------------------------------------
// File : a3dRenderTargetView.cpp
// Desc : RenderTargetView Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      レンダーターゲットビューの構成設定に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeRTVDesc(const a3d::TargetViewDesc* pDesc, D3D11_RENDER_TARGET_VIEW_DESC& rtvDesc)
{
    rtvDesc.Format = a3d::ToNativeFormat(pDesc->Format);

    switch(pDesc->Dimension)
    {
    case a3d::VIEW_DIMENSION_TEXTURE1D:
        {
            rtvDesc.ViewDimension       = D3D11_RTV_DIMENSION_TEXTURE1D;
            rtvDesc.Texture1D.MipSlice  = pDesc->MipSlice;
        }
        break;

    case a3d::VIEW_DIMENSION_TEXTURE1D_ARRAY:
        {
            rtvDesc.ViewDimension                   = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
            rtvDesc.Texture1DArray.ArraySize        = pDesc->ArraySize;
            rtvDesc.Texture1DArray.FirstArraySlice  = pDesc->FirstArraySlice;
            rtvDesc.Texture1DArray.MipSlice         = pDesc->MipSlice;
        }
        break;

    case a3d::VIEW_DIMENSION_TEXTURE2D:
        {
            if (pDesc->MipLevels > 1)
            {
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
            }
            else
            {
                rtvDesc.ViewDimension       = D3D11_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice  = pDesc->MipSlice;
            }
        }
        break;

    case a3d::VIEW_DIMENSION_CUBEMAP:
    case a3d::VIEW_DIMENSION_CUBEMAP_ARRAY:
    case a3d::VIEW_DIMENSION_TEXTURE2D_ARRAY:
        {
            if (pDesc->MipLevels > 1)
            {
                rtvDesc.ViewDimension                       = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                rtvDesc.Texture2DMSArray.ArraySize          = pDesc->ArraySize;
                rtvDesc.Texture2DMSArray.FirstArraySlice    = pDesc->FirstArraySlice;
            }
            else
            {
                rtvDesc.ViewDimension                   = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                rtvDesc.Texture2DArray.ArraySize        = pDesc->ArraySize;
                rtvDesc.Texture2DArray.FirstArraySlice  = pDesc->FirstArraySlice;
                rtvDesc.Texture2DArray.MipSlice         = pDesc->MipSlice;
            }
        }
        break;

    case a3d::VIEW_DIMENSION_TEXTURE3D:
        {
            rtvDesc.ViewDimension           = D3D11_RTV_DIMENSION_TEXTURE3D;
            rtvDesc.Texture3D.FirstWSlice   = pDesc->FirstArraySlice;
            rtvDesc.Texture3D.MipSlice      = pDesc->MipSlice;
            rtvDesc.Texture3D.WSize         = pDesc->ArraySize;
        }
        break;
    }
}

} // namespace /* anonymous */

namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderTargetView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
RenderTargetView::RenderTargetView()
: m_RefCount(1)
, m_pDevice (nullptr)
, m_pTexture(nullptr)
, m_pRTV    (nullptr)
{ memset(&m_Desc, 0, sizeof(m_Desc)); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
RenderTargetView::~RenderTargetView()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool RenderTargetView::Init(IDevice* pDevice, ITexture* pTexture, const TargetViewDesc* pDesc)
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
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        ToNativeRTVDesc(pDesc, rtvDesc);

        auto hr = pD3D11Device->CreateRenderTargetView(
            pWrapTexture->GetD3D11Resource(),
            &rtvDesc,
            &m_pRTV);
        if ( FAILED(hr) )
        {
            A3D_LOG("Error : ID3D11Device::CreateRenderTargetView() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void RenderTargetView::Term()
{
    SafeRelease( m_pRTV );
    SafeRelease( m_pTexture );
    SafeRelease( m_pDevice );
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void RenderTargetView::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void RenderTargetView::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t RenderTargetView::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void RenderTargetView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
TargetViewDesc RenderTargetView::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      レンダーターゲットビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11RenderTargetView* RenderTargetView::GetD3D11RenderTargetView() const
{ return m_pRTV; }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ITexture* RenderTargetView::GetResource() const
{ return m_pTexture; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool RenderTargetView::Create
(
    IDevice*                pDevice,
    ITexture*               pTexture,
    const TargetViewDesc*   pDesc,
    IRenderTargetView**     ppTextureView
)
{
    if (pDevice == nullptr || pTexture == nullptr || pDesc == nullptr || ppTextureView == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new RenderTargetView();
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

    *ppTextureView = instance;
    return true;
}

} // namespace a3d
