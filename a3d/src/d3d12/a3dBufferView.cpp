//-------------------------------------------------------------------------------------------------
// File : a3dBufferView.cpp
// Desc : BufferView Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// BufferView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
BufferView::BufferView()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pBuffer     (nullptr)
, m_pDescriptor (nullptr)
{ memset( &m_Desc, 0, sizeof(m_Desc) ); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
BufferView::~BufferView()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool BufferView::Init(IDevice* pDevice, IBuffer* pBuffer, const BufferViewDesc* pDesc)
{
    if (pDevice == nullptr || pBuffer == nullptr || pDesc == nullptr)
    { return false; }

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    auto pWrapDevice = reinterpret_cast<Device*>(pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    auto pWrapBuffer = reinterpret_cast<Buffer*>(pBuffer);
    A3D_ASSERT(pWrapBuffer != nullptr);

    m_pBuffer = pWrapBuffer;
    m_pBuffer->AddRef();

    m_pDescriptor = pWrapDevice
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
void BufferView::Term()
{
    SafeRelease(m_pDescriptor);
    SafeRelease(m_pBuffer);
    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void BufferView::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void BufferView::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t BufferView::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void BufferView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
BufferViewDesc BufferView::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      ディスクリプタを取得します.
//-------------------------------------------------------------------------------------------------
const Descriptor* BufferView::GetDescriptor() const
{ return m_pDescriptor; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool BufferView::Create
(
    IDevice*                pDevice,
    IBuffer*                pBuffer,
    const BufferViewDesc*   pDesc,
    IBufferView**           ppBufferView
)
{
    if (pDevice == nullptr || pBuffer == nullptr || pDesc == nullptr || ppBufferView == nullptr)
    { return false; }

    auto instance = new BufferView;
    if ( instance == nullptr )
    { return false; }

    if ( !instance->Init(pDevice, pBuffer, pDesc) )
    {
        SafeRelease(instance);
        return false;
    }

    *ppBufferView = instance;
    return true;
}

} // namespace a3d
