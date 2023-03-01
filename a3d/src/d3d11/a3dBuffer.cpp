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
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    auto pD3D11Device = m_pDevice->GetD3D11Device();
    A3D_ASSERT(pD3D11Device != nullptr);

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
        desc.Usage          = ToNativeUsage(pDesc->HeapType);
        desc.BindFlags      = ToNativeBindFlags(pDesc->Usage);
        desc.CPUAccessFlags = accessFlags;

        if (pDesc->Usage & RESOURCE_USAGE_UNORDERED_ACCESS)
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
        {
            A3D_LOG("Error : ID3D11Device::CreateBuffer() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    m_UniqueId = m_pDevice->AllocUniqueId();

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
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void Buffer::SetName(const char* name)
{
    m_Name = name;
    m_pBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, m_Name.size(), m_Name.c_str());
}

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* Buffer::GetName() const
{ return m_Name.c_str(); }

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
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
BufferDesc IBuffer::GetDesc() const
{
    auto pThis = static_cast<const Buffer*>(this);
    A3D_ASSERT(pThis != nullptr);

    return pThis->m_Desc;
}

//-------------------------------------------------------------------------------------------------
//      デバイスアドレスを取得します.
//-------------------------------------------------------------------------------------------------
uint64_t IBuffer::GetDeviceAddress() const
{
    auto pThis = static_cast<const Buffer*>(this);
    A3D_ASSERT(pThis != nullptr);

    if (pThis->m_pBuffer == nullptr)
    { return 0; }

    // API的にサポートされていないのでユニークIDを返す.
    return pThis->m_UniqueId;
}

//-------------------------------------------------------------------------------------------------
//      メモリマッピングします.
//-------------------------------------------------------------------------------------------------
void* IBuffer::Map()
{
    auto pThis = static_cast<Buffer*>(this);
    A3D_ASSERT(pThis != nullptr);

    if (pThis->m_pSubresource != nullptr)
    { return pThis->m_pSubresource; }

    auto pDeviceContext = pThis->m_pDevice->GetD3D11DeviceContext();
    A3D_ASSERT(pDeviceContext != nullptr);

    D3D11_MAPPED_SUBRESOURCE subresource;
    auto hr = pDeviceContext->Map(pThis->m_pBuffer, 0, pThis->m_MapType, 0, &subresource);
    if ( FAILED(hr) )
    { return nullptr; }

    return subresource.pData;
}

//-------------------------------------------------------------------------------------------------
//      メモリマッピングを解除します.
//-------------------------------------------------------------------------------------------------
void IBuffer::Unmap()
{
    auto pThis = static_cast<Buffer*>(this);
    A3D_ASSERT(pThis != nullptr);

    if (pThis->m_pSubresource != nullptr)
    { return; }

    auto pDeviceContext = pThis->m_pDevice->GetD3D11DeviceContext();
    A3D_ASSERT(pDeviceContext != nullptr);

    pDeviceContext->Unmap(pThis->m_pBuffer, 0);
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Buffer::Create(IDevice* pDevice, const BufferDesc* pDesc, IBuffer** ppResource)
{
    if (pDevice == nullptr || pDesc == nullptr || ppResource == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new Buffer;
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

} // namespace a3d
