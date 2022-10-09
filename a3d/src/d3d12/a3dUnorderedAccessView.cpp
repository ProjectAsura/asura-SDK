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
, m_pDescriptor (nullptr)
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
    {
        A3D_LOG("Invalid Argument.");
        return false;
    }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    m_pResource = pResource;
    m_pResource->AddRef();

    auto pNativeDevice = m_pDevice->GetD3D12Device();

    if (pResource->GetKind() == RESOURCE_KIND_BUFFER)
    {
        auto pWrapBuffer = static_cast<Buffer*>(m_pResource);
        A3D_ASSERT(pWrapBuffer != nullptr);

        auto bufferDesc = pWrapBuffer->GetDesc();

        if ((bufferDesc.Usage & RESOURCE_USAGE_UNORDERED_ACCESS) != RESOURCE_USAGE_UNORDERED_ACCESS)
        {
            A3D_LOG("Error : Invalid Argument.");
            return false;
        }

        m_pDescriptor = m_pDevice
                            ->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
                            ->CreateDescriptor();
        if (m_pDescriptor == nullptr)
        {
            A3D_LOG("Error : CreateDescriptor() Failed.");
            return false;
        }

        D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
        uav_desc.Format                      = ToNativeViewFormat(pDesc->Format, false);
        uav_desc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
        uav_desc.Buffer.CounterOffsetInBytes = 0;
        uav_desc.Buffer.FirstElement         = pDesc->FirstElement;
        uav_desc.Buffer.NumElements          = pDesc->ElementCount;
        uav_desc.Buffer.StructureByteStride  = pDesc->StructuredByteStride;
        uav_desc.Buffer.Flags                = (pDesc->StructuredByteStride == 0) ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;

        pNativeDevice->CreateUnorderedAccessView(
            pWrapBuffer->GetD3D12Resource(),
            nullptr,
            &uav_desc,
            m_pDescriptor->GetHandleCPU());
    }
    else if (pResource->GetKind() == RESOURCE_KIND_TEXTURE)
    {
        auto pWrapTexture = static_cast<Texture*>(m_pResource);
        A3D_ASSERT(pWrapTexture != nullptr);

        auto textureDesc = pWrapTexture->GetDesc();

        if ((textureDesc.Usage & RESOURCE_USAGE_UNORDERED_ACCESS) != RESOURCE_USAGE_UNORDERED_ACCESS)
        {
            A3D_LOG("Error : Invalid Argument.");
            return false;
        }

        m_pDescriptor = m_pDevice
                            ->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
                            ->CreateDescriptor();
        if (m_pDescriptor == nullptr)
        { return false; }

        D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
        uav_desc.Format         = ToNativeViewFormat(pDesc->Format, false);
        uav_desc.ViewDimension  = ToNativeUAVDimension(pDesc->Dimension);

        switch (uav_desc.ViewDimension)
        {
        case D3D12_UAV_DIMENSION_UNKNOWN:
            {
                A3D_LOG("Error : Invalid Argument.");
                return false;
            }

        case D3D12_UAV_DIMENSION_BUFFER:
            {
                A3D_LOG("Error : Invalid Argument.");
                return false;
            }

        case D3D12_UAV_DIMENSION_TEXTURE1D:
            {
                uav_desc.Texture1D.MipSlice = pDesc->MipSlice;
            }
            break;

        case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
            {
                uav_desc.Texture1DArray.ArraySize       = pDesc->ElementCount;
                uav_desc.Texture1DArray.FirstArraySlice = UINT(pDesc->FirstElement);
                uav_desc.Texture1DArray.MipSlice        = pDesc->MipSlice;
            }
            break;

        case D3D12_UAV_DIMENSION_TEXTURE2D:
            {
                uav_desc.Texture2D.MipSlice     = pDesc->MipSlice;
                uav_desc.Texture2D.PlaneSlice   = 0;
            }
            break;

        case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
            {
                uav_desc.Texture2DArray.ArraySize       = pDesc->ElementCount;
                uav_desc.Texture2DArray.FirstArraySlice = UINT(pDesc->FirstElement);
                uav_desc.Texture2DArray.MipSlice        = pDesc->MipSlice;
                uav_desc.Texture2DArray.PlaneSlice      = 0;
            }
            break;

        case D3D12_UAV_DIMENSION_TEXTURE3D:
            {
                uav_desc.Texture3D.FirstWSlice  = UINT(pDesc->FirstElement);
                uav_desc.Texture3D.MipSlice     = pDesc->MipSlice;
                uav_desc.Texture3D.WSize        = pDesc->ElementCount;
            }
            break;
        }

        pNativeDevice->CreateUnorderedAccessView(
            pWrapTexture->GetD3D12Resource(),
            nullptr,
            &uav_desc,
            m_pDescriptor->GetHandleCPU());
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void UnorderedAccessView::Term()
{
    SafeRelease( m_pDescriptor );
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
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void UnorderedAccessView::SetName(const char* name)
{ m_Name = name; }

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* UnorderedAccessView::GetName() const
{ return m_Name.c_str(); }

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
//      ディスクリプタを取得します.
//-------------------------------------------------------------------------------------------------
const Descriptor* UnorderedAccessView::GetDescriptor() const
{ return m_pDescriptor; }

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
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new UnorderedAccessView();
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if (!instance->Init(pDevice, pResource, pDesc))
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppStorageView = instance;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      D3D12_CPU_DESCRIPTOR_HANDLEを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE A3D_APIENTRY GetD3D12CpuDescriptorHandle(IUnorderedAccessView* pUAV)
{
    auto pWrapUAV = static_cast<UnorderedAccessView*>(pUAV);
    if (pWrapUAV == nullptr)
    { return D3D12_CPU_DESCRIPTOR_HANDLE(); }

    A3D_ASSERT(pWrapUAV->GetDescriptor() != nullptr);
    return pWrapUAV->GetDescriptor()->GetHandleCPU();
}

//-------------------------------------------------------------------------------------------------
//      D3D12_GPU_DESCRIPTOR_HANDLEを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE A3D_APIENTRY GetD3D12GpuDescriptorHandle(IUnorderedAccessView* pUAV)
{
    auto pWrapUAV = static_cast<UnorderedAccessView*>(pUAV);
    if (pWrapUAV == nullptr)
    { return D3D12_GPU_DESCRIPTOR_HANDLE(); }

    A3D_ASSERT(pWrapUAV->GetDescriptor() != nullptr);
    return pWrapUAV->GetDescriptor()->GetHandleGPU();
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタ番号を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t APIENTRY GetD3D12DescriptorIndex(IUnorderedAccessView* pUAV)
{
    auto pWrapUAV = static_cast<UnorderedAccessView*>(pUAV);
    if (pWrapUAV == nullptr)
    { return UINT32_MAX; }

    A3D_ASSERT(pWrapUAV->GetDescriptor() != nullptr);
    return pWrapUAV->GetDescriptor()->GetDescriptorIndex();
}

} // namespace a3d
