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
    { return false; }

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
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void ConstantBufferView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
ConstantBufferViewDesc ConstantBufferView::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      ディスクリプタを取得します.
//-------------------------------------------------------------------------------------------------
const Descriptor* ConstantBufferView::GetDescriptor() const
{ return m_pDescriptor; }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
IBuffer* ConstantBufferView::GetResource() const
{ return m_pBuffer; }

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
    { return false; }

    auto instance = new ConstantBufferView;
    if ( instance == nullptr )
    { return false; }

    if ( !instance->Init(pDevice, pBuffer, pDesc) )
    {
        SafeRelease(instance);
        return false;
    }

    *ppConstantBufferView = instance;
    return true;
}

} // namespace a3d
