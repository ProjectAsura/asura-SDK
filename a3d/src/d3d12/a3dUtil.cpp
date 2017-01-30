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
    uint32_t                BitPerPixel;        //!< 1ピクセルあたりのビット数です.
    a3d::RESOURCE_FORMAT    Format;             //!< A3D形式です.
    bool                    IsCompress;         //!< 圧縮フォーマットかどうか?
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// StateTable structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct StateConvertTable
{
    D3D12_RESOURCE_STATES   NativeState;
    a3d::RESOURCE_STATE     State;
};

FormatConvertTable g_FormatTable[] = {
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 0     , a3d::RESOURCE_FORMAT_UNKNOWN               , false },  // 0
    { DXGI_FORMAT_R32G32B32A32_FLOAT        , DXGI_FORMAT_R32G32B32A32_TYPELESS     , 16    , a3d::RESOURCE_FORMAT_R32G32B32A32_FLOAT    , false },  // 1
    { DXGI_FORMAT_R32G32B32A32_UINT         , DXGI_FORMAT_R32G32B32A32_TYPELESS     , 16    , a3d::RESOURCE_FORMAT_R32G32B32A32_UINT     , false },  // 2
    { DXGI_FORMAT_R32G32B32A32_SINT         , DXGI_FORMAT_R32G32B32A32_TYPELESS     , 16    , a3d::RESOURCE_FORMAT_R32G32B32A32_SINT     , false },  // 3
    { DXGI_FORMAT_R16G16B16A16_FLOAT        , DXGI_FORMAT_R16G16B16A16_TYPELESS     , 8     , a3d::RESOURCE_FORMAT_R16G16B16A16_FLOAT    , false },  // 4
    { DXGI_FORMAT_R16G16B16A16_UINT         , DXGI_FORMAT_R16G16B16A16_TYPELESS     , 8     , a3d::RESOURCE_FORMAT_R16G16B16A16_UINT     , false },  // 5
    { DXGI_FORMAT_R16G16B16A16_SINT         , DXGI_FORMAT_R16G16B16A16_TYPELESS     , 8     , a3d::RESOURCE_FORMAT_R16G16B16A16_SINT     , false },  // 6
    { DXGI_FORMAT_R32G32B32_FLOAT           , DXGI_FORMAT_R32G32B32_TYPELESS        , 12    , a3d::RESOURCE_FORMAT_R32G32B32_FLOAT       , false },  // 7
    { DXGI_FORMAT_R32G32B32_UINT            , DXGI_FORMAT_R32G32B32_TYPELESS        , 12    , a3d::RESOURCE_FORMAT_R32G32B32_UINT        , false },  // 8
    { DXGI_FORMAT_R32G32B32_SINT            , DXGI_FORMAT_R32G32B32_TYPELESS        , 12    , a3d::RESOURCE_FORMAT_R32G32B32_SINT        , false },  // 9
    { DXGI_FORMAT_R32G32_FLOAT              , DXGI_FORMAT_R32G32_TYPELESS           , 8     , a3d::RESOURCE_FORMAT_R32G32_FLOAT          , false },  // 10
    { DXGI_FORMAT_R32G32_UINT               , DXGI_FORMAT_R32G32_TYPELESS           , 8     , a3d::RESOURCE_FORMAT_R32G32_UINT           , false },  // 11
    { DXGI_FORMAT_R32G32_SINT               , DXGI_FORMAT_R32G32_TYPELESS           , 8     , a3d::RESOURCE_FORMAT_R32G32_SINT           , false },  // 12
    { DXGI_FORMAT_R16G16_FLOAT              , DXGI_FORMAT_R16G16_TYPELESS           , 4     , a3d::RESOURCE_FORMAT_R16G16_FLOAT          , false },  // 13
    { DXGI_FORMAT_R16G16_UINT               , DXGI_FORMAT_R16G16_TYPELESS           , 4     , a3d::RESOURCE_FORMAT_R16G16_UINT           , false },  // 14
    { DXGI_FORMAT_R16G16_SINT               , DXGI_FORMAT_R16G16_TYPELESS           , 4     , a3d::RESOURCE_FORMAT_R16G16_SINT           , false },  // 15
    { DXGI_FORMAT_R32_FLOAT                 , DXGI_FORMAT_R32_TYPELESS              , 4     , a3d::RESOURCE_FORMAT_R32_FLOAT             , false },  // 16
    { DXGI_FORMAT_R32_UINT                  , DXGI_FORMAT_R32_TYPELESS              , 4     , a3d::RESOURCE_FORMAT_R32_UINT              , false },  // 17
    { DXGI_FORMAT_R32_SINT                  , DXGI_FORMAT_R32_TYPELESS              , 4     , a3d::RESOURCE_FORMAT_R32_SINT              , false },  // 18
    { DXGI_FORMAT_R16_FLOAT                 , DXGI_FORMAT_R16_TYPELESS              , 2     , a3d::RESOURCE_FORMAT_R16_FLOAT             , false },  // 19
    { DXGI_FORMAT_R16_UINT                  , DXGI_FORMAT_R16_TYPELESS              , 2     , a3d::RESOURCE_FORMAT_R16_UINT              , false },  // 20
    { DXGI_FORMAT_R16_SINT                  , DXGI_FORMAT_R16_TYPELESS              , 2     , a3d::RESOURCE_FORMAT_R16_SINT              , false },  // 21
    { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB       , DXGI_FORMAT_R8G8B8A8_TYPELESS         , 4     , a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM_SRGB   , false },  // 22
    { DXGI_FORMAT_R8G8B8A8_UNORM            , DXGI_FORMAT_R8G8B8A8_TYPELESS         , 4     , a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM        , false },  // 23
    { DXGI_FORMAT_B8G8R8A8_UNORM_SRGB       , DXGI_FORMAT_B8G8R8A8_TYPELESS         , 4     , a3d::RESOURCE_FORMAT_B8G8R8A8_UNORM_SRGB   , false },  // 24
    { DXGI_FORMAT_B8G8R8A8_UNORM            , DXGI_FORMAT_B8G8R8A8_TYPELESS         , 4     , a3d::RESOURCE_FORMAT_B8G8R8A8_UNORM        , false },  // 25
    { DXGI_FORMAT_R8G8_UNORM                , DXGI_FORMAT_R8G8_TYPELESS             , 2     , a3d::RESOURCE_FORMAT_R8G8_UNORM            , false },  // 26
    { DXGI_FORMAT_R8_UNORM                  , DXGI_FORMAT_R8_TYPELESS               , 1     , a3d::RESOURCE_FORMAT_R8_UNORM              , false },  // 27
    { DXGI_FORMAT_D32_FLOAT                 , DXGI_FORMAT_R32_TYPELESS              , 4     , a3d::RESOURCE_FORMAT_D32_FLOAT             , false },  // 28
    { DXGI_FORMAT_D24_UNORM_S8_UINT         , DXGI_FORMAT_R24_UNORM_X8_TYPELESS     , 4     , a3d::RESOURCE_FORMAT_D24_UNORM_S8_UINT     , false },  // 29
    { DXGI_FORMAT_D16_UNORM                 , DXGI_FORMAT_R16_TYPELESS              , 2     , a3d::RESOURCE_FORMAT_D16_UNORM             , false },  // 30
    { DXGI_FORMAT_BC1_UNORM_SRGB            , DXGI_FORMAT_BC1_TYPELESS              , 8     , a3d::RESOURCE_FORMAT_BC1_UNORM_SRGB        , true  },  // 31
    { DXGI_FORMAT_BC1_UNORM                 , DXGI_FORMAT_BC1_TYPELESS              , 8     , a3d::RESOURCE_FORMAT_BC1_UNORM             , true  },  // 32
    { DXGI_FORMAT_BC2_UNORM_SRGB            , DXGI_FORMAT_BC2_TYPELESS              , 16    , a3d::RESOURCE_FORMAT_BC2_UNORM_SRGB        , true  },  // 33
    { DXGI_FORMAT_BC2_UNORM                 , DXGI_FORMAT_BC2_TYPELESS              , 16    , a3d::RESOURCE_FORMAT_BC2_UNORM             , true  },  // 34
    { DXGI_FORMAT_BC3_UNORM_SRGB            , DXGI_FORMAT_BC3_TYPELESS              , 16    , a3d::RESOURCE_FORMAT_BC3_UNORM_SRGB        , true  },  // 35
    { DXGI_FORMAT_BC3_UNORM                 , DXGI_FORMAT_BC3_TYPELESS              , 16    , a3d::RESOURCE_FORMAT_BC3_UNORM             , true  },  // 36
    { DXGI_FORMAT_BC4_UNORM                 , DXGI_FORMAT_BC4_TYPELESS              , 8     , a3d::RESOURCE_FORMAT_BC4_UNORM             , true  },  // 37
    { DXGI_FORMAT_BC4_SNORM                 , DXGI_FORMAT_BC4_TYPELESS              , 8     , a3d::RESOURCE_FORMAT_BC4_SNORM             , true  },  // 38
    { DXGI_FORMAT_BC5_UNORM                 , DXGI_FORMAT_BC5_TYPELESS              , 16    , a3d::RESOURCE_FORMAT_BC5_UNORM             , true  },  // 39
    { DXGI_FORMAT_BC5_SNORM                 , DXGI_FORMAT_BC5_TYPELESS              , 16    , a3d::RESOURCE_FORMAT_BC5_SNORM             , true  },  // 40
    { DXGI_FORMAT_BC6H_UF16                 , DXGI_FORMAT_BC6H_TYPELESS             , 16    , a3d::RESOURCE_FORMAT_BC6H_UF16             , true  },  // 41
    { DXGI_FORMAT_BC6H_SF16                 , DXGI_FORMAT_BC6H_TYPELESS             , 16    , a3d::RESOURCE_FORMAT_BC6H_SF16             , true  },  // 42
    { DXGI_FORMAT_BC7_UNORM_SRGB            , DXGI_FORMAT_BC7_TYPELESS              , 16    , a3d::RESOURCE_FORMAT_BC7_UNORM_SRGB        , true  },  // 43
    { DXGI_FORMAT_BC7_UNORM                 , DXGI_FORMAT_BC7_TYPELESS              , 16    , a3d::RESOURCE_FORMAT_BC7_UNORN             , true  },  // 44
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_4X4_UNORM_SRGB   , true  },  // 45
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_4X4_UNORM        , true  },  // 46
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_5X4_UNORM_SRGB   , true  },  // 47
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_5X4_UNORM        , true  },  // 48
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_5X5_UNORM_SRGB   , true  },  // 49
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_5X5_UNORM        , true  },  // 50
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_6X5_UNORM_SRGB   , true  },  // 51
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_6X5_UNORM        , true  },  // 52
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_6X6_UNORM_SRGB   , true  },  // 53
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_6X6_UNORM        , true  },  // 54
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_8X5_UNORM_SRGB   , true  },  // 55
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_8X5_UNORM        , true  },  // 56
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_8X6_UNORM_SRGB   , true  },  // 57
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_8X6_UNORM        , true  },  // 58
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_8X8_UNORM_SRGB   , true  },  // 59
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_8X8_UNORM        , true  },  // 60
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_10X5_UNORM_SRGB  , true  },  // 61
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_10X5_UNORM       , true  },  // 62
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_10X6_UNORM_SRGB  , true  },  // 63
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_10X6_UNORM       , true  },  // 64
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_10X8_UNORM_SRGB  , true  },  // 65
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_10X8_UNORM       , true  },  // 66
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_10X10_UNORM_SRGB , true  },  // 67
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_10X10_UNORM      , true  },  // 68
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_12X10_UNORM_SRGB , true  },  // 69
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_12X10_UNORM      , true  },  // 70
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_12X12_UNORM_SRGB , true  },  // 71
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 16    , a3d::RESOURCE_FORMAT_ASTC_12X12_UNORM      , true  },  // 72
    { DXGI_FORMAT_R10G10B10A2_UNORM         , DXGI_FORMAT_R10G10B10A2_TYPELESS      , 4     , a3d::RESOURCE_FORMAT_R10G10B10A2_UNORM     , false },  // 73
    { DXGI_FORMAT_R10G10B10A2_UINT          , DXGI_FORMAT_R10G10B10A2_TYPELESS      , 4     , a3d::RESOURCE_FORMAT_R10G10B10A2_UINT      , false },  // 74
    { DXGI_FORMAT_R11G11B10_FLOAT           , DXGI_FORMAT_R11G11B10_FLOAT           , 4     , a3d::RESOURCE_FORMAT_R11G11B10_FLOAT       , false },  // 75
};

