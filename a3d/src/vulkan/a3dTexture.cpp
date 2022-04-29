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

    if (usage & a3d::RESOURCE_USAGE_RENDER_TARGET)
    { result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; }

    if (usage & a3d::RESOURCE_USAGE_DEPTH_STENCIL)
    { result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; }

    if (usage & a3d::RESOURCE_USAGE_UNORDERED_ACCESS)
    { result |= VK_IMAGE_USAGE_STORAGE_BIT; }

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

#if 0
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

    default:
        break;
    }

    return result;
}
#endif


//-------------------------------------------------------------------------------------------------
//      フォーマットがサポートされているかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsSupportFormat(a3d::Device* pDevice, const a3d::TextureDesc* pDesc)
{
    auto physicalDevice = pDevice->GetVkPhysicalDevice(0);
    A3D_ASSERT(physicalDevice != null_handle);

    auto format = a3d::ToNativeFormat(pDesc->Format);

    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);

    if (pDesc->Usage & a3d::RESOURCE_USAGE_RENDER_TARGET)
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

    if (pDesc->Usage & a3d::RESOURCE_USAGE_DEPTH_STENCIL)
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

    if (pDesc->Usage & a3d::RESOURCE_USAGE_UNORDERED_ACCESS)
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
, m_Image           (null_handle)
, m_Allocation      (null_handle)
, m_ImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
{ /* DO_NOTHING */ }

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
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pNativeDevice = m_pDevice->GetVkDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    if (!IsSupportFormat(m_pDevice, pDesc))
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto deviceMemoryProps = m_pDevice->GetVkPhysicalDeviceMemoryProperties(0);
    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    auto imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

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

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = ToVmaMemoryUsage(pDesc->HeapType);

        auto ret = vmaCreateImage(m_pDevice->GetAllocator(), &info, &allocInfo, &m_Image, &m_Allocation, nullptr);
        if ( ret != VK_SUCCESS )
        {
            A3D_LOG("Error : vmaCreateImage() Failed. VkResult = %s", ToString(ret));
            return false;
        }
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
    if (RESOURCE_STATE_UNKNOWN != pDesc->InitState)
    {
        ICommandList* pCmdList;
        CommandListDesc desc = {};
        desc.Type = COMMANDLIST_TYPE_DIRECT;
        if (!m_pDevice->CreateCommandList(&desc, &pCmdList))
        {
            A3D_LOG("Error : Device::CretaeCommandList() Failed.");
            return false;
        }

        auto pWrapCmdList = static_cast<CommandList*>(pCmdList);
        A3D_ASSERT(pWrapCmdList != nullptr);

        pWrapCmdList->Begin();
        pWrapCmdList->TextureBarrier(this, a3d::RESOURCE_STATE_UNKNOWN, pDesc->InitState);
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
        auto pNativeDevice = m_pDevice->GetVkDevice();
        A3D_ASSERT(pNativeDevice != null_handle);

        if (m_Image != null_handle)
        {
            vmaDestroyImage(m_pDevice->GetAllocator(), m_Image, m_Allocation);
            m_Image      = null_handle;
            m_Allocation = null_handle;
        }
    }
    else
    {
        m_Image      = null_handle;
        m_Allocation = null_handle;
    }

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
//      メモリマッピングします.
//-------------------------------------------------------------------------------------------------
void* Texture::Map()
{
    auto pNativeDevice = m_pDevice->GetVkDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    void* pData;
    auto ret = vmaMapMemory(m_pDevice->GetAllocator(), m_Allocation, &pData);
    if (ret != VK_SUCCESS)
    { return nullptr; }

    return pData;
}

//-------------------------------------------------------------------------------------------------
//      メモリマッピングを解除します.
//-------------------------------------------------------------------------------------------------
void Texture::Unmap()
{
    auto pNativeDevice = m_pDevice->GetVkDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    vmaUnmapMemory(m_pDevice->GetAllocator(), m_Allocation);
}

//-------------------------------------------------------------------------------------------------
//      サブリソースレイアウトを取得します.
//-------------------------------------------------------------------------------------------------
SubresourceLayout Texture::GetSubresourceLayout(uint32_t subresource) const
{
    // Vulkanの仕様変更にVkSubresourceLayout()はLINEARモードしか呼び出しできなくなった(1.0.42あたりから).
    if (m_Desc.Layout == RESOURCE_LAYOUT_LINEAR)
    {
        VkImageSubresource subres = {};
        subres.aspectMask = m_ImageAspectFlags;

        uint32_t placeSlice;
        DecomposeSubresource(
            subresource,
            m_Desc.MipLevels,
            m_Desc.DepthOrArraySize,
            subres.mipLevel,
            subres.arrayLayer,
            placeSlice);

        auto pNativeDevice = m_pDevice->GetVkDevice();
        A3D_ASSERT(pNativeDevice != null_handle);

        SubresourceLayout result = {};
        
        VkSubresourceLayout layout = {};
        vkGetImageSubresourceLayout(pNativeDevice, m_Image, &subres, &layout);

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

    return CalcSubresourceLayout(
        subresource,
        m_Desc.Format,
        m_Desc.Width,
        m_Desc.Height);
}

//-------------------------------------------------------------------------------------------------
//      イメージを取得します.
//-------------------------------------------------------------------------------------------------
VkImage Texture::GetVkImage() const
{ return m_Image; }

//-------------------------------------------------------------------------------------------------
//      イメージアスペクトフラグを取得します.
//-------------------------------------------------------------------------------------------------
VkImageAspectFlags Texture::GetVkImageAspectFlags() const
{ return m_ImageAspectFlags; }

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
    if ( instance == nullptr )
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
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new Texture;
    if ( instance == nullptr )
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    auto pWrapDevice = static_cast<Device*>(pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVkDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    bool isDepth = false;
    if (pDesc->Format == RESOURCE_FORMAT_D16_UNORM ||
        pDesc->Format == RESOURCE_FORMAT_D24_UNORM_S8_UINT ||
        pDesc->Format == RESOURCE_FORMAT_D32_FLOAT)
    { isDepth = true;}

    instance->m_IsExternal = true;
    instance->m_pDevice    = pWrapDevice;
    instance->m_pDevice->AddRef();

    instance->m_Image                   = image;
    instance->m_ImageAspectFlags        = (isDepth) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    instance->m_Desc.Dimension          = RESOURCE_DIMENSION_TEXTURE2D;
    instance->m_Desc.Width              = pDesc->Extent.Width;
    instance->m_Desc.Height             = pDesc->Extent.Height;
    instance->m_Desc.DepthOrArraySize   = 1;
    instance->m_Desc.Format             = pDesc->Format;
    instance->m_Desc.MipLevels          = pDesc->MipLevels;
    instance->m_Desc.SampleCount        = pDesc->SampleCount;
    instance->m_Desc.Layout             = RESOURCE_LAYOUT_OPTIMAL;
    instance->m_Desc.InitState          = RESOURCE_STATE_UNKNOWN;
    instance->m_Desc.HeapType           = HEAP_TYPE_DEFAULT;

    *ppResource = instance;
    return true;
}

} // namespace a3d
