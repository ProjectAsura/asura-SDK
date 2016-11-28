//-------------------------------------------------------------------------------------------------
// File : a3dUtil.cpp
// Desc : Utility Module.
// Copyright(c) Project Asura. All right reseved.
//-------------------------------------------------------------------------------------------------

namespace /* anonymous */ {

///////////////////////////////////////////////////////////////////////////////////////////////////
// FormatConvertTable structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct FormatConvertTable
{
    DXGI_FORMAT             NativeFormat;       //!< ネイティブ形式です.
    DXGI_FORMAT             TypelessFormat;     //!< タイプレス形式です.
    float                   BitPerPixel;        //!< ビットレートです.
    a3d::RESOURCE_FORMAT    Format;             //!< A3D形式です.
};

FormatConvertTable g_FormatTable[] = {
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 0.0f     , a3d::RESOURCE_FORMAT_UNKNOWN               },
    { DXGI_FORMAT_R32G32B32A32_FLOAT        , DXGI_FORMAT_R32G32B32A32_TYPELESS     , 128.0f   , a3d::RESOURCE_FORMAT_R32G32B32A32_FLOAT    },
    { DXGI_FORMAT_R32G32B32A32_UINT         , DXGI_FORMAT_R32G32B32A32_TYPELESS     , 128.0f   , a3d::RESOURCE_FORMAT_R32G32B32A32_UINT     },
    { DXGI_FORMAT_R32G32B32A32_SINT         , DXGI_FORMAT_R32G32B32A32_TYPELESS     , 128.0f   , a3d::RESOURCE_FORMAT_R32G32B32A32_SINT     },
    { DXGI_FORMAT_R16G16B16A16_FLOAT        , DXGI_FORMAT_R16G16B16A16_TYPELESS     , 64.0f    , a3d::RESOURCE_FORMAT_R16G16B16A16_FLOAT    },
    { DXGI_FORMAT_R16G16B16A16_UINT         , DXGI_FORMAT_R16G16B16A16_TYPELESS     , 64.0f    , a3d::RESOURCE_FORMAT_R16G16B16A16_UINT     },
    { DXGI_FORMAT_R16G16B16A16_SINT         , DXGI_FORMAT_R16G16B16A16_TYPELESS     , 64.0f    , a3d::RESOURCE_FORMAT_R16G16B16A16_SINT     },
    { DXGI_FORMAT_R32G32B32_FLOAT           , DXGI_FORMAT_R32G32B32_TYPELESS        , 96.0f    , a3d::RESOURCE_FORMAT_R32G32B32_FLOAT       },
    { DXGI_FORMAT_R32G32B32_UINT            , DXGI_FORMAT_R32G32B32_TYPELESS        , 96.0f    , a3d::RESOURCE_FORMAT_R32G32B32_UINT        },
    { DXGI_FORMAT_R32G32B32_SINT            , DXGI_FORMAT_R32G32B32_TYPELESS        , 96.0f    , a3d::RESOURCE_FORMAT_R32G32B32_SINT        },
    { DXGI_FORMAT_R32G32_FLOAT              , DXGI_FORMAT_R32G32_TYPELESS           , 64.0f    , a3d::RESOURCE_FORMAT_R32G32_FLOAT          },
    { DXGI_FORMAT_R32G32_UINT               , DXGI_FORMAT_R32G32_TYPELESS           , 64.0f    , a3d::RESOURCE_FORMAT_R32G32_UINT           },
    { DXGI_FORMAT_R32G32_SINT               , DXGI_FORMAT_R32G32_TYPELESS           , 64.0f    , a3d::RESOURCE_FORMAT_R32G32_SINT           },
    { DXGI_FORMAT_R16G16_FLOAT              , DXGI_FORMAT_R16G16_TYPELESS           , 32.0f    , a3d::RESOURCE_FORMAT_R16G16_FLOAT          },
    { DXGI_FORMAT_R16G16_UINT               , DXGI_FORMAT_R16G16_TYPELESS           , 32.0f    , a3d::RESOURCE_FORMAT_R16G16_UINT           },
    { DXGI_FORMAT_R16G16_SINT               , DXGI_FORMAT_R16G16_TYPELESS           , 32.0f    , a3d::RESOURCE_FORMAT_R16G16_SINT           },
    { DXGI_FORMAT_R32_FLOAT                 , DXGI_FORMAT_R32_TYPELESS              , 32.0f    , a3d::RESOURCE_FORMAT_R32_FLOAT             },
    { DXGI_FORMAT_R32_UINT                  , DXGI_FORMAT_R32_TYPELESS              , 32.0f    , a3d::RESOURCE_FORMAT_R32_UINT              },
    { DXGI_FORMAT_R32_SINT                  , DXGI_FORMAT_R32_TYPELESS              , 32.0f    , a3d::RESOURCE_FORMAT_R32_SINT              },
    { DXGI_FORMAT_R16_FLOAT                 , DXGI_FORMAT_R16_TYPELESS              , 16.0f    , a3d::RESOURCE_FORMAT_R16_FLOAT             },
    { DXGI_FORMAT_R16_UINT                  , DXGI_FORMAT_R16_TYPELESS              , 16.0f    , a3d::RESOURCE_FORMAT_R16_UINT              },
    { DXGI_FORMAT_R16_SINT                  , DXGI_FORMAT_R16_TYPELESS              , 16.0f    , a3d::RESOURCE_FORMAT_R16_SINT              },
    { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB       , DXGI_FORMAT_R8G8B8A8_TYPELESS         , 32.0f    , a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM_SRGB   },
    { DXGI_FORMAT_R8G8B8A8_UNORM            , DXGI_FORMAT_R8G8B8A8_TYPELESS         , 32.0f    , a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM        },
    { DXGI_FORMAT_B8G8R8A8_UNORM_SRGB       , DXGI_FORMAT_B8G8R8A8_TYPELESS         , 32.0f    , a3d::RESOURCE_FORMAT_B8G8R8A8_UNORM_SRGB   },
    { DXGI_FORMAT_B8G8R8A8_UNORM            , DXGI_FORMAT_B8G8R8A8_TYPELESS         , 32.0f    , a3d::RESOURCE_FORMAT_B8G8R8A8_UNORM        },
    { DXGI_FORMAT_R8G8_UNORM                , DXGI_FORMAT_R8G8_TYPELESS             , 16.0f    , a3d::RESOURCE_FORMAT_R8G8_UNORM            },
    { DXGI_FORMAT_R8_UNORM                  , DXGI_FORMAT_R8_TYPELESS               , 8.0f     , a3d::RESOURCE_FORMAT_R8_UNORM              },
    { DXGI_FORMAT_D32_FLOAT                 , DXGI_FORMAT_R32_TYPELESS              , 32.0f    , a3d::RESOURCE_FORMAT_D32_FLOAT             },
    { DXGI_FORMAT_D24_UNORM_S8_UINT         , DXGI_FORMAT_R24_UNORM_X8_TYPELESS     , 32.0f    , a3d::RESOURCE_FORMAT_D24_UNORM_S8_UINT     },
    { DXGI_FORMAT_D16_UNORM                 , DXGI_FORMAT_R16_TYPELESS              , 16.0f    , a3d::RESOURCE_FORMAT_D16_UNORM             },
    { DXGI_FORMAT_BC1_UNORM_SRGB            , DXGI_FORMAT_BC1_TYPELESS              , 4.0f     , a3d::RESOURCE_FORMAT_BC1_UNORM_SRGB        },
    { DXGI_FORMAT_BC1_UNORM                 , DXGI_FORMAT_BC1_TYPELESS              , 4.0f     , a3d::RESOURCE_FORMAT_BC1_UNORM             },
    { DXGI_FORMAT_BC2_UNORM_SRGB            , DXGI_FORMAT_BC2_TYPELESS              , 8.0f     , a3d::RESOURCE_FORMAT_BC2_UNORM_SRGB        },
    { DXGI_FORMAT_BC2_UNORM                 , DXGI_FORMAT_BC2_TYPELESS              , 8.0f     , a3d::RESOURCE_FORMAT_BC2_UNORM             },
    { DXGI_FORMAT_BC3_UNORM_SRGB            , DXGI_FORMAT_BC3_TYPELESS              , 8.0f     , a3d::RESOURCE_FORMAT_BC3_UNORM_SRGB        },
    { DXGI_FORMAT_BC3_UNORM                 , DXGI_FORMAT_BC3_TYPELESS              , 8.0f     , a3d::RESOURCE_FORMAT_BC3_UNORM             },
    { DXGI_FORMAT_BC4_UNORM                 , DXGI_FORMAT_BC4_TYPELESS              , 4.0f     , a3d::RESOURCE_FORMAT_BC4_UNORM             },
    { DXGI_FORMAT_BC4_SNORM                 , DXGI_FORMAT_BC4_TYPELESS              , 4.0f     , a3d::RESOURCE_FORMAT_BC4_SNORM             },
    { DXGI_FORMAT_BC5_UNORM                 , DXGI_FORMAT_BC5_TYPELESS              , 8.0f     , a3d::RESOURCE_FORMAT_BC5_UNORM             },
    { DXGI_FORMAT_BC5_SNORM                 , DXGI_FORMAT_BC5_TYPELESS              , 8.0f     , a3d::RESOURCE_FORMAT_BC5_SNORM             },
    { DXGI_FORMAT_BC6H_UF16                 , DXGI_FORMAT_BC6H_TYPELESS             , 8.0f     , a3d::RESOURCE_FORMAT_BC6H_UF16             },
    { DXGI_FORMAT_BC6H_SF16                 , DXGI_FORMAT_BC6H_TYPELESS             , 8.0f     , a3d::RESOURCE_FORMAT_BC6H_SF16             },
    { DXGI_FORMAT_BC7_UNORM_SRGB            , DXGI_FORMAT_BC7_TYPELESS              , 8.0f     , a3d::RESOURCE_FORMAT_BC7_UNORM_SRGB        },
    { DXGI_FORMAT_BC7_UNORM                 , DXGI_FORMAT_BC7_TYPELESS              , 8.0f     , a3d::RESOURCE_FORMAT_BC7_UNORN             },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 8.0f     , a3d::RESOURCE_FORMAT_ASTC_4X4_UNORM_SRGB   },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 8.0f     , a3d::RESOURCE_FORMAT_ASTC_4X4_UNORM        },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 6.4f     , a3d::RESOURCE_FORMAT_ASTC_5X4_UNORM_SRGB   },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 5.12f    , a3d::RESOURCE_FORMAT_ASTC_5X5_UNORM        },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 5.12f    , a3d::RESOURCE_FORMAT_ASTC_5X5_UNORM        },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 4.27f    , a3d::RESOURCE_FORMAT_ASTC_6X5_UNORM_SRGB   },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 4.27f    , a3d::RESOURCE_FORMAT_ASTC_6X5_UNORM        },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 3.56f    , a3d::RESOURCE_FORMAT_ASTC_6X6_UNORM_SRGB   },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 3.56f    , a3d::RESOURCE_FORMAT_ASTC_6X6_UNORM        },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 3.20f    , a3d::RESOURCE_FORMAT_ASTC_8X5_UNORM_SRGB   },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 3.20f    , a3d::RESOURCE_FORMAT_ASTC_8X5_UNORM        },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 2.67f    , a3d::RESOURCE_FORMAT_ASTC_8X6_UNORM_SRGB   },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 2.67f    , a3d::RESOURCE_FORMAT_ASTC_8X6_UNORM        },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 2.0f     , a3d::RESOURCE_FORMAT_ASTC_8X8_UNORM_SRGB   },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 2.0f     , a3d::RESOURCE_FORMAT_ASTC_8X8_UNORM        },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 2.56f    , a3d::RESOURCE_FORMAT_ASTC_10X5_UNORM_SRGB  },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 2.56f    , a3d::RESOURCE_FORMAT_ASTC_10X5_UNORM       },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 2.13f    , a3d::RESOURCE_FORMAT_ASTC_10X6_UNORM_SRGB  },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 2.13f    , a3d::RESOURCE_FORMAT_ASTC_10X6_UNORM       },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 1.60f    , a3d::RESOURCE_FORMAT_ASTC_10X8_UNORM_SRGB  },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 1.60f    , a3d::RESOURCE_FORMAT_ASTC_10X8_UNORM       },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 1.28f    , a3d::RESOURCE_FORMAT_ASTC_10X10_UNORM_SRGB },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 1.28f    , a3d::RESOURCE_FORMAT_ASTC_10X10_UNORM      },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 1.07f    , a3d::RESOURCE_FORMAT_ASTC_12X10_UNORM_SRGB },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 1.07f    , a3d::RESOURCE_FORMAT_ASTC_12X10_UNORM      },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 0.89f    , a3d::RESOURCE_FORMAT_ASTC_12X12_UNORM_SRGB },
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 0.89f    , a3d::RESOURCE_FORMAT_ASTC_12X12_UNORM      },
};

} // namespace /* anonymous */