StateConvertTable g_StateTable[] = {
    { D3D12_RESOURCE_STATE_COMMON                       , a3d::RESOURCE_STATE_UNKNOWN           },
    { D3D12_RESOURCE_STATE_GENERIC_READ                 , a3d::RESOURCE_STATE_GENERAL           },
    { D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER   , a3d::RESOURCE_STATE_VERTEX_BUFFER     },
    { D3D12_RESOURCE_STATE_INDEX_BUFFER                 , a3d::RESOURCE_STATE_INDEX_BUFFER      },
    { D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER   , a3d::RESOURCE_STATE_CONSTANT_BUFFER   },
    { D3D12_RESOURCE_STATE_RENDER_TARGET                , a3d::RESOURCE_STATE_COLOR_WRITE       },
    { D3D12_RESOURCE_STATE_GENERIC_READ                 , a3d::RESOURCE_STATE_COLOR_READ        },
    { D3D12_RESOURCE_STATE_UNORDERED_ACCESS             , a3d::RESOURCE_STATE_UNORDERED_ACCESS  },
    { D3D12_RESOURCE_STATE_DEPTH_WRITE                  , a3d::RESOURCE_STATE_DEPTH_WRITE       },
    { D3D12_RESOURCE_STATE_DEPTH_READ                   , a3d::RESOURCE_STATE_DEPTH_READ        },
    { D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE    , a3d::RESOURCE_STATE_SHADER_READ       },
    { D3D12_RESOURCE_STATE_STREAM_OUT                   , a3d::RESOURCE_STATE_STREAM_OUT        },
    { D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT            , a3d::RESOURCE_STATE_INDIRECT_ARGUMENT },
    { D3D12_RESOURCE_STATE_COPY_DEST                    , a3d::RESOURCE_STATE_COPY_DST          },
    { D3D12_RESOURCE_STATE_COPY_SOURCE                  , a3d::RESOURCE_STATE_COPY_SRC          },
    { D3D12_RESOURCE_STATE_RESOLVE_DEST                 , a3d::RESOURCE_STATE_RESOLVE_DST       },
    { D3D12_RESOURCE_STATE_RESOLVE_SOURCE               , a3d::RESOURCE_STATE_RESOLVE_SRC       },
    { D3D12_RESOURCE_STATE_PRESENT                      , a3d::RESOURCE_STATE_PRESENT           },
    { D3D12_RESOURCE_STATE_PREDICATION                  , a3d::RESORUCE_STATE_PREDICATION       },
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
uint32_t ToBits(RESOURCE_FORMAT format)
{ return g_FormatTable[format].BitPerPixel; }

//-------------------------------------------------------------------------------------------------
//      リソースフォーマットのバイト数に変換します.
//-------------------------------------------------------------------------------------------------
uint32_t ToByte(RESOURCE_FORMAT format)
{ return g_FormatTable[format].BitPerPixel / 8; }

//-------------------------------------------------------------------------------------------------
//      圧縮フォーマットかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsCompressFormat(RESOURCE_FORMAT format)
{ return g_FormatTable[format].IsCompress; }

//-------------------------------------------------------------------------------------------------
//      SRGBフォーマットかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsSRGBFormat(RESOURCE_FORMAT format)
{
    switch(format)
    {
    case RESOURCE_FORMAT_R8G8B8A8_UNORM_SRGB :
    case RESOURCE_FORMAT_B8G8R8A8_UNORM_SRGB :
    case RESOURCE_FORMAT_BC1_UNORM_SRGB :
    case RESOURCE_FORMAT_BC2_UNORM_SRGB :
    case RESOURCE_FORMAT_BC3_UNORM_SRGB :
    case RESOURCE_FORMAT_BC7_UNORM_SRGB :
    case RESOURCE_FORMAT_ASTC_4X4_UNORM_SRGB :
    case RESOURCE_FORMAT_ASTC_5X4_UNORM_SRGB :
    case RESOURCE_FORMAT_ASTC_5X5_UNORM_SRGB :
    case RESOURCE_FORMAT_ASTC_6X5_UNORM_SRGB :
    case RESOURCE_FORMAT_ASTC_6X6_UNORM_SRGB :
    case RESOURCE_FORMAT_ASTC_8X5_UNORM_SRGB :
    case RESOURCE_FORMAT_ASTC_8X6_UNORM_SRGB :
    case RESOURCE_FORMAT_ASTC_8X8_UNORM_SRGB :
    case RESOURCE_FORMAT_ASTC_10X5_UNORM_SRGB :
    case RESOURCE_FORMAT_ASTC_10X6_UNORM_SRGB :
    case RESOURCE_FORMAT_ASTC_10X8_UNORM_SRGB :
    case RESOURCE_FORMAT_ASTC_10X10_UNORM_SRGB :
    case RESOURCE_FORMAT_ASTC_12X10_UNORM_SRGB :
    case RESOURCE_FORMAT_ASTC_12X12_UNORM_SRGB :
        return true;

    default:
        return false;
    }
}

//-------------------------------------------------------------------------------------------------
//      ブロックの横幅を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t GetBlockWidth(RESOURCE_FORMAT format)
{
    switch(format)
    {
    case RESOURCE_FORMAT_ASTC_5X4_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_5X4_UNORM:
    case RESOURCE_FORMAT_ASTC_5X5_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_5X5_UNORM:
        { return 5; }

    case RESOURCE_FORMAT_ASTC_6X5_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_6X5_UNORM:
    case RESOURCE_FORMAT_ASTC_6X6_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_6X6_UNORM:
        { return 6; }

    case RESOURCE_FORMAT_ASTC_8X5_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_8X5_UNORM:
    case RESOURCE_FORMAT_ASTC_8X6_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_8X6_UNORM:
    case RESOURCE_FORMAT_ASTC_8X8_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_8X8_UNORM:
        { return 8; }

    case RESOURCE_FORMAT_ASTC_10X5_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_10X5_UNORM:
    case RESOURCE_FORMAT_ASTC_10X6_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_10X6_UNORM:
    case RESOURCE_FORMAT_ASTC_10X8_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_10X8_UNORM:
    case RESOURCE_FORMAT_ASTC_10X10_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_10X10_UNORM:
        { return 10; }

    case RESOURCE_FORMAT_ASTC_12X10_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_12X10_UNORM:
    case RESOURCE_FORMAT_ASTC_12X12_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_12X12_UNORM:
        { return 12; }

    default:
        return 4;
    }
}

//-------------------------------------------------------------------------------------------------
//      ブロックの縦幅を求めます.
//-------------------------------------------------------------------------------------------------
uint32_t GetBlockHeight(RESOURCE_FORMAT format)
{
    switch(format)
    {
    case RESOURCE_FORMAT_ASTC_5X5_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_5X5_UNORM:
    case RESOURCE_FORMAT_ASTC_8X5_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_8X5_UNORM:
    case RESOURCE_FORMAT_ASTC_10X5_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_10X5_UNORM:
        { return 5; }

    case RESOURCE_FORMAT_ASTC_6X6_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_6X6_UNORM:
    case RESOURCE_FORMAT_ASTC_8X6_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_8X6_UNORM:
    case RESOURCE_FORMAT_ASTC_10X6_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_10X6_UNORM:
        { return 6; }

    case RESOURCE_FORMAT_ASTC_10X10_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_10X10_UNORM:
    case RESOURCE_FORMAT_ASTC_12X10_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_12X10_UNORM:
        { return 10; }

    case RESOURCE_FORMAT_ASTC_12X12_UNORM_SRGB:
    case RESOURCE_FORMAT_ASTC_12X12_UNORM:
        { return 12; }

    default:
        return 4;
    }
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_RESOURCE_STATES ToNativeState(RESOURCE_STATE state)
{ return g_StateTable[state].NativeState; }

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_RESOURCE_DIMENSION ToNativeResorceDimension(RESOURCE_DIMENSION dimension)
{
    static const D3D12_RESOURCE_DIMENSION table[] = {
        D3D12_RESOURCE_DIMENSION_UNKNOWN,
        D3D12_RESOURCE_DIMENSION_BUFFER,
        D3D12_RESOURCE_DIMENSION_TEXTURE1D,
        D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        D3D12_RESOURCE_DIMENSION_TEXTURE3D,
        D3D12_RESOURCE_DIMENSION_TEXTURE2D
    };

    return table[dimension];
}

//-------------------------------------------------------------------------------------------------
//      A3D形式に変換します.
//-------------------------------------------------------------------------------------------------
RESOURCE_DIMENSION ToWrapDimension(D3D12_RESOURCE_DIMENSION dimension)
{
    static const RESOURCE_DIMENSION table[] = {
        RESOURCE_DIMENSION_UNKNOWN,
        RESOURCE_DIMENSION_BUFFER,
        RESOURCE_DIMENSION_TEXTURE1D,
        RESOURCE_DIMENSION_TEXTURE2D,
        RESOURCE_DIMENSION_TEXTURE3D
    };

    return table[dimension];
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_RESOURCE_FLAGS ToNativeResourceFlags(uint32_t usage)
{
    D3D12_RESOURCE_FLAGS result = D3D12_RESOURCE_FLAG_NONE;

    if (usage & RESOURCE_USAGE_COLOR_TARGET)
    { result |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; }

    if (usage & RESOURCE_USAGE_DEPTH_TARGET)
    { result |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; }

    if (usage & RESOURCE_USAGE_UNORDERD_ACCESS)
    { result |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; }

    return result;
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_COMPARISON_FUNC ToNativeComparisonFunc(COMPARE_OP operaton)
{
    static const D3D12_COMPARISON_FUNC table[] = {
        D3D12_COMPARISON_FUNC_NEVER,
        D3D12_COMPARISON_FUNC_LESS,
        D3D12_COMPARISON_FUNC_EQUAL,
        D3D12_COMPARISON_FUNC_LESS_EQUAL,
        D3D12_COMPARISON_FUNC_GREATER,
        D3D12_COMPARISON_FUNC_GREATER_EQUAL,
        D3D12_COMPARISON_FUNC_ALWAYS,
    };

    return table[operaton];
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_PRIMITIVE_TOPOLOGY_TYPE ToNativePrimitiveTopology(PRIMITIVE_TOPOLOGY topology)
{
    static const D3D12_PRIMITIVE_TOPOLOGY_TYPE table[] = {
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH
    };

    return table[topology];
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D_PRIMITIVE_TOPOLOGY ToNativePrimitive(PRIMITIVE_TOPOLOGY topology, uint32_t controlCount)
{
    if ( topology != PRIMITIVE_TOPOLOGY_PATCHLIST )
    {
        static const D3D_PRIMITIVE_TOPOLOGY table[] = {
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
        static const D3D_PRIMITIVE_TOPOLOGY table[] = {
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
D3D12_HEAP_TYPE ToNativeHeapType(HEAP_TYPE type)
{
    static const D3D12_HEAP_TYPE table[] = {
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_HEAP_TYPE_READBACK
    };

    return table[type];
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_CPU_PAGE_PROPERTY ToNativeCpuPageProperty(CPU_PAGE_PROPERTY prop)
{
    static const D3D12_CPU_PAGE_PROPERTY table[] = {
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE,
        D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE,
        D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
    };

    return table[prop];
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_RTV_DIMENSION ToNativeRTVDimension(VIEW_DIMENSION value)
{
    static const D3D12_RTV_DIMENSION table[] = {
        D3D12_RTV_DIMENSION_BUFFER,
        D3D12_RTV_DIMENSION_TEXTURE1D,
        D3D12_RTV_DIMENSION_TEXTURE1DARRAY,
        D3D12_RTV_DIMENSION_TEXTURE2D,
        D3D12_RTV_DIMENSION_TEXTURE2DARRAY,
        D3D12_RTV_DIMENSION_TEXTURE2DARRAY,
        D3D12_RTV_DIMENSION_TEXTURE2DARRAY,
        D3D12_RTV_DIMENSION_TEXTURE3D
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_DSV_DIMENSION ToNativeDSVDimension(VIEW_DIMENSION value)
{
    static const D3D12_DSV_DIMENSION table[] = {
        D3D12_DSV_DIMENSION_UNKNOWN,
        D3D12_DSV_DIMENSION_TEXTURE1D,
        D3D12_DSV_DIMENSION_TEXTURE1DARRAY,
        D3D12_DSV_DIMENSION_TEXTURE2D,
        D3D12_DSV_DIMENSION_TEXTURE2DARRAY,
        D3D12_DSV_DIMENSION_TEXTURE2DARRAY,
        D3D12_DSV_DIMENSION_TEXTURE2DARRAY,
        D3D12_DSV_DIMENSION_TEXTURE2DARRAY
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_SRV_DIMENSION ToNativeSRVDimension(VIEW_DIMENSION value)
{
    static const D3D12_SRV_DIMENSION table[] = {
        D3D12_SRV_DIMENSION_BUFFER,
        D3D12_SRV_DIMENSION_TEXTURE1D,
        D3D12_SRV_DIMENSION_TEXTURE1DARRAY,
        D3D12_SRV_DIMENSION_TEXTURE2D,
        D3D12_SRV_DIMENSION_TEXTURE2DARRAY,
        D3D12_SRV_DIMENSION_TEXTURECUBE,
        D3D12_SRV_DIMENSION_TEXTURECUBEARRAY,
        D3D12_SRV_DIMENSION_TEXTURE3D
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_UAV_DIMENSION ToNativeUAVDimension(VIEW_DIMENSION value)
{
    static const D3D12_UAV_DIMENSION table[] = {
        D3D12_UAV_DIMENSION_BUFFER,
        D3D12_UAV_DIMENSION_TEXTURE1D,
        D3D12_UAV_DIMENSION_TEXTURE1DARRAY,
        D3D12_UAV_DIMENSION_TEXTURE2D,
        D3D12_UAV_DIMENSION_TEXTURE2DARRAY,
        D3D12_UAV_DIMENSION_TEXTURE2DARRAY,
        D3D12_UAV_DIMENSION_TEXTURE2DARRAY,
        D3D12_UAV_DIMENSION_TEXTURE3D
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      カラースペースを変換します.
//-------------------------------------------------------------------------------------------------
DXGI_COLOR_SPACE_TYPE ToNativeColorSpace(COLOR_SPACE_TYPE value)
{
    static const DXGI_COLOR_SPACE_TYPE table[] = {
        DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709,
        DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P709,
        DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P2020,
        DXGI_COLOR_SPACE_YCBCR_FULL_G22_NONE_P709_X601,
        DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P601,
        DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P601,
        DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709,
        DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P709,
        DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P2020,
        DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P2020,
        DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020,
        DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_LEFT_P2020,
        DXGI_COLOR_SPACE_RGB_STUDIO_G2084_NONE_P2020,
        DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_TOPLEFT_P2020,
        DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_TOPLEFT_P2020,
        DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P2020,
        DXGI_COLOR_SPACE_CUSTOM
    };

    return table[value];
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

//-------------------------------------------------------------------------------------------------
//      サブリソースサイズを求めます.
//-------------------------------------------------------------------------------------------------
void CalcSubresourceSize
(
    RESOURCE_FORMAT format,
    uint32_t        width,
    uint32_t        height,
    uint32_t        depth,
    uint64_t&       slicePitch,
    uint64_t&       rowPitch,
    uint64_t&       rowCount
)
{
    if (IsCompressFormat( format ))
    {
        auto w = GetBlockWidth ( format );
        auto h = GetBlockHeight( format );
        width  = ( width  + w - 1 ) / w;
        height = ( height + h - 1 ) / h;
    }

    rowPitch    = width * ToByte( format );
    rowCount    = height;
    slicePitch  = rowPitch * rowCount;
}

//-------------------------------------------------------------------------------------------------
//      サブリソースレイアウトを求めます.
//-------------------------------------------------------------------------------------------------
SubresourceLayout CalcSubresourceLayout
(
    uint32_t        subresource,
    RESOURCE_FORMAT format,
    uint32_t        width,
    uint32_t        height,
    uint32_t        depth
)
{
    uint64_t offset   = 0;
    uint64_t size     = 0;
    uint64_t rowCount = 0;
    uint64_t rowPitch = 0;

    auto w = width;
    auto h = height;
    auto d = depth;

    SubresourceLayout result = {};

    for(auto i=0u; i<=subresource; ++i)
    {
        CalcSubresourceSize(format, w, h, d, size, rowPitch, rowCount);
        result.Offset       = offset;
        result.RowCount     = rowCount;
        result.RowPitch     = rowPitch;
        result.Size         = size;
        result.SlicePitch   = size;
    
        offset += size;

        w = w >> 1;
        h = h >> 1;
        d = d >> 1;

        if (w == 0) { w = 1; }
        if (h == 0) { h = 1; }
        if (d == 0) { d = 1; }
    }

    return result;
}


} // namespace a3d
