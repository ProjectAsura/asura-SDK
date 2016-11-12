//-------------------------------------------------------------------------------------------------
// File : a3dTexture.cpp
// Desc : Texture Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      イメージ用途フラグに変換します.
//------------------------------------------------------------------------------------------------
VkImageUsageFlags ToNativeImageUsage(uint32_t usage)
{
    VkImageUsageFlags result = 0;

    if (usage & a3d::RESOURCE_USAGE_COLOR_TARGET)
    { result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; }

    if (usage & a3d::RESOURCE_USAGE_DEPTH_TARGET)
    { result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; }

    if (usage & a3d::RESOURCE_USAGE_UNORDERD_ACCESS)
    {
        // TODO : 要確認.
        result |= VK_IMAGE_USAGE_STORAGE_BIT;
        result |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    if (usage & a3d::RESOURCE_USAGE_SHADER_RESOURCE)
    { result |= VK_IMAGE_USAGE_SAMPLED_BIT; }

    if (usage & a3d::RESOURCE_USAGE_COPY_SRC)
    { result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; }

    if (usage & a3d::RESOURCE_USAGE_COPY_DST)
    { result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; }

    return result;
}

//-------------------------------------------------------------------------------------------------
//      イメージタイプに変換します.
//-------------------------------------------------------------------------------------------------
VkImageType ToNativeImageType(a3d::RESOURCE_DIMENSION dimension)
{
    VkImageType table[] = {
        VK_IMAGE_TYPE_1D,       // UNKNOWN
        VK_IMAGE_TYPE_1D,       // BUFFER
        VK_IMAGE_TYPE_1D,       // TEXTURE1D
        VK_IMAGE_TYPE_2D,       // TEXTURE2D
        VK_IMAGE_TYPE_3D,       // TEXTURE3D
        VK_IMAGE_TYPE_2D,       // CUBEMAP
    };

    return table[dimension];
}

//-------------------------------------------------------------------------------------------------
//      イメージタイプに変換します.
//-------------------------------------------------------------------------------------------------
VkImageViewType ToNativeImageViewType(const a3d::TextureDesc* pDesc)
{
    VkImageViewType result = VK_IMAGE_VIEW_TYPE_1D;

    switch(pDesc->Dimension)
    {
    case a3d::RESOURCE_DIMENSION_TEXTURE1D:
        {
            if (pDesc->DepthOrArraySize > 1)
            { result = VK_IMAGE_VIEW_TYPE_1D_ARRAY; }
            else
            { result = VK_IMAGE_VIEW_TYPE_1D; }
        }
        break;

    case a3d::RESOURCE_DIMENSION_TEXTURE2D:
        {
            if (pDesc->DepthOrArraySize > 1)
            { result = VK_IMAGE_VIEW_TYPE_2D_ARRAY; }
            else 
            { result = VK_IMAGE_VIEW_TYPE_2D; }
        }
        break;

    case a3d::RESOURCE_DIMENSION_TEXTURE3D:
        { result = VK_IMAGE_VIEW_TYPE_3D; }
        break;

    case a3d::RESOURCE_DIMENSION_CUBEMAP:
        {
            if (pDesc->DepthOrArraySize > 6)
            { result = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY; }
            else
            { result = VK_IMAGE_VIEW_TYPE_CUBE; }
        }
        break;
    }

    return result;
}


//-------------------------------------------------------------------------------------------------
//      フォーマットがサポートされているかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsSupportFormat(a3d::Device* pDevice, const a3d::TextureDesc* pDesc)
{
    auto physicalDevice = pDevice->GetVulkanPhysicalDevice(0);
    A3D_ASSERT(physicalDevice != null_handle);

    auto format = a3d::ToNativeFormat(pDesc->Format);

    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);

    if (pDesc->Usage & a3d::RESOURCE_USAGE_COLOR_TARGET)
    {
        if (pDesc->Layout == a3d::RESOURCE_LAYOUT_LINEAR)
        {
            if ((formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0)
            { return false; }
        }
        else
        {
            if ((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0)
            { return false; }
        }
    }

    if (pDesc->Usage & a3d::RESOURCE_USAGE_DEPTH_TARGET)
    {
        if (pDesc->Layout == a3d::RESOURCE_LAYOUT_LINEAR)
        {
            if ((formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0)
            { return false; }
        }
        else
        {
            if ((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0)
            { return false; }
        }
    }

    if (pDesc->Usage & a3d::RESOURCE_USAGE_UNORDERD_ACCESS)
    {
        if (pDesc->Layout == a3d::RESOURCE_LAYOUT_LINEAR)
        {
            if ((formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) == 0)
            { return false; }
        }
        else
        {
            if ((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) == 0)
            { return false; }
        }
    }

    if (pDesc->Usage & a3d::RESOURCE_USAGE_CONSTANT_BUFFER)
    {
        if (pDesc->Layout == a3d::RESOURCE_LAYOUT_LINEAR)
        {
            if ((formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) == 0)
            { return false; }
        }
        else
        {
            if ((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) == 0)
            { return false; }
        }
    }

    if (pDesc->Usage & a3d::RESOURCE_USAGE_INDEX_BUFFER)
    { return false; }

    if (pDesc->Usage & a3d::RESOURCE_USAGE_VERTEX_BUFFER)
    { return false; }

    if (pDesc->Usage & a3d::RESOURCE_USAGE_INDIRECT_BUFFER)
    { return false; }

    return true;
}

} // namespace /* anonymous */


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Texture class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Texture::Texture()
: m_RefCount        (1)
, m_pDevice         (nullptr)
, m_State           (RESOURCE_STATE_UNKNOWN)
, m_Image           (null_handle)
, m_DeviceMemory    (null_handle)
, m_ImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
{ memset( &m_MemoryRequirements, 0, sizeof(m_MemoryRequirements) ); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Texture::~Texture()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool Texture::Init(IDevice* pDevice, const TextureDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    auto pWrapDevice = reinterpret_cast<Device*>(pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    if (!IsSupportFormat(pWrapDevice, pDesc))
    { return false; }

    auto deviceMemoryProps = pWrapDevice->GetVulkanPhysicalDeviceMemoryProperties(0);
    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    auto imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_State = a3d::RESOURCE_STATE_UNKNOWN;

    // イメージを生成します.
    {
        VkImageCreateInfo info = {};
        info.sType                  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext                  = nullptr;
        info.flags                  = 0;
        info.imageType              = ToNativeImageType(pDesc->Dimension);
        info.format                 = ToNativeFormat(pDesc->Format);
        info.extent.width           = pDesc->Width;
        info.extent.height          = pDesc->Height;
        info.extent.depth           = (pDesc->Dimension == RESOURCE_DIMENSION_TEXTURE3D) ? pDesc->DepthOrArraySize : 1;
        info.mipLevels              = pDesc->MipLevels;
        info.arrayLayers            = (pDesc->Dimension != RESOURCE_DIMENSION_TEXTURE3D) ? pDesc->DepthOrArraySize : 1;
        info.samples                = ToNativeSampleCountFlags(pDesc->SampleCount);
        info.tiling                 = (pDesc->Layout == RESOURCE_LAYOUT_LINEAR) ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
        info.usage                  = ToNativeImageUsage(pDesc->Usage);
        info.sharingMode            = VK_SHARING_MODE_EXCLUSIVE;
        info.queueFamilyIndexCount  = 0;
        info.pQueueFamilyIndices    = nullptr;
        info.initialLayout          = imageLayout;

        auto ret = vkCreateImage(pNativeDevice, &info, nullptr, &m_Image);
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    // デバイスメモリを生成します.
    {
        vkGetImageMemoryRequirements(pNativeDevice, m_Image, &m_MemoryRequirements);

        bool isMappable = (pDesc->Layout == RESOURCE_LAYOUT_LINEAR);
        auto flags = ToNativeMemoryPropertyFlags(pDesc->HeapProperty.CpuPageProperty, isMappable);

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

        ret = vkBindImageMemory(pNativeDevice, m_Image, m_DeviceMemory, 0);
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    // イメージアスペクトフラグの設定.
    {
       
            if (pDesc->Format == RESOURCE_FORMAT_D24_UNORM_S8_UINT)
            { m_ImageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT; }
            else if (pDesc->Format == RESOURCE_FORMAT_D16_UNORM
                  || pDesc->Format == RESOURCE_FORMAT_D32_FLOAT )
            { m_ImageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT; }
            else
            { m_ImageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT; }
    }

    // イメージレイアウトを変更
    if (m_State != pDesc->InitState)
    {
        ICommandList* pCmdList;
        if (!m_pDevice->CreateCommandList(COMMANDLIST_TYPE_DIRECT, nullptr, &pCmdList))
        { return false; }

        auto pWrapCmdList = reinterpret_cast<CommandList*>(pCmdList);
        A3D_ASSERT(pWrapCmdList != nullptr);

        pWrapCmdList->Begin();
        pWrapCmdList->TextureBarrier(this, pDesc->InitState);
        pWrapCmdList->End();
        pWrapCmdList->Flush();

        SafeRelease(pCmdList);
    }

    m_IsExternal = false;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Texture::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    if (!m_IsExternal)
    {
        auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
        A3D_ASSERT(pWrapDevice != nullptr);

        auto pNativeDevice = pWrapDevice->GetVulkanDevice();
        A3D_ASSERT(pNativeDevice != null_handle);

        if (m_Image != null_handle)
        {
            vkDestroyImage(pNativeDevice, m_Image, nullptr);
            m_Image = null_handle;
        }

        if (m_DeviceMemory != null_handle)
        {
            vkFreeMemory(pNativeDevice, m_DeviceMemory, nullptr);
            m_DeviceMemory = null_handle;
        }
    }
    else
    {
        m_Image        = null_handle;
        m_DeviceMemory = null_handle;
    }

    memset( &m_MemoryRequirements, 0, sizeof(m_MemoryRequirements) );
    memset( &m_Desc, 0, sizeof(m_Desc) );

    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void Texture::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
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
//      リソースステートを設定します.
//-------------------------------------------------------------------------------------------------
void Texture::SetState(RESOURCE_STATE state)
{ m_State = state; }

//-------------------------------------------------------------------------------------------------
//      メモリマッピングします.
//-------------------------------------------------------------------------------------------------
void* Texture::Map()
{
    if (m_DeviceMemory == null_handle)
    { return nullptr; }

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    void* pData;
    auto ret = vkMapMemory(pNativeDevice, m_DeviceMemory, 0, m_Desc.Width, 0, &pData);
    if (ret != VK_SUCCESS)
    { return nullptr; }

    return pData;
}

//-------------------------------------------------------------------------------------------------
//      メモリマッピングを解除します.
//-------------------------------------------------------------------------------------------------
void Texture::Unmap()
{
    if (m_DeviceMemory == null_handle)
    { return; }

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    vkUnmapMemory(pNativeDevice, m_DeviceMemory);
}

//-------------------------------------------------------------------------------------------------
//      サブリソースレイアウトを取得します.
//-------------------------------------------------------------------------------------------------
SubresourceLayout Texture::GetSubresourceLayout(uint32_t subresource) const
{
    VkImageSubresource subres = {};
    subres.aspectMask = m_ImageAspectFlags;
    DecomposeSubresource(
        subresource,
        m_Desc.MipLevels,
        m_Desc.DepthOrArraySize,
        subres.mipLevel,
        subres.arrayLayer);

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    VkSubresourceLayout layout = {};
    vkGetImageSubresourceLayout(pNativeDevice, m_Image, &subres, &layout);

    SubresourceLayout result = {};
    result.Offset   = layout.offset;
    result.Size     = layout.size;

    if (m_Desc.Dimension == RESOURCE_DIMENSION_TEXTURE3D)
    { result.SlicePitch = layout.depthPitch; }
    else
    { result.SlicePitch = layout.arrayPitch; }

    if (layout.rowPitch != 0)
    {
        result.RowPitch = layout.rowPitch;
        result.RowCount = layout.size / layout.rowPitch;
    }
    else
    {
        result.RowPitch = result.SlicePitch / m_Desc.Height;
        result.RowCount = m_Desc.Height;
    }

    return result;
}

//-------------------------------------------------------------------------------------------------
//      イメージを取得します.
//-------------------------------------------------------------------------------------------------
VkImage Texture::GetVulkanImage() const
{ return m_Image; }

//-------------------------------------------------------------------------------------------------
//      デバイスメモリを取得します.
//-------------------------------------------------------------------------------------------------
VkDeviceMemory Texture::GetVulkanDeviceMemory() const
{ return m_DeviceMemory; }

//-------------------------------------------------------------------------------------------------
//      メモリ要件を取得します.
//-------------------------------------------------------------------------------------------------
VkMemoryRequirements Texture::GetVulkanMemoryRequirements() const
{ return m_MemoryRequirements; }

//-------------------------------------------------------------------------------------------------
//      イメージアスペクトフラグを取得します.
//-------------------------------------------------------------------------------------------------
VkImageAspectFlags Texture::GetVulkanImageAspectFlags() const
{ return m_ImageAspectFlags; }

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
    if ( instance == nullptr )
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
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Texture::Create
(
    IDevice*                pDevice,
    const SwapChainDesc*    pDesc,
    VkImage                 image,
    VkImageView             view,
    ITexture**              ppResource
)
{
    if (pDevice == nullptr || pDesc == nullptr || ppResource == nullptr)
    { return false; }

    auto instance = new Texture;
    if ( instance == nullptr )
    { return false; }

    auto pWrapDevice = reinterpret_cast<Device*>(pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    bool isDepth = false;
    if (pDesc->Format == RESOURCE_FORMAT_D16_UNORM ||
        pDesc->Format == RESOURCE_FORMAT_D24_UNORM_S8_UINT ||
        pDesc->Format == RESOURCE_FORMAT_D32_FLOAT)
    { isDepth = true;}

    instance->m_IsExternal = true;
    instance->m_pDevice    = pDevice;
    instance->m_pDevice->AddRef();

    instance->m_Image                               = image;
    instance->m_ImageAspectFlags                    = (isDepth) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    instance->m_DeviceMemory                        = null_handle;
    instance->m_State                               = RESOURCE_STATE_UNKNOWN;
    instance->m_Desc.Dimension                      = RESOURCE_DIMENSION_TEXTURE2D;
    instance->m_Desc.Width                          = pDesc->Extent.Width;
    instance->m_Desc.Height                         = pDesc->Extent.Height;
    instance->m_Desc.DepthOrArraySize               = 1;
    instance->m_Desc.Format                         = pDesc->Format;
    instance->m_Desc.MipLevels                      = pDesc->MipLevels;
    instance->m_Desc.SampleCount                    = pDesc->SampleCount;
    instance->m_Desc.Layout                         = RESOURCE_LAYOUT_OPTIMAL;
    instance->m_Desc.InitState                      = RESOURCE_STATE_UNKNOWN;
    instance->m_Desc.HeapProperty.Type              = HEAP_TYPE_DEFAULT;
    instance->m_Desc.HeapProperty.CpuPageProperty   = CPU_PAGE_PROPERTY_NOT_AVAILABLE;

    vkGetImageMemoryRequirements(pNativeDevice, image, &instance->m_MemoryRequirements);

    *ppResource = instance;
    return true;
}

} // namespace a3d
