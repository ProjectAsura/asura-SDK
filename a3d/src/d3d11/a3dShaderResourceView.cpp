//-------------------------------------------------------------------------------------------------
// File : a3dShaderResourceView.cpp
// Desc : ShaderResourceView Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューの構成設定に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeSRVDesc(const a3d::ShaderResourceViewDesc* pDesc, D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc, bool isStencil)
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
            srvDesc.Texture1DArray.ArraySize        = pDesc->ElementCount;
            srvDesc.Texture1DArray.FirstArraySlice  = pDesc->FirstElement;
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
                srvDesc.Texture2DMSArray.ArraySize       = pDesc->ElementCount;
                srvDesc.Texture2DMSArray.FirstArraySlice = pDesc->FirstElement;
            }
            else
            {
                srvDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                srvDesc.Texture2DArray.ArraySize        = pDesc->ElementCount;
                srvDesc.Texture2DArray.FirstArraySlice  = pDesc->FirstElement;
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
            srvDesc.TextureCubeArray.First2DArrayFace   = pDesc->FirstElement;
            srvDesc.TextureCubeArray.MipLevels          = pDesc->MipLevels;
            srvDesc.TextureCubeArray.NumCubes           = pDesc->ElementCount / 6;
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

} // namespace /* anonymous */

namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ShaderResourceView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
ShaderResourceView::ShaderResourceView()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pResource   (nullptr)
, m_pSRV        (nullptr)
{ memset(&m_Desc, 0, sizeof(m_Desc)); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
ShaderResourceView::~ShaderResourceView()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool ShaderResourceView::Init(IDevice* pDevice, IResource* pResource, const ShaderResourceViewDesc* pDesc)
{
    if (pDevice == nullptr || pResource == nullptr || pDesc == nullptr)
    { return false; }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    m_pResource = pResource;
    m_pResource->AddRef();

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    auto pD3D11Device = m_pDevice->GetD3D11Device();
    A3D_ASSERT(pD3D11Device != nullptr);


    if (pResource->GetKind() == RESOURCE_KIND_BUFFER)
    {
        auto pWrapBuffer = static_cast<Buffer*>(pResource);
        A3D_ASSERT(pWrapBuffer != nullptr);

        auto bufferDesc = pWrapBuffer->GetDesc();

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension           = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.ElementOffset    = 0;
        srvDesc.Buffer.ElementWidth     = UINT(bufferDesc.Size);
        srvDesc.Buffer.FirstElement     = pDesc->FirstElement;
        srvDesc.Buffer.NumElements      = pDesc->ElementCount;

        auto hr = pD3D11Device->CreateShaderResourceView(
            pWrapBuffer->GetD3D11Buffer(),
            &srvDesc,
            &m_pSRV);
        if (FAILED(hr))
        { return false; }
    }
    else
    {
        auto pWrapTexture = static_cast<Texture*>(pResource);
        A3D_ASSERT( pWrapTexture != nullptr );

        auto textureDesc = pWrapTexture->GetDesc();

        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            ToNativeSRVDesc(pDesc, srvDesc, false);

            auto hr = pD3D11Device->CreateShaderResourceView(
                pWrapTexture->GetD3D11Resource(),
                &srvDesc,
                &m_pSRV);
            if ( FAILED(hr) )
            { return false; }
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void ShaderResourceView::Term()
{
    SafeRelease( m_pSRV );
    SafeRelease( m_pResource );
    SafeRelease( m_pDevice );
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void ShaderResourceView::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void ShaderResourceView::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t ShaderResourceView::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void ShaderResourceView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
ShaderResourceViewDesc ShaderResourceView::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11ShaderResourceView* ShaderResourceView::GetD3D11ShaderResourceView() const
{ return m_pSRV; }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
IResource* ShaderResourceView::GetResource() const
{ return m_pResource; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool ShaderResourceView::Create
(
    IDevice*                        pDevice,
    IResource*                      pResource,
    const ShaderResourceViewDesc*   pDesc,
    IShaderResourceView**           ppTextureView
)
{
    if (pDevice == nullptr || pResource == nullptr || pDesc == nullptr || ppTextureView == nullptr)
    { return false; }

    auto instance = new ShaderResourceView();
    if ( instance == nullptr )
    { return false; }

    if ( !instance->Init(pDevice, pResource, pDesc) )
    {
        SafeRelease(instance);
        return false;
    }

    *ppTextureView = instance;
    return true;
}

} // namespace a3d
