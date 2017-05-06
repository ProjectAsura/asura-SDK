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

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    auto pD3D11Device = m_pDevice->GetD3D11Device();
    A3D_ASSERT(pD3D11Device != nullptr);

    switch(pDesc->HeapProperty.Type)
    {
    case HEAP_TYPE_DEFAULT:
        { m_MapType = D3D11_MAP_READ_WRITE; }
        break;

    case HEAP_TYPE_UPLOAD:
        { m_MapType = D3D11_MAP_WRITE_DISCARD; }
        break;

    case HEAP_TYPE_READBACK:
        { m_MapType = D3D11_MAP_READ; }
        break;
    }

    auto size = static_cast<size_t>(pDesc->Size);

    if (pDesc->Usage == RESOURCE_USAGE_COPY_SRC ||
        pDesc->Usage == RESOURCE_USAGE_COPY_DST)
    {
        m_pSubresource = new uint8_t [size];
        if (m_pSubresource == nullptr)
        { return false; }

        memset(m_pSubresource, 0, size);
    }
    else
    {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth      = static_cast<uint32_t>(size);
        desc.Usage          = ToNativeUsage(pDesc->HeapProperty.Type);
        desc.BindFlags      = ToNativeBindFlags(pDesc->Usage);
        desc.CPUAccessFlags = ToNativeCpuAccessFlags(
                                pDesc->HeapProperty.Type,
                                pDesc->HeapProperty.CpuPageProperty);

        if (pDesc->Usage & RESOURCE_USAGE_UNORDERD_ACCESS)
        { desc.StructureByteStride = pDesc->Stride; }

        if (pDesc->Usage == RESOURCE_USAGE_CONSTANT_BUFFER)
        {
            m_pSubresource = new uint8_t [size];
            memset(m_pSubresource, 0, size);
            desc.Usage          = D3D11_USAGE_DEFAULT;
            desc.CPUAccessFlags = 0;
            m_MapType           = D3D11_MAP_WRITE;
        }
    
        auto hr = pD3D11Device->CreateBuffer(&desc, nullptr, &m_pBuffer);
        if ( FAILED(hr) )
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void Buffer::Term()
{
    if (m_pSubresource != nullptr)
    {
        delete [] m_pSubresource;
        m_pSubresource = nullptr;
    }

    SafeRelease(m_pBuffer);
    SafeRelease(m_pDevice);

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
//      メモリマッピングします.
//-------------------------------------------------------------------------------------------------
void* Buffer::Map()
{
    if (m_pSubresource != nullptr)
    { return m_pSubresource; }

    auto pDeviceContext = m_pDevice->GetD3D11DeviceContext();
    A3D_ASSERT(pDeviceContext != nullptr);

    D3D11_MAPPED_SUBRESOURCE subresource;
    auto hr = pDeviceContext->Map(m_pBuffer, 0, m_MapType, 0, &subresource);
    if ( FAILED(hr) )
    { return nullptr; }

    return subresource.pData;
}

//-------------------------------------------------------------------------------------------------
//      メモリマッピングを解除します.
//-------------------------------------------------------------------------------------------------
void Buffer::Unmap()
{
    if (m_pSubresource != nullptr)
    { return; }

    auto pDeviceContext = m_pDevice->GetD3D11DeviceContext();
    A3D_ASSERT(pDeviceContext != nullptr);

    pDeviceContext->Unmap(m_pBuffer, 0);
}

//-------------------------------------------------------------------------------------------------
//      リソース種別を取得します.
//-------------------------------------------------------------------------------------------------
RESOURCE_KIND Buffer::GetKind() const
{ return RESOURCE_KIND_BUFFER; }

//-------------------------------------------------------------------------------------------------
//      バッファを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Buffer* Buffer::GetD3D11Buffer() const
{ return m_pBuffer; }

//-------------------------------------------------------------------------------------------------
//      サブリソースへのポインタを取得します.
//-------------------------------------------------------------------------------------------------
void* Buffer::GetSubresourcePointer() const
{ return m_pSubresource; }

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
