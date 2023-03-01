//-------------------------------------------------------------------------------------------------
// File : a3dFunc.cpp
// Desc : Common Function.
// Copyright(c) Project Asura. All right reseved.
//-------------------------------------------------------------------------------------------------

namespace {

///////////////////////////////////////////////////////////////////////////////////////////////////
// FormatTable structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct FormatTable
{
    uint32_t    BitPerPixel;        //!< 1ピクセルあたりのビット数です.
    bool        IsCompress;         //!< 圧縮フォーマットかどうか?
};

constexpr FormatTable g_FormatTable[] = {
    { 0     , false },  // 0
    { 128   , false },  // 1
    { 128   , false },  // 2
    { 128   , false },  // 3
    { 64    , false },  // 4
    { 64    , false },  // 5
    { 64    , false },  // 6
    { 96    , false },  // 7
    { 96    , false },  // 8
    { 96    , false },  // 9
    { 64    , false },  // 10
    { 64    , false },  // 11
    { 64    , false },  // 12
    { 32    , false },  // 13
    { 32    , false },  // 14
    { 32    , false },  // 15
    { 32    , false },  // 16
    { 32    , false },  // 17
    { 32    , false },  // 18
    { 32    , false },  // 19
    { 16    , false },  // 20
    { 16    , false },  // 21
    { 32    , false },  // 22
    { 32    , false },  // 23
    { 32    , false },  // 24
    { 32    , false },  // 25
    { 16    , false },  // 26
    { 8     , false },  // 27
    { 32    , false },  // 28
    { 32    , false },  // 29
    { 16    , false },  // 30
    { 8     , true  },  // 31
    { 8     , true  },  // 32
    { 16    , true  },  // 33
    { 16    , true  },  // 34
    { 16    , true  },  // 35
    { 16    , true  },  // 36
    { 8     , true  },  // 37
    { 8     , true  },  // 38
    { 16    , true  },  // 39
    { 16    , true  },  // 40
    { 16    , true  },  // 41
    { 16    , true  },  // 42
    { 16    , true  },  // 43
    { 16    , true  },  // 44
    { 16    , true  },  // 45
    { 16    , true  },  // 46
    { 16    , true  },  // 47
    { 16    , true  },  // 48
    { 16    , true  },  // 49
    { 16    , true  },  // 50
    { 16    , true  },  // 51
    { 16    , true  },  // 52
    { 16    , true  },  // 53
    { 16    , true  },  // 54
    { 16    , true  },  // 55
    { 16    , true  },  // 56
    { 16    , true  },  // 57
    { 16    , true  },  // 58
    { 16    , true  },  // 59
    { 16    , true  },  // 60
    { 16    , true  },  // 61
    { 16    , true  },  // 62
    { 16    , true  },  // 63
    { 16    , true  },  // 64
    { 16    , true  },  // 65
    { 16    , true  },  // 66
    { 16    , true  },  // 67
    { 16    , true  },  // 68
    { 16    , true  },  // 69
    { 16    , true  },  // 70
    { 16    , true  },  // 71
    { 16    , true  },  // 72
    { 32    , false },  // 73
    { 32    , false },  // 74
    { 32    , false },  // 75
    { 64    , false },  // 76
    { 32    , false },  // 77
    { 32    , false },  // 78
};
} // namespace


namespace a3d {

//-------------------------------------------------------------------------------------------------
//      リソースフォーマットのバイト数に変換します.
//-------------------------------------------------------------------------------------------------
uint32_t A3D_APIENTRY ToByte(RESOURCE_FORMAT format)
{ return g_FormatTable[format].BitPerPixel / 8; }

//-------------------------------------------------------------------------------------------------
//      圧縮フォーマットかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool A3D_APIENTRY IsCompressFormat(RESOURCE_FORMAT format)
{ return g_FormatTable[format].IsCompress; }

//-------------------------------------------------------------------------------------------------
//      SRGBフォーマットかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool A3D_APIENTRY IsSRGBFormat(RESOURCE_FORMAT format)
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
uint32_t A3D_APIENTRY GetBlockWidth(RESOURCE_FORMAT format)
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
uint32_t A3D_APIENTRY GetBlockHeight(RESOURCE_FORMAT format)
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
//      サブリソース番号を計算します.
//-------------------------------------------------------------------------------------------------
uint32_t A3D_APIENTRY CalcSubresource
(
    uint32_t    mipSlice,
    uint32_t    arraySlice,
    uint32_t    planeSlice,
    uint32_t    mipLevels,
    uint32_t    arraySize
)
{ return mipSlice + arraySlice * mipLevels + planeSlice * mipLevels * arraySize; }

//-------------------------------------------------------------------------------------------------
//      サブリソース番号からミップ番号，配列番号を求めます.
//-------------------------------------------------------------------------------------------------
void A3D_APIENTRY DecomposeSubresource
(
    uint32_t    subresource,
    uint32_t    mipLevels,
    uint32_t    arraySize,
    uint32_t&   mipSlice,
    uint32_t&   arraySlice,
    uint32_t&   planeSlice
)
{
    mipSlice   = subresource % mipLevels;
    arraySlice = (subresource / mipLevels) % arraySize;
    planeSlice = (subresource / (mipLevels * arraySize));
}

//-------------------------------------------------------------------------------------------------
//      サブリソースサイズを求めます.
//-------------------------------------------------------------------------------------------------
void A3D_APIENTRY CalcSubresourceSize
(
    RESOURCE_FORMAT format,
    uint32_t        width,
    uint32_t        height,
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

    rowPitch    = uint64_t(width) * ToByte( format );
    rowCount    = height;
    slicePitch  = rowPitch * rowCount;
}

//-------------------------------------------------------------------------------------------------
//      サブリソースレイアウトを求めます.
//-------------------------------------------------------------------------------------------------
SubresourceLayout A3D_APIENTRY CalcSubresourceLayout
(
    uint32_t        subresource,
    RESOURCE_FORMAT format,
    uint32_t        width, 
    uint32_t        height
)
{
    uint64_t offset   = 0;
    uint64_t size     = 0;
    uint64_t rowCount = 0;
    uint64_t rowPitch = 0;

    auto w = width;
    auto h = height;

    SubresourceLayout result = {};

    for(auto i=0u; i<=subresource; ++i)
    {
        CalcSubresourceSize(format, w, h, size, rowPitch, rowCount);
        result.Offset       = offset;
        result.RowCount     = rowCount;
        result.RowPitch     = rowPitch;
        result.Size         = size;
        result.SlicePitch   = size;
    
        offset += size;

        w = w >> 1;
        h = h >> 1;

        if (w == 0) { w = 1; }
        if (h == 0) { h = 1; }
    }
    return result;
}

} // namespace a3d
