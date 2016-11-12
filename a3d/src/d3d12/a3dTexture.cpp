//-------------------------------------------------------------------------------------------------
// File : a3dTexture.cpp
// Desc : Texture Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Texture class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Texture::Texture()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_State       (RESOURCE_STATE_UNKNOWN)
, m_pResource   (nullptr)
{ /* DO_NOTIHNG */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Texture::~Texture()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理です.
//-------------------------------------------------------------------------------------------------
bool Texture::Init(IDevice* pDevice, const TextureDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    auto pWrapDevice = reinterpret_cast<Device*>(pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    D3D12_HEAP_PROPERTIES prop = {
        ToNativeHeapType(pDesc->HeapProperty.Type),
        ToNativeCpuPageProperty(pDesc->HeapProperty.CpuPageProperty),
        D3D12_MEMORY_POOL_UNKNOWN,
        0,
        0
    };

    auto format = ToNativeFormat(pDesc->Format);

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = ToNativeResorceDimension(pDesc->Dimension);
    desc.Width              = pDesc->Width;
    desc.Height             = pDesc->Height;
    desc.DepthOrArraySize   = pDesc->DepthOrArraySize;
    desc.Format             = format;
    desc.MipLevels          = pDesc->MipLevels;
    desc.SampleDesc.Count   = pDesc->SampleCount;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = (pDesc->Layout == RESOURCE_LAYOUT_LINEAR) 
                              ? D3D12_TEXTURE_LAYOUT_ROW_MAJOR 
                              : D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags              = ToNativeResourceFlags(pDesc->Usage);

    D3D12_RESOURCE_STATES   state = ToNativeState(pDesc->InitState);
    D3D12_HEAP_FLAGS        flags = D3D12_HEAP_FLAG_NONE;

    bool isTarget = false;
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format;

    if (pDesc->Usage & RESOURCE_USAGE_DEPTH_TARGET)
    {
        clearValue.DepthStencil.Depth   = 1.0f;
        clearValue.DepthStencil.Stencil = 0;
        isTarget = true;
    }

    if (pDesc->Usage & RESOURCE_USAGE_COLOR_TARGET)
    {
        clearValue.Color[0] = 0.0f;
        clearValue.Color[1] = 0.0f;
        clearValue.Color[2] = 0.0f;
        clearValue.Color[3] = 0.0f;
        isTarget = true;
    }

    auto hr = pNativeDevice->CreateCommittedResource(
        &prop, flags, &desc, state, (isTarget ? &clearValue : nullptr), IID_PPV_ARGS(&m_pResource));
    if ( FAILED(hr) )
    { return false; }

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    m_State = pDesc->InitState;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void Texture::Term()
{
    SafeRelease(m_pResource);
    SafeRelease(m_pDevice);

    m_State = RESOURCE_STATE_UNKNOWN;
    memset(&m_Desc, 0, sizeof(m_Desc));
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void Texture::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解法処理を行います.
//-------------------------------------------------------------------------------------------------
void Texture::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Texture::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void Texture::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
TextureDesc Texture::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      リソースステートを取得します.
//-------------------------------------------------------------------------------------------------
RESOURCE_STATE Texture::GetState() const
{ return m_State; }

//-------------------------------------------------------------------------------------------------
//      メモリマッピングします.
//-------------------------------------------------------------------------------------------------
void* Texture::Map()
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
void Texture::Unmap()
{ m_pResource->Unmap(0, nullptr); }

//-------------------------------------------------------------------------------------------------
//      サブリソースレイアウトを取得します.
//-------------------------------------------------------------------------------------------------
SubresourceLayout Texture::GetSubresourceLayout(uint32_t subresource) const
{
    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    auto nativeDesc = m_pResource->GetDesc();

    uint32_t rowCount = 0;
    uint64_t rowPitch = 0;
    uint64_t size = 0;

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout = {};
    pNativeDevice->GetCopyableFootprints(
        &nativeDesc,
        0,
        1,
        0,
        &layout,
        &rowCount,
        &rowPitch,
        &size);

    SubresourceLayout result = {};
    result.Offset       = layout.Offset;
    result.RowCount     = rowCount;
    result.RowPitch     = rowPitch;
    result.SlicePitch   = layout.Footprint.RowPitch * layout.Footprint.Height;
    result.Size         = size;
    return result;
}

//-------------------------------------------------------------------------------------------------
//      リソースステートを設定します.
//-------------------------------------------------------------------------------------------------
void Texture::SetState(RESOURCE_STATE state)
{ m_State = state; }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* Texture::GetD3D12Resource() const
{ return m_pResource; }

//-------------------------------------------------------------------------------------------------
//      プライベートデータを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Texture::GetPrivateData() const
{ return RESOURCE_OBJECT_TYPE_TEXTURE; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Texture::Create(IDevice* pDevice, const TextureDesc* pDesc, ITexture** ppResource)
{
    if (pDevice == nullptr || pDesc == nullptr || ppResource == nullptr)
    { return false; }

    auto instance = new Texture;
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

//-------------------------------------------------------------------------------------------------
//      ネイティブリソースから生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Texture::CreateFromNative
(
    IDevice*            pDevice,
    ID3D12Resource*     pNativeResource,
    RESOURCE_USAGE      usage,
    ComponentMapping    componentMapping,
    ITexture**          ppResource
)
{
    if (pDevice == nullptr || pNativeResource == nullptr || ppResource == nullptr)
    { return false; }

    auto instance = new Texture;
    if (instance == nullptr)
    { return false; }

    instance->m_pDevice = pDevice;
    instance->m_pDevice->AddRef();

    instance->m_pResource = pNativeResource;
    instance->m_pResource->AddRef();

    D3D12_HEAP_PROPERTIES prop = {};
    D3D12_HEAP_FLAGS flag;
    pNativeResource->GetHeapProperties(&prop, &flag);

    auto nativeDesc = pNativeResource->GetDesc();

    instance->m_Desc.Width              = static_cast<uint32_t>(nativeDesc.Width);
    instance->m_Desc.Height             = nativeDesc.Height;
    instance->m_Desc.DepthOrArraySize   = nativeDesc.DepthOrArraySize;
    instance->m_Desc.Format             = ToWrapFormat( nativeDesc.Format );
    instance->m_Desc.MipLevels          = nativeDesc.MipLevels;
    instance->m_Desc.Dimension          = ToWrapDimension( nativeDesc.Dimension );
    instance->m_Desc.SampleCount        = nativeDesc.SampleDesc.Count;
    instance->m_Desc.InitState          = RESOURCE_STATE_UNKNOWN;
    instance->m_Desc.Usage              = usage;

    instance->m_Desc.HeapProperty.Type            = static_cast<HEAP_TYPE>(prop.Type);
    instance->m_Desc.HeapProperty.CpuPageProperty = static_cast<CPU_PAGE_PROPERTY>(prop.CPUPageProperty);

    instance->m_State = RESOURCE_STATE_UNKNOWN;

    *ppResource = instance;
    return true;
}

} // namespace a3d
