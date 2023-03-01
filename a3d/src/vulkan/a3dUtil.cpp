//-------------------------------------------------------------------------------------------------
// File : a3dUtil.cpp
// Desc : Utility Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

#define STR(var) #var

namespace {

///////////////////////////////////////////////////////////////////////////////////////////////////
// FormatConvertTable structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct FormatConvertTable
{
    VkFormat                NativeFormat;       //!< ネイティブ形式です.
    a3d::RESOURCE_FORMAT    Format;             //!< A3D形式です.
};

// フォーマット変換テーブル.
FormatConvertTable g_FormatTable[] = {
    { VK_FORMAT_UNDEFINED               , a3d::RESOURCE_FORMAT_UNKNOWN               },
    { VK_FORMAT_R32G32B32A32_SFLOAT     , a3d::RESOURCE_FORMAT_R32G32B32A32_FLOAT    },
    { VK_FORMAT_R32G32B32A32_UINT       , a3d::RESOURCE_FORMAT_R32G32B32A32_UINT     },
    { VK_FORMAT_R32G32B32A32_SINT       , a3d::RESOURCE_FORMAT_R32G32B32A32_SINT     },
    { VK_FORMAT_R16G16B16A16_SFLOAT     , a3d::RESOURCE_FORMAT_R16G16B16A16_FLOAT    },
    { VK_FORMAT_R16G16B16A16_UINT       , a3d::RESOURCE_FORMAT_R16G16B16A16_UINT     },
    { VK_FORMAT_R16G16B16A16_SINT       , a3d::RESOURCE_FORMAT_R16G16B16A16_SINT     },
    { VK_FORMAT_R32G32B32_SFLOAT        , a3d::RESOURCE_FORMAT_R32G32B32_FLOAT       },
    { VK_FORMAT_R32G32B32_UINT          , a3d::RESOURCE_FORMAT_R32G32B32_UINT        },
    { VK_FORMAT_R32G32B32_SINT          , a3d::RESOURCE_FORMAT_R32G32B32_SINT        },
    { VK_FORMAT_R32G32_SFLOAT           , a3d::RESOURCE_FORMAT_R32G32_FLOAT          },
    { VK_FORMAT_R32G32_UINT             , a3d::RESOURCE_FORMAT_R32G32_UINT           },
    { VK_FORMAT_R32G32_SINT             , a3d::RESOURCE_FORMAT_R32G32_SINT           },
    { VK_FORMAT_R16G16_SFLOAT           , a3d::RESOURCE_FORMAT_R16G16_FLOAT          },
    { VK_FORMAT_R16G16_UINT             , a3d::RESOURCE_FORMAT_R16G16_UINT           },
    { VK_FORMAT_R16G16_SINT             , a3d::RESOURCE_FORMAT_R16G16_SINT           },
    { VK_FORMAT_R32_SFLOAT              , a3d::RESOURCE_FORMAT_R32_FLOAT             },
    { VK_FORMAT_R32_UINT                , a3d::RESOURCE_FORMAT_R32_UINT              },
    { VK_FORMAT_R32_SINT                , a3d::RESOURCE_FORMAT_R32_SINT              },
    { VK_FORMAT_R16_SFLOAT              , a3d::RESOURCE_FORMAT_R16_FLOAT             },
    { VK_FORMAT_R16_UINT                , a3d::RESOURCE_FORMAT_R16_UINT              },
    { VK_FORMAT_R16_SINT                , a3d::RESOURCE_FORMAT_R16_SINT              },
    { VK_FORMAT_R8G8B8A8_SRGB           , a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM_SRGB   },
    { VK_FORMAT_R8G8B8A8_UNORM          , a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM        },
    { VK_FORMAT_B8G8R8A8_SRGB           , a3d::RESOURCE_FORMAT_B8G8R8A8_UNORM_SRGB   },
    { VK_FORMAT_B8G8R8A8_UNORM          , a3d::RESOURCE_FORMAT_B8G8R8A8_UNORM        },
    { VK_FORMAT_R8G8_UNORM              , a3d::RESOURCE_FORMAT_R8G8_UNORM            },
    { VK_FORMAT_R8_UNORM                , a3d::RESOURCE_FORMAT_R8_UNORM              },
    { VK_FORMAT_D32_SFLOAT              , a3d::RESOURCE_FORMAT_D32_FLOAT             },
    { VK_FORMAT_D24_UNORM_S8_UINT       , a3d::RESOURCE_FORMAT_D24_UNORM_S8_UINT     },
    { VK_FORMAT_D16_UNORM               , a3d::RESOURCE_FORMAT_D16_UNORM             },
    { VK_FORMAT_BC1_RGBA_SRGB_BLOCK     , a3d::RESOURCE_FORMAT_BC1_UNORM_SRGB        },
    { VK_FORMAT_BC1_RGBA_UNORM_BLOCK    , a3d::RESOURCE_FORMAT_BC1_UNORM             },
    { VK_FORMAT_BC2_SRGB_BLOCK          , a3d::RESOURCE_FORMAT_BC2_UNORM_SRGB        },
    { VK_FORMAT_BC2_UNORM_BLOCK         , a3d::RESOURCE_FORMAT_BC2_UNORM             },
    { VK_FORMAT_BC3_SRGB_BLOCK          , a3d::RESOURCE_FORMAT_BC3_UNORM_SRGB        },
    { VK_FORMAT_BC3_UNORM_BLOCK         , a3d::RESOURCE_FORMAT_BC3_UNORM             },
    { VK_FORMAT_BC4_UNORM_BLOCK         , a3d::RESOURCE_FORMAT_BC4_UNORM             },
    { VK_FORMAT_BC4_SNORM_BLOCK         , a3d::RESOURCE_FORMAT_BC4_SNORM             },
    { VK_FORMAT_BC5_UNORM_BLOCK         , a3d::RESOURCE_FORMAT_BC5_UNORM             },
    { VK_FORMAT_BC5_SNORM_BLOCK         , a3d::RESOURCE_FORMAT_BC5_SNORM             },
    { VK_FORMAT_BC6H_UFLOAT_BLOCK       , a3d::RESOURCE_FORMAT_BC6H_UF16             },
    { VK_FORMAT_BC6H_SFLOAT_BLOCK       , a3d::RESOURCE_FORMAT_BC6H_SF16             },
    { VK_FORMAT_BC7_SRGB_BLOCK          , a3d::RESOURCE_FORMAT_BC7_UNORM_SRGB        },
    { VK_FORMAT_BC7_UNORM_BLOCK         , a3d::RESOURCE_FORMAT_BC7_UNORM             },
    { VK_FORMAT_ASTC_4x4_SRGB_BLOCK     , a3d::RESOURCE_FORMAT_ASTC_4X4_UNORM_SRGB   },
    { VK_FORMAT_ASTC_4x4_UNORM_BLOCK    , a3d::RESOURCE_FORMAT_ASTC_4X4_UNORM        },
    { VK_FORMAT_ASTC_5x4_SRGB_BLOCK     , a3d::RESOURCE_FORMAT_ASTC_5X4_UNORM_SRGB   },
    { VK_FORMAT_ASTC_5x4_UNORM_BLOCK    , a3d::RESOURCE_FORMAT_ASTC_5X4_UNORM        },
    { VK_FORMAT_ASTC_5x5_SRGB_BLOCK     , a3d::RESOURCE_FORMAT_ASTC_5X5_UNORM_SRGB   },
    { VK_FORMAT_ASTC_5x5_UNORM_BLOCK    , a3d::RESOURCE_FORMAT_ASTC_5X5_UNORM        },
    { VK_FORMAT_ASTC_6x5_SRGB_BLOCK     , a3d::RESOURCE_FORMAT_ASTC_6X5_UNORM_SRGB   },
    { VK_FORMAT_ASTC_6x5_UNORM_BLOCK    , a3d::RESOURCE_FORMAT_ASTC_6X5_UNORM        },
    { VK_FORMAT_ASTC_6x6_SRGB_BLOCK     , a3d::RESOURCE_FORMAT_ASTC_6X6_UNORM_SRGB   },
    { VK_FORMAT_ASTC_6x6_UNORM_BLOCK    , a3d::RESOURCE_FORMAT_ASTC_6X6_UNORM        },
    { VK_FORMAT_ASTC_8x5_SRGB_BLOCK     , a3d::RESOURCE_FORMAT_ASTC_8X5_UNORM_SRGB   },
    { VK_FORMAT_ASTC_8x5_UNORM_BLOCK    , a3d::RESOURCE_FORMAT_ASTC_8X5_UNORM        },
    { VK_FORMAT_ASTC_8x6_SRGB_BLOCK     , a3d::RESOURCE_FORMAT_ASTC_8X6_UNORM_SRGB   },
    { VK_FORMAT_ASTC_8x6_UNORM_BLOCK    , a3d::RESOURCE_FORMAT_ASTC_8X6_UNORM        },
    { VK_FORMAT_ASTC_8x8_SRGB_BLOCK     , a3d::RESOURCE_FORMAT_ASTC_8X8_UNORM_SRGB   },
    { VK_FORMAT_ASTC_8x8_UNORM_BLOCK    , a3d::RESOURCE_FORMAT_ASTC_8X8_UNORM        },
    { VK_FORMAT_ASTC_10x5_SRGB_BLOCK    , a3d::RESOURCE_FORMAT_ASTC_10X5_UNORM_SRGB  },
    { VK_FORMAT_ASTC_10x5_UNORM_BLOCK   , a3d::RESOURCE_FORMAT_ASTC_10X5_UNORM       },
    { VK_FORMAT_ASTC_10x6_SRGB_BLOCK    , a3d::RESOURCE_FORMAT_ASTC_10X6_UNORM_SRGB  },
    { VK_FORMAT_ASTC_10x6_UNORM_BLOCK   , a3d::RESOURCE_FORMAT_ASTC_10X6_UNORM       },
    { VK_FORMAT_ASTC_10x8_SRGB_BLOCK    , a3d::RESOURCE_FORMAT_ASTC_10X8_UNORM_SRGB  },
    { VK_FORMAT_ASTC_10x8_UNORM_BLOCK   , a3d::RESOURCE_FORMAT_ASTC_10X8_UNORM       },
    { VK_FORMAT_ASTC_10x10_SRGB_BLOCK   , a3d::RESOURCE_FORMAT_ASTC_10X10_UNORM_SRGB },
    { VK_FORMAT_ASTC_10x10_UNORM_BLOCK  , a3d::RESOURCE_FORMAT_ASTC_10X10_UNORM      },
    { VK_FORMAT_ASTC_12x10_SRGB_BLOCK   , a3d::RESOURCE_FORMAT_ASTC_12X10_UNORM_SRGB },
    { VK_FORMAT_ASTC_12x10_UNORM_BLOCK  , a3d::RESOURCE_FORMAT_ASTC_12X10_UNORM      },
    { VK_FORMAT_ASTC_12x12_SRGB_BLOCK   , a3d::RESOURCE_FORMAT_ASTC_12X12_UNORM_SRGB },
    { VK_FORMAT_ASTC_12x12_UNORM_BLOCK  , a3d::RESOURCE_FORMAT_ASTC_12X12_UNORM      },
    { VK_FORMAT_A2B10G10R10_UNORM_PACK32, a3d::RESOURCE_FORMAT_R10G10B10A2_UNORM     },
    { VK_FORMAT_A2B10G10R10_UINT_PACK32 , a3d::RESOURCE_FORMAT_R10G10B10A2_UINT      },
    { VK_FORMAT_B10G11R11_UFLOAT_PACK32 , a3d::RESOURCE_FORMAT_R11G11B10_FLOAT       },
    { VK_FORMAT_UNDEFINED               , a3d::RESOURCE_FORMAT_B16G16R16A16_FLOAT    },
    { VK_FORMAT_A2R10G10B10_UNORM_PACK32, a3d::RESOURCE_FORMAT_B10G10R10A2_UNORM     },
    { VK_FORMAT_A2R10G10B10_UINT_PACK32 ,  a3d::RESOURCE_FORMAT_B10G10R10A2_UINT     },
};

} // namespace /* anonymous */


