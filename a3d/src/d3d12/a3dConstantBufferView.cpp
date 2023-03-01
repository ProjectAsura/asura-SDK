//-------------------------------------------------------------------------------------------------
// File : a3dConstantBufferView.cpp
// Desc : ConstantBufferView Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ConstantBufferView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
ConstantBufferView::ConstantBufferView()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pBuffer     (nullptr)
, m_pDescriptor (nullptr)
{ memset( &m_Desc, 0, sizeof(m_Desc) ); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
ConstantBufferView::~ConstantBufferView()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool ConstantBufferView::Init(IDevice* pDevice, IBuffer* pBuffer, const ConstantBufferViewDesc* pDesc)
{
    if (pDevice == nullptr || pBuffer == nullptr || pDesc == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    auto pWrapBuffer = reinterpret_cast<Buffer*>(pBuffer);
    A3D_ASSERT(pWrapBuffer != nullptr);

    m_pBuffer = pWrapBuffer;
    m_pBuffer->AddRef();

    m_pDescriptor = m_pDevice
                        ->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
                        ->CreateDescriptor();
    if (m_pDescriptor == nullptr)
    { return false; }

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
    desc.BufferLocation = pWrapBuffer->GetD3D12Resource()->GetGPUVirtualAddress() + pDesc->Offset;
    desc.SizeInBytes    = uint32_t(pDesc->Range);

    pNativeDevice->CreateConstantBufferView(&desc, m_pDescriptor->GetHandleCPU() );

    return true;
}

//----------------- --------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void ConstantBufferView::Term()
{
    SafeRelease(m_pDescriptor);
    SafeRelease(m_pBuffer);
    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void ConstantBufferView::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void ConstantBufferView::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t ConstantBufferView::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void ConstantBufferView::SetName(const char* name)
{ m_Name = name; }

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* ConstantBufferView::GetName() const
{ return m_Name.c_str(); }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void ConstantBufferView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタを取得します.
//-------------------------------------------------------------------------------------------------
const Descriptor* ConstantBufferView::GetDescriptor() const
{ return m_pDescriptor; }

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
ConstantBufferViewDesc IConstantBufferView::GetDesc() const
{
    auto pThis = static_cast<const ConstantBufferView*>(this);
    A3D_ASSERT(pThis != nullptr);

    return pThis->m_Desc;
}

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
IBuffer* IConstantBufferView::GetResource() const
{
    auto pThis = static_cast<const ConstantBufferView*>(this);
    A3D_ASSERT(pThis != nullptr);

    return pThis->m_pBuffer;
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool ConstantBufferView::Create
(
    IDevice*                        pDevice,
    IBuffer*                        pBuffer,
    const ConstantBufferViewDesc*   pDesc,
    IConstantBufferView**           ppConstantBufferView
)
{
    if (pDevice == nullptr || pBuffer == nullptr || pDesc == nullptr || ppConstantBufferView == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new ConstantBufferView;
    if ( instance == nullptr )
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if ( !instance->Init(pDevice, pBuffer, pDesc) )
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppConstantBufferView = instance;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      D3D12_CPU_DESCRIPTOR_HANDLEを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE A3D_APIENTRY GetD3D12CpuDescriptorHandle(IConstantBufferView* pCBV)
{
    auto pWrapCBV = static_cast<ConstantBufferView*>(pCBV);
    if (pWrapCBV == nullptr)
    { return D3D12_CPU_DESCRIPTOR_HANDLE(); }

    A3D_ASSERT(pWrapCBV->GetDescriptor() != nullptr);
    return pWrapCBV->GetDescriptor()->GetHandleCPU();
}

//-------------------------------------------------------------------------------------------------
//      D3D12_GPU_DESCRIPTOR_HANDLEを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE A3D_APIENTRY GetD3D12GpuDescriptorHandle(IConstantBufferView* pCBV)
{
    auto pWrapCBV = static_cast<ConstantBufferView*>(pCBV);
    if (pWrapCBV == nullptr)
    { return D3D12_GPU_DESCRIPTOR_HANDLE(); }

    A3D_ASSERT(pWrapCBV->GetDescriptor() != nullptr);
    return pWrapCBV->GetDescriptor()->GetHandleGPU();
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタ番号を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t APIENTRY GetD3D12DescriptorIndex(IConstantBufferView* pCBV)
{
    auto pWrapCBV = static_cast<ConstantBufferView*>(pCBV);
    if (pWrapCBV == nullptr)
    { return UINT32_MAX; }

    A3D_ASSERT(pWrapCBV->GetDescriptor() != nullptr);
    return pWrapCBV->GetDescriptor()->GetDescriptorIndex();
}

} // namespace a3d
