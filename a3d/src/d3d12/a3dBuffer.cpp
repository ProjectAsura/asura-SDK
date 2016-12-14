//-------------------------------------------------------------------------------------------------
// File : a3dBuffer.cpp
// Desc : Buffer Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Buffer::Buffer()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_State       (RESOURCE_STATE_UNKNOWN)
, m_pResource   (nullptr)
{ /* DO_NOTIHNG */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Buffer::~Buffer()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理です.
//-------------------------------------------------------------------------------------------------
bool Buffer::Init(IDevice* pDevice, const BufferDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    auto pWrapDevice = reinterpret_cast<Device*>(pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type                   = ToNativeHeapType(pDesc->HeapProperty.Type);
    prop.CPUPageProperty        = ToNativeCpuPageProperty(pDesc->HeapProperty.CpuPageProperty);
    prop.MemoryPoolPreference   = D3D12_MEMORY_POOL_UNKNOWN;
    prop.VisibleNodeMask        = 1;
    prop.CreationNodeMask       = 1;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width              = pDesc->Size;
    desc.Height             = 1;
    desc.DepthOrArraySize   = 1;
    desc.Format             = DXGI_FORMAT_UNKNOWN;
    desc.MipLevels          = 1;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags              = ToNativeResourceFlags(pDesc->Usage);

    D3D12_RESOURCE_STATES   state = ToNativeState(pDesc->InitState);
    D3D12_HEAP_FLAGS        flags = D3D12_HEAP_FLAG_NONE;

    auto hr = pNativeDevice->CreateCommittedResource(
        &prop, flags, &desc, state, nullptr, IID_PPV_ARGS(&m_pResource));
    if ( FAILED(hr) )
    { return false; }

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    m_State = pDesc->InitState;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void Buffer::Term()
{
    SafeRelease(m_pResource);
    SafeRelease(m_pDevice);

    m_State = RESOURCE_STATE_UNKNOWN;
    memset(&m_Desc, 0, sizeof(m_Desc));
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void Buffer::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解法処理を行います.
//-------------------------------------------------------------------------------------------------
void Buffer::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Buffer::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void Buffer::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
BufferDesc Buffer::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      リソースステートを取得します.
//-------------------------------------------------------------------------------------------------
RESOURCE_STATE Buffer::GetState() const
{ return m_State; }

//-------------------------------------------------------------------------------------------------
//      メモリマッピングします.
//-------------------------------------------------------------------------------------------------
void* Buffer::Map()
{ 
    void* ptr = nullptr;
    
    auto hr = m_pResource->Map(0, nullptr, &ptr);
    if ( FAILED(hr) )
    { return nullptr; }

    return ptr;
}

//-------------------------------------------------------------------------------------------------
//      メモリマッピングを解除します.
//-------------------------------------------------------------------------------------------------
void Buffer::Unmap()
{ m_pResource->Unmap(0, nullptr); }

//-------------------------------------------------------------------------------------------------
//      リソースタイプを取得します.
//-------------------------------------------------------------------------------------------------
RESOURCE_KIND Buffer::GetKind() const
{ return RESOURCE_KIND_BUFFER; }

//-------------------------------------------------------------------------------------------------
//      リソースステートを設定します.
//-------------------------------------------------------------------------------------------------
void Buffer::SetState(RESOURCE_STATE state)
{ m_State = state; }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* Buffer::GetD3D12Resource() const
{ return m_pResource; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Buffer::Create(IDevice* pDevice, const BufferDesc* pDesc, IBuffer** ppResource)
{
    if (pDevice == nullptr || pDesc == nullptr || ppResource == nullptr)
    { return false; }

    auto instance = new Buffer;
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, pDesc))
    {
        SafeRelease(instance);
        return false;
    }

    *ppResource = instance;
    return true;
}

} // namespace a3d