namespace a3d {

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
VkFormat ToNativeFormat(RESOURCE_FORMAT value)
{ return g_FormatTable[value].NativeFormat; }

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
VkColorSpaceKHR ToNativeColorSpace(RESOURCE_FORMAT value)
{
    switch(value)
    {
    case RESOURCE_FORMAT_R16G16B16A16_FLOAT:
        return VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT;
    }

    return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
VkCompareOp ToNativeCompareOp(COMPARE_OP value)
{
    static const VkCompareOp table[] = {
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
    static const VkImageLayout table[] = {
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
    static const VkDescriptorType table[] = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          // DESCRIPTOR_TYPE_CBV
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,          // DESCRIPTOR_TYPE_SRV_B
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,           // DESCRIPTOR_TYPE_SRV_T
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,          // DESCRIPTOR_TYPE_UAV_B
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,           // DESCRIPTOR_TYPE_UAV_T
        VK_DESCRIPTOR_TYPE_SAMPLER,                 // DESCRIPTOR_TYPE_SMP
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,        // DESCRIPTOR_TYPE_RTV
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,        // DESCRIPTOR_TYPE_DSV
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

    //case a3d::RESOURCE_STATE_STREAM_OUT:
    //    { result = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT; }
    //    break;

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

    case a3d::RESOURCE_STATE_PREDICATION:
        { result = VK_ACCESS_SHADER_WRITE_BIT; }
        break;
    }
    return result;
}

//-------------------------------------------------------------------------------------------------
//      イメージビュータイプに変換します.
//-------------------------------------------------------------------------------------------------
VkImageViewType ToNativeImageViewType(a3d::VIEW_DIMENSION value)
{
    static VkImageViewType table[] = {
        VK_IMAGE_VIEW_TYPE_1D,              // VIEW_DIMENSION_BUFFER
        VK_IMAGE_VIEW_TYPE_1D,              // VIEW_DIMENSION_TEXTURE1D
        VK_IMAGE_VIEW_TYPE_1D_ARRAY,        // VIEW_DIMENSION_TEXTURE1D_ARRAY
        VK_IMAGE_VIEW_TYPE_2D,              // VIEW_DIMENSION_TEXTURE2D
        VK_IMAGE_VIEW_TYPE_2D_ARRAY,        // VIEW_DIMENSION_TEXTURE2D_ARRAY
        VK_IMAGE_VIEW_TYPE_CUBE,            // VIEW_DIMENSION_CUBEMAP
        VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,      // VIEW_DIMENSION_CUBEMAP_ARRAY
        VK_IMAGE_VIEW_TYPE_3D               // VIEW_DIMENSION_TEXTURE3D
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      色空間を変換します.
//-------------------------------------------------------------------------------------------------
VkColorSpaceKHR ToNativeColorSpace(a3d::COLOR_SPACE_TYPE value)
{
    switch(value)
    {
    case COLOR_SPACE_SRGB:
        return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    case COLOR_SPACE_BT709_170M:
        return VK_COLOR_SPACE_BT709_NONLINEAR_EXT;

    case COLOR_SPACE_BT2100_PQ:
        return VK_COLOR_SPACE_HDR10_ST2084_EXT;

    case COLOR_SPACE_BT2100_HLG:
        return VK_COLOR_SPACE_HDR10_HLG_EXT;
    }

    return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
}

//-------------------------------------------------------------------------------------------------
//      メモリ用途フラグに変換します.
//-------------------------------------------------------------------------------------------------
VmaMemoryUsage ToVmaMemoryUsage(a3d::HEAP_TYPE heapType)
{
    switch(heapType)
    {
    case a3d::HEAP_TYPE_DEFAULT:
            return VMA_MEMORY_USAGE_GPU_ONLY;

    case a3d::HEAP_TYPE_UPLOAD:
            return VMA_MEMORY_USAGE_CPU_ONLY;

    case a3d::HEAP_TYPE_READBACK:
            return VMA_MEMORY_USAGE_GPU_TO_CPU;
    }

    return VMA_MEMORY_USAGE_UNKNOWN;
}

#define TABLE_ITEM(var)     { var, STR(var) }

//-------------------------------------------------------------------------------------------------
//      文字列を取得します.
//-------------------------------------------------------------------------------------------------
const char* ToString(VkResult result)
{
    struct Table
    {
        VkResult        value;
        const char*     tag;
    };
    static const Table g_Tables[] = {
        TABLE_ITEM(VK_SUCCESS),
        TABLE_ITEM(VK_NOT_READY),
        TABLE_ITEM(VK_TIMEOUT),
        TABLE_ITEM(VK_EVENT_SET),
        TABLE_ITEM(VK_EVENT_RESET),
        TABLE_ITEM(VK_INCOMPLETE),
        TABLE_ITEM(VK_ERROR_OUT_OF_HOST_MEMORY),
        TABLE_ITEM(VK_ERROR_OUT_OF_DEVICE_MEMORY),
        TABLE_ITEM(VK_ERROR_INITIALIZATION_FAILED),
        TABLE_ITEM(VK_ERROR_DEVICE_LOST),
        TABLE_ITEM(VK_ERROR_MEMORY_MAP_FAILED),
        TABLE_ITEM(VK_ERROR_LAYER_NOT_PRESENT),
        TABLE_ITEM(VK_ERROR_EXTENSION_NOT_PRESENT),
        TABLE_ITEM(VK_ERROR_FEATURE_NOT_PRESENT),
        TABLE_ITEM(VK_ERROR_INCOMPATIBLE_DRIVER),
        TABLE_ITEM(VK_ERROR_TOO_MANY_OBJECTS),
        TABLE_ITEM(VK_ERROR_FORMAT_NOT_SUPPORTED),
        TABLE_ITEM(VK_ERROR_FRAGMENTED_POOL),
        TABLE_ITEM(VK_ERROR_UNKNOWN),
        TABLE_ITEM(VK_ERROR_OUT_OF_POOL_MEMORY),
        TABLE_ITEM(VK_ERROR_INVALID_EXTERNAL_HANDLE),
        TABLE_ITEM(VK_ERROR_FRAGMENTATION),
        TABLE_ITEM(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS),
        TABLE_ITEM(VK_ERROR_SURFACE_LOST_KHR),
        TABLE_ITEM(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR),
        TABLE_ITEM(VK_SUBOPTIMAL_KHR),
        TABLE_ITEM(VK_ERROR_OUT_OF_DATE_KHR),
        TABLE_ITEM(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR),
        TABLE_ITEM(VK_ERROR_VALIDATION_FAILED_EXT),
        TABLE_ITEM(VK_ERROR_INVALID_SHADER_NV),
        TABLE_ITEM(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT),
        TABLE_ITEM(VK_ERROR_NOT_PERMITTED_EXT),
        TABLE_ITEM(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT),
        TABLE_ITEM(VK_THREAD_IDLE_KHR),
        TABLE_ITEM(VK_THREAD_DONE_KHR),
        TABLE_ITEM(VK_OPERATION_DEFERRED_KHR),
        TABLE_ITEM(VK_OPERATION_NOT_DEFERRED_KHR),
        TABLE_ITEM(VK_PIPELINE_COMPILE_REQUIRED_EXT),
        TABLE_ITEM(VK_ERROR_OUT_OF_POOL_MEMORY_KHR),
        TABLE_ITEM(VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR),
        TABLE_ITEM(VK_ERROR_FRAGMENTATION_EXT),
        TABLE_ITEM(VK_ERROR_INVALID_DEVICE_ADDRESS_EXT),
        TABLE_ITEM(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR),
        TABLE_ITEM(VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT),
        TABLE_ITEM(VK_RESULT_MAX_ENUM)
    };

    auto count = _countof(g_Tables);
    for(auto i=0; i<count; ++i)
    {
        if (g_Tables[i].value == result)
        { return g_Tables[i].tag; }
    }

    return "UNKNOWN";
}

//-------------------------------------------------------------------------------------------------
//      文字列を取得します.
//-------------------------------------------------------------------------------------------------
const char* ToString(VkFormat format)
{
    struct Table
    {
        VkFormat    format;
        const char* tag;
    };
    static const Table g_Tables[] = {
        TABLE_ITEM(VK_FORMAT_UNDEFINED), //0,
        TABLE_ITEM(VK_FORMAT_R4G4_UNORM_PACK8), //1,
        TABLE_ITEM(VK_FORMAT_R4G4B4A4_UNORM_PACK16), //2,
        TABLE_ITEM(VK_FORMAT_B4G4R4A4_UNORM_PACK16), //3,
        TABLE_ITEM(VK_FORMAT_R5G6B5_UNORM_PACK16), //4,
        TABLE_ITEM(VK_FORMAT_B5G6R5_UNORM_PACK16), //5,
        TABLE_ITEM(VK_FORMAT_R5G5B5A1_UNORM_PACK16), //6,
        TABLE_ITEM(VK_FORMAT_B5G5R5A1_UNORM_PACK16), //7,
        TABLE_ITEM(VK_FORMAT_A1R5G5B5_UNORM_PACK16), //8,
        TABLE_ITEM(VK_FORMAT_R8_UNORM), //9,
        TABLE_ITEM(VK_FORMAT_R8_SNORM), //10,
        TABLE_ITEM(VK_FORMAT_R8_USCALED), //11,
        TABLE_ITEM(VK_FORMAT_R8_SSCALED), //12,
        TABLE_ITEM(VK_FORMAT_R8_UINT), //13,
        TABLE_ITEM(VK_FORMAT_R8_SINT), //14,
        TABLE_ITEM(VK_FORMAT_R8_SRGB), //15,
        TABLE_ITEM(VK_FORMAT_R8G8_UNORM), //16,
        TABLE_ITEM(VK_FORMAT_R8G8_SNORM), //17,
        TABLE_ITEM(VK_FORMAT_R8G8_USCALED), //18,
        TABLE_ITEM(VK_FORMAT_R8G8_SSCALED), //19,
        TABLE_ITEM(VK_FORMAT_R8G8_UINT), //20,
        TABLE_ITEM(VK_FORMAT_R8G8_SINT), //21,
        TABLE_ITEM(VK_FORMAT_R8G8_SRGB), //22,
        TABLE_ITEM(VK_FORMAT_R8G8B8_UNORM), //23,
        TABLE_ITEM(VK_FORMAT_R8G8B8_SNORM), //24,
        TABLE_ITEM(VK_FORMAT_R8G8B8_USCALED), //25,
        TABLE_ITEM(VK_FORMAT_R8G8B8_SSCALED), //26,
        TABLE_ITEM(VK_FORMAT_R8G8B8_UINT), //27,
        TABLE_ITEM(VK_FORMAT_R8G8B8_SINT), //28,
        TABLE_ITEM(VK_FORMAT_R8G8B8_SRGB), //29,
        TABLE_ITEM(VK_FORMAT_B8G8R8_UNORM), //30,
        TABLE_ITEM(VK_FORMAT_B8G8R8_SNORM), //31,
        TABLE_ITEM(VK_FORMAT_B8G8R8_USCALED), //32,
        TABLE_ITEM(VK_FORMAT_B8G8R8_SSCALED), //33,
        TABLE_ITEM(VK_FORMAT_B8G8R8_UINT), //34,
        TABLE_ITEM(VK_FORMAT_B8G8R8_SINT), //35,
        TABLE_ITEM(VK_FORMAT_B8G8R8_SRGB), //36,
        TABLE_ITEM(VK_FORMAT_R8G8B8A8_UNORM), //37,
        TABLE_ITEM(VK_FORMAT_R8G8B8A8_SNORM), //38,
        TABLE_ITEM(VK_FORMAT_R8G8B8A8_USCALED), //39,
        TABLE_ITEM(VK_FORMAT_R8G8B8A8_SSCALED), //40,
        TABLE_ITEM(VK_FORMAT_R8G8B8A8_UINT), //41,
        TABLE_ITEM(VK_FORMAT_R8G8B8A8_SINT), //42,
        TABLE_ITEM(VK_FORMAT_R8G8B8A8_SRGB), //43,
        TABLE_ITEM(VK_FORMAT_B8G8R8A8_UNORM), //44,
        TABLE_ITEM(VK_FORMAT_B8G8R8A8_SNORM), //45,
        TABLE_ITEM(VK_FORMAT_B8G8R8A8_USCALED), //46,
        TABLE_ITEM(VK_FORMAT_B8G8R8A8_SSCALED), //47,
        TABLE_ITEM(VK_FORMAT_B8G8R8A8_UINT), //48,
        TABLE_ITEM(VK_FORMAT_B8G8R8A8_SINT), //49,
        TABLE_ITEM(VK_FORMAT_B8G8R8A8_SRGB), //50,
        TABLE_ITEM(VK_FORMAT_A8B8G8R8_UNORM_PACK32), //51,
        TABLE_ITEM(VK_FORMAT_A8B8G8R8_SNORM_PACK32), //52,
        TABLE_ITEM(VK_FORMAT_A8B8G8R8_USCALED_PACK32), //53,
        TABLE_ITEM(VK_FORMAT_A8B8G8R8_SSCALED_PACK32), //54,
        TABLE_ITEM(VK_FORMAT_A8B8G8R8_UINT_PACK32), //55,
        TABLE_ITEM(VK_FORMAT_A8B8G8R8_SINT_PACK32), //56,
        TABLE_ITEM(VK_FORMAT_A8B8G8R8_SRGB_PACK32), //57,
        TABLE_ITEM(VK_FORMAT_A2R10G10B10_UNORM_PACK32), //58,
        TABLE_ITEM(VK_FORMAT_A2R10G10B10_SNORM_PACK32), //59,
        TABLE_ITEM(VK_FORMAT_A2R10G10B10_USCALED_PACK32), //60,
        TABLE_ITEM(VK_FORMAT_A2R10G10B10_SSCALED_PACK32), //61,
        TABLE_ITEM(VK_FORMAT_A2R10G10B10_UINT_PACK32), //62,
        TABLE_ITEM(VK_FORMAT_A2R10G10B10_SINT_PACK32), //63,
        TABLE_ITEM(VK_FORMAT_A2B10G10R10_UNORM_PACK32), //64,
        TABLE_ITEM(VK_FORMAT_A2B10G10R10_SNORM_PACK32), //65,
        TABLE_ITEM(VK_FORMAT_A2B10G10R10_USCALED_PACK32), //66,
        TABLE_ITEM(VK_FORMAT_A2B10G10R10_SSCALED_PACK32), //67,
        TABLE_ITEM(VK_FORMAT_A2B10G10R10_UINT_PACK32), //68,
        TABLE_ITEM(VK_FORMAT_A2B10G10R10_SINT_PACK32), //69,
        TABLE_ITEM(VK_FORMAT_R16_UNORM), //70,
        TABLE_ITEM(VK_FORMAT_R16_SNORM), //71,
        TABLE_ITEM(VK_FORMAT_R16_USCALED), //72,
        TABLE_ITEM(VK_FORMAT_R16_SSCALED), //73,
        TABLE_ITEM(VK_FORMAT_R16_UINT), //74,
        TABLE_ITEM(VK_FORMAT_R16_SINT), //75,
        TABLE_ITEM(VK_FORMAT_R16_SFLOAT), //76,
        TABLE_ITEM(VK_FORMAT_R16G16_UNORM), //77,
        TABLE_ITEM(VK_FORMAT_R16G16_SNORM), //78,
        TABLE_ITEM(VK_FORMAT_R16G16_USCALED), //79,
        TABLE_ITEM(VK_FORMAT_R16G16_SSCALED), //80,
        TABLE_ITEM(VK_FORMAT_R16G16_UINT), //81,
        TABLE_ITEM(VK_FORMAT_R16G16_SINT), //82,
        TABLE_ITEM(VK_FORMAT_R16G16_SFLOAT), //83,
        TABLE_ITEM(VK_FORMAT_R16G16B16_UNORM), //84,
        TABLE_ITEM(VK_FORMAT_R16G16B16_SNORM), //85,
        TABLE_ITEM(VK_FORMAT_R16G16B16_USCALED), //86,
        TABLE_ITEM(VK_FORMAT_R16G16B16_SSCALED), //87,
        TABLE_ITEM(VK_FORMAT_R16G16B16_UINT), //88,
        TABLE_ITEM(VK_FORMAT_R16G16B16_SINT), //89,
        TABLE_ITEM(VK_FORMAT_R16G16B16_SFLOAT), //90,
        TABLE_ITEM(VK_FORMAT_R16G16B16A16_UNORM), //91,
        TABLE_ITEM(VK_FORMAT_R16G16B16A16_SNORM), //92,
        TABLE_ITEM(VK_FORMAT_R16G16B16A16_USCALED), //93,
        TABLE_ITEM(VK_FORMAT_R16G16B16A16_SSCALED), //94,
        TABLE_ITEM(VK_FORMAT_R16G16B16A16_UINT), //95,
        TABLE_ITEM(VK_FORMAT_R16G16B16A16_SINT), //96,
        TABLE_ITEM(VK_FORMAT_R16G16B16A16_SFLOAT), //97,
        TABLE_ITEM(VK_FORMAT_R32_UINT), //98,
        TABLE_ITEM(VK_FORMAT_R32_SINT), //99,
        TABLE_ITEM(VK_FORMAT_R32_SFLOAT), //100,
        TABLE_ITEM(VK_FORMAT_R32G32_UINT), //101,
        TABLE_ITEM(VK_FORMAT_R32G32_SINT), //102,
        TABLE_ITEM(VK_FORMAT_R32G32_SFLOAT), //103,
        TABLE_ITEM(VK_FORMAT_R32G32B32_UINT), //104,
        TABLE_ITEM(VK_FORMAT_R32G32B32_SINT), //105,
        TABLE_ITEM(VK_FORMAT_R32G32B32_SFLOAT), //106,
        TABLE_ITEM(VK_FORMAT_R32G32B32A32_UINT), //107,
        TABLE_ITEM(VK_FORMAT_R32G32B32A32_SINT), //108,
        TABLE_ITEM(VK_FORMAT_R32G32B32A32_SFLOAT), //109,
        TABLE_ITEM(VK_FORMAT_R64_UINT), //110,
        TABLE_ITEM(VK_FORMAT_R64_SINT), //111,
        TABLE_ITEM(VK_FORMAT_R64_SFLOAT), //112,
        TABLE_ITEM(VK_FORMAT_R64G64_UINT), //113,
        TABLE_ITEM(VK_FORMAT_R64G64_SINT), //114,
        TABLE_ITEM(VK_FORMAT_R64G64_SFLOAT), //115,
        TABLE_ITEM(VK_FORMAT_R64G64B64_UINT), //116,
        TABLE_ITEM(VK_FORMAT_R64G64B64_SINT), //117,
        TABLE_ITEM(VK_FORMAT_R64G64B64_SFLOAT), //118,
        TABLE_ITEM(VK_FORMAT_R64G64B64A64_UINT), //119,
        TABLE_ITEM(VK_FORMAT_R64G64B64A64_SINT), //120,
        TABLE_ITEM(VK_FORMAT_R64G64B64A64_SFLOAT), //121,
        TABLE_ITEM(VK_FORMAT_B10G11R11_UFLOAT_PACK32), //122,
        TABLE_ITEM(VK_FORMAT_E5B9G9R9_UFLOAT_PACK32), //123,
        TABLE_ITEM(VK_FORMAT_D16_UNORM), //124,
        TABLE_ITEM(VK_FORMAT_X8_D24_UNORM_PACK32), //125,
        TABLE_ITEM(VK_FORMAT_D32_SFLOAT), //126,
        TABLE_ITEM(VK_FORMAT_S8_UINT), //127,
        TABLE_ITEM(VK_FORMAT_D16_UNORM_S8_UINT), //128,
        TABLE_ITEM(VK_FORMAT_D24_UNORM_S8_UINT), //129,
        TABLE_ITEM(VK_FORMAT_D32_SFLOAT_S8_UINT), //130,
        TABLE_ITEM(VK_FORMAT_BC1_RGB_UNORM_BLOCK), //131,
        TABLE_ITEM(VK_FORMAT_BC1_RGB_SRGB_BLOCK), //132,
        TABLE_ITEM(VK_FORMAT_BC1_RGBA_UNORM_BLOCK), //133,
        TABLE_ITEM(VK_FORMAT_BC1_RGBA_SRGB_BLOCK), //134,
        TABLE_ITEM(VK_FORMAT_BC2_UNORM_BLOCK), //135,
        TABLE_ITEM(VK_FORMAT_BC2_SRGB_BLOCK), //136,
        TABLE_ITEM(VK_FORMAT_BC3_UNORM_BLOCK), //137,
        TABLE_ITEM(VK_FORMAT_BC3_SRGB_BLOCK), //138,
        TABLE_ITEM(VK_FORMAT_BC4_UNORM_BLOCK), //139,
        TABLE_ITEM(VK_FORMAT_BC4_SNORM_BLOCK), //140,
        TABLE_ITEM(VK_FORMAT_BC5_UNORM_BLOCK), //141,
        TABLE_ITEM(VK_FORMAT_BC5_SNORM_BLOCK), //142,
        TABLE_ITEM(VK_FORMAT_BC6H_UFLOAT_BLOCK), //143,
        TABLE_ITEM(VK_FORMAT_BC6H_SFLOAT_BLOCK), //144,
        TABLE_ITEM(VK_FORMAT_BC7_UNORM_BLOCK), //145,
        TABLE_ITEM(VK_FORMAT_BC7_SRGB_BLOCK), //146,
        TABLE_ITEM(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK), //147,
        TABLE_ITEM(VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK), //148,
        TABLE_ITEM(VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK), //149,
        TABLE_ITEM(VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK), //150,
        TABLE_ITEM(VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK), //151,
        TABLE_ITEM(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK), //152,
        TABLE_ITEM(VK_FORMAT_EAC_R11_UNORM_BLOCK), //153,
        TABLE_ITEM(VK_FORMAT_EAC_R11_SNORM_BLOCK), //154,
        TABLE_ITEM(VK_FORMAT_EAC_R11G11_UNORM_BLOCK), //155,
        TABLE_ITEM(VK_FORMAT_EAC_R11G11_SNORM_BLOCK), //156,
        TABLE_ITEM(VK_FORMAT_ASTC_4x4_UNORM_BLOCK), //157,
        TABLE_ITEM(VK_FORMAT_ASTC_4x4_SRGB_BLOCK), //158,
        TABLE_ITEM(VK_FORMAT_ASTC_5x4_UNORM_BLOCK), //159,
        TABLE_ITEM(VK_FORMAT_ASTC_5x4_SRGB_BLOCK), //160,
        TABLE_ITEM(VK_FORMAT_ASTC_5x5_UNORM_BLOCK), //161,
        TABLE_ITEM(VK_FORMAT_ASTC_5x5_SRGB_BLOCK), //162,
        TABLE_ITEM(VK_FORMAT_ASTC_6x5_UNORM_BLOCK), //163,
        TABLE_ITEM(VK_FORMAT_ASTC_6x5_SRGB_BLOCK), //164,
        TABLE_ITEM(VK_FORMAT_ASTC_6x6_UNORM_BLOCK), //165,
        TABLE_ITEM(VK_FORMAT_ASTC_6x6_SRGB_BLOCK), //166,
        TABLE_ITEM(VK_FORMAT_ASTC_8x5_UNORM_BLOCK), //167,
        TABLE_ITEM(VK_FORMAT_ASTC_8x5_SRGB_BLOCK), //168,
        TABLE_ITEM(VK_FORMAT_ASTC_8x6_UNORM_BLOCK), //169,
        TABLE_ITEM(VK_FORMAT_ASTC_8x6_SRGB_BLOCK), //170,
        TABLE_ITEM(VK_FORMAT_ASTC_8x8_UNORM_BLOCK), //171,
        TABLE_ITEM(VK_FORMAT_ASTC_8x8_SRGB_BLOCK), //172,
        TABLE_ITEM(VK_FORMAT_ASTC_10x5_UNORM_BLOCK), //173,
        TABLE_ITEM(VK_FORMAT_ASTC_10x5_SRGB_BLOCK), //174,
        TABLE_ITEM(VK_FORMAT_ASTC_10x6_UNORM_BLOCK), //175,
        TABLE_ITEM(VK_FORMAT_ASTC_10x6_SRGB_BLOCK), //176,
        TABLE_ITEM(VK_FORMAT_ASTC_10x8_UNORM_BLOCK), //177,
        TABLE_ITEM(VK_FORMAT_ASTC_10x8_SRGB_BLOCK), //178,
        TABLE_ITEM(VK_FORMAT_ASTC_10x10_UNORM_BLOCK), //179,
        TABLE_ITEM(VK_FORMAT_ASTC_10x10_SRGB_BLOCK), //180,
        TABLE_ITEM(VK_FORMAT_ASTC_12x10_UNORM_BLOCK), //181,
        TABLE_ITEM(VK_FORMAT_ASTC_12x10_SRGB_BLOCK), //182,
        TABLE_ITEM(VK_FORMAT_ASTC_12x12_UNORM_BLOCK), //183,
        TABLE_ITEM(VK_FORMAT_ASTC_12x12_SRGB_BLOCK), //184,
        TABLE_ITEM(VK_FORMAT_G8B8G8R8_422_UNORM), //1000156000,
        TABLE_ITEM(VK_FORMAT_B8G8R8G8_422_UNORM), //1000156001,
        TABLE_ITEM(VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM), //1000156002,
        TABLE_ITEM(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM), //1000156003,
        TABLE_ITEM(VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM), //1000156004,
        TABLE_ITEM(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM), //1000156005,
        TABLE_ITEM(VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM), //1000156006,
        TABLE_ITEM(VK_FORMAT_R10X6_UNORM_PACK16), //1000156007,
        TABLE_ITEM(VK_FORMAT_R10X6G10X6_UNORM_2PACK16), //1000156008,
        TABLE_ITEM(VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16), //1000156009,
        TABLE_ITEM(VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16), //1000156010,
        TABLE_ITEM(VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16), //1000156011,
        TABLE_ITEM(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16), //1000156012,
        TABLE_ITEM(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16), //1000156013,
        TABLE_ITEM(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16), //1000156014,
        TABLE_ITEM(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16), //1000156015,
        TABLE_ITEM(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16), //1000156016,
        TABLE_ITEM(VK_FORMAT_R12X4_UNORM_PACK16), //1000156017,
        TABLE_ITEM(VK_FORMAT_R12X4G12X4_UNORM_2PACK16), //1000156018,
        TABLE_ITEM(VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16), //1000156019,
        TABLE_ITEM(VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16), //1000156020,
        TABLE_ITEM(VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16), //1000156021,
        TABLE_ITEM(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16), //1000156022,
        TABLE_ITEM(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16), //1000156023,
        TABLE_ITEM(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16), //1000156024,
        TABLE_ITEM(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16), //1000156025,
        TABLE_ITEM(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16), //1000156026,
        TABLE_ITEM(VK_FORMAT_G16B16G16R16_422_UNORM), //1000156027,
        TABLE_ITEM(VK_FORMAT_B16G16R16G16_422_UNORM), //1000156028,
        TABLE_ITEM(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM), //1000156029,
        TABLE_ITEM(VK_FORMAT_G16_B16R16_2PLANE_420_UNORM), //1000156030,
        TABLE_ITEM(VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM), //1000156031,
        TABLE_ITEM(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM), //1000156032,
        TABLE_ITEM(VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM), //1000156033,
        TABLE_ITEM(VK_FORMAT_G8_B8R8_2PLANE_444_UNORM), //1000330000,
        TABLE_ITEM(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16), //1000330001,
        TABLE_ITEM(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16), //1000330002,
        TABLE_ITEM(VK_FORMAT_G16_B16R16_2PLANE_444_UNORM), //1000330003,
        TABLE_ITEM(VK_FORMAT_A4R4G4B4_UNORM_PACK16), //1000340000,
        TABLE_ITEM(VK_FORMAT_A4B4G4R4_UNORM_PACK16), //1000340001,
        TABLE_ITEM(VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK), //1000066000,
        TABLE_ITEM(VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK), //1000066001,
        TABLE_ITEM(VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK), //1000066002,
        TABLE_ITEM(VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK), //1000066003,
        TABLE_ITEM(VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK), //1000066004,
        TABLE_ITEM(VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK), //1000066005,
        TABLE_ITEM(VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK), //1000066006,
        TABLE_ITEM(VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK), //1000066007,
        TABLE_ITEM(VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK), //1000066008,
        TABLE_ITEM(VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK), //1000066009,
        TABLE_ITEM(VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK), //1000066010,
        TABLE_ITEM(VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK), //1000066011,
        TABLE_ITEM(VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK), //1000066012,
        TABLE_ITEM(VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK), //1000066013,
        TABLE_ITEM(VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG), //1000054000,
        TABLE_ITEM(VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG), //1000054001,
        TABLE_ITEM(VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG), //1000054002,
        TABLE_ITEM(VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG), //1000054003,
        TABLE_ITEM(VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG), //1000054004,
        TABLE_ITEM(VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG), //1000054005,
        TABLE_ITEM(VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG), //1000054006,
        TABLE_ITEM(VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG), //1000054007,
        TABLE_ITEM(VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT), //TABLE_ITEM(VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK,
        TABLE_ITEM(VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT), //TABLE_ITEM(VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK,
        TABLE_ITEM(VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT), //TABLE_ITEM(VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK,
        TABLE_ITEM(VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT), //TABLE_ITEM(VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK,
        TABLE_ITEM(VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT), //TABLE_ITEM(VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK,
        TABLE_ITEM(VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT), //TABLE_ITEM(VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK,
        TABLE_ITEM(VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT), //TABLE_ITEM(VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK,
        TABLE_ITEM(VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT), //TABLE_ITEM(VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK,
        TABLE_ITEM(VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT), //TABLE_ITEM(VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK,
        TABLE_ITEM(VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT), //TABLE_ITEM(VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK,
        TABLE_ITEM(VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT), //TABLE_ITEM(VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK,
        TABLE_ITEM(VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT), //TABLE_ITEM(VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK,
        TABLE_ITEM(VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT), //TABLE_ITEM(VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK,
        TABLE_ITEM(VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT), //TABLE_ITEM(VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK,
        TABLE_ITEM(VK_FORMAT_G8B8G8R8_422_UNORM_KHR), //TABLE_ITEM(VK_FORMAT_G8B8G8R8_422_UNORM,
        TABLE_ITEM(VK_FORMAT_B8G8R8G8_422_UNORM_KHR), //TABLE_ITEM(VK_FORMAT_B8G8R8G8_422_UNORM,
        TABLE_ITEM(VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR), //TABLE_ITEM(VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
        TABLE_ITEM(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR), //TABLE_ITEM(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
        TABLE_ITEM(VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR), //TABLE_ITEM(VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
        TABLE_ITEM(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR), //TABLE_ITEM(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
        TABLE_ITEM(VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR), //TABLE_ITEM(VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
        TABLE_ITEM(VK_FORMAT_R10X6_UNORM_PACK16_KHR), //TABLE_ITEM(VK_FORMAT_R10X6_UNORM_PACK16,
        TABLE_ITEM(VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR), //TABLE_ITEM(VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
        TABLE_ITEM(VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR), //TABLE_ITEM(VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
        TABLE_ITEM(VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR), //TABLE_ITEM(VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
        TABLE_ITEM(VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR), //TABLE_ITEM(VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
        TABLE_ITEM(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR), //TABLE_ITEM(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
        TABLE_ITEM(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR), //TABLE_ITEM(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
        TABLE_ITEM(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR), //TABLE_ITEM(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
        TABLE_ITEM(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR), //TABLE_ITEM(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
        TABLE_ITEM(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR), //TABLE_ITEM(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
        TABLE_ITEM(VK_FORMAT_R12X4_UNORM_PACK16_KHR), //TABLE_ITEM(VK_FORMAT_R12X4_UNORM_PACK16,
        TABLE_ITEM(VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR), //TABLE_ITEM(VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
        TABLE_ITEM(VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR), //TABLE_ITEM(VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
        TABLE_ITEM(VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR), //TABLE_ITEM(VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
        TABLE_ITEM(VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR), //TABLE_ITEM(VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
        TABLE_ITEM(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR), //TABLE_ITEM(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
        TABLE_ITEM(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR), //TABLE_ITEM(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
        TABLE_ITEM(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR), //TABLE_ITEM(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
        TABLE_ITEM(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR), //TABLE_ITEM(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
        TABLE_ITEM(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR), //TABLE_ITEM(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
        TABLE_ITEM(VK_FORMAT_G16B16G16R16_422_UNORM_KHR), //TABLE_ITEM(VK_FORMAT_G16B16G16R16_422_UNORM,
        TABLE_ITEM(VK_FORMAT_B16G16R16G16_422_UNORM_KHR), //TABLE_ITEM(VK_FORMAT_B16G16R16G16_422_UNORM,
        TABLE_ITEM(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR), //TABLE_ITEM(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
        TABLE_ITEM(VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR), //TABLE_ITEM(VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
        TABLE_ITEM(VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR), //TABLE_ITEM(VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
        TABLE_ITEM(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR), //TABLE_ITEM(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
        TABLE_ITEM(VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR), //TABLE_ITEM(VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
        TABLE_ITEM(VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT), //TABLE_ITEM(VK_FORMAT_G8_B8R8_2PLANE_444_UNORM,
        TABLE_ITEM(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT), //TABLE_ITEM(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16,
        TABLE_ITEM(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT), //TABLE_ITEM(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16,
        TABLE_ITEM(VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT), //TABLE_ITEM(VK_FORMAT_G16_B16R16_2PLANE_444_UNORM,
        TABLE_ITEM(VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT), //TABLE_ITEM(VK_FORMAT_A4R4G4B4_UNORM_PACK16,
        TABLE_ITEM(VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT), //TABLE_ITEM(VK_FORMAT_A4B4G4R4_UNORM_PACK16,
        TABLE_ITEM(VK_FORMAT_MAX_ENUM), //0x7FFFFFFF
    };

    auto count = _countof(g_Tables);
    for(auto i=0; i<count; ++i)
    {
        if (g_Tables[i].format == format)
        { return g_Tables[i].tag; }
    }

    return "UNKNOWN";
}

//-------------------------------------------------------------------------------------------------
//      文字列を取得します.
//-------------------------------------------------------------------------------------------------
const char* ToString(VkColorSpaceKHR colorSpace)
{
    struct Table
    {
        VkColorSpaceKHR colorSpace;
        const char*     tag;
    };

    static const Table g_Tables[] = {
        TABLE_ITEM(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR), //0,
        TABLE_ITEM(VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT), //1000104001,
        TABLE_ITEM(VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT), //1000104002,
        TABLE_ITEM(VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT), //1000104003,
        TABLE_ITEM(VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT), //1000104004,
        TABLE_ITEM(VK_COLOR_SPACE_BT709_LINEAR_EXT), //1000104005,
        TABLE_ITEM(VK_COLOR_SPACE_BT709_NONLINEAR_EXT), //1000104006,
        TABLE_ITEM(VK_COLOR_SPACE_BT2020_LINEAR_EXT), //1000104007,
        TABLE_ITEM(VK_COLOR_SPACE_HDR10_ST2084_EXT), //1000104008,
        TABLE_ITEM(VK_COLOR_SPACE_DOLBYVISION_EXT), //1000104009,
        TABLE_ITEM(VK_COLOR_SPACE_HDR10_HLG_EXT), //1000104010,
        TABLE_ITEM(VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT), //1000104011,
        TABLE_ITEM(VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT), //1000104012,
        TABLE_ITEM(VK_COLOR_SPACE_PASS_THROUGH_EXT), //1000104013,
        TABLE_ITEM(VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT), //1000104014,
        TABLE_ITEM(VK_COLOR_SPACE_DISPLAY_NATIVE_AMD), //1000213000,
        TABLE_ITEM(VK_COLORSPACE_SRGB_NONLINEAR_KHR), //VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        TABLE_ITEM(VK_COLOR_SPACE_DCI_P3_LINEAR_EXT), //VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT,
        TABLE_ITEM(VK_COLOR_SPACE_MAX_ENUM_KHR), //0x7FFFFFFF
    };

    auto count = _countof(g_Tables);
    for(auto i=0; i<count; ++i)
    {
        if (g_Tables[i].colorSpace == colorSpace)
        { return g_Tables[i].tag; }
    }

    return "UNKNOWN";
}

//-------------------------------------------------------------------------------------------------
//      シェーダステージフラグに変換します.
//-------------------------------------------------------------------------------------------------
VkShaderStageFlagBits ToNativeShaderStageFlag(a3d::SHADER_STAGE stage)
{
    uint32_t result = 0;
    switch(stage)
    {
    case a3d::SHADER_STAGE_VS:
        result |= VK_SHADER_STAGE_VERTEX_BIT;
        break;

    case a3d::SHADER_STAGE_DS:
        result |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        break;

    case a3d::SHADER_STAGE_HS:
        result |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        break;

    case a3d::SHADER_STAGE_PS:
        result |= VK_SHADER_STAGE_FRAGMENT_BIT;
        break;

    case a3d::SHADER_STAGE_CS:
        result |= VK_SHADER_STAGE_COMPUTE_BIT;
        break;

    case a3d::SHADER_STAGE_AS:
        result |= VK_SHADER_STAGE_TASK_BIT_NV;
        break;

    case a3d::SHADER_STAGE_MS:
        result |= VK_SHADER_STAGE_MESH_BIT_NV;
        break;

    case a3d::SHADER_STAGE_RAYGEN:
        result |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        break;

    case a3d::SHADER_STAGE_MISS:
        result |= VK_SHADER_STAGE_MISS_BIT_KHR;
        break;

    case a3d::SHADER_STAGE_INTERSECTION:
        result |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        break;

    case a3d::SHADER_STAGE_CLOSEST_HIT:
        result |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        break;

    case a3d::SHADER_STAGE_ANY_HIT:
        result |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        break;

    case a3d::SHADER_STAGE_CALLABLE:
        result |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;
        break;
    }
    return VkShaderStageFlagBits(result);
}


} // namespace a3d
