//-------------------------------------------------------------------------------------------------
// File : a3dUtil.cpp
// Desc : Utility Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
VkFormat ToNativeFormat(RESOURCE_FORMAT value)
{
    static VkFormat table[] = {
        VK_FORMAT_UNDEFINED,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_FORMAT_R32G32B32A32_UINT,
        VK_FORMAT_R32G32B32A32_SINT,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_FORMAT_R16G16B16A16_UINT,
        VK_FORMAT_R16G16B16A16_SINT,
        VK_FORMAT_R32G32B32_SFLOAT,
        VK_FORMAT_R32G32B32_UINT,
        VK_FORMAT_R32G32B32_SINT,
        VK_FORMAT_R32G32_SFLOAT,
        VK_FORMAT_R32G32_UINT,
        VK_FORMAT_R32G32_SINT,
        VK_FORMAT_R16G16_SFLOAT,
        VK_FORMAT_R16G16_UINT,
        VK_FORMAT_R16G16_SINT,
        VK_FORMAT_R32_SFLOAT,
        VK_FORMAT_R32_UINT,
        VK_FORMAT_R32_SINT,
        VK_FORMAT_R16_SFLOAT,
        VK_FORMAT_R16_UINT,
        VK_FORMAT_R16_SINT,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_R8G8_UNORM,
        VK_FORMAT_R8_UNORM,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM,
        VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
        VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
        VK_FORMAT_BC2_SRGB_BLOCK,
        VK_FORMAT_BC2_UNORM_BLOCK,
        VK_FORMAT_BC3_SRGB_BLOCK,
        VK_FORMAT_BC3_UNORM_BLOCK,
        VK_FORMAT_BC4_UNORM_BLOCK,
        VK_FORMAT_BC4_SNORM_BLOCK,
        VK_FORMAT_BC5_UNORM_BLOCK,
        VK_FORMAT_BC5_SNORM_BLOCK,
        VK_FORMAT_BC6H_UFLOAT_BLOCK,
        VK_FORMAT_BC6H_SFLOAT_BLOCK,
        VK_FORMAT_BC7_SRGB_BLOCK,
        VK_FORMAT_BC7_UNORM_BLOCK,
        VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
        VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
        VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
        VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
        VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
        VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
        VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
        VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
        VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
        VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
        VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
        VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
        VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
        VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
        VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
        VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
        VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
        VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
        VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
        VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
        VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
        VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
        VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
        VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
        VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
        VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
        VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
        VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
        VK_FORMAT_A2R10G10B10_UNORM_PACK32,
        VK_FORMAT_A2B10G10R10_UINT_PACK32,
        VK_FORMAT_B10G11R11_UFLOAT_PACK32
    };

    return table[value];
}


