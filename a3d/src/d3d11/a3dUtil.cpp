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
    a3d::RESOURCE_FORMAT    Format;             //!< A3D形式です.
};

FormatConvertTable g_FormatTable[] = {
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_UNKNOWN                },  // 0
    { DXGI_FORMAT_R32G32B32A32_FLOAT        , DXGI_FORMAT_R32G32B32A32_TYPELESS     , a3d::RESOURCE_FORMAT_R32G32B32A32_FLOAT     },  // 1
    { DXGI_FORMAT_R32G32B32A32_UINT         , DXGI_FORMAT_R32G32B32A32_TYPELESS     , a3d::RESOURCE_FORMAT_R32G32B32A32_UINT      },  // 2
    { DXGI_FORMAT_R32G32B32A32_SINT         , DXGI_FORMAT_R32G32B32A32_TYPELESS     , a3d::RESOURCE_FORMAT_R32G32B32A32_SINT      },  // 3
    { DXGI_FORMAT_R16G16B16A16_FLOAT        , DXGI_FORMAT_R16G16B16A16_TYPELESS     , a3d::RESOURCE_FORMAT_R16G16B16A16_FLOAT     },  // 4
    { DXGI_FORMAT_R16G16B16A16_UINT         , DXGI_FORMAT_R16G16B16A16_TYPELESS     , a3d::RESOURCE_FORMAT_R16G16B16A16_UINT      },  // 5
    { DXGI_FORMAT_R16G16B16A16_SINT         , DXGI_FORMAT_R16G16B16A16_TYPELESS     , a3d::RESOURCE_FORMAT_R16G16B16A16_SINT      },  // 6
    { DXGI_FORMAT_R32G32B32_FLOAT           , DXGI_FORMAT_R32G32B32_TYPELESS        , a3d::RESOURCE_FORMAT_R32G32B32_FLOAT        },  // 7
    { DXGI_FORMAT_R32G32B32_UINT            , DXGI_FORMAT_R32G32B32_TYPELESS        , a3d::RESOURCE_FORMAT_R32G32B32_UINT         },  // 8
    { DXGI_FORMAT_R32G32B32_SINT            , DXGI_FORMAT_R32G32B32_TYPELESS        , a3d::RESOURCE_FORMAT_R32G32B32_SINT         },  // 9
    { DXGI_FORMAT_R32G32_FLOAT              , DXGI_FORMAT_R32G32_TYPELESS           , a3d::RESOURCE_FORMAT_R32G32_FLOAT           },  // 10
    { DXGI_FORMAT_R32G32_UINT               , DXGI_FORMAT_R32G32_TYPELESS           , a3d::RESOURCE_FORMAT_R32G32_UINT            },  // 11
    { DXGI_FORMAT_R32G32_SINT               , DXGI_FORMAT_R32G32_TYPELESS           , a3d::RESOURCE_FORMAT_R32G32_SINT            },  // 12
    { DXGI_FORMAT_R16G16_FLOAT              , DXGI_FORMAT_R16G16_TYPELESS           , a3d::RESOURCE_FORMAT_R16G16_FLOAT           },  // 13
    { DXGI_FORMAT_R16G16_UINT               , DXGI_FORMAT_R16G16_TYPELESS           , a3d::RESOURCE_FORMAT_R16G16_UINT            },  // 14
    { DXGI_FORMAT_R16G16_SINT               , DXGI_FORMAT_R16G16_TYPELESS           , a3d::RESOURCE_FORMAT_R16G16_SINT            },  // 15
    { DXGI_FORMAT_R32_FLOAT                 , DXGI_FORMAT_R32_TYPELESS              , a3d::RESOURCE_FORMAT_R32_FLOAT              },  // 16
    { DXGI_FORMAT_R32_UINT                  , DXGI_FORMAT_R32_TYPELESS              , a3d::RESOURCE_FORMAT_R32_UINT               },  // 17
    { DXGI_FORMAT_R32_SINT                  , DXGI_FORMAT_R32_TYPELESS              , a3d::RESOURCE_FORMAT_R32_SINT               },  // 18
    { DXGI_FORMAT_R16_FLOAT                 , DXGI_FORMAT_R16_TYPELESS              , a3d::RESOURCE_FORMAT_R16_FLOAT              },  // 19
    { DXGI_FORMAT_R16_UINT                  , DXGI_FORMAT_R16_TYPELESS              , a3d::RESOURCE_FORMAT_R16_UINT               },  // 20
    { DXGI_FORMAT_R16_SINT                  , DXGI_FORMAT_R16_TYPELESS              , a3d::RESOURCE_FORMAT_R16_SINT               },  // 21
    { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB       , DXGI_FORMAT_R8G8B8A8_TYPELESS         , a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM_SRGB    },  // 22
    { DXGI_FORMAT_R8G8B8A8_UNORM            , DXGI_FORMAT_R8G8B8A8_TYPELESS         , a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM         },  // 23
    { DXGI_FORMAT_B8G8R8A8_UNORM_SRGB       , DXGI_FORMAT_B8G8R8A8_TYPELESS         , a3d::RESOURCE_FORMAT_B8G8R8A8_UNORM_SRGB    },  // 24
    { DXGI_FORMAT_B8G8R8A8_UNORM            , DXGI_FORMAT_B8G8R8A8_TYPELESS         , a3d::RESOURCE_FORMAT_B8G8R8A8_UNORM         },  // 25
    { DXGI_FORMAT_R8G8_UNORM                , DXGI_FORMAT_R8G8_TYPELESS             , a3d::RESOURCE_FORMAT_R8G8_UNORM             },  // 26
    { DXGI_FORMAT_R8_UNORM                  , DXGI_FORMAT_R8_TYPELESS               , a3d::RESOURCE_FORMAT_R8_UNORM               },  // 27
    { DXGI_FORMAT_D32_FLOAT                 , DXGI_FORMAT_R32_TYPELESS              , a3d::RESOURCE_FORMAT_D32_FLOAT              },  // 28
    { DXGI_FORMAT_D24_UNORM_S8_UINT         , DXGI_FORMAT_R24G8_TYPELESS            , a3d::RESOURCE_FORMAT_D24_UNORM_S8_UINT      },  // 29
    { DXGI_FORMAT_D16_UNORM                 , DXGI_FORMAT_R16_TYPELESS              , a3d::RESOURCE_FORMAT_D16_UNORM              },  // 30
    { DXGI_FORMAT_BC1_UNORM_SRGB            , DXGI_FORMAT_BC1_TYPELESS              , a3d::RESOURCE_FORMAT_BC1_UNORM_SRGB         },  // 31
    { DXGI_FORMAT_BC1_UNORM                 , DXGI_FORMAT_BC1_TYPELESS              , a3d::RESOURCE_FORMAT_BC1_UNORM              },  // 32
    { DXGI_FORMAT_BC2_UNORM_SRGB            , DXGI_FORMAT_BC2_TYPELESS              , a3d::RESOURCE_FORMAT_BC2_UNORM_SRGB         },  // 33
    { DXGI_FORMAT_BC2_UNORM                 , DXGI_FORMAT_BC2_TYPELESS              , a3d::RESOURCE_FORMAT_BC2_UNORM              },  // 34
    { DXGI_FORMAT_BC3_UNORM_SRGB            , DXGI_FORMAT_BC3_TYPELESS              , a3d::RESOURCE_FORMAT_BC3_UNORM_SRGB         },  // 35
    { DXGI_FORMAT_BC3_UNORM                 , DXGI_FORMAT_BC3_TYPELESS              , a3d::RESOURCE_FORMAT_BC3_UNORM              },  // 36
    { DXGI_FORMAT_BC4_UNORM                 , DXGI_FORMAT_BC4_TYPELESS              , a3d::RESOURCE_FORMAT_BC4_UNORM              },  // 37
    { DXGI_FORMAT_BC4_SNORM                 , DXGI_FORMAT_BC4_TYPELESS              , a3d::RESOURCE_FORMAT_BC4_SNORM              },  // 38
    { DXGI_FORMAT_BC5_UNORM                 , DXGI_FORMAT_BC5_TYPELESS              , a3d::RESOURCE_FORMAT_BC5_UNORM              },  // 39
    { DXGI_FORMAT_BC5_SNORM                 , DXGI_FORMAT_BC5_TYPELESS              , a3d::RESOURCE_FORMAT_BC5_SNORM              },  // 40
    { DXGI_FORMAT_BC6H_UF16                 , DXGI_FORMAT_BC6H_TYPELESS             , a3d::RESOURCE_FORMAT_BC6H_UF16              },  // 41
    { DXGI_FORMAT_BC6H_SF16                 , DXGI_FORMAT_BC6H_TYPELESS             , a3d::RESOURCE_FORMAT_BC6H_SF16              },  // 42
    { DXGI_FORMAT_BC7_UNORM_SRGB            , DXGI_FORMAT_BC7_TYPELESS              , a3d::RESOURCE_FORMAT_BC7_UNORM_SRGB         },  // 43
    { DXGI_FORMAT_BC7_UNORM                 , DXGI_FORMAT_BC7_TYPELESS              , a3d::RESOURCE_FORMAT_BC7_UNORM              },  // 44
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_4X4_UNORM_SRGB    },  // 45
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_4X4_UNORM         },  // 46
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_5X4_UNORM_SRGB    },  // 47
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_5X4_UNORM         },  // 48
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_5X5_UNORM_SRGB    },  // 49
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_5X5_UNORM         },  // 50
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_6X5_UNORM_SRGB    },  // 51
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_6X5_UNORM         },  // 52
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_6X6_UNORM_SRGB    },  // 53
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_6X6_UNORM         },  // 54
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_8X5_UNORM_SRGB    },  // 55
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_8X5_UNORM         },  // 56
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_8X6_UNORM_SRGB    },  // 57
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_8X6_UNORM         },  // 58
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_8X8_UNORM_SRGB    },  // 59
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_8X8_UNORM         },  // 60
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_10X5_UNORM_SRGB   },  // 61
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_10X5_UNORM        },  // 62
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_10X6_UNORM_SRGB   },  // 63
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_10X6_UNORM        },  // 64
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_10X8_UNORM_SRGB   },  // 65
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_10X8_UNORM        },  // 66
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_10X10_UNORM_SRGB  },  // 67
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_10X10_UNORM       },  // 68
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_12X10_UNORM_SRGB  },  // 69
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_12X10_UNORM       },  // 70
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_12X12_UNORM_SRGB  },  // 71
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_ASTC_12X12_UNORM       },  // 72
    { DXGI_FORMAT_R10G10B10A2_UNORM         , DXGI_FORMAT_R10G10B10A2_TYPELESS      , a3d::RESOURCE_FORMAT_R10G10B10A2_UNORM      },  // 73
    { DXGI_FORMAT_R10G10B10A2_UINT          , DXGI_FORMAT_R10G10B10A2_TYPELESS      , a3d::RESOURCE_FORMAT_R10G10B10A2_UINT       },  // 74
    { DXGI_FORMAT_R11G11B10_FLOAT           , DXGI_FORMAT_R11G11B10_FLOAT           , a3d::RESOURCE_FORMAT_R11G11B10_FLOAT        },  // 75
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_B16G16R16A16_FLOAT     },  // 76
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_B10G10R10A2_UNORM      },  // 77
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , a3d::RESOURCE_FORMAT_B10G10R10A2_UINT       }   // 78
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
DXGI_FORMAT ToNativeTypelessFormat(RESOURCE_FORMAT format)
{ return g_FormatTable[format].TypelessFormat; }

