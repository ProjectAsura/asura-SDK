//-------------------------------------------------------------------------------------------------
// File : a3dDepthStencilView.cpp
// Desc : DepthStencilView Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// DepthStencilView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DepthStencilView::DepthStencilView()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pTexture    (nullptr)
, m_pDescriptor (nullptr)
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

    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT( pNativeDevice != nullptr );

    auto pWrapTexture = reinterpret_cast<Texture*>(pTexture);
    A3D_ASSERT( pWrapTexture != nullptr );

    m_pTexture = pWrapTexture;
    m_pTexture->AddRef();

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    auto textureDesc = m_pTexture->GetDesc();

    {
        m_pDescriptor = m_pDevice->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
                                 ->CreateDescriptor();
        if (m_pDescriptor == nullptr)
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
            m_pDescriptor->GetHandleCPU() );
    }


    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DepthStencilView::Term()
{
    SafeRelease( m_pDescriptor );
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
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void DepthStencilView::SetName(const char* name)
{ m_Name = name; }

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* DepthStencilView::GetName() const
{ return m_Name.c_str(); }

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
//      ディスクリプタを取得します.
//-------------------------------------------------------------------------------------------------
const Descriptor* DepthStencilView::GetDescriptor() const
{ return m_pDescriptor; }

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
TargetViewDesc IDepthStencilView::GetDesc() const
{
    auto pThis = static_cast<const DepthStencilView*>(this);
    A3D_ASSERT(pThis != nullptr);

    return pThis->m_Desc;
}

//-------------------------------------------------------------------------------------------------
//      リソースを取得します,
//-------------------------------------------------------------------------------------------------
ITexture* IDepthStencilView::GetResource() const
{
    auto pThis = static_cast<const DepthStencilView*>(this);
    A3D_ASSERT(pThis != nullptr);

    return pThis->m_pTexture;
}

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

    auto instance = new DepthStencilView;
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
