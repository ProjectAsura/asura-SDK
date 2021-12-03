//-------------------------------------------------------------------------------------------------
// File : a3dRenderTargetView.cpp
// Desc : RenderTargetView Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderTargetView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
RenderTargetView::RenderTargetView()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pTexture    (nullptr)
, m_pDescriptor (nullptr)
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
    {
        m_pDescriptor = m_pDevice->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
                                 ->CreateDescriptor();
        if (m_pDescriptor == nullptr)
        { return false; }

        D3D12_RENDER_TARGET_VIEW_DESC desc = {};
        desc.Format         = ToNativeFormat(pDesc->Format);
        desc.ViewDimension  = ToNativeRTVDimension(pDesc->Dimension);

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
            m_pDescriptor->GetHandleCPU() );
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void RenderTargetView::Term()
{
    SafeRelease( m_pDescriptor );
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
//      ディスクリプタを取得します.
//-------------------------------------------------------------------------------------------------
const Descriptor* RenderTargetView::GetDescriptor() const
{ return m_pDescriptor; }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します,
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
    { return false; }

    auto instance = new RenderTargetView;
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
