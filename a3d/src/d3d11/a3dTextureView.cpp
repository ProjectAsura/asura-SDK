//-------------------------------------------------------------------------------------------------
// File : a3dTextureView.cpp
// Desc : Texture View Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      レンダーターゲットビューの構成設定に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeRTVDesc(const a3d::TextureViewDesc* pDesc, D3D11_RENDER_TARGET_VIEW_DESC& rtvDesc)
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

//-------------------------------------------------------------------------------------------------
//      深度ステンシルビューの構成設定に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeDSVDesc(const a3d::TextureViewDesc* pDesc, D3D11_DEPTH_STENCIL_VIEW_DESC& dsvDesc)
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

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューの構成設定に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeSRVDesc(const a3d::TextureViewDesc* pDesc, D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc, bool isStencil)
{
    srvDesc.Format = a3d::ToNativeViewFormat(pDesc->Format, isStencil);

    switch(pDesc->Dimension)
    {
    case a3d::VIEW_DIMENSION_TEXTURE1D:
        {
            srvDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE1D;
            srvDesc.Texture1D.MipLevels = pDesc->MipLevels;
        }
        break;

    case a3d::VIEW_DIMENSION_TEXTURE1D_ARRAY:
        {
            srvDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
            srvDesc.Texture1DArray.ArraySize        = pDesc->ArraySize;
            srvDesc.Texture1DArray.FirstArraySlice  = pDesc->FirstArraySlice;
            srvDesc.Texture1DArray.MipLevels        = pDesc->MipLevels;
            srvDesc.Texture1DArray.MostDetailedMip  = 0;
        }
        break;

    case a3d::VIEW_DIMENSION_TEXTURE2D:
        {
            if (pDesc->MipLevels > 1)
            {
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
            }
            else
            {
                srvDesc.ViewDimension               = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels         = pDesc->MipLevels;
                srvDesc.Texture2D.MostDetailedMip   = 0;
            }
        }
        break;

    case a3d::VIEW_DIMENSION_TEXTURE2D_ARRAY:
        {
            if (pDesc->MipLevels > 1)
            {
                srvDesc.ViewDimension                    = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
                srvDesc.Texture2DMSArray.ArraySize       = pDesc->ArraySize;
                srvDesc.Texture2DMSArray.FirstArraySlice = pDesc->FirstArraySlice;
            }
            else
            {
                srvDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                srvDesc.Texture2DArray.ArraySize        = pDesc->ArraySize;
                srvDesc.Texture2DArray.FirstArraySlice  = pDesc->FirstArraySlice;
                srvDesc.Texture2DArray.MipLevels        = pDesc->MipLevels;
                srvDesc.Texture2DArray.MostDetailedMip  = 0;
            }
        }
        break;

    case a3d::VIEW_DIMENSION_CUBEMAP:
        {
            srvDesc.ViewDimension               = D3D11_SRV_DIMENSION_TEXTURECUBE;
            srvDesc.TextureCube.MipLevels       = pDesc->MipLevels;
            srvDesc.TextureCube.MostDetailedMip = 0;
        }
        break;

    case a3d::VIEW_DIMENSION_CUBEMAP_ARRAY:
        {
            srvDesc.ViewDimension                       = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
            srvDesc.TextureCubeArray.First2DArrayFace   = pDesc->FirstArraySlice;
            srvDesc.TextureCubeArray.MipLevels          = pDesc->MipLevels;
            srvDesc.TextureCubeArray.NumCubes           = pDesc->ArraySize / 6;
            srvDesc.TextureCubeArray.MostDetailedMip    = 0;
        }
        break;

    case a3d::VIEW_DIMENSION_TEXTURE3D:
        {
            srvDesc.ViewDimension               = D3D11_SRV_DIMENSION_TEXTURE3D;
            srvDesc.Texture3D.MipLevels         = pDesc->MipLevels;
            srvDesc.Texture3D.MostDetailedMip   = 0;
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
//      アンオーダードアクセスビューの構成設定に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeUAVDesc(const a3d::TextureViewDesc* pDesc, D3D11_UNORDERED_ACCESS_VIEW_DESC& uavDesc, bool isStencil)
{
    uavDesc.Format = a3d::ToNativeViewFormat(pDesc->Format, isStencil);

    switch(pDesc->Dimension)
    {
    case a3d::VIEW_DIMENSION_TEXTURE1D:
        {
            uavDesc.ViewDimension       = D3D11_UAV_DIMENSION_TEXTURE1D;
            uavDesc.Texture1D.MipSlice  = pDesc->MipSlice;
        }
        break;

    case a3d::VIEW_DIMENSION_TEXTURE1D_ARRAY:
        {
            uavDesc.ViewDimension                   = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
            uavDesc.Texture1DArray.ArraySize        = pDesc->ArraySize;
            uavDesc.Texture1DArray.FirstArraySlice  = pDesc->FirstArraySlice;
            uavDesc.Texture1DArray.MipSlice         = pDesc->MipSlice;
        }
        break;

    case a3d::VIEW_DIMENSION_TEXTURE2D:
        {
            uavDesc.ViewDimension       = D3D11_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Texture2D.MipSlice  = pDesc->MipSlice;
        }
        break;

    case a3d::VIEW_DIMENSION_CUBEMAP:
    case a3d::VIEW_DIMENSION_CUBEMAP_ARRAY:
    case a3d::VIEW_DIMENSION_TEXTURE2D_ARRAY:
        {
            uavDesc.ViewDimension                   = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
            uavDesc.Texture2DArray.ArraySize        = pDesc->ArraySize;
            uavDesc.Texture2DArray.FirstArraySlice  = pDesc->FirstArraySlice;
            uavDesc.Texture2DArray.MipSlice         = pDesc->MipSlice;
        }
        break;

    case a3d::VIEW_DIMENSION_TEXTURE3D:
        {
            uavDesc.ViewDimension           = D3D11_UAV_DIMENSION_TEXTURE3D;
            uavDesc.Texture3D.FirstWSlice   = pDesc->FirstArraySlice;
            uavDesc.Texture3D.MipSlice      = pDesc->MipSlice;
            uavDesc.Texture3D.WSize         = pDesc->ArraySize;
        }
        break;
    }
}

} // namespace /* anonymous */

namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// TextureView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
TextureView::TextureView()
: m_RefCount(1)
, m_pDevice (nullptr)
, m_pTexture(nullptr)
, m_pSRV    (nullptr)
, m_pRTV    (nullptr)
, m_pDSV    (nullptr)
, m_pUAV    (nullptr)
{ memset(&m_Desc, 0, sizeof(m_Desc)); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
TextureView::~TextureView()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool TextureView::Init(IDevice* pDevice, ITexture* pTexture, const TextureViewDesc* pDesc)
{
    if (pDevice == nullptr || pTexture == nullptr || pDesc == nullptr)
    { return false; }

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
    auto& textureDesc = m_pTexture->GetDesc();
 
    if (textureDesc.Usage & RESOURCE_USAGE_COLOR_TARGET)
    {
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        ToNativeRTVDesc(pDesc, rtvDesc);

        auto hr = pD3D11Device->CreateRenderTargetView(
            pWrapTexture->GetD3D11Resource(),
            &rtvDesc,
            &m_pRTV);
        if ( FAILED(hr) )
        { return false; }
    }

    if (textureDesc.Usage & RESOURCE_USAGE_DEPTH_TARGET)
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        ToNativeDSVDesc(pDesc, dsvDesc);

        auto hr = pD3D11Device->CreateDepthStencilView(
            pWrapTexture->GetD3D11Resource(),
            &dsvDesc,
            &m_pDSV);
        if ( FAILED(hr) )
        { return false; }
    }

    if (textureDesc.Usage & RESOURCE_USAGE_SHADER_RESOURCE)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        ToNativeSRVDesc(pDesc, srvDesc, pDesc->TextureAspect == TEXTURE_ASPECT_STENCIL);

        auto hr = pD3D11Device->CreateShaderResourceView(
            pWrapTexture->GetD3D11Resource(),
            &srvDesc,
            &m_pSRV);
        if ( FAILED(hr) )
        { return false; }
    }

    if (textureDesc.Usage & RESOURCE_USAGE_UNORDERD_ACCESS)
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        ToNativeUAVDesc(pDesc, uavDesc, pDesc->TextureAspect == TEXTURE_ASPECT_STENCIL);

        auto hr = pD3D11Device->CreateUnorderedAccessView(
            pWrapTexture->GetD3D11Resource(),
            &uavDesc,
            &m_pUAV);
        if ( FAILED(hr) )
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void TextureView::Term()
{
    SafeRelease( m_pSRV );
    SafeRelease( m_pRTV );
    SafeRelease( m_pDSV );
    SafeRelease( m_pUAV );
    SafeRelease( m_pTexture );
    SafeRelease( m_pDevice );
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void TextureView::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void TextureView::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t TextureView::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void TextureView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
TextureViewDesc TextureView::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11ShaderResourceView* TextureView::GetD3D11ShaderResourceView() const
{ return m_pSRV; }

//-------------------------------------------------------------------------------------------------
//      レンダーターゲットビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11RenderTargetView* TextureView::GetD3D11RenderTargetView() const
{ return m_pRTV; }

//-------------------------------------------------------------------------------------------------
//      深度ステンシルビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11DepthStencilView* TextureView::GetD3D11DepthStencilView() const
{ return m_pDSV; }

//-------------------------------------------------------------------------------------------------
//      アンオーダードアクセスビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11UnorderedAccessView* TextureView::GetD3D11UnorderedAccessView() const
{ return m_pUAV; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool TextureView::Create
(
    IDevice*                pDevice,
    ITexture*               pTexture,
    const TextureViewDesc*  pDesc,
    ITextureView**          ppTextureView
)
{
    if (pDevice == nullptr || pTexture == nullptr || pDesc == nullptr || ppTextureView == nullptr)
    { return false; }

    auto instance = new TextureView;
    if ( instance == nullptr )
    { return false; }

    if ( !instance->Init(pDevice, pTexture, pDesc) )
    {
        SafeRelease(instance);
        return false;
    }

    *ppTextureView = instance;
    return true;
}

} // namespace a3d