//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
VkFormat ToNativeViewFormat(RESOURCE_FORMAT value)
{
    static VkFormat table[] = {
        VK_FORMAT_UNDEFINED,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_FORMAT_R32G32B32A32_UINT,
        VK_FORMAT_R32G32B32A32_SINT,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_FORMAT_R16G16B16A16_UINT,
        VK_FORMAT_R16G16B16A16_SINT,
        VK_FORMAT_R32G32B32_SFLOAT,
        VK_FORMAT_R32G32B32_UINT,
        VK_FORMAT_R32G32B32_SINT,
        VK_FORMAT_R32G32_SFLOAT,
        VK_FORMAT_R32G32_UINT,
        VK_FORMAT_R32G32_SINT,
        VK_FORMAT_R16G16_SFLOAT,
        VK_FORMAT_R16G16_UINT,
        VK_FORMAT_R16G16_SINT,
        VK_FORMAT_R32_SFLOAT,
        VK_FORMAT_R32_UINT,
        VK_FORMAT_R32_SINT,
        VK_FORMAT_R16_SFLOAT,
        VK_FORMAT_R16_UINT,
        VK_FORMAT_R16_SINT,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_R8G8_UNORM,
        VK_FORMAT_R8_UNORM,
        VK_FORMAT_R32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_R16_UNORM,
        VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
        VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
        VK_FORMAT_BC2_SRGB_BLOCK,
        VK_FORMAT_BC2_UNORM_BLOCK,
        VK_FORMAT_BC3_SRGB_BLOCK,
        VK_FORMAT_BC3_UNORM_BLOCK,
        VK_FORMAT_BC4_UNORM_BLOCK,
        VK_FORMAT_BC4_SNORM_BLOCK,
        VK_FORMAT_BC5_UNORM_BLOCK,
        VK_FORMAT_BC5_SNORM_BLOCK,
        VK_FORMAT_BC6H_UFLOAT_BLOCK,
        VK_FORMAT_BC6H_SFLOAT_BLOCK,
        VK_FORMAT_BC7_SRGB_BLOCK,
        VK_FORMAT_BC7_UNORM_BLOCK,
        VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
        VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
        VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
        VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
        VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
        VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
        VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
        VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
        VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
        VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
        VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
        VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
        VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
        VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
        VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
        VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
        VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
        VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
        VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
        VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
        VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
        VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
        VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
        VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
        VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
        VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
        VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
        VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
        VK_FORMAT_A2R10G10B10_UNORM_PACK32,
        VK_FORMAT_A2B10G10R10_UINT_PACK32,
        VK_FORMAT_B10G11R11_UFLOAT_PACK32
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
VkCompareOp ToNativeCompareOp(COMPARE_OP value)
{
    static VkCompareOp table[] = {
        VK_COMPARE_OP_NEVER,
        VK_COMPARE_OP_LESS,
        VK_COMPARE_OP_EQUAL,
        VK_COMPARE_OP_LESS_OR_EQUAL,
        VK_COMPARE_OP_GREATER,
        VK_COMPARE_OP_GREATER_OR_EQUAL,
        VK_COMPARE_OP_ALWAYS
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
VkImageLayout ToNativeImageLayout(RESOURCE_STATE value)
{
    static VkImageLayout table[] = {
        VK_IMAGE_LAYOUT_UNDEFINED,                                  // UNKNOWN
        VK_IMAGE_LAYOUT_GENERAL,                                    // GENERAL
        VK_IMAGE_LAYOUT_GENERAL,                                    // VERTEX_BUFFER
        VK_IMAGE_LAYOUT_GENERAL,                                    // INDEX_BUFFER
        VK_IMAGE_LAYOUT_GENERAL,                                    // CONSTANT_BUFFER
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,                   // COLOR_WRITE
        VK_IMAGE_LAYOUT_GENERAL,                                    // COLOR_READ
        VK_IMAGE_LAYOUT_GENERAL,                                    // UNORDERED_ACCESS
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,           // DEPTH_WRITE
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,            // DEPTH_READ
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,                   // SHADER_READ
        VK_IMAGE_LAYOUT_GENERAL,                                    // STREAM_OUT
        VK_IMAGE_LAYOUT_GENERAL,                                    // INDIRECT_ARGUMENT
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,                       // COPY_DST
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,                       // COPY_SRC
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,                       // RESOLVE_DST
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,                       // RESOLVE_SRC
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                            // PRESENT
        VK_IMAGE_LAYOUT_GENERAL,                                    // PREDICATION
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタタイプをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
VkDescriptorType ToNativeDescriptorType(DESCRIPTOR_TYPE type)
{
    static VkDescriptorType table[] = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
    };

    return table[type];
}

//-------------------------------------------------------------------------------------------------
//      サンプル数をネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
VkSampleCountFlagBits ToNativeSampleCountFlags(uint32_t sampleCount)
{
    if (sampleCount <= 1)
    { return VK_SAMPLE_COUNT_1_BIT; }
    else if (sampleCount <= 2)
    { return VK_SAMPLE_COUNT_2_BIT; }
    else if (sampleCount <= 4 )
    { return VK_SAMPLE_COUNT_4_BIT; }
    else if (sampleCount <= 8 )
    { return VK_SAMPLE_COUNT_8_BIT; }
    else if (sampleCount <= 16)
    { return VK_SAMPLE_COUNT_16_BIT; }
    else if (sampleCount <= 32)
    { return VK_SAMPLE_COUNT_32_BIT; }
    else
    { return VK_SAMPLE_COUNT_64_BIT; }
}

//-------------------------------------------------------------------------------------------------
//      メモリプロパティフラグに変換します.
//-------------------------------------------------------------------------------------------------
VkMemoryPropertyFlags ToNativeMemoryPropertyFlags(a3d::CPU_PAGE_PROPERTY prop, bool isMappable)
{
    VkMemoryPropertyFlags result = 0;

    if (prop == a3d::CPU_PAGE_PROPERTY_DEFAULT && isMappable)
    {
        result |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }
    else if (prop == a3d::CPU_PAGE_PROPERTY_DEFAULT && !isMappable)
    {
        result |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
    else if (prop == a3d::CPU_PAGE_PROPERTY_NOT_AVAILABLE)
    {
        result |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
    else if (prop == a3d::CPU_PAGE_PROPERTY_WRITE_COMBINE)
    {
        result |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        result |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        result |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
    else if (prop == a3d::CPU_PAGE_PROPERTY_WRITE_BACK)
    {
        result |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        result |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        result |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    }

    return result;
}

//-------------------------------------------------------------------------------------------------
//      メモリタイプインデックスを取得します.
//-------------------------------------------------------------------------------------------------
bool GetMemoryTypeIndex
(
    const VkPhysicalDeviceMemoryProperties& deviceMemoryProps,
    const VkMemoryRequirements&             requirements,
    VkMemoryPropertyFlags                   flags,
    uint32_t&                               result
)
{
    auto typeBits  = requirements.memoryTypeBits;
    result = 0u;
    for (auto i=0u; i<VK_MAX_MEMORY_TYPES; ++i)
    {
        auto& propFlags = deviceMemoryProps.memoryTypes[i].propertyFlags;
        if ((typeBits & 0x1) == 0x1)
        {
            if ((propFlags & flags) == flags)
            {
                result = i;
                return true;
            }
        }
        typeBits >>= 1;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      イメージレイアウトを設定します.
//-------------------------------------------------------------------------------------------------
void SetImageLayout
(
    VkCommandBuffer         commandBuffer,
    VkImage                 image,
    VkPipelineStageFlags    srcStageMask,
    VkPipelineStageFlags    dstStageMask,
    VkImageLayout           oldLayout,
    VkImageLayout           newLayout,
    VkImageSubresourceRange range
)
{
    A3D_ASSERT(commandBuffer != null_handle);

    VkImageMemoryBarrier barrier = {};
    barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext               = nullptr;
    barrier.srcAccessMask       = 0;
    barrier.dstAccessMask       = 0;
    barrier.oldLayout           = oldLayout;
    barrier.newLayout           = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image               = image;
    barrier.subresourceRange    = range;

    switch(oldLayout)
    {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            barrier.srcAccessMask = 0;
            break;

        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            barrier.srcAccessMask = VK_ACCESS_HOST_READ_BIT;
            break;

        default:
            break;
    }

    switch (newLayout)
    {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            if (barrier.srcAccessMask == 0)
            { barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT; }
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            break;

        default:
            break;
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        srcStageMask,
        dstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
}

//-------------------------------------------------------------------------------------------------
//      イメージレイアウトを設定します.
//-------------------------------------------------------------------------------------------------
void SetImageLayout(
    VkCommandBuffer         commandBuffer,
    VkPipelineStageFlags    pipelineStage,
    VkImage                 image,
    VkImageLayout           oldLayout,
    VkImageLayout           newLayout,
    VkImageSubresourceRange range
)
{
    SetImageLayout(
        commandBuffer,
        image,
        pipelineStage,
        pipelineStage,
        oldLayout,
        newLayout,
        range);
}

//-------------------------------------------------------------------------------------------------
//      アクセスフラグに変換します.
//-------------------------------------------------------------------------------------------------
VkAccessFlags ToNativeAccessFlags(a3d::RESOURCE_STATE state)
{
    VkAccessFlags result = 0;
    switch( state )
    {
    case a3d::RESOURCE_STATE_UNKNOWN:
    case a3d::RESOURCE_STATE_GENERAL:
        { result = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT; }
        break;

    case a3d::RESOURCE_STATE_VERTEX_BUFFER:
        { result = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT; }
        break;

    case a3d::RESOURCE_STATE_INDEX_BUFFER:
        { result = VK_ACCESS_INDEX_READ_BIT; }
        break;

    case a3d::RESOURCE_STATE_CONSTANT_BUFFER:
        { result = VK_ACCESS_UNIFORM_READ_BIT; }
        break;

    case a3d::RESOURCE_STATE_COLOR_WRITE:
        { result = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; }
        break;

    case a3d::RESOURCE_STATE_COLOR_READ:
        { result = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT; }
        break;

    case a3d::RESOURCE_STATE_UNORDERED_ACCESS:
        { result = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT; }
        break;

    case a3d::RESOURCE_STATE_DEPTH_WRITE:
        { result = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; }
        break;

    case a3d::RESOURCE_STATE_DEPTH_READ:
        { result = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT; }
        break;

    case a3d::RESOURCE_STATE_SHADER_READ:
        { result = VK_ACCESS_SHADER_READ_BIT; }
        break;

    case a3d::RESOURCE_STATE_STREAM_OUT:
        { result = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT; }
        break;

    case a3d::RESOURCE_STATE_INDIRECT_ARGUMENT:
        { result = VK_ACCESS_INDIRECT_COMMAND_READ_BIT; }
        break;

    case a3d::RESOURCE_STATE_COPY_DST:
    case a3d::RESOURCE_STATE_RESOLVE_DST:
        { result = VK_ACCESS_TRANSFER_WRITE_BIT; }
        break;

    case a3d::RESOURCE_STATE_COPY_SRC:
    case a3d::RESOURCE_STATE_RESOLVE_SRC:
        { result = VK_ACCESS_TRANSFER_READ_BIT; }
        break;

    case a3d::RESOURCE_STATE_PRESENT:
        { result = VK_ACCESS_MEMORY_READ_BIT; }
        break;

    case a3d::RESORUCE_STATE_PREDICATION:
        { result = VK_ACCESS_SHADER_WRITE_BIT; }
        break;
    }
    return result;
}

//-------------------------------------------------------------------------------------------------
//      サブリソースを計算します.
//-------------------------------------------------------------------------------------------------
uint32_t CalcSubresource
(
    uint32_t mipSlice,
    uint32_t arraySlice,
    uint32_t mipLevels,
    uint32_t arraySize
)
{ return mipSlice + arraySlice * mipLevels; }

//-------------------------------------------------------------------------------------------------
//      サブリソース番号からミップレベル・配列番号・平面スライスを求めます.
//-------------------------------------------------------------------------------------------------
void DecomposeSubresource
(
    uint32_t subresource,
    uint32_t mipLevels,
    uint32_t arraySize,
    uint32_t& mipSlice,
    uint32_t& arraySlice
)
{
    mipSlice   = subresource % mipLevels;
    arraySlice = (subresource / mipLevels) % arraySize;
}

} // namespace a3d
