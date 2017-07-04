//-------------------------------------------------------------------------------------------------
// File : a3dTextureView.cpp
// Desc : Texture View Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// TextureView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
TextureView::TextureView()
: m_RefCount            (1)
, m_pDevice             (nullptr)
, m_pTexture            (nullptr)
, m_pTargetDescriptor   (nullptr)
, m_pShaderDescriptor   (nullptr)
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

    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT( pNativeDevice != nullptr );

    auto pWrapTexture = reinterpret_cast<Texture*>(pTexture);
    A3D_ASSERT( pWrapTexture != nullptr );

    m_pTexture = pWrapTexture;
    m_pTexture->AddRef();

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    auto textureDesc = m_pTexture->GetDesc();
    if (textureDesc.Usage & RESOURCE_USAGE_COLOR_TARGET)
    {
        m_pTargetDescriptor = m_pDevice
                                    ->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
                                    ->CreateDescriptor();
        if (m_pTargetDescriptor == nullptr)
        { return false; }

        D3D12_RENDER_TARGET_VIEW_DESC desc = {};
        desc.Format = ToNativeFormat(pDesc->Format);
        desc.ViewDimension = ToNativeRTVDimension(pDesc->Dimension);

        switch(desc.ViewDimension)
        {
        case D3D12_RTV_DIMENSION_BUFFER:
            {
                desc.Buffer.FirstElement = pDesc->FirstArraySlice;
                desc.Buffer.NumElements  = pDesc->ArraySize;
            }
            break;

        case D3D12_RTV_DIMENSION_TEXTURE1D:
            {
                desc.Texture1D.MipSlice = pDesc->MipSlice;
            }
            break;

        case D3D12_RTV_DIMENSION_TEXTURE1DARRAY:
            {
                desc.Texture1DArray.FirstArraySlice = pDesc->FirstArraySlice;
                desc.Texture1DArray.MipSlice        = pDesc->MipSlice;
                desc.Texture1DArray.ArraySize       = pDesc->ArraySize;
            }
            break;

        case D3D12_RTV_DIMENSION_TEXTURE2D:
            {
                if (textureDesc.SampleCount > 1)
                {
                    desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    desc.Texture2D.MipSlice     = pDesc->MipSlice;
                    desc.Texture2D.PlaneSlice   = 0;
                }
            }
            break;

        case D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
            {
                if (textureDesc.SampleCount > 1)
                {
                    desc.ViewDimension                      = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
                    desc.Texture2DMSArray.ArraySize         = pDesc->ArraySize;
                    desc.Texture2DMSArray.FirstArraySlice   = pDesc->FirstArraySlice;
                }
                else
                {
                    desc.Texture2DArray.ArraySize       = pDesc->ArraySize;
                    desc.Texture2DArray.FirstArraySlice = pDesc->FirstArraySlice;
                    desc.Texture2DArray.MipSlice        = pDesc->MipSlice;
                    desc.Texture2DArray.PlaneSlice      = 0;
                }
            }
            break;

        case D3D12_RTV_DIMENSION_TEXTURE3D:
            {
                desc.Texture3D.FirstWSlice  = pDesc->FirstArraySlice;
                desc.Texture3D.MipSlice     = pDesc->MipSlice;
                desc.Texture3D.WSize        = pDesc->ArraySize;
            }
            break;
        }

        pNativeDevice->CreateRenderTargetView(
            m_pTexture->GetD3D12Resource(),
            &desc,
            m_pTargetDescriptor->GetHandleCPU() );
    }
    else if (textureDesc.Usage & RESOURCE_USAGE_DEPTH_TARGET)
    {
        m_pTargetDescriptor = m_pDevice
                                    ->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
                                    ->CreateDescriptor();
        if (m_pTargetDescriptor == nullptr)
        { return false; }

        D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
        desc.Format         = ToNativeFormat(pDesc->Format);
        desc.ViewDimension  = ToNativeDSVDimension(pDesc->Dimension);
 
        switch(desc.ViewDimension)
        {
        case D3D12_DSV_DIMENSION_UNKNOWN:
            break;

        case D3D12_DSV_DIMENSION_TEXTURE1D:
            {
                desc.Texture1D.MipSlice = pDesc->MipSlice;
            }
            break;

        case D3D12_DSV_DIMENSION_TEXTURE1DARRAY:
            {
                desc.Texture1DArray.ArraySize       = pDesc->ArraySize;
                desc.Texture1DArray.FirstArraySlice = pDesc->FirstArraySlice;
                desc.Texture1DArray.MipSlice        = pDesc->MipSlice;
            }
            break;

        case D3D12_DSV_DIMENSION_TEXTURE2D:
            {
                if (textureDesc.SampleCount > 1)
                {
                    desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    desc.Texture2D.MipSlice = pDesc->MipSlice;
                }
            }
            break;

        case D3D12_DSV_DIMENSION_TEXTURE2DARRAY:
            {
                if (textureDesc.SampleCount > 1)
                {
                    desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
                    desc.Texture2DMSArray.ArraySize         = pDesc->ArraySize;
                    desc.Texture2DMSArray.FirstArraySlice   = pDesc->FirstArraySlice;
                }
                else
                {
                    desc.Texture2DArray.ArraySize       = pDesc->ArraySize;
                    desc.Texture2DArray.FirstArraySlice = pDesc->FirstArraySlice;
                    desc.Texture2DArray.MipSlice        = pDesc->MipSlice;
                }
            }
            break;
        }

        pNativeDevice->CreateDepthStencilView(
            m_pTexture->GetD3D12Resource(),
            &desc,
            m_pTargetDescriptor->GetHandleCPU() );
    }

    if (textureDesc.Usage & RESOURCE_USAGE_SHADER_RESOURCE)
    {
        m_pShaderDescriptor = m_pDevice
                                    ->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
                                    ->CreateDescriptor();
        if (m_pShaderDescriptor == nullptr)
        { return false; }

        auto viewFormat = ToNativeViewFormat(pDesc->Format, pDesc->TextureAspect == TEXTURE_ASPECT_STENCIL);

        if (textureDesc.Usage & RESOURCE_USAGE_UNORDERD_ACCESS)
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
            desc.Format         = viewFormat;
            desc.ViewDimension  = ToNativeUAVDimension(pDesc->Dimension);

            switch(desc.ViewDimension)
            {
            case D3D12_UAV_DIMENSION_BUFFER:
                {
                    desc.Buffer.CounterOffsetInBytes = 0;
                    desc.Buffer.FirstElement         = pDesc->FirstArraySlice;
                    desc.Buffer.NumElements          = pDesc->ArraySize;
                    desc.Buffer.StructureByteStride  = 0;
                    desc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE;
                }
                break;

            case D3D12_UAV_DIMENSION_TEXTURE1D:
                {
                    desc.Texture1D.MipSlice = pDesc->MipSlice;
                }
                break;

            case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
                {
                    desc.Texture1DArray.ArraySize       = pDesc->ArraySize;
                    desc.Texture1DArray.FirstArraySlice = pDesc->FirstArraySlice;
                    desc.Texture1DArray.MipSlice        = pDesc->MipSlice;
                }
                break;

            case D3D12_UAV_DIMENSION_TEXTURE2D:
                {
                    desc.Texture2D.MipSlice     = pDesc->MipSlice;
                    desc.Texture2D.PlaneSlice   = 0;
                }
                break;

            case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
                {
                    desc.Texture2DArray.ArraySize       = pDesc->ArraySize;
                    desc.Texture2DArray.FirstArraySlice = pDesc->FirstArraySlice;
                    desc.Texture2DArray.MipSlice        = pDesc->MipSlice;
                    desc.Texture2DArray.PlaneSlice      = 0;
                }
                break;

            case D3D12_UAV_DIMENSION_TEXTURE3D:
                {
                    desc.Texture3D.FirstWSlice  = pDesc->FirstArraySlice;
                    desc.Texture3D.MipSlice     = pDesc->MipSlice;
                    desc.Texture3D.WSize        = pDesc->ArraySize;
                }
                break;
            }

            pNativeDevice->CreateUnorderedAccessView(
                m_pTexture->GetD3D12Resource(),
                nullptr,
                &desc,
                m_pShaderDescriptor->GetHandleCPU() );
        }
        else
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
            desc.Format         = viewFormat;
            desc.ViewDimension  = ToNativeSRVDimension(pDesc->Dimension);
            desc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(
                                            pDesc->ComponentMapping.R,
                                            pDesc->ComponentMapping.G,
                                            pDesc->ComponentMapping.B,
                                            pDesc->ComponentMapping.A);

            switch(desc.ViewDimension)
            {
            case D3D12_SRV_DIMENSION_BUFFER:
                {
                    desc.Buffer.FirstElement        = pDesc->FirstArraySlice;
                    desc.Buffer.NumElements         = pDesc->ArraySize;
                    desc.Buffer.StructureByteStride = 0;
                    desc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
                }
                break;

            case D3D12_SRV_DIMENSION_TEXTURE1D:
                {
                    desc.Texture1D.MipLevels            = pDesc->MipLevels;
                    desc.Texture1D.MostDetailedMip      = 0;
                    desc.Texture1D.ResourceMinLODClamp  = 0.0f;
                }
                break;

            case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
                {
                    desc.Texture1DArray.ArraySize           = pDesc->ArraySize;
                    desc.Texture1DArray.FirstArraySlice     = pDesc->FirstArraySlice;
                    desc.Texture1DArray.MipLevels           = pDesc->MipLevels;
                    desc.Texture1DArray.MostDetailedMip     = 0;
                    desc.Texture1DArray.ResourceMinLODClamp = 0.0f;
                }
                break;

            case D3D12_SRV_DIMENSION_TEXTURE2D:
                {
                    if (textureDesc.SampleCount > 1)
                    {
                        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                    }
                    else
                    {
                        desc.Texture2D.MipLevels            = pDesc->MipLevels;
                        desc.Texture2D.MostDetailedMip      = 0;
                        desc.Texture2D.PlaneSlice           = 0;
                        desc.Texture2D.ResourceMinLODClamp  = 0.0f;
                    }
                }
                break;

            case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
                {
                    if (textureDesc.SampleCount > 1)
                    {
                        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                        desc.Texture2DMSArray.ArraySize         = pDesc->ArraySize;
                        desc.Texture2DMSArray.FirstArraySlice   = pDesc->FirstArraySlice;
                    }
                    else
                    {
                        desc.Texture2DArray.ArraySize           = pDesc->ArraySize;
                        desc.Texture2DArray.FirstArraySlice     = pDesc->FirstArraySlice;
                        desc.Texture2DArray.MipLevels           = pDesc->MipLevels;
                        desc.Texture2DArray.MostDetailedMip     = 0;
                        desc.Texture2DArray.PlaneSlice          = 0;
                        desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
                    }
                }
                break;

            case D3D12_SRV_DIMENSION_TEXTURE3D:
                {
                    desc.Texture3D.MipLevels            = pDesc->MipLevels;
                    desc.Texture3D.MostDetailedMip      = 0;
                    desc.Texture3D.ResourceMinLODClamp  = 0.0f;
                }
                break;

            case D3D12_SRV_DIMENSION_TEXTURECUBE:
                {
                    desc.TextureCube.MipLevels              = pDesc->MipLevels;
                    desc.TextureCube.MostDetailedMip        = 0;
                    desc.TextureCube.ResourceMinLODClamp    = 0.0f;
                }
                break;

            case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
                {
                    desc.TextureCubeArray.First2DArrayFace      = pDesc->FirstArraySlice;
                    desc.TextureCubeArray.MipLevels             = pDesc->MipLevels;
                    desc.TextureCubeArray.MostDetailedMip       = 0;
                    desc.TextureCubeArray.NumCubes              = pDesc->ArraySize / 6;
                    desc.TextureCubeArray.ResourceMinLODClamp   = 0.0f;
                }
                break;
            }

            pNativeDevice->CreateShaderResourceView(
                m_pTexture->GetD3D12Resource(),
                &desc,
                m_pShaderDescriptor->GetHandleCPU() );
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void TextureView::Term()
{
    SafeRelease( m_pShaderDescriptor );
    SafeRelease( m_pTargetDescriptor );
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
//      ターゲット用ディスクリプタを取得します.
//-------------------------------------------------------------------------------------------------
const Descriptor* TextureView::GetTargetDescriptor() const
{ return m_pTargetDescriptor; }

//-------------------------------------------------------------------------------------------------
//      シェーダ用ディスクリプタを取得します.
//-------------------------------------------------------------------------------------------------
const Descriptor* TextureView::GetShaderDescriptor() const
{ return m_pShaderDescriptor; }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します,
//-------------------------------------------------------------------------------------------------
ITexture* TextureView::GetResource() const
{ return m_pTexture; }

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
