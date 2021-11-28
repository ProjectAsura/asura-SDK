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

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    auto pD3D11Device = m_pDevice->GetD3D11Device();
    A3D_ASSERT(pD3D11Device != nullptr);

    DXGI_FORMAT format;
    if (pDesc->Usage & RESOURCE_USAGE_SHADER_RESOURCE ||
        pDesc->Usage & RESOURCE_USAGE_UNORDERED_ACCESS_VIEW)
    { format = ToNativeTypelessFormat(pDesc->Format); }
    else
    { format = ToNativeFormat(pDesc->Format); }

    uint32_t accessFlags = 0;
    switch(pDesc->HeapType)
    {
    case HEAP_TYPE_DEFAULT:
        {
            m_MapType = D3D11_MAP_READ_WRITE;
            accessFlags |= D3D11_CPU_ACCESS_READ;
            accessFlags |= D3D11_CPU_ACCESS_WRITE;
        }
        break;

    case HEAP_TYPE_UPLOAD:
        {
            m_MapType = D3D11_MAP_WRITE_DISCARD;
            accessFlags |= D3D11_CPU_ACCESS_WRITE;
        }
        break;

    case HEAP_TYPE_READBACK:
        {
            m_MapType = D3D11_MAP_READ;
            accessFlags |= D3D11_CPU_ACCESS_READ;
        }
        break;
    }

    if (pDesc->Dimension == RESOURCE_DIMENSION_BUFFER)
    { return false; }
    else if (pDesc->Dimension == RESOURCE_DIMENSION_TEXTURE1D)
    {
        D3D11_TEXTURE1D_DESC desc = {};
        desc.Width          = pDesc->Width;
        desc.MipLevels      = pDesc->MipLevels;
        desc.ArraySize      = pDesc->DepthOrArraySize;
        desc.Format         = format;
        desc.Usage          = ToNativeUsage(pDesc->HeapType);
        desc.BindFlags      = ToNativeBindFlags(pDesc->Usage);
        desc.CPUAccessFlags = accessFlags;

        ID3D11Texture1D* pTexture;
        auto hr = pD3D11Device->CreateTexture1D(&desc, nullptr, &pTexture);
        if ( FAILED(hr) )
        { return false; }

        m_pResource = pTexture;
    }
    else if (pDesc->Dimension == RESOURCE_DIMENSION_TEXTURE2D)
    {
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width              = pDesc->Width;
        desc.Height             = pDesc->Height;
        desc.ArraySize          = pDesc->DepthOrArraySize;
        desc.MipLevels          = pDesc->MipLevels;
        desc.Format             = format;
        desc.SampleDesc.Count   = pDesc->SampleCount;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = ToNativeUsage(pDesc->HeapType);
        desc.BindFlags          = ToNativeBindFlags(pDesc->Usage);
        desc.CPUAccessFlags     = accessFlags;

        if (pDesc->Usage & RESOURCE_USAGE_COLOR_TARGET)
        { desc.CPUAccessFlags = 0; }

        if (pDesc->Usage & RESOURCE_USAGE_DEPTH_TARGET)
        { desc.CPUAccessFlags = 0; }

        ID3D11Texture2D* pTexture;
        auto hr = pD3D11Device->CreateTexture2D(&desc, nullptr, &pTexture);
        if ( FAILED(hr) )
        { return false; }

        m_pResource = pTexture;
    }
    else if (pDesc->Dimension == RESOURCE_DIMENSION_CUBEMAP)
    {
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width              = pDesc->Width;
        desc.Height             = pDesc->Height;
        desc.ArraySize          = pDesc->DepthOrArraySize;
        desc.MipLevels          = pDesc->MipLevels;
        desc.Format             = format;
        desc.SampleDesc.Count   = pDesc->SampleCount;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = ToNativeUsage(pDesc->HeapType);
        desc.BindFlags          = ToNativeBindFlags(pDesc->Usage);
        desc.CPUAccessFlags     = accessFlags;
        desc.MiscFlags          = D3D11_RESOURCE_MISC_TEXTURECUBE;

        ID3D11Texture2D* pTexture;
        auto hr = pD3D11Device->CreateTexture2D(&desc, nullptr, &pTexture);
        if ( FAILED(hr) )
        { return false; }

        m_pResource = pTexture;
    }
    else if (pDesc->Dimension == RESOURCE_DIMENSION_TEXTURE3D)
    {
        D3D11_TEXTURE3D_DESC desc = {};
        desc.Width          = pDesc->Width;
        desc.Height         = pDesc->Height;
        desc.Depth          = pDesc->DepthOrArraySize;
        desc.MipLevels      = pDesc->MipLevels;
        desc.Format         = format;
        desc.Usage          = ToNativeUsage(pDesc->HeapType);
        desc.BindFlags      = ToNativeBindFlags(pDesc->Usage);
        desc.CPUAccessFlags = accessFlags;

        ID3D11Texture3D* pTexture;
        auto hr = pD3D11Device->CreateTexture3D(&desc, nullptr, &pTexture);
        if ( FAILED(hr) )
        { return false; }

        m_pResource = pTexture;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void Texture::Term()
{
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
    auto pDeviceContext = m_pDevice->GetD3D11DeviceContext();
    A3D_ASSERT(pDeviceContext != nullptr);

    D3D11_MAPPED_SUBRESOURCE subresource;
    auto hr = pDeviceContext->Map(m_pResource, 0, m_MapType, 0, &subresource);
    if ( FAILED(hr) )
    { return nullptr; }

    return subresource.pData;
}

//-------------------------------------------------------------------------------------------------
//      メモリマッピングを解除します.
//-------------------------------------------------------------------------------------------------
void Texture::Unmap()
{
    auto pDeviceContext = m_pDevice->GetD3D11DeviceContext();
    A3D_ASSERT(pDeviceContext != nullptr);

    pDeviceContext->Unmap(m_pResource, 0);
}

//-------------------------------------------------------------------------------------------------
//      サブリソースレイアウトを取得します.
//-------------------------------------------------------------------------------------------------
SubresourceLayout Texture::GetSubresourceLayout(uint32_t subResource) const
{
    return CalcSubresourceLayout(
        subResource,
        m_Desc.Format,
        m_Desc.Width,
        m_Desc.Height);
}

//-------------------------------------------------------------------------------------------------
//      リソース種別を取得します.
//-------------------------------------------------------------------------------------------------
RESOURCE_KIND Texture::GetKind() const
{ return RESOURCE_KIND_TEXTURE; }

//-------------------------------------------------------------------------------------------------
//      1次元テクスチャとして取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Texture1D* A3D_APIENTRY Texture::GetAsD3D11Texture1D() const
{ return static_cast<ID3D11Texture1D*>(m_pResource); }

//-------------------------------------------------------------------------------------------------
//      2次元テクスチャとして取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Texture2D* A3D_APIENTRY Texture::GetAsD3D11Texture2D() const
{ return static_cast<ID3D11Texture2D*>(m_pResource); }

//-------------------------------------------------------------------------------------------------
//      3次元テクスチャとして取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Texture3D* A3D_APIENTRY Texture::GetAsD3D11Texture3D() const
{ return static_cast<ID3D11Texture3D*>(m_pResource); }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Resource* A3D_APIENTRY Texture::GetD3D11Resource() const
{ return m_pResource; }

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
    ID3D11Texture2D*    pNativeResource,
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

    instance->m_pDevice = static_cast<Device*>(pDevice);
    instance->m_pDevice->AddRef();

    instance->m_pResource = pNativeResource;
    instance->m_pResource->AddRef();

    D3D11_TEXTURE2D_DESC nativeDesc;
    pNativeResource->GetDesc(&nativeDesc);

    instance->m_Desc.Width              = static_cast<uint32_t>(nativeDesc.Width);
    instance->m_Desc.Height             = nativeDesc.Height;
    instance->m_Desc.DepthOrArraySize   = 1;
    instance->m_Desc.Format             = ToWrapFormat( nativeDesc.Format );
    instance->m_Desc.MipLevels          = nativeDesc.MipLevels;
    instance->m_Desc.Dimension          = RESOURCE_DIMENSION_TEXTURE2D;
    instance->m_Desc.SampleCount        = nativeDesc.SampleDesc.Count;
    instance->m_Desc.InitState          = RESOURCE_STATE_UNKNOWN;
    instance->m_Desc.Usage              = usage;
    instance->m_Desc.HeapType           = HEAP_TYPE_DEFAULT;

    bool writable = false;
    bool readable = false;
    bool dynamic  = false;
    if (nativeDesc.CPUAccessFlags & D3D11_CPU_ACCESS_WRITE)
    { writable = true; }
    if (nativeDesc.CPUAccessFlags & D3D11_CPU_ACCESS_READ)
    { readable = true; }
    if (nativeDesc.Usage & D3D11_USAGE_DYNAMIC)
    { dynamic = true; }

    if (writable && readable)
    { instance->m_MapType = D3D11_MAP_READ_WRITE; }
    else if (writable & dynamic)
    { instance->m_MapType = D3D11_MAP_WRITE_DISCARD; }
    else if (readable)
    { instance->m_MapType = D3D11_MAP_READ; }

    *ppResource = instance;
    return true;
}

} // namespace a3d