//-------------------------------------------------------------------------------------------------
//      ビューフォーマットに変換します.
//-------------------------------------------------------------------------------------------------
DXGI_FORMAT ToNativeViewFormat(RESOURCE_FORMAT format, bool isStencil)
{
    if (format == RESOURCE_FORMAT_D24_UNORM_S8_UINT)
    {
        if (isStencil)
        { return DXGI_FORMAT_X24_TYPELESS_G8_UINT; }
        else
        { return DXGI_FORMAT_R24_UNORM_X8_TYPELESS; }
    }

    if (format == RESOURCE_FORMAT_D16_UNORM)
    { return DXGI_FORMAT_R16_UNORM; }

    if (format == RESOURCE_FORMAT_D32_FLOAT)
    { return DXGI_FORMAT_R32_FLOAT; }

    return g_FormatTable[format].NativeFormat;
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
    if (value & RESOURCE_USAGE_RENDER_TARGET)
    { result |= D3D11_BIND_RENDER_TARGET; }
    if (value & RESOURCE_USAGE_DEPTH_STENCIL)
    { result |= D3D11_BIND_DEPTH_STENCIL; }
    if (value & RESOURCE_USAGE_UNORDERED_ACCESS)
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

#if defined(A3D_FOR_WINDOWS10)
//-------------------------------------------------------------------------------------------------
//      カラースペースを変換します.
//-------------------------------------------------------------------------------------------------
DXGI_COLOR_SPACE_TYPE ToNativeColorSpace(COLOR_SPACE_TYPE value)
{
    switch(value)
    {
    case COLOR_SPACE_SRGB:
        return DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

    case COLOR_SPACE_BT709_170M:
        return DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

    case COLOR_SPACE_BT2100_PQ:
        return DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;

    case COLOR_SPACE_BT2100_HLG:
        return DXGI_COLOR_SPACE_YCBCR_FULL_GHLG_TOPLEFT_P2020;
    }

    return DXGI_COLOR_SPACE_CUSTOM;
}
#endif//defined(A3D_FOR_WINDOWS10)

} // namespace a3d
