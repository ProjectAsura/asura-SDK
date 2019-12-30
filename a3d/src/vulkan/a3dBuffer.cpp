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
, m_DeviceMemory(null_handle)
{ memset(&m_MemoryRequirements, 0, sizeof(m_MemoryRequirements)); }

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

        auto ret = vkCreateBuffer(pNativeDevice, &info, nullptr, &m_Buffer);
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    // デバイスメモリを生成します.
    {
        vkGetBufferMemoryRequirements(pNativeDevice, m_Buffer, &m_MemoryRequirements);

        auto flags = ToNativeMemoryPropertyFlags(pDesc->HeapProperty.CpuPageProperty, true);

        uint32_t index = 0;
        GetMemoryTypeIndex(deviceMemoryProps, m_MemoryRequirements, flags, index);

        VkMemoryAllocateInfo info = {};
        info.sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.pNext              = nullptr;
        info.memoryTypeIndex    = index;
        info.allocationSize     = m_MemoryRequirements.size;

        auto ret = vkAllocateMemory(pNativeDevice, &info, nullptr, &m_DeviceMemory);
        if ( ret != VK_SUCCESS )
        { return false; }

        ret = vkBindBufferMemory(pNativeDevice, m_Buffer, m_DeviceMemory, 0);
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

    if (m_DeviceMemory != null_handle)
    {
        vkFreeMemory(pNativeDevice, m_DeviceMemory, nullptr);
        m_DeviceMemory = null_handle;
    }

    if (m_Buffer != null_handle)
    {
        vkDestroyBuffer(pNativeDevice, m_Buffer, nullptr);
        m_Buffer = null_handle;
    }

    memset( &m_MemoryRequirements, 0, sizeof(m_MemoryRequirements) );
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
    ret = vkMapMemory(pNativeDevice, m_DeviceMemory, 0, m_Desc.Size, 0, &pData);
    if (ret != VK_SUCCESS)
    { return nullptr; }

    VkMappedMemoryRange range = {};
    range.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.pNext  = nullptr;
    range.memory = m_DeviceMemory;
    range.offset = 0;
    range.size   = m_Desc.Size;
    ret = vkInvalidateMappedMemoryRanges(pNativeDevice, 1, &range);
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

    VkMappedMemoryRange range = {};
    range.sType     = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.pNext     = nullptr;
    range.memory    = m_DeviceMemory;
    range.offset    = 0;
    range.size      = m_Desc.Size;
    vkFlushMappedMemoryRanges(pNativeDevice, 1, &range);

    vkUnmapMemory(pNativeDevice, m_DeviceMemory);
}

//-------------------------------------------------------------------------------------------------
//      バッファを取得します.
//-------------------------------------------------------------------------------------------------
VkBuffer Buffer::GetVulkanBuffer() const
{ return m_Buffer; }

//-------------------------------------------------------------------------------------------------
//      デバイスメモリを取得します.
//-------------------------------------------------------------------------------------------------
VkDeviceMemory Buffer::GetVulkanDeviceMemory() const
{ return m_DeviceMemory; }

//-------------------------------------------------------------------------------------------------
//      メモリ要件を取得します.
//-------------------------------------------------------------------------------------------------
VkMemoryRequirements Buffer::GetVulkanMemoryRequirements() const
{ return m_MemoryRequirements; }

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
