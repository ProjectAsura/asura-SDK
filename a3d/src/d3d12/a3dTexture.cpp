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
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

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

    bool isTarget = false;
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format;

    if ((pDesc->Usage & RESOURCE_USAGE_DEPTH_STENCIL) == RESOURCE_USAGE_DEPTH_STENCIL)
    {
        clearValue.DepthStencil.Depth   = 1.0f;
        clearValue.DepthStencil.Stencil = 0;
        isTarget = true;
    }

    if ((pDesc->Usage & RESOURCE_USAGE_RENDER_TARGET) == RESOURCE_USAGE_RENDER_TARGET)
    {
        clearValue.Color[0] = 0.0f;
        clearValue.Color[1] = 0.0f;
        clearValue.Color[2] = 0.0f;
        clearValue.Color[3] = 0.0f;
        isTarget = true;
    }

    auto allocFlags = (isTarget) ? D3D12MA::ALLOCATION_FLAG_COMMITTED : D3D12MA::ALLOCATION_FLAG_NONE;

    D3D12MA::ALLOCATION_DESC allocDesc = {};
    allocDesc.HeapType = ToNativeHeapType(pDesc->HeapType);
    allocDesc.Flags    = allocFlags;

    auto hr = m_pDevice->GetAllocator()->CreateResource(
        &allocDesc,
        &desc,
        state,
        (isTarget) ? &clearValue : nullptr,
        &m_pAllocation,
        IID_PPV_ARGS(&m_pResource));
    if ( FAILED(hr) )
    {
        A3D_LOG("Error : CreateResource() Failed. errcode = 0x%x", hr);
        return false;
    }

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void Texture::Term()
{
    SafeRelease(m_pAllocation);
    SafeRelease(m_pResource);
    SafeRelease(m_pDevice);

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
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void Texture::SetName(const char* name)
{
    m_Name = name;
    m_pResource->SetPrivateData(WKPDID_D3DDebugObjectName, m_Name.size(), m_Name.c_str());
}

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* Texture::GetName() const
{ return m_Name.c_str(); }

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
    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    auto nativeDesc = m_pResource->GetDesc();

    uint32_t rowCount = 0;
    uint64_t rowPitch = 0;
    uint64_t size = 0;

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout = {};
    pNativeDevice->GetCopyableFootprints(
        &nativeDesc,
        subresource,
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
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* Texture::GetD3D12Resource() const
{ return m_pResource; }

//-------------------------------------------------------------------------------------------------
//      リソースタイプを取得します.
//-------------------------------------------------------------------------------------------------
RESOURCE_KIND Texture::GetKind() const
{ return RESOURCE_KIND_TEXTURE; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Texture::Create(IDevice* pDevice, const TextureDesc* pDesc, ITexture** ppResource)
{
    if (pDevice == nullptr || pDesc == nullptr || ppResource == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new Texture;
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if (!instance->Init(pDevice, pDesc))
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
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
    ITexture**          ppResource
)
{
    if (pDevice == nullptr || pNativeResource == nullptr || ppResource == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new Texture;
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    instance->m_pDevice = static_cast<Device*>(pDevice);
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
    instance->m_Desc.HeapType           = static_cast<HEAP_TYPE>(prop.Type);
    //instance->m_Desc.HeapProperty.CpuPageProperty = static_cast<CPU_PAGE_PROPERTY>(prop.CPUPageProperty);

    *ppResource = instance;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      ネイティブリソースを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* A3D_APIENTRY GetNativeResource(ITexture* pTexture)
{
    auto pWrapTexture = static_cast<Texture*>(pTexture);
    if (pWrapTexture == nullptr)
    { return nullptr; }

    return pWrapTexture->GetD3D12Resource();
}

} // namespace a3d
