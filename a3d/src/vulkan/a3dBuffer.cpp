//-------------------------------------------------------------------------------------------------
// File : a3dBuffer.cpp
// Desc : Buffer Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      バッファ用途フラグに変換します.
//-------------------------------------------------------------------------------------------------
VkBufferUsageFlags ToNativeBufferUsage(uint32_t usage)
{
    VkBufferUsageFlags result = 0;

    if (usage & a3d::RESOURCE_USAGE_UNORDERED_ACCESS_VIEW)
    { result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; }

    if (usage & a3d::RESOURCE_USAGE_INDEX_BUFFER)
    { result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT; }

    if (usage & a3d::RESOURCE_USAGE_VERTEX_BUFFER)
    { result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; }

    if (usage & a3d::RESOURCE_USAGE_CONSTANT_BUFFER)
    { result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; }

    if (usage & a3d::RESOURCE_USAGE_INDIRECT_BUFFER)
    { result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT; }

    if (usage & a3d::RESOURCE_USAGE_COPY_SRC)
    { result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT; }

    if (usage & a3d::RESOURCE_USAGE_COPY_DST)
    { result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; }

    if (usage & a3d::RESOURCE_USAGE_QUERY_BUFFER)
    { result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; }

    return result;
}

} // namespace /* anonymous */


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
, m_Buffer      (null_handle)
, m_Allocation  (null_handle)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Buffer::~Buffer()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool Buffer::Init(IDevice* pDevice, const BufferDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    auto deviceMemoryProps = m_pDevice->GetVulkanPhysicalDeviceMemoryProperties(0);

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    // バッファを生成します.
    {
        VkBufferCreateInfo info = {};
        info.sType                  = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.pNext                  = nullptr;
        info.flags                  = 0;
        info.pQueueFamilyIndices    = nullptr;
        info.queueFamilyIndexCount  = 0;
        info.sharingMode            = VK_SHARING_MODE_EXCLUSIVE;
        info.size                   = pDesc->Size;
        info.usage                  = ToNativeBufferUsage(pDesc->Usage);

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = ToVmaMemoryUsage(pDesc->HeapProperty.Type);

        auto ret = vmaCreateBuffer(m_pDevice->GetAllocator(), &info, &allocInfo, &m_Buffer, &m_Allocation, nullptr);
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Buffer::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    if (m_Buffer != null_handle)
    {
        vmaDestroyBuffer(m_pDevice->GetAllocator(), m_Buffer, m_Allocation);
        m_Buffer = null_handle;
        m_Allocation = null_handle;
    }

    memset( &m_Desc, 0, sizeof(m_Desc) );

    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void Buffer::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void Buffer::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
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
    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    VkResult ret;

    void* pData;
    ret = vmaMapMemory(m_pDevice->GetAllocator(), m_Allocation, &pData);
    if (ret != VK_SUCCESS)
    { return nullptr; }

    return pData;
}

//-------------------------------------------------------------------------------------------------
//      メモリマッピングを解除します.
//-------------------------------------------------------------------------------------------------
void Buffer::Unmap()
{
    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    vmaUnmapMemory(m_pDevice->GetAllocator(), m_Allocation);
}

//-------------------------------------------------------------------------------------------------
//      バッファを取得します.
//-------------------------------------------------------------------------------------------------
VkBuffer Buffer::GetVulkanBuffer() const
{ return m_Buffer; }

//-------------------------------------------------------------------------------------------------
//      リソースタイプを取得します.
//-------------------------------------------------------------------------------------------------
RESOURCE_KIND Buffer::GetKind() const
{ return RESOURCE_KIND_BUFFER; }

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