namespace a3d {

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
DXGI_FORMAT ToNativeFormat(RESOURCE_FORMAT format)
{ return g_FormatTable[format].NativeFormat; }

//-------------------------------------------------------------------------------------------------
//      タイプレスのネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
DXGI_FORMAT ToNativeTypelessFormat(RESOURCE_FORMAT format, bool isStencilFormat)
{
    if (isStencilFormat && format == RESOURCE_FORMAT_D24_UNORM_S8_UINT)
    { return DXGI_FORMAT_X24_TYPELESS_G8_UINT; }

    return g_FormatTable[format].TypelessFormat;
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式からA3D形式に変換します.
//-------------------------------------------------------------------------------------------------
RESOURCE_FORMAT ToWrapFormat(DXGI_FORMAT format)
{
    auto size = sizeof(g_FormatTable) / sizeof(g_FormatTable[0]);
    for(size_t i=0; i<size; ++i)
    {
        if (g_FormatTable[i].NativeFormat == format)
        { return g_FormatTable[i].Format; }
    }

    return RESOURCE_FORMAT_UNKNOWN;
}

//-------------------------------------------------------------------------------------------------
//      リソースフォーマットのビット数に変換します.
//-------------------------------------------------------------------------------------------------
float ToBits(RESOURCE_FORMAT format)
{ return g_FormatTable[format].BitPerPixel; }

//-------------------------------------------------------------------------------------------------
//      リソースフォーマットのバイト数に変換します.
//-------------------------------------------------------------------------------------------------
float ToByte(RESOURCE_FORMAT format)
{ return ToBits(format) / 8.0f; }

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D11_COMPARISON_FUNC ToNativeComparisonFunc(COMPARE_OP operaton)
{
    static D3D11_COMPARISON_FUNC table[] = {
        D3D11_COMPARISON_NEVER,
        D3D11_COMPARISON_LESS,
        D3D11_COMPARISON_EQUAL,
        D3D11_COMPARISON_LESS_EQUAL,
        D3D11_COMPARISON_GREATER,
        D3D11_COMPARISON_GREATER_EQUAL,
        D3D11_COMPARISON_ALWAYS,
    };

    return table[operaton];
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D_PRIMITIVE_TOPOLOGY ToNativePrimitive(PRIMITIVE_TOPOLOGY topology, uint32_t controlCount)
{
    if ( topology != PRIMITIVE_TOPOLOGY_PATCHLIST )
    {
        static D3D_PRIMITIVE_TOPOLOGY table[] = {
            D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
            D3D_PRIMITIVE_TOPOLOGY_LINELIST,
            D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
            D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ,
            D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ,
        };

        return table[topology];
    }
    else
    {
        static D3D_PRIMITIVE_TOPOLOGY table[] = {
            D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
            D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST,
            D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST,
        };

        return table[controlCount];
    }
}


//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D11_RTV_DIMENSION ToNativeRTVDimension(VIEW_DIMENSION value)
{
    static D3D11_RTV_DIMENSION table[] = {
        D3D11_RTV_DIMENSION_BUFFER,
        D3D11_RTV_DIMENSION_TEXTURE1D,
        D3D11_RTV_DIMENSION_TEXTURE1DARRAY,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        D3D11_RTV_DIMENSION_TEXTURE2DARRAY,
        D3D11_RTV_DIMENSION_TEXTURE2DARRAY,
        D3D11_RTV_DIMENSION_TEXTURE2DARRAY,
        D3D11_RTV_DIMENSION_TEXTURE3D
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D11_DSV_DIMENSION ToNativeDSVDimension(VIEW_DIMENSION value)
{
    static D3D11_DSV_DIMENSION table[] = {
        D3D11_DSV_DIMENSION_UNKNOWN,
        D3D11_DSV_DIMENSION_TEXTURE1D,
        D3D11_DSV_DIMENSION_TEXTURE1DARRAY,
        D3D11_DSV_DIMENSION_TEXTURE2D,
        D3D11_DSV_DIMENSION_TEXTURE2DARRAY,
        D3D11_DSV_DIMENSION_TEXTURE2DARRAY,
        D3D11_DSV_DIMENSION_TEXTURE2DARRAY,
        D3D11_DSV_DIMENSION_TEXTURE2DARRAY
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D11_SRV_DIMENSION ToNativeSRVDimension(VIEW_DIMENSION value)
{
    static D3D11_SRV_DIMENSION table[] = {
        D3D11_SRV_DIMENSION_BUFFER,
        D3D11_SRV_DIMENSION_TEXTURE1D,
        D3D11_SRV_DIMENSION_TEXTURE1DARRAY,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
        D3D11_SRV_DIMENSION_TEXTURECUBE,
        D3D11_SRV_DIMENSION_TEXTURECUBEARRAY,
        D3D11_SRV_DIMENSION_TEXTURE3D
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D11_UAV_DIMENSION ToNativeUAVDimension(VIEW_DIMENSION value)
{
    static D3D11_UAV_DIMENSION table[] = {
        D3D11_UAV_DIMENSION_BUFFER,
        D3D11_UAV_DIMENSION_TEXTURE1D,
        D3D11_UAV_DIMENSION_TEXTURE1DARRAY,
        D3D11_UAV_DIMENSION_TEXTURE2D,
        D3D11_UAV_DIMENSION_TEXTURE2DARRAY,
        D3D11_UAV_DIMENSION_TEXTURE2DARRAY,
        D3D11_UAV_DIMENSION_TEXTURE2DARRAY,
        D3D11_UAV_DIMENSION_TEXTURE3D
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      使用用途を取得します.
//-------------------------------------------------------------------------------------------------
D3D11_USAGE ToNativeUsage(HEAP_TYPE type)
{
    static D3D11_USAGE table[] = {
        D3D11_USAGE_DEFAULT,
        D3D11_USAGE_DYNAMIC,
        D3D11_USAGE_STAGING
    };

    return table[type];
}

//-------------------------------------------------------------------------------------------------
//      バインドフラグを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t ToNativeBindFlags(uint32_t value)
{
    uint32_t result = 0;
    if (value & RESOURCE_USAGE_COLOR_TARGET)
    { result |= D3D11_BIND_RENDER_TARGET; }
    if (value & RESOURCE_USAGE_DEPTH_TARGET)
    { result |= D3D11_BIND_DEPTH_STENCIL; }
    if (value & RESOURCE_USAGE_UNORDERD_ACCESS)
    { result |= D3D11_BIND_UNORDERED_ACCESS; }
    if (value & RESOURCE_USAGE_INDEX_BUFFER)
    { result |= D3D11_BIND_INDEX_BUFFER; }
    if (value & RESOURCE_USAGE_VERTEX_BUFFER)
    { result |= D3D11_BIND_VERTEX_BUFFER; }
    if (value & RESOURCE_USAGE_CONSTANT_BUFFER)
    { result |= D3D11_BIND_CONSTANT_BUFFER; }
    if (value & RESOURCE_USAGE_SHADER_RESOURCE)
    { result |= D3D11_BIND_SHADER_RESOURCE; }

    return result;
}

//-------------------------------------------------------------------------------------------------
//      CPUアクセスフラグを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t ToNativeCpuAccessFlags(HEAP_TYPE type, CPU_PAGE_PROPERTY prop)
{
    uint32_t result = 0;

    switch(type)
    {
    case HEAP_TYPE_DEFAULT:
        {
            if (prop == CPU_PAGE_PROPERTY_DEFAULT)
            {
                result |= D3D11_CPU_ACCESS_READ;
                result |= D3D11_CPU_ACCESS_WRITE;
            }
            else if (prop == CPU_PAGE_PROPERTY_NOT_AVAILABLE)
            {
                /* DO_NOTHING */
            }
            else if (prop == CPU_PAGE_PROPERTY_WRITE_COMBINE)
            {
                result |= D3D11_CPU_ACCESS_WRITE;
            }
            else if (prop == CPU_PAGE_PROPERTY_WRITE_BACK)
            {
                result |= D3D11_CPU_ACCESS_WRITE;
            }
        }
        break;

    case HEAP_TYPE_UPLOAD:
        {
            if (prop == CPU_PAGE_PROPERTY_DEFAULT)
            {
                result |= D3D11_CPU_ACCESS_WRITE;
            }
            else if (prop == CPU_PAGE_PROPERTY_NOT_AVAILABLE)
            {
                /* DO_NOTHING */
            }
            else if (prop == CPU_PAGE_PROPERTY_WRITE_COMBINE)
            {
                result |= D3D11_CPU_ACCESS_WRITE;
            }
            else if (prop == CPU_PAGE_PROPERTY_WRITE_BACK)
            {
                result |= D3D11_CPU_ACCESS_WRITE;
            }
        }
        break;

    case HEAP_TYPE_READBACK:
        {
            if (prop == CPU_PAGE_PROPERTY_DEFAULT)
            {
                result |= D3D11_CPU_ACCESS_READ;
            }
            else if (prop == CPU_PAGE_PROPERTY_NOT_AVAILABLE)
            {
                /* DO_NOTHING */
            }
            else if (prop == CPU_PAGE_PROPERTY_WRITE_COMBINE)
            {
                /* DO_NOTHING */
            }
            else if (prop == CPU_PAGE_PROPERTY_WRITE_BACK)
            {
                /* DO_NOTHING */
            }
        }
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
    uint32_t planeSlice,
    uint32_t mipLevels,
    uint32_t arraySize
)
{ return mipSlice + arraySlice * mipLevels + planeSlice * mipLevels * arraySize; }

//-------------------------------------------------------------------------------------------------
//      サブリソース番号からミップレベル・配列番号・平面スライスを求めます.
//-------------------------------------------------------------------------------------------------
void DecomposeSubresource
(
    uint32_t subresource,
    uint32_t mipLevels,
    uint32_t arraySize,
    uint32_t& mipSlice,
    uint32_t& arraySlice,
    uint32_t& planeSlice
)
{
    mipSlice   = subresource % mipLevels;
    arraySlice = (subresource / mipLevels) % arraySize;
    planeSlice = (subresource / (mipLevels * arraySize));
}

} // namespace a3d
