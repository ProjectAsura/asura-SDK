//-------------------------------------------------------------------------------------------------
// File : ResTexture.cpp
// Desc : Resource Texture Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <ResTexture.h>
#include <dxgiformat.h>
#include <wincodec.h>
#include <wrl.h>
#include <cassert>
#include <memory>
#include <string>
#include <codecvt>
#include <Allocator.h>


//-------------------------------------------------------------------------------------------------
// Linker
//-------------------------------------------------------------------------------------------------
#if defined(_DEBUG) || defined(PROFILE)
#pragma comment(lib,"dxguid.lib")
#endif// defined(_DEBUG) || defined(PROFILE)

#ifndef ELOG
#define ELOG(x, ...) fprintf_s(stderr, "[File:%s, Line:%d] "x"\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif//ELOG


namespace /* aonymous */ {

//-------------------------------------------------------------------------------------------------
// Using Statements
//-------------------------------------------------------------------------------------------------
using Microsoft::WRL::ComPtr;


//-------------------------------------------------------------------------------------------------
// Constant Values.
//-------------------------------------------------------------------------------------------------
//static const uint32_t MAX_TEXTURE_SIZE = 4096;   // D3D_FEATURE_LEVEL_9_3
//static const uint32_t MAX_TEXTURE_SIZE = 8192;   // D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_10_1
static const uint32_t MAX_TEXTURE_SIZE = 16384;  // D3D_FEATURE_LEVEL_11_0

// dwFlags Value
static const unsigned int DDSD_CAPS         = 0x00000001;   // dwCaps/dwCaps2が有効.
static const unsigned int DDSD_HEIGHT       = 0x00000002;   // dwHeightが有効.
static const unsigned int DDSD_WIDTH        = 0x00000004;   // dwWidthが有効.
static const unsigned int DDSD_PITCH        = 0x00000008;   // dwPitchOrLinearSizeがPitchを表す.
static const unsigned int DDSD_PIXELFORMAT  = 0x00001000;   // dwPfSize/dwPfFlags/dwRGB～等の直接定義が有効.
static const unsigned int DDSD_MIPMAPCOUNT  = 0x00020000;   // dwMipMapCountが有効.
static const unsigned int DDSD_LINEARSIZE   = 0x00080000;   // dwPitchOrLinearSizeがLinerSizeを表す.
static const unsigned int DDSD_DEPTH        = 0x00800000;   // dwDepthが有効.

// dwPfFlags Value
static const unsigned int DDPF_ALPHAPIXELS      = 0x00000001;   // RGB以外にalphaが含まれている.
static const unsigned int DDPF_ALPHA            = 0x00000002;   // pixelはAlpha成分のみ.
static const unsigned int DDPF_FOURCC           = 0x00000004;   // dwFourCCが有効.
static const unsigned int DDPF_PALETTE_INDEXED4 = 0x00000008;   // Palette 16 colors.
static const unsigned int DDPF_PALETTE_INDEXED8 = 0x00000020;   // Palette 256 colors.
static const unsigned int DDPF_RGB              = 0x00000040;   // dwRGBBitCount/dwRBitMask/dwGBitMask/dwBBitMask/dwRGBAlphaBitMaskによってフォーマットが定義されていることを示す.
static const unsigned int DDPF_LUMINANCE        = 0x00020000;   // 1chのデータがRGB全てに展開される.
static const unsigned int DDPF_BUMPDUDV         = 0x00080000;   // pixelが符号付きであることを示す.

// dwCaps Value
static const unsigned int DDSCAPS_ALPHA     = 0x00000002;       // Alphaが含まれている場合.
static const unsigned int DDSCAPS_COMPLEX   = 0x00000008;       // 複数のデータが含まれている場合Palette/Mipmap/Cube/Volume等.
static const unsigned int DDSCAPS_TEXTURE   = 0x00001000;       // 常に1.
static const unsigned int DDSCAPS_MIPMAP    = 0x00400000;       // MipMapが存在する場合.

// dwCaps2 Value
static const unsigned int DDSCAPS2_CUBEMAP              = 0x00000200;   // CubeMapが存在する場合.
static const unsigned int DDSCAPS2_CUBEMAP_POSITIVE_X   = 0x00000400;   // CubeMap X+
static const unsigned int DDSCAPS2_CUBEMAP_NEGATIVE_X   = 0x00000800;   // CubeMap X-
static const unsigned int DDSCAPS2_CUBEMAP_POSITIVE_Y   = 0x00001000;   // CubeMap Y+
static const unsigned int DDSCAPS2_CUBEMAP_NEGATIVE_Y   = 0x00002000;   // CubeMap Y-
static const unsigned int DDSCAPS2_CUBEMAP_POSITIVE_Z   = 0x00004000;   // CubeMap Z+
static const unsigned int DDSCAPS2_CUBEMAP_NEGATIVE_Z   = 0x00008000;   // CubeMap Z-
static const unsigned int DDSCAPS2_VOLUME               = 0x00400000;   // VolumeTextureの場合.

// dwFourCC Value
static const unsigned int FOURCC_DXT1           = '1TXD';           // DXT1
static const unsigned int FOURCC_DXT2           = '2TXD';           // DXT2
static const unsigned int FOURCC_DXT3           = '3TXD';           // DXT3
static const unsigned int FOURCC_DXT4           = '4TXD';           // DXT4
static const unsigned int FOURCC_DXT5           = '5TXD';           // DXT5
static const unsigned int FOURCC_ATI1           = '1ITA';           // 3Dc ATI2
static const unsigned int FOURCC_ATI2           = '2ITA';           // 3Dc ATI2
static const unsigned int FOURCC_DX10           = '01XD';           // DX10
static const unsigned int FOURCC_BC4U           = 'U4CB';           // BC4U
static const unsigned int FOURCC_BC4S           = 'S4CB';           // BC4S
static const unsigned int FOURCC_BC5U           = 'U5CB';           // BC5U
static const unsigned int FOURCC_BC5S           = 'S5CB';           // BC5S
static const unsigned int FOURCC_A16B16G16R16   = 0x00000024;
static const unsigned int FOURCC_Q16W16V16U16   = 0x0000006e;
static const unsigned int FOURCC_R16F           = 0x0000006f;
static const unsigned int FOURCC_G16R16F        = 0x00000070;
static const unsigned int FOURCC_A16B16G16R16F  = 0x00000071;
static const unsigned int FOURCC_R32F           = 0x00000072;
static const unsigned int FOURCC_G32R32F        = 0x00000073;
static const unsigned int FOURCC_A32B32G32R32F  = 0x00000074;
static const unsigned int FOURCC_CxV8uint8_t         = 0x00000075;
static const unsigned int FOURCC_Q8W8V8uint8_t       = 0x0000003f;


///////////////////////////////////////////////////////////////////////////////////////////////////
// NATIVE_TEXTURE_FORMAT enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum NATIVE_TEXTURE_FORMAT
{
    NATIVE_TEXTURE_FORMAT_ARGB_8888 = 0,
    NATIVE_TEXTURE_FORMAT_ABGR_8888,
    NATIVE_TEXTURE_FORMAT_XRGB_8888,
    NATIVE_TEXTURE_FORMAT_XBGR_8888,
    NATIVE_TEXTURE_FORMAT_BC1,
    NATIVE_TEXTURE_FORMAT_BC2,
    NATIVE_TEXTURE_FORMAT_BC3,
    NATIVE_TEXTURE_FORMAT_A8,
    NATIVE_TEXTURE_FORMAT_R8,
    NATIVE_TEXTURE_FORMAT_L8A8,
    NATIVE_TEXTURE_FORMAT_BC4U,
    NATIVE_TEXTURE_FORMAT_BC4S,
    NATIVE_TEXTURE_FORMAT_BC5U,
    NATIVE_TEXTURE_FORMAT_BC5S,
    NATIVE_TEXTURE_FORMAT_R16_FLOAT,
    NATIVE_TEXTURE_FORMAT_G16R16_FLOAT,
    NATIVE_TEXTURE_FORMAT_A16B16G16R16_FLOAT,
    NATIVE_TEXTURE_FORMAT_R32_FLOAT,
    NATIVE_TEXTURE_FORMAT_G32R32_FLOAT,
    NATIVE_TEXTURE_FORMAT_A32B32G32R32_FLOAT,
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DDPixelFormat structure
///////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct __DDPixelFormat
{
    unsigned int    size;
    unsigned int    flags;
    unsigned int    fourCC;
    unsigned int    bpp;
    unsigned int    maskR;
    unsigned int    maskG;
    unsigned int    maskB;
    unsigned int    maskA;
} DDPixelFormat;


///////////////////////////////////////////////////////////////////////////////////////////////////
// DDColorKey structure
///////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct __DDColorKey
{
    unsigned int    low;
    unsigned int    hight;
} DDColorKey;


///////////////////////////////////////////////////////////////////////////////////////////////////
// DDSurfaceDesc structure
///////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct __DDSurfaceDesc
{
    unsigned int    size;
    unsigned int    flags;
    unsigned int    height;
    unsigned int    width;
    unsigned int    pitch;
    unsigned int    depth;
    unsigned int    mipMapLevels;
    unsigned int    alphaBitDepth;
    unsigned int    reserved;
    unsigned int    surface;

    DDColorKey      ckDestOverlay;
    DDColorKey      ckDestBit;
    DDColorKey      ckSrcOverlay;
    DDColorKey      ckSrcBit;

    DDPixelFormat   pixelFormat;
    unsigned int    caps;
    unsigned int    caps2;
    unsigned int    reservedCaps[ 2 ];

    unsigned int    textureStage;
} DDSurfaceDesc;

struct DDSHEADER_DX10 
{
    unsigned int    Format;         // DXGI_FORMAT (dxgiformat.h 参照)
    unsigned int    Dimension;      // D3D10_RESOURCE_DIMENSION が入る。(1D=2, 2D=3, 3D=4)
    unsigned int    MiscFlag;       // 0
    unsigned int    ArraySize;      // Texture Array を格納する場合に必要
    unsigned int    MiscFlag2;      // 0
};

enum DDS10_DEIMENSION 
{
    DDS10_DIMENSION_1D = 2,
    DDS10_DIMENSION_2D = 3,
    DDS10_DIMENSION_3D = 4,
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// WIC Pixel Format Translation Data
///////////////////////////////////////////////////////////////////////////////////////////////////
struct WICTranslate
{
    GUID                wic;
    DXGI_FORMAT         format;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// WIC Pixel Format nearest conversion table
///////////////////////////////////////////////////////////////////////////////////////////////////
struct WICConvert
{
    GUID        source;
    GUID        target;
};


//-------------------------------------------------------------------------------------------------
// Global Variables.
//-------------------------------------------------------------------------------------------------
static bool         g_WIC2               = false;
static WICTranslate g_WICFormats[]       = {
    { GUID_WICPixelFormat128bppRGBAFloat,       DXGI_FORMAT_R32G32B32A32_FLOAT },

    { GUID_WICPixelFormat64bppRGBAHalf,         DXGI_FORMAT_R16G16B16A16_FLOAT },
    { GUID_WICPixelFormat64bppRGBA,             DXGI_FORMAT_R16G16B16A16_UNORM },

    { GUID_WICPixelFormat32bppRGBA,             DXGI_FORMAT_R8G8B8A8_UNORM },
    { GUID_WICPixelFormat32bppBGRA,             DXGI_FORMAT_B8G8R8A8_UNORM }, // DXGI 1.1
    { GUID_WICPixelFormat32bppBGR,              DXGI_FORMAT_B8G8R8X8_UNORM }, // DXGI 1.1

    { GUID_WICPixelFormat32bppRGBA1010102XR,    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM }, // DXGI 1.1
    { GUID_WICPixelFormat32bppRGBA1010102,      DXGI_FORMAT_R10G10B10A2_UNORM },

    { GUID_WICPixelFormat16bppBGRA5551,         DXGI_FORMAT_B5G5R5A1_UNORM },
    { GUID_WICPixelFormat16bppBGR565,           DXGI_FORMAT_B5G6R5_UNORM },

    { GUID_WICPixelFormat32bppGrayFloat,        DXGI_FORMAT_R32_FLOAT },
    { GUID_WICPixelFormat16bppGrayHalf,         DXGI_FORMAT_R16_FLOAT },
    { GUID_WICPixelFormat16bppGray,             DXGI_FORMAT_R16_UNORM },
    { GUID_WICPixelFormat8bppGray,              DXGI_FORMAT_R8_UNORM },

    { GUID_WICPixelFormat8bppAlpha,             DXGI_FORMAT_A8_UNORM },
};

static WICConvert g_WICConvert[] = {
    // Note target GUID in this conversion table must be one of those directly supported formats (above).

    { GUID_WICPixelFormatBlackWhite,            GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

    { GUID_WICPixelFormat1bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat2bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat4bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat8bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM

    { GUID_WICPixelFormat2bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM
    { GUID_WICPixelFormat4bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

    { GUID_WICPixelFormat16bppGrayFixedPoint,   GUID_WICPixelFormat16bppGrayHalf }, // DXGI_FORMAT_R16_FLOAT
    { GUID_WICPixelFormat32bppGrayFixedPoint,   GUID_WICPixelFormat32bppGrayFloat }, // DXGI_FORMAT_R32_FLOAT

    { GUID_WICPixelFormat16bppBGR555,           GUID_WICPixelFormat16bppBGRA5551 }, // DXGI_FORMAT_B5G5R5A1_UNORM

    { GUID_WICPixelFormat32bppBGR101010,        GUID_WICPixelFormat32bppRGBA1010102 }, // DXGI_FORMAT_R10G10B10A2_UNORM

    { GUID_WICPixelFormat24bppBGR,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat24bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat32bppPBGRA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat32bppPRGBA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM

    { GUID_WICPixelFormat48bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat48bppBGR,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat64bppBGRA,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat64bppPRGBA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat64bppPBGRA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

    { GUID_WICPixelFormat48bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat48bppBGRFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat64bppRGBAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat64bppBGRAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat64bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat64bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat48bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT

    { GUID_WICPixelFormat128bppPRGBAFloat,      GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
    { GUID_WICPixelFormat128bppRGBFloat,        GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
    { GUID_WICPixelFormat128bppRGBAFixedPoint,  GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
    { GUID_WICPixelFormat128bppRGBFixedPoint,   GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
    { GUID_WICPixelFormat32bppRGBE,             GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT

    { GUID_WICPixelFormat32bppCMYK,             GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat64bppCMYK,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat40bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat80bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
    { GUID_WICPixelFormat32bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat64bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat64bppPRGBAHalf,        GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
#endif

    // We don't support n-channel formats
};

#if 1
// Viewerアプリ用カスタマイズ処理
//  テクスチャフェッチを正しく行うために，1チャンネルのものを強制的に4チャンネルフォーマットに変換する.
static WICConvert g_WICCustomConvert[] = {
    // Note target GUID in this conversion table must be one of those directly supported formats (above).

    { GUID_WICPixelFormatBlackWhite,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8_UNORM

    { GUID_WICPixelFormat2bppGray,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8_UNORM
    { GUID_WICPixelFormat4bppGray,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8_UNORM

    { GUID_WICPixelFormat16bppGrayFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16_FLOAT
    { GUID_WICPixelFormat32bppGrayFixedPoint,   GUID_WICPixelFormat32bppGrayFloat }, // DXGI_FORMAT_R32_FLOAT

    { GUID_WICPixelFormat32bppGrayFloat,        GUID_WICPixelFormat128bppRGBAFloat },
    { GUID_WICPixelFormat16bppGrayHalf,         GUID_WICPixelFormat64bppRGBAHalf },
    { GUID_WICPixelFormat16bppGray,             GUID_WICPixelFormat64bppRGBAHalf },
    { GUID_WICPixelFormat8bppGray,              GUID_WICPixelFormat32bppRGBA },

    { GUID_WICPixelFormat8bppAlpha,             GUID_WICPixelFormat32bppRGBA },
};
#endif

//-------------------------------------------------------------------------------------------------
//          nullptrを考慮して破棄します.
//-------------------------------------------------------------------------------------------------
template<typename T> 
void SafeDeleteArray(T*& ptr)
{
    if (ptr != nullptr)
    {
        delete[] ptr;
        ptr = nullptr;
    }
}

//-------------------------------------------------------------------------------------------------
static IWICImagingFactory* GetWIC()
{
    static IWICImagingFactory* s_Factory = nullptr;

    if ( s_Factory )
        return s_Factory;

  #if(_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory2,
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(IWICImagingFactory2),
        (LPVOID*)&s_Factory
        );

    if ( SUCCEEDED(hr) )
    {
        // WIC2 is available on Windows 8 and Windows 7 SP1 with KB 2670838 installed
        g_WIC2 = true;
    }
    else
    {
        hr = CoCreateInstance(
            CLSID_WICImagingFactory1,
            nullptr,
            CLSCTX_INPROC_SERVER,
            __uuidof(IWICImagingFactory),
            (LPVOID*)&s_Factory
            );

        if ( FAILED(hr) )
        {
            s_Factory = nullptr;
            return nullptr;
        }
    }
  #else
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(IWICImagingFactory),
        (LPVOID*)&s_Factory
        );

    if ( FAILED(hr) )
    {
        s_Factory = nullptr;
        return nullptr;
    }
  #endif

    return s_Factory;
}

//-------------------------------------------------------------------------------------------------
static DXGI_FORMAT WICToDXGI( const GUID& guid )
{
    for( size_t i=0; i < _countof(g_WICFormats); ++i )
    {
        if ( memcmp( &g_WICFormats[i].wic, &guid, sizeof(GUID) ) == 0 )
            return g_WICFormats[i].format;
    }

  #if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
    if ( g_WIC2 )
    {
        if ( memcmp( &GUID_WICPixelFormat96bppRGBFloat, &guid, sizeof(GUID) ) == 0 )
            return DXGI_FORMAT_R32G32B32_FLOAT;
    }
  #endif

    return DXGI_FORMAT_UNKNOWN;
}

//-------------------------------------------------------------------------------------------------
static size_t WICBitsPerPixel( REFGUID targetGuid )
{
    IWICImagingFactory* pWIC = GetWIC();
    if ( !pWIC )
        return 0;

    ComPtr<IWICComponentInfo> cinfo;
    if ( FAILED( pWIC->CreateComponentInfo( targetGuid, cinfo.GetAddressOf() ) ) )
        return 0;

    WICComponentType type;
    if ( FAILED( cinfo->GetComponentType( &type ) ) )
        return 0;

    if ( type != WICPixelFormat )
        return 0;

    ComPtr<IWICPixelFormatInfo> pfinfo;
    if ( FAILED( cinfo.As( &pfinfo ) ) )
        return 0;

    UINT bpp;
    if ( FAILED( pfinfo->GetBitsPerPixel( &bpp ) ) )
        return 0;

    return bpp;
}


//-------------------------------------------------------------------------------------------------
static DXGI_FORMAT MakeSRGB( _In_ DXGI_FORMAT format )
{
    switch( format )
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    case DXGI_FORMAT_BC1_UNORM:
        return DXGI_FORMAT_BC1_UNORM_SRGB;

    case DXGI_FORMAT_BC2_UNORM:
        return DXGI_FORMAT_BC2_UNORM_SRGB;

    case DXGI_FORMAT_BC3_UNORM:
        return DXGI_FORMAT_BC3_UNORM_SRGB;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
        return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

    case DXGI_FORMAT_B8G8R8X8_UNORM:
        return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

    case DXGI_FORMAT_BC7_UNORM:
        return DXGI_FORMAT_BC7_UNORM_SRGB;

    default:
        return format;
    }
}


//-------------------------------------------------------------------------------------------------
//      マスクをチェックします.
//-------------------------------------------------------------------------------------------------
bool CheckMask
(
    const DDPixelFormat& pixelFormat,
    unsigned int maskR,
    unsigned int maskG,
    unsigned int maskB,
    unsigned int maskA
)
{
    if ( ( pixelFormat.maskR == maskR )
      && ( pixelFormat.maskG == maskG )
      && ( pixelFormat.maskB == maskB )
      && ( pixelFormat.maskA == maskA ) )
    { return true; }

    return false;
}

size_t GetBitPerPixel( unsigned int format )
{
    switch( format )
    {
    case NATIVE_TEXTURE_FORMAT_R8:
    case NATIVE_TEXTURE_FORMAT_A8:
        { return 8; }

    case NATIVE_TEXTURE_FORMAT_ARGB_8888:
    case NATIVE_TEXTURE_FORMAT_ABGR_8888:
    case NATIVE_TEXTURE_FORMAT_XRGB_8888:
    case NATIVE_TEXTURE_FORMAT_XBGR_8888:
        { return 32; }

    case NATIVE_TEXTURE_FORMAT_BC1:
    case NATIVE_TEXTURE_FORMAT_BC4U:
    case NATIVE_TEXTURE_FORMAT_BC4S:
        { return 4; }

    case NATIVE_TEXTURE_FORMAT_BC2:
    case NATIVE_TEXTURE_FORMAT_BC3:
    case NATIVE_TEXTURE_FORMAT_BC5U:
    case NATIVE_TEXTURE_FORMAT_BC5S:
        { return 8; }

    case NATIVE_TEXTURE_FORMAT_R16_FLOAT:
        { return 16; }

    case NATIVE_TEXTURE_FORMAT_G16R16_FLOAT:
        { return 32; }

    case NATIVE_TEXTURE_FORMAT_A16B16G16R16_FLOAT:
        { return 64; }

    case NATIVE_TEXTURE_FORMAT_R32_FLOAT:
        { return 32; }

    case NATIVE_TEXTURE_FORMAT_G32R32_FLOAT:
        { return 64; }

    case NATIVE_TEXTURE_FORMAT_A32B32G32R32_FLOAT:
        { return 128; }

    default:
        { return 0; }
    }
}

//-------------------------------------------------------------------------------------------------
//      最大値を取得します.
//-------------------------------------------------------------------------------------------------
template<typename T>
T Max( const T a, const T b )
{ return ( a > b ) ? a : b; }


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

FormatConvertTable g_FormatTable[] = {
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 0     , a3d::RESOURCE_FORMAT_UNKNOWN               , false },  // 0
    { DXGI_FORMAT_R32G32B32A32_FLOAT        , DXGI_FORMAT_R32G32B32A32_TYPELESS     , 128   , a3d::RESOURCE_FORMAT_R32G32B32A32_FLOAT    , false },  // 1
    { DXGI_FORMAT_R32G32B32A32_UINT         , DXGI_FORMAT_R32G32B32A32_TYPELESS     , 128   , a3d::RESOURCE_FORMAT_R32G32B32A32_UINT     , false },  // 2
    { DXGI_FORMAT_R32G32B32A32_SINT         , DXGI_FORMAT_R32G32B32A32_TYPELESS     , 128   , a3d::RESOURCE_FORMAT_R32G32B32A32_SINT     , false },  // 3
    { DXGI_FORMAT_R16G16B16A16_FLOAT        , DXGI_FORMAT_R16G16B16A16_TYPELESS     , 64    , a3d::RESOURCE_FORMAT_R16G16B16A16_FLOAT    , false },  // 4
    { DXGI_FORMAT_R16G16B16A16_UINT         , DXGI_FORMAT_R16G16B16A16_TYPELESS     , 64    , a3d::RESOURCE_FORMAT_R16G16B16A16_UINT     , false },  // 5
    { DXGI_FORMAT_R16G16B16A16_SINT         , DXGI_FORMAT_R16G16B16A16_TYPELESS     , 64    , a3d::RESOURCE_FORMAT_R16G16B16A16_SINT     , false },  // 6
    { DXGI_FORMAT_R32G32B32_FLOAT           , DXGI_FORMAT_R32G32B32_TYPELESS        , 96    , a3d::RESOURCE_FORMAT_R32G32B32_FLOAT       , false },  // 7
    { DXGI_FORMAT_R32G32B32_UINT            , DXGI_FORMAT_R32G32B32_TYPELESS        , 96    , a3d::RESOURCE_FORMAT_R32G32B32_UINT        , false },  // 8
    { DXGI_FORMAT_R32G32B32_SINT            , DXGI_FORMAT_R32G32B32_TYPELESS        , 96    , a3d::RESOURCE_FORMAT_R32G32B32_SINT        , false },  // 9
    { DXGI_FORMAT_R32G32_FLOAT              , DXGI_FORMAT_R32G32_TYPELESS           , 64    , a3d::RESOURCE_FORMAT_R32G32_FLOAT          , false },  // 10
    { DXGI_FORMAT_R32G32_UINT               , DXGI_FORMAT_R32G32_TYPELESS           , 64    , a3d::RESOURCE_FORMAT_R32G32_UINT           , false },  // 11
    { DXGI_FORMAT_R32G32_SINT               , DXGI_FORMAT_R32G32_TYPELESS           , 64    , a3d::RESOURCE_FORMAT_R32G32_SINT           , false },  // 12
    { DXGI_FORMAT_R16G16_FLOAT              , DXGI_FORMAT_R16G16_TYPELESS           , 32    , a3d::RESOURCE_FORMAT_R16G16_FLOAT          , false },  // 13
    { DXGI_FORMAT_R16G16_UINT               , DXGI_FORMAT_R16G16_TYPELESS           , 32    , a3d::RESOURCE_FORMAT_R16G16_UINT           , false },  // 14
    { DXGI_FORMAT_R16G16_SINT               , DXGI_FORMAT_R16G16_TYPELESS           , 32    , a3d::RESOURCE_FORMAT_R16G16_SINT           , false },  // 15
    { DXGI_FORMAT_R32_FLOAT                 , DXGI_FORMAT_R32_TYPELESS              , 32    , a3d::RESOURCE_FORMAT_R32_FLOAT             , false },  // 16
    { DXGI_FORMAT_R32_UINT                  , DXGI_FORMAT_R32_TYPELESS              , 32    , a3d::RESOURCE_FORMAT_R32_UINT              , false },  // 17
    { DXGI_FORMAT_R32_SINT                  , DXGI_FORMAT_R32_TYPELESS              , 32    , a3d::RESOURCE_FORMAT_R32_SINT              , false },  // 18
    { DXGI_FORMAT_R16_FLOAT                 , DXGI_FORMAT_R16_TYPELESS              , 16    , a3d::RESOURCE_FORMAT_R16_FLOAT             , false },  // 19
    { DXGI_FORMAT_R16_UINT                  , DXGI_FORMAT_R16_TYPELESS              , 16    , a3d::RESOURCE_FORMAT_R16_UINT              , false },  // 20
    { DXGI_FORMAT_R16_SINT                  , DXGI_FORMAT_R16_TYPELESS              , 16    , a3d::RESOURCE_FORMAT_R16_SINT              , false },  // 21
    { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB       , DXGI_FORMAT_R8G8B8A8_TYPELESS         , 32    , a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM_SRGB   , false },  // 22
    { DXGI_FORMAT_R8G8B8A8_UNORM            , DXGI_FORMAT_R8G8B8A8_TYPELESS         , 32    , a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM        , false },  // 23
    { DXGI_FORMAT_B8G8R8A8_UNORM_SRGB       , DXGI_FORMAT_B8G8R8A8_TYPELESS         , 32    , a3d::RESOURCE_FORMAT_B8G8R8A8_UNORM_SRGB   , false },  // 24
    { DXGI_FORMAT_B8G8R8A8_UNORM            , DXGI_FORMAT_B8G8R8A8_TYPELESS         , 32    , a3d::RESOURCE_FORMAT_B8G8R8A8_UNORM        , false },  // 25
    { DXGI_FORMAT_R8G8_UNORM                , DXGI_FORMAT_R8G8_TYPELESS             , 16    , a3d::RESOURCE_FORMAT_R8G8_UNORM            , false },  // 26
    { DXGI_FORMAT_R8_UNORM                  , DXGI_FORMAT_R8_TYPELESS               , 8     , a3d::RESOURCE_FORMAT_R8_UNORM              , false },  // 27
    { DXGI_FORMAT_D32_FLOAT                 , DXGI_FORMAT_R32_TYPELESS              , 32    , a3d::RESOURCE_FORMAT_D32_FLOAT             , false },  // 28
    { DXGI_FORMAT_D24_UNORM_S8_UINT         , DXGI_FORMAT_R24G8_TYPELESS            , 32    , a3d::RESOURCE_FORMAT_D24_UNORM_S8_UINT     , false },  // 29
    { DXGI_FORMAT_D16_UNORM                 , DXGI_FORMAT_R16_TYPELESS              , 16    , a3d::RESOURCE_FORMAT_D16_UNORM             , false },  // 30
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
    { DXGI_FORMAT_BC7_UNORM                 , DXGI_FORMAT_BC7_TYPELESS              , 16    , a3d::RESOURCE_FORMAT_BC7_UNORM             , true  },  // 44
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
    { DXGI_FORMAT_R10G10B10A2_UNORM         , DXGI_FORMAT_R10G10B10A2_TYPELESS      , 32    , a3d::RESOURCE_FORMAT_R10G10B10A2_UNORM     , false },  // 73
    { DXGI_FORMAT_R10G10B10A2_UINT          , DXGI_FORMAT_R10G10B10A2_TYPELESS      , 32    , a3d::RESOURCE_FORMAT_R10G10B10A2_UINT      , false },  // 74
    { DXGI_FORMAT_R11G11B10_FLOAT           , DXGI_FORMAT_R11G11B10_FLOAT           , 32    , a3d::RESOURCE_FORMAT_R11G11B10_FLOAT       , false },  // 75
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 64    , a3d::RESOURCE_FORMAT_B16G16R16A16_FLOAT    , false },  // 76
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 32    , a3d::RESOURCE_FORMAT_B10G10R10A2_UNORM     , false },  // 77
    { DXGI_FORMAT_UNKNOWN                   , DXGI_FORMAT_UNKNOWN                   , 32    , a3d::RESOURCE_FORMAT_B10G10R10A2_UINT      , false }   // 78
};

a3d::RESOURCE_FORMAT ToWrapFormat(DXGI_FORMAT format)
{
    for(auto& t : g_FormatTable)
    {
        if (format == t.NativeFormat)
        { return t.Format; }
    }

    return a3d::RESOURCE_FORMAT_UNKNOWN;
}

} // namespace /* anonymous */


namespace a3dx {

//-------------------------------------------------------------------------------------------------
//      WICからリソーステクスチャを生成します.
//      (BMP, JPEG, PNG, TIFF, GIF, HD Photoファイルからリソーステクスチャを生成します).
//-------------------------------------------------------------------------------------------------
bool CreateResTextureFromWICFile( const char* filename, a3d::ResTexture& resTexture )
{
    bool forceSRGB = true;

    if ( filename == nullptr )
    { return false; }

    IWICImagingFactory* pWIC = GetWIC();
    if ( !pWIC )
    { return false; }

    std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> cv;
    std::wstring path = cv.from_bytes(filename);

    // WICの初期化.
    ComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = pWIC->CreateDecoderFromFilename( path.c_str(), 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf() );
    if ( FAILED( hr ) )
    { return false; }

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame( 0, frame.GetAddressOf() );
    if ( FAILED( hr ) )
    { return false; }

    uint32_t width  = 0;
    uint32_t height = 0;
    hr = frame->GetSize( &width, &height );
    if ( FAILED( hr ) )
    { return false; }

    assert( width > 0 && height > 0 );

    uint32_t origWidth  = width;
    uint32_t origHeight = height;

    if ( width  > MAX_TEXTURE_SIZE
      || height > MAX_TEXTURE_SIZE )
    {
        float ar = float( height ) / float( width );
        if ( width > height )
        {
            width  = MAX_TEXTURE_SIZE;
            height = uint32_t( float( MAX_TEXTURE_SIZE ) * ar );
        }
        else
        {
            width = uint32_t( float( MAX_TEXTURE_SIZE ) / ar );
            height = MAX_TEXTURE_SIZE;
        }
    }

    WICPixelFormatGUID pixelFormat;
    hr = frame->GetPixelFormat( &pixelFormat );
    if ( FAILED( hr ) )
    { return false; }

    WICPixelFormatGUID convertGUID;
    memcpy( &convertGUID, &pixelFormat, sizeof(WICPixelFormatGUID) );

    size_t bpp = 0;
    DXGI_FORMAT format = WICToDXGI( pixelFormat );
    if ( format == DXGI_FORMAT_UNKNOWN )
    {
        if ( memcmp( &GUID_WICPixelFormat96bppRGBFixedPoint, &pixelFormat, sizeof(WICPixelFormatGUID) ) == 0 )
        {
        #if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
            if ( g_WIC2 )
            {
                memcpy( &convertGUID, &GUID_WICPixelFormat96bppRGBFloat, sizeof(WICPixelFormatGUID) );
                format = DXGI_FORMAT_R32G32B32_FLOAT;
            }
            else
        #endif
            {
                memcpy( &convertGUID, &GUID_WICPixelFormat128bppRGBAFloat, sizeof(WICPixelFormatGUID) );
                format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            }
        }
        else
        {
            for( size_t i=0; i < _countof(g_WICConvert); ++i )
            {
                if ( memcmp( &g_WICConvert[i].source, &pixelFormat, sizeof(WICPixelFormatGUID) ) == 0 )
                {
                    memcpy( &convertGUID, &g_WICConvert[i].target, sizeof(WICPixelFormatGUID) );

                    format = WICToDXGI( g_WICConvert[i].target );
                    assert( format != DXGI_FORMAT_UNKNOWN );
                    bpp = WICBitsPerPixel( convertGUID );
                    break;
                }
            }
        }

        if ( format == DXGI_FORMAT_UNKNOWN )
        { return false; }
    }
#if 1
    // Viewerアプリ用の専用処理.
    //  テクスチャフェッチをした結果が正しくなるようにチャンネル数を4チャンネルにする.
    else if ( format == DXGI_FORMAT_R8_UNORM
           || format == DXGI_FORMAT_A8_UNORM
           || format == DXGI_FORMAT_R16_UNORM
           || format == DXGI_FORMAT_R16_FLOAT
           || format == DXGI_FORMAT_R32_FLOAT )
    {
        for( size_t i=0; i < _countof(g_WICCustomConvert); ++i )
        {
            if ( memcmp( &g_WICCustomConvert[i].source, &pixelFormat, sizeof(WICPixelFormatGUID) ) == 0 )
            {
                memcpy( &convertGUID, &g_WICCustomConvert[i].target, sizeof(WICPixelFormatGUID) );

                format = WICToDXGI( g_WICCustomConvert[i].target );
                assert( format != DXGI_FORMAT_UNKNOWN );
                bpp = WICBitsPerPixel( convertGUID );
                break;
            }
        }

        if ( format == DXGI_FORMAT_UNKNOWN )
        { return false; }
    }
#endif
    else
    {
        bpp = WICBitsPerPixel( pixelFormat );
    }

    if ( !bpp )
    { return false; }

    // Handle sRGB formats
    if ( forceSRGB )
    {
        format = MakeSRGB( format );
    }
    else
    {
        ComPtr<IWICMetadataQueryReader> metareader;
        if ( SUCCEEDED( frame->GetMetadataQueryReader( metareader.GetAddressOf() ) ) )
        {
            GUID containerFormat;
            if ( SUCCEEDED( metareader->GetContainerFormat( &containerFormat ) ) )
            {
                // Check for sRGB colorspace metadata
                bool sRGB = false;

                PROPVARIANT value;
                PropVariantInit( &value );

                if ( memcmp( &containerFormat, &GUID_ContainerFormatPng, sizeof(GUID) ) == 0 )
                {
                    // Check for sRGB chunk
                    if ( SUCCEEDED( metareader->GetMetadataByName( L"/sRGB/RenderingIntent", &value ) ) && value.vt == VT_UI1 )
                    {
                        sRGB = true;
                    }
                }
                else if ( SUCCEEDED( metareader->GetMetadataByName( L"System.Image.ColorSpace", &value ) ) && value.vt == VT_UI2 && value.uiVal == 1 )
                {
                    sRGB = true;
                }

                PropVariantClear( &value );

                if ( sRGB )
                { format = MakeSRGB( format ); }
            }
        }
    }

    // １行当たりのバイト数.
    size_t rowPitch = ( width * bpp + 7 ) / 8;

    // ピクセルデータのサイズ.
    size_t imageSize = rowPitch * height;

    // ピクセルデータのメモリを確保.
    uint8_t* pPixels = new (std::nothrow) uint8_t [ imageSize ];
    if ( !pPixels )
    { return false; }

    // 元サイズと同じ場合.
    if ( 0 == memcmp( &convertGUID, &pixelFormat, sizeof(GUID) )
      && width  == origWidth
      && height == origHeight )
    {
        hr = frame->CopyPixels( 0, uint32_t( rowPitch ), uint32_t( imageSize ), pPixels );
        if ( FAILED( hr ) )
        {
            SafeDeleteArray( pPixels );
            return false;
        }
    }
    else if ( width != origWidth
          || height != origHeight )
    {
        // リサイズ処理.

        ComPtr<IWICBitmapScaler> scaler;
        hr = pWIC->CreateBitmapScaler( scaler.GetAddressOf() );
        if ( FAILED( hr ) )
        {
            SafeDeleteArray( pPixels );
            return false;
        }

        hr = scaler->Initialize( frame.Get(), width, height, WICBitmapInterpolationModeFant );
        if ( FAILED( hr ) )
        {
            SafeDeleteArray( pPixels );
            return false;
        }

        WICPixelFormatGUID pfScalar;
        hr = scaler->GetPixelFormat( &pfScalar );
        if ( FAILED( hr ) )
        {
            SafeDeleteArray( pPixels );
            return false;
        }

        if ( 0 == memcmp( &convertGUID, &pfScalar, sizeof(GUID) ) )
        {
            hr = scaler->CopyPixels( 0, uint32_t( rowPitch ), uint32_t( imageSize ), pPixels );
            if ( FAILED( hr ) )
            {
                SafeDeleteArray( pPixels );
                return false;
            }
        }
        else
        {
            ComPtr<IWICFormatConverter> conv;
            hr =  pWIC->CreateFormatConverter( conv.GetAddressOf() );
            if ( FAILED( hr ) )
            {
                SafeDeleteArray( pPixels );
                return false;
            }

            hr = conv->Initialize( scaler.Get(), convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom );
            if ( FAILED( hr ) )
            {
                SafeDeleteArray( pPixels );
                return false;
            }

            hr = conv->CopyPixels( 0, uint32_t( rowPitch ), uint32_t( imageSize ), pPixels );
            if ( FAILED( hr ) )
            {
                SafeDeleteArray( pPixels );
                return false;
            }
        }
    }
    else
    {
        // フォーマットのみが違う場合.

        ComPtr<IWICFormatConverter> conv;
        hr = pWIC->CreateFormatConverter( conv.GetAddressOf() );
        if ( FAILED( hr ) )
        {
            SafeDeleteArray( pPixels );
            return false;
        }

        hr = conv->Initialize( frame.Get(), convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom );
        if ( FAILED( hr ) )
        {
            SafeDeleteArray( pPixels );
            return false;
        }

        hr = conv->CopyPixels( 0, uint32_t( rowPitch ), uint32_t( imageSize ), pPixels );
        if ( FAILED( hr ) )
        {
            SafeDeleteArray( pPixels );
            return false;
        }
    }

    // リソーステクスチャを設定.
    resTexture.Dimension        = a3d::RESOURCE_DIMENSION_TEXTURE2D;
    resTexture.Width            = width;
    resTexture.Height           = height;
    resTexture.DepthOrArraySize = 1;
    resTexture.Format           = ToWrapFormat(format);
    resTexture.MipLevels        = 1;
    if (!resTexture.Surfaces.init(Allocator::Instance(), 1))
    {
        a3d::DisposeTexture(resTexture);
        return false;
    }

    resTexture.Surfaces[0].RowPitch     = uint32_t(rowPitch);
    resTexture.Surfaces[0].SlicePitch   = uint32_t(imageSize);
    resTexture.Surfaces[0].MipIndex     = 0;
    if (!resTexture.Surfaces[0].Pixels.init(Allocator::Instance(), uint32_t(imageSize)))
    {
        a3d::DisposeTexture(resTexture);
        return false;
    }

    memcpy(resTexture.Surfaces[0].Pixels.data(), pPixels, imageSize);
    SafeDeleteArray( pPixels );

    // 正常終了.
    return true;
}



//-------------------------------------------------------------------------------------------------
//      WICからリソーステクスチャを生成します.
//      (BMP, JPEG, PNG, TIFF, GIF, HD Photoファイルからリソーステクスチャを生成します).
//-------------------------------------------------------------------------------------------------
bool CreateResTextureFromWICMemory( const uint8_t* pBinary, const uint32_t bufferSize, a3d::ResTexture& resTexture )
{
    bool forceSRGB = true;

    if ( pBinary == nullptr )
    { return false; }

    IWICImagingFactory* pWIC = GetWIC();
    if ( !pWIC )
    { return false; }

    ComPtr<IWICStream> pStream;
    HRESULT hr = pWIC->CreateStream( &pStream );
    if ( FAILED( hr ) )
    { return false; }

    hr = pStream->InitializeFromMemory( (BYTE*)pBinary, bufferSize );
    if ( FAILED( hr ) )
    { return false; }

    // WICの初期化.
    ComPtr<IWICBitmapDecoder> decoder;
    hr = pWIC->CreateDecoderFromStream( pStream.Get(), 0, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf() );
    if ( FAILED( hr ) )
    { return false; }

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame( 0, frame.GetAddressOf() );
    if ( FAILED( hr ) )
    { return false; }

    uint32_t width  = 0;
    uint32_t height = 0;
    hr = frame->GetSize( &width, &height );
    if ( FAILED( hr ) )
    { return false; }

    assert( width > 0 && height > 0 );

    uint32_t origWidth  = width;
    uint32_t origHeight = height;

    if ( width  > MAX_TEXTURE_SIZE
      || height > MAX_TEXTURE_SIZE )
    {
        float ar = float( height ) / float( width );
        if ( width > height )
        {
            width  = MAX_TEXTURE_SIZE;
            height = uint32_t( float( MAX_TEXTURE_SIZE ) * ar );
        }
        else
        {
            width = uint32_t( float( MAX_TEXTURE_SIZE ) / ar );
            height = MAX_TEXTURE_SIZE;
        }
    }

    WICPixelFormatGUID pixelFormat;
    hr = frame->GetPixelFormat( &pixelFormat );
    if ( FAILED( hr ) )
    { return false; }

    WICPixelFormatGUID convertGUID;
    memcpy( &convertGUID, &pixelFormat, sizeof(WICPixelFormatGUID) );

    size_t bpp = 0;
    DXGI_FORMAT format = WICToDXGI( pixelFormat );
    if ( format == DXGI_FORMAT_UNKNOWN )
    {
        if ( memcmp( &GUID_WICPixelFormat96bppRGBFixedPoint, &pixelFormat, sizeof(WICPixelFormatGUID) ) == 0 )
        {
        #if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
            if ( g_WIC2 )
            {
                memcpy( &convertGUID, &GUID_WICPixelFormat96bppRGBFloat, sizeof(WICPixelFormatGUID) );
                format = DXGI_FORMAT_R32G32B32_FLOAT;
            }
            else
        #endif
            {
                memcpy( &convertGUID, &GUID_WICPixelFormat128bppRGBAFloat, sizeof(WICPixelFormatGUID) );
                format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            }
        }
        else
        {
            for( size_t i=0; i < _countof(g_WICConvert); ++i )
            {
                if ( memcmp( &g_WICConvert[i].source, &pixelFormat, sizeof(WICPixelFormatGUID) ) == 0 )
                {
                    memcpy( &convertGUID, &g_WICConvert[i].target, sizeof(WICPixelFormatGUID) );

                    format = WICToDXGI( g_WICConvert[i].target );
                    assert( format != DXGI_FORMAT_UNKNOWN );
                    bpp = WICBitsPerPixel( convertGUID );
                    break;
                }
            }
        }

        if ( format == DXGI_FORMAT_UNKNOWN )
        { return false; }
    }
#if 1
    // Viewerアプリ用の専用処理.
    //  テクスチャフェッチをした結果が正しくなるようにチャンネル数を4チャンネルにする.
    else if ( format == DXGI_FORMAT_R8_UNORM
           || format == DXGI_FORMAT_A8_UNORM
           || format == DXGI_FORMAT_R16_UNORM
           || format == DXGI_FORMAT_R16_FLOAT
           || format == DXGI_FORMAT_R32_FLOAT )
    {
        for( size_t i=0; i < _countof(g_WICCustomConvert); ++i )
        {
            if ( memcmp( &g_WICCustomConvert[i].source, &pixelFormat, sizeof(WICPixelFormatGUID) ) == 0 )
            {
                memcpy( &convertGUID, &g_WICCustomConvert[i].target, sizeof(WICPixelFormatGUID) );

                format = WICToDXGI( g_WICCustomConvert[i].target );
                assert( format != DXGI_FORMAT_UNKNOWN );
                bpp = WICBitsPerPixel( convertGUID );
                break;
            }
        }

        if ( format == DXGI_FORMAT_UNKNOWN )
        { return false; }
    }
#endif
    else
    {
        bpp = WICBitsPerPixel( pixelFormat );
    }

    if ( !bpp )
    { return false; }

    // Handle sRGB formats
    if ( forceSRGB )
    {
        format = MakeSRGB( format );
    }
    else
    {
        ComPtr<IWICMetadataQueryReader> metareader;
        if ( SUCCEEDED( frame->GetMetadataQueryReader( metareader.GetAddressOf() ) ) )
        {
            GUID containerFormat;
            if ( SUCCEEDED( metareader->GetContainerFormat( &containerFormat ) ) )
            {
                // Check for sRGB colorspace metadata
                bool sRGB = false;

                PROPVARIANT value;
                PropVariantInit( &value );

                if ( memcmp( &containerFormat, &GUID_ContainerFormatPng, sizeof(GUID) ) == 0 )
                {
                    // Check for sRGB chunk
                    if ( SUCCEEDED( metareader->GetMetadataByName( L"/sRGB/RenderingIntent", &value ) ) && value.vt == VT_UI1 )
                    {
                        sRGB = true;
                    }
                }
                else if ( SUCCEEDED( metareader->GetMetadataByName( L"System.Image.ColorSpace", &value ) ) && value.vt == VT_UI2 && value.uiVal == 1 )
                {
                    sRGB = true;
                }

                PropVariantClear( &value );

                if ( sRGB )
                { format = MakeSRGB( format ); }
            }
        }
    }

    // １行当たりのバイト数.
    size_t rowPitch = ( width * bpp + 7 ) / 8;

    // ピクセルデータのサイズ.
    size_t imageSize = rowPitch * height;

    // ピクセルデータのメモリを確保.
    uint8_t* pPixels = new (std::nothrow) uint8_t [ imageSize ];
    if ( !pPixels )
    { return false; }

    // 元サイズと同じ場合.
    if ( 0 == memcmp( &convertGUID, &pixelFormat, sizeof(GUID) )
      && width  == origWidth
      && height == origHeight )
    {
        hr = frame->CopyPixels( 0, uint32_t( rowPitch ), uint32_t( imageSize ), pPixels );
        if ( FAILED( hr ) )
        {
            SafeDeleteArray( pPixels );
            return false;
        }
    }
    else if ( width != origWidth
          || height != origHeight )
    {
        // リサイズ処理.

        ComPtr<IWICBitmapScaler> scaler;
        hr = pWIC->CreateBitmapScaler( scaler.GetAddressOf() );
        if ( FAILED( hr ) )
        {
            SafeDeleteArray( pPixels );
            return false;
        }

        hr = scaler->Initialize( frame.Get(), width, height, WICBitmapInterpolationModeFant );
        if ( FAILED( hr ) )
        {
            SafeDeleteArray( pPixels );
            return false;
        }

        WICPixelFormatGUID pfScalar;
        hr = scaler->GetPixelFormat( &pfScalar );
        if ( FAILED( hr ) )
        {
            SafeDeleteArray( pPixels );
            return false;
        }

        if ( 0 == memcmp( &convertGUID, &pfScalar, sizeof(GUID) ) )
        {
            hr = scaler->CopyPixels( 0, uint32_t( rowPitch ), uint32_t( imageSize ), pPixels );
            if ( FAILED( hr ) )
            {
                SafeDeleteArray( pPixels );
                return false;
            }
        }
        else
        {
            ComPtr<IWICFormatConverter> conv;
            hr =  pWIC->CreateFormatConverter( conv.GetAddressOf() );
            if ( FAILED( hr ) )
            {
                SafeDeleteArray( pPixels );
                return false;
            }

            hr = conv->Initialize( scaler.Get(), convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom );
            if ( FAILED( hr ) )
            {
                SafeDeleteArray( pPixels );
                return false;
            }

            hr = conv->CopyPixels( 0, uint32_t( rowPitch ), uint32_t( imageSize ), pPixels );
            if ( FAILED( hr ) )
            {
                SafeDeleteArray( pPixels );
                return false;
            }
        }
    }
    else
    {
        // フォーマットのみが違う場合.

        ComPtr<IWICFormatConverter> conv;
        hr = pWIC->CreateFormatConverter( conv.GetAddressOf() );
        if ( FAILED( hr ) )
        {
            SafeDeleteArray( pPixels );
            return false;
        }

        hr = conv->Initialize( frame.Get(), convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom );
        if ( FAILED( hr ) )
        {
            SafeDeleteArray( pPixels );
            return false;
        }

        hr = conv->CopyPixels( 0, uint32_t( rowPitch ), uint32_t( imageSize ), pPixels );
        if ( FAILED( hr ) )
        {
            SafeDeleteArray( pPixels );
            return false;
        }
    }

    // リソーステクスチャを設定.
    resTexture.Dimension        = a3d::RESOURCE_DIMENSION_TEXTURE2D;
    resTexture.Width            = width;
    resTexture.Height           = height;
    resTexture.DepthOrArraySize = 1;
    resTexture.Format           = ToWrapFormat(format);
    resTexture.MipLevels        = 1;
    if (!resTexture.Surfaces.init(Allocator::Instance(), 1))
    {
        a3d::DisposeTexture(resTexture);
        return false;
    }

    resTexture.Surfaces[0].RowPitch     = uint32_t(rowPitch);
    resTexture.Surfaces[0].SlicePitch   = uint32_t(imageSize);
    resTexture.Surfaces[0].MipIndex     = 0;
    if (!resTexture.Surfaces[0].Pixels.init(Allocator::Instance(), uint32_t(imageSize)))
    {
        a3d::DisposeTexture(resTexture);
        return false;
    }

    memcpy(resTexture.Surfaces[0].Pixels.data(), pPixels, imageSize);
    SafeDeleteArray( pPixels );

    // 正常終了.
    return true;
}


//-------------------------------------------------------------------------------------------------
//      DDSからリソーステクスチャを生成します.
//-------------------------------------------------------------------------------------------------
bool CreateResTextureFromDDSFile( const char* filename, a3d::ResTexture& resTexture )
{
    FILE*           pFile;
    DDSurfaceDesc   ddsd;
    char            magic[ 4 ];
    uint32_t        width  = 0;
    uint32_t        height = 0;
    uint32_t        depth  = 0;
    uint32_t        nativeFormat = 0;


    // ファイルを開きます.
    errno_t err = fopen_s( &pFile, filename, "rb" );

    // ファイルオープンチェック.
    if ( err != 0 )
    {
        ELOG( "Error : File Open Failed. filename = %s", filename );
        return false;
    }

   // マジックを読み込む.
    fread( magic, sizeof(char), 4, pFile);

    // マジックをチェック.
    if ( ( magic[0] != 'D' )
      || ( magic[1] != 'D' )
      || ( magic[2] != 'S' )
      || ( magic[3] != ' ' ) )
    {
        // エラーログ出力.
        ELOG( "Error : Invalid File. filename = %s", filename );

        // ファイルを閉じる.
        fclose( pFile );

        // 異常終了.
        return false;
    }


    // サーフェイスデスクリプションを読み込み.
    fread( &ddsd, sizeof( DDSurfaceDesc ), 1, pFile );

    resTexture.Dimension = a3d::RESOURCE_DIMENSION_TEXTURE1D;

    // 高さ有効.
    if ( ddsd.flags & DDSD_HEIGHT )
    {
        resTexture.Dimension = a3d::RESOURCE_DIMENSION_TEXTURE2D;
        height = ddsd.height;
    }

    // 幅有効.
    if ( ddsd.flags & DDSD_WIDTH )
    { width = ddsd.width; }

    // 奥行有効.
    if ( ddsd.flags & DDSD_DEPTH )
    { depth = ddsd.depth; }

    resTexture.Width            = width;
    resTexture.Height           = height;
    resTexture.DepthOrArraySize = 1;
    resTexture.MipLevels        = 1;

    auto surfaces = 1;
    auto dxgi_format = DXGI_FORMAT_UNKNOWN;

    // ミップマップ数.有効
    if ( ddsd.flags & DDSD_MIPMAPCOUNT )
    { resTexture.MipLevels = ddsd.mipMapLevels; }

    // キューブマップとボリュームテクスチャのチェック.
    if ( ddsd.caps & DDSCAPS_COMPLEX )
    {
        // キューブマップの場合.
        if ( ddsd.caps2 & DDSCAPS2_CUBEMAP )
        {
            unsigned int surfaceCount = 0;

            // サーフェイス数をチェック.
            if ( ddsd.caps2 & DDSCAPS2_CUBEMAP_POSITIVE_X ) { surfaceCount++; }
            if ( ddsd.caps2 & DDSCAPS2_CUBEMAP_NEGATIVE_X ) { surfaceCount++; }
            if ( ddsd.caps2 & DDSCAPS2_CUBEMAP_POSITIVE_Y ) { surfaceCount++; }
            if ( ddsd.caps2 & DDSCAPS2_CUBEMAP_NEGATIVE_Y ) { surfaceCount++; }
            if ( ddsd.caps2 & DDSCAPS2_CUBEMAP_POSITIVE_Z ) { surfaceCount++; }
            if ( ddsd.caps2 & DDSCAPS2_CUBEMAP_NEGATIVE_Z ) { surfaceCount++; }

            // 一応チェック.
            assert( surfaceCount == 6 );

            // サーフェイス数を設定.
            surfaces *= 6;
            resTexture.Dimension = a3d::RESOURCE_DIMENSION_CUBEMAP;
        }
        // ボリュームテクスチャの場合.
        else if ( ddsd.caps2 & DDSCAPS2_VOLUME )
        {
            // 奥行の値を設定.
            resTexture.DepthOrArraySize = depth;
            resTexture.Dimension = a3d::RESOURCE_DIMENSION_TEXTURE3D;
            surfaces = depth;
        }
    }

    // サポートフォーマットのチェックフラグ.
    bool isSupportFormat = false;

    // ピクセルフォーマット有効.
    if ( ddsd.flags & DDSD_PIXELFORMAT )
    {
        // dwFourCC有効
        if ( ddsd.pixelFormat.flags & DDPF_FOURCC )
        {
            switch( ddsd.pixelFormat.fourCC )
            {
            case FOURCC_DXT1:
                {
                    dxgi_format       = DXGI_FORMAT_BC1_UNORM_SRGB;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC1;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_DXT2:
                {
                    dxgi_format       = DXGI_FORMAT_BC2_UNORM_SRGB;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC2;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_DXT3:
                {
                    dxgi_format       = DXGI_FORMAT_BC2_UNORM_SRGB;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC2;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_DXT4:
                {
                    dxgi_format       = DXGI_FORMAT_BC3_UNORM_SRGB;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC3;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_DXT5:
                {
                    dxgi_format       = DXGI_FORMAT_BC3_UNORM_SRGB;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC3;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_ATI1:
            case FOURCC_BC4U:
                {
                    dxgi_format       = DXGI_FORMAT_BC4_UNORM;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC4U;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_BC4S:
                {
                    dxgi_format       = DXGI_FORMAT_BC5_SNORM;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC4S;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_ATI2:
            case FOURCC_BC5U:
                {
                    dxgi_format       = DXGI_FORMAT_BC5_UNORM;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC5U;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_BC5S:
                {
                    dxgi_format       = DXGI_FORMAT_BC5_SNORM;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC5S;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_DX10:
                {
                    DDSHEADER_DX10 ext_header; 

                    fread(&ext_header, sizeof(ext_header), 1, pFile);
                    dxgi_format = DXGI_FORMAT(ext_header.Format);

                    switch(ext_header.Dimension)
                    {
                    case DDS10_DIMENSION_1D:
                        resTexture.Dimension = a3d::RESOURCE_DIMENSION_TEXTURE1D;
                        break;

                    case DDS10_DIMENSION_2D:
                        resTexture.Dimension = a3d::RESOURCE_DIMENSION_TEXTURE2D;
                        break;

                    case DDS10_DIMENSION_3D:
                        resTexture.Dimension = a3d::RESOURCE_DIMENSION_TEXTURE3D;
                        break;
                    }

                    resTexture.DepthOrArraySize = ext_header.ArraySize;
                    surfaces = ext_header.ArraySize;
                    isSupportFormat = true;
                }
                break;

            case 36: // D3DFMT_A16B16G16R16
                {
                    // DXGI_FORMAT_R16G61B16A16_UNORM
                }
                break;

            case 110: // D3DFMT_W16W16V16U16
                {
                    // DXGI_FORMAT_R16G16B16A16_SNORM;
                }
                break;

            case 111: // D3DFMT_R16F
                {
                    // DXGI_FORMAT_R16_FLOAT
                    dxgi_format       = DXGI_FORMAT_R16_FLOAT;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_R16_FLOAT;
                    isSupportFormat   = true;
                }
                break;

            case 112: // D3DFMT_G16R16F
                {
                    // DXGI_FORMAT_R16G16_FLOAT
                    dxgi_format       = DXGI_FORMAT_R16G16_FLOAT;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_G16R16_FLOAT;
                    isSupportFormat   = true;
                }
                break;

            case 113: // D3DFMT_A16B16G16R16F
                {
                    // DXGI_FORMAT_R16G16B16A16_FLOAT
                    dxgi_format       = DXGI_FORMAT_R16G16B16A16_FLOAT;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_A16B16G16R16_FLOAT;
                    isSupportFormat   = true;
                }
                break;

            case 114: // D3DFMT_R32F
                {
                    // DXGI_FORMAT_R32_FLOAT
                    dxgi_format       = DXGI_FORMAT_R32_FLOAT;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_R32_FLOAT;
                    isSupportFormat   = true;
                }
                break;

            case 115: // D3DFMT_G32R32F
                {
                    // DXGI_FORMAT_R32G32_FLOAT
                    dxgi_format       = DXGI_FORMAT_R32G32_FLOAT;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_G32R32_FLOAT;
                    isSupportFormat   = true;
                }
                break;

            case 116: // D3DFMT_A32B32G32R32F
                {
                    // DXGI_FORMAT_R32G32B32A32_FLOAT
                    dxgi_format       = DXGI_FORMAT_R32G32B32A32_FLOAT;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_A32B32G32R32_FLOAT;
                    isSupportFormat   = true;
                }
                break;

            default:
                {
                    isSupportFormat = false;
                }
                break;
            }
        }
        else if ( ddsd.pixelFormat.flags & DDPF_RGB )
        {
            switch( ddsd.pixelFormat.bpp )
            {
            case 32:
                {
                    if ( CheckMask( ddsd.pixelFormat, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 ) )
                    {
                        // A8 R8 G8 B8
                        isSupportFormat   = true;
                        dxgi_format       = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                        nativeFormat      = NATIVE_TEXTURE_FORMAT_ARGB_8888;
                    }

                    if ( CheckMask( ddsd.pixelFormat, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 ) )
                    {
                        // A8 B8 G8 R8
                        isSupportFormat   = true;
                        dxgi_format       = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                        nativeFormat      = NATIVE_TEXTURE_FORMAT_ABGR_8888;
                    }

                    if ( CheckMask( ddsd.pixelFormat, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 ) )
                    {
                        // X8 R8 G8 B8
                        isSupportFormat   = true;
                        dxgi_format       = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                        nativeFormat      = NATIVE_TEXTURE_FORMAT_XRGB_8888;
                    }

                    if ( CheckMask( ddsd.pixelFormat, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000 ) )
                    {
                        // X8 B8 G8 R8
                        isSupportFormat = true;
                        dxgi_format     = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                        nativeFormat    = NATIVE_TEXTURE_FORMAT_XBGR_8888;
                    }

                #if 0
                    //if ( CheckMask( ddsd.pixelFormat, 0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000 ) )
                    //{
                    //    // R10 G10 B10 A2
                    //    /* NOT_SUPPORT */
                    //}

                    //if ( CheckMask( ddsd.pixelFormat, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 ) )
                    //{
                    //    // R16 G16
                    //    /* NOT_SUPPORT */
                    //}
                    //if ( CheckMask( ddsd.pixelFormat, 0xffffffff, 0x00000000,0x00000000,0x00000000 ) )
                    //{
                    //    // R32
                    //    /* NOT_SUPPORT */
                    //}
                #endif
                }
                break;

            case 24:
                {
                #if 0
                    /* NOT_SUPPORT */
                #endif
                }
                break;

            case 16:
                {
                #if 0
                    //if ( CheckMask( ddsd.pixelFormat, 0x7c00, 0x03e0, 0x001f, 0x8000 ) )
                    //{
                    //    // B5 G5 R5 A1
                    //    /* NOT_SUPPORT */
                    //}

                    //if ( CheckMask( ddsd.pixelFormat, 0xf800, 0x07e0, 0x001f, 0x0000 ) )
                    //{
                    //    // B5 G6 R5
                    //    /* NOT_SUPPORT */
                    //}
                #endif
                }
                break;
            }
        }
        else if ( ddsd.pixelFormat.flags & DDPF_LUMINANCE )
        {
            switch( ddsd.pixelFormat.bpp )
            {
            case 8:
                {
                    if ( CheckMask( ddsd.pixelFormat, 0x000000ff, 0x00000000, 0x00000000, 0x00000000 ) )
                    {
                        // R8
                        isSupportFormat   = true;
                        nativeFormat      = NATIVE_TEXTURE_FORMAT_R8;
                        dxgi_format       = DXGI_FORMAT_R8_UNORM;
                    }
                }
                break;

            case 16:
                {
                #if 0
                    //if ( CheckMask( ddsd.pixelFormat, 0x0000ffff, 0x00000000, 0x00000000, 0x00000000 ) )
                    //{
                    //    // R16
                    //    /* NOT_SUPPORT */
                    //}

                    //if ( CheckMask( ddsd.pixelFormat, 0x000000ff, 0x00000000, 0x00000000, 0x0000ff00 ) )
                    //{
                    //    // R8 G8
                    //    /* NOT_SUPPORT */
                    //}
                #endif
                }
                break;
            }
        }
        else if ( ddsd.pixelFormat.flags & DDPF_ALPHA )
        {
            if ( 8 == ddsd.pixelFormat.bpp )
            {
                // A8
                isSupportFormat   = true;
                nativeFormat      = NATIVE_TEXTURE_FORMAT_A8;
                dxgi_format       = DXGI_FORMAT_R8_UNORM;
            }
        }
    }

    // サポートフォーマットがあったかチェック.
    if ( !isSupportFormat )
    {
        // ファイルを閉じる.
        fclose( pFile );

        // エラーログ出力.
        ELOG( "Error : Unsupported Format." );

        // 異常終了.
        return false;
    }

    resTexture.Format = ToWrapFormat(dxgi_format);

    // 現在のファイル位置を記憶.
    long curr = ftell( pFile );

    // ファイル末端に移動.
    fseek( pFile, 0, SEEK_END );

    // ファイル末端位置を記憶.
    long end = ftell( pFile );

    // ファイル位置を元に戻す.
    fseek( pFile, curr, SEEK_SET );

    // ピクセルデータのサイズを算出.
    size_t pixelSize = end - curr;

    // ピクセルデータのメモリを確保.
    unsigned char* pPixelData = new unsigned char [pixelSize];

    // NULLチェック.
    if ( pPixelData == nullptr )
    {
        // ファイルを閉じる.
        fclose( pFile );

        // エラーログ出力.
        ELOG( "Error : Memory Allocate Failed." );

        // 異常終了.
        return false;
    }

    // ピクセルデータ読み込み.
    fread( pPixelData, sizeof( unsigned char ), pixelSize, pFile );

    // ファイルを閉じる.
    fclose( pFile );

    // リトルエンディアンなのでピクセルの並びを補正.
    switch( nativeFormat )
    {
        // 一括読み込みでくるっているので修正.
        case NATIVE_TEXTURE_FORMAT_ARGB_8888:
        case NATIVE_TEXTURE_FORMAT_XRGB_8888:
        {
            for( size_t i=0; i<pixelSize; i+=4 )
            {
                // BGRA -> RGBA
                unsigned char R = pPixelData[ i + 0 ];
                unsigned char B = pPixelData[ i + 2 ];
                pPixelData[ i + 0 ] = B;
                pPixelData[ i + 2 ] = R;
            }
        }
        break;
    }

    size_t offset = 0;
    size_t skipMip = 0;
    int idx    = 0;

    // リソースデータのメモリを確保.
    if (!resTexture.Surfaces.init(Allocator::Instance(), surfaces))
    {
        return false;
    }


    for ( size_t j=0; j<size_t(surfaces); ++j )
    {
        size_t w = width;
        size_t h = height;
        size_t d = depth;

        // 各ミップレベルごとに処理.
        for( size_t i=0; i<resTexture.MipLevels; ++i )
        {
            size_t rowBytes = 0;
            size_t numRows  = 0;
            size_t numBytes = 0;

            // ブロック圧縮フォーマットの場合.
            if ( ( dxgi_format == DXGI_FORMAT_BC1_UNORM )//NATIVE_TEXTURE_FORMAT_BC1 )
              || ( dxgi_format == DXGI_FORMAT_BC1_UNORM_SRGB )//NATIVE_TEXTURE_FORMAT_BC2 )
              || ( dxgi_format == DXGI_FORMAT_BC2_UNORM )//NATIVE_TEXTURE_FORMAT_BC2 )
              || ( dxgi_format == DXGI_FORMAT_BC2_UNORM_SRGB )//NATIVE_TEXTURE_FORMAT_BC2 )
              || ( dxgi_format == DXGI_FORMAT_BC3_UNORM )//NATIVE_TEXTURE_FORMAT_BC3 )
              || ( dxgi_format == DXGI_FORMAT_BC3_UNORM_SRGB )//NATIVE_TEXTURE_FORMAT_BC3 )
              || ( dxgi_format == DXGI_FORMAT_BC4_UNORM )//NATIVE_TEXTURE_FORMAT_BC4U )
              || ( dxgi_format == DXGI_FORMAT_BC4_SNORM )//NATIVE_TEXTURE_FORMAT_BC4S )
              || ( dxgi_format == DXGI_FORMAT_BC5_UNORM )//NATIVE_TEXTURE_FORMAT_BC5U )
              || ( dxgi_format == DXGI_FORMAT_BC5_SNORM ) )//NATIVE_TEXTURE_FORMAT_BC5S ) )
            {
                size_t bcPerBlock = 0;
                size_t blockWide  = 0;
                size_t blockHigh  = 0;

                // BC1, BC4の場合.
                if ( ( dxgi_format == DXGI_FORMAT_BC1_UNORM )
                  || ( dxgi_format == DXGI_FORMAT_BC1_UNORM_SRGB )
                  || ( dxgi_format == DXGI_FORMAT_BC4_SNORM )
                  || ( dxgi_format == DXGI_FORMAT_BC4_UNORM ) )
                { bcPerBlock = 8; }
                // BC2, BC3, BC5, BC6H, BC7の場合.
                else
                { bcPerBlock = 16; }

                // 横幅が1以上の場合.
                if ( w > 0 )
                { blockWide = Max< size_t >( 1, ( w + 3 ) / 4 ); }

                // 縦幅が異常の場合.
                if ( h > 0 )
                { blockHigh = Max< size_t >( 1, ( h + 3 ) / 4 ); }

                // 一行のバイト数.
                rowBytes = blockWide * bcPerBlock;

                // 行数.
                numRows  = blockHigh;
            }
            // ブロック圧縮フォーマット以外の場合.
            else
            {
                // 1ピクセル当たりのビット数.
                size_t bpp = GetBitPerPixel( nativeFormat );

                // 一行のバイト数.
                rowBytes = ( w * bpp + 7 ) / 8;

                // 行数.
                numRows  = h;
            }

            // データ数 = (1行当たりのバイト数) * 行数.
            numBytes = rowBytes * numRows;

            // リソースデータを設定.
            resTexture.Surfaces[idx].MipIndex   = uint32_t(i);
            resTexture.Surfaces[idx].RowPitch   = uint32_t(rowBytes);
            resTexture.Surfaces[idx].SlicePitch = uint32_t(numBytes);
            if (!resTexture.Surfaces[idx].Pixels.init(Allocator::Instance(), uint32_t(numBytes)))
            {
                // エラーログ出力.
                ELOG( "Error : Memory Allocate Failed." );

                // 異常終了.
                return false;

            }
            // ピクセルデータをコピー.
            std::memcpy( resTexture.Surfaces[ idx ].Pixels.data(), pPixelData + offset, numBytes );

            // オフセットをカウントアップ.
            if ( depth != 0 )
            {
                offset += numBytes * d;
            }
            else
            {
                offset += numBytes;
            }

            // 横幅，縦幅を更新.
            w = w >> 1;
            h = h >> 1;
            d = d >> 1;

            // クランプ処理.
            if ( w == 0 ) { w = 1; }
            if ( h == 0 ) { h = 1; }
            if ( d == 0 ) { d = 1; }

            // インデックスをカウントアップ.
            idx++;
        }
    }

    // 不要になったメモリを解放.
    delete [] pPixelData;
    pPixelData = nullptr;

    // 正常終了.
    return true;
}

#if 0
//-------------------------------------------------------------------------------------------------
//      DDSからリソーステクスチャを生成します.
//-------------------------------------------------------------------------------------------------
bool CreateResTextureFromDDSMemory( const uint8_t* pBinary, const uint32_t bufferSize, a3d::ResTexture& resTexture )
{
    uint32_t             width  = 0;
    uint32_t             height = 0;
    uint32_t             depth  = 0;
    uint32_t             nativeFormat = 0;

    if ( pBinary == nullptr || bufferSize == 0 )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    // マジックをチェック.
    if ( ( pBinary[0] != 'D' )
      || ( pBinary[1] != 'D' )
      || ( pBinary[2] != 'S' )
      || ( pBinary[3] != ' ' ) )
    {
        // エラーログ出力.
        ELOG( "Error : Invalid File. " );

        // 異常終了.
        return false;
    }

    uint8_t* pCur = (uint8_t*)pBinary + sizeof(char) * 4;
    uint32_t offset = sizeof(char) * 4;
    if ( offset > bufferSize )
    {
        ELOG( "Error : Out of Range." );
        return false;
    }
    DDSurfaceDesc* ddsd = (DDSurfaceDesc*)pCur;
    offset += sizeof(DDSurfaceDesc);
    if ( offset > bufferSize )
    {
        ELOG( "Error : Out of Range." );
        return false;
    }
    pCur += sizeof(DDSurfaceDesc);

    // 高さ有効.
    if ( ddsd->flags & DDSD_HEIGHT )
    { height = ddsd->height; }

    // 幅有効.
    if ( ddsd->flags & DDSD_WIDTH )
    { width = ddsd->width; }

    // 奥行有効.
    if ( ddsd->flags & DDSD_DEPTH )
    { depth = ddsd->depth; }

    resTexture.Width        = width;
    resTexture.Height       = height;
    resTexture.Depth        = 0;
    resTexture.SurfaceCount = 1;
    resTexture.MipMapCount  = 1;

    // ミップマップ数.有効
    if ( ddsd->flags & DDSD_MIPMAPCOUNT )
    { resTexture.MipMapCount = ddsd->mipMapLevels; }

    // キューブマップとボリュームテクスチャのチェック.
    if ( ddsd->caps & DDSCAPS_COMPLEX )
    {
        // キューブマップの場合.
        if ( ddsd->caps2 & DDSCAPS2_CUBEMAP )
        {
            unsigned int surfaceCount = 0;

            // サーフェイス数をチェック.
            if ( ddsd->caps2 & DDSCAPS2_CUBEMAP_POSITIVE_X ) { surfaceCount++; }
            if ( ddsd->caps2 & DDSCAPS2_CUBEMAP_NEGATIVE_X ) { surfaceCount++; }
            if ( ddsd->caps2 & DDSCAPS2_CUBEMAP_POSITIVE_Y ) { surfaceCount++; }
            if ( ddsd->caps2 & DDSCAPS2_CUBEMAP_NEGATIVE_Y ) { surfaceCount++; }
            if ( ddsd->caps2 & DDSCAPS2_CUBEMAP_POSITIVE_Z ) { surfaceCount++; }
            if ( ddsd->caps2 & DDSCAPS2_CUBEMAP_NEGATIVE_Z ) { surfaceCount++; }

            // 一応チェック.
            assert( surfaceCount == 6 );

            // サーフェイス数を設定.
            resTexture.SurfaceCount = surfaceCount;
            resTexture.Option |= SUBRESOURCE_OPTION_CUBEMAP;
        }
        // ボリュームテクスチャの場合.
        else if ( ddsd->caps2 & DDSCAPS2_VOLUME )
        {
            // 奥行の値を設定.
            resTexture.Depth = depth;
            resTexture.Option |= SUBRESOURCE_OPTION_VOLUME;
        }
    }

    // サポートフォーマットのチェックフラグ.
    bool isSupportFormat = false;

    // ピクセルフォーマット有効.
    if ( ddsd->flags & DDSD_PIXELFORMAT )
    {
        // dwFourCC有効
        if ( ddsd->pixelFormat.flags & DDPF_FOURCC )
        {
            switch( ddsd->pixelFormat.fourCC )
            {
            case FOURCC_DXT1:
                {
                    resTexture.Format = DXGI_FORMAT_BC1_UNORM_SRGB;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC1;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_DXT2:
                {
                    resTexture.Format = DXGI_FORMAT_BC2_UNORM_SRGB;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC2;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_DXT3:
                {
                    resTexture.Format = DXGI_FORMAT_BC2_UNORM_SRGB;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC2;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_DXT4:
                {
                    resTexture.Format = DXGI_FORMAT_BC3_UNORM_SRGB;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC3;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_DXT5:
                {
                    resTexture.Format = DXGI_FORMAT_BC3_UNORM_SRGB;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC3;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_ATI1:
            case FOURCC_BC4U:
                {
                    resTexture.Format = DXGI_FORMAT_BC4_UNORM;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC4U;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_BC4S:
                {
                    resTexture.Format = DXGI_FORMAT_BC5_SNORM;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC4S;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_ATI2:
            case FOURCC_BC5U:
                {
                    resTexture.Format = DXGI_FORMAT_BC5_UNORM;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC5U;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_BC5S:
                {
                    resTexture.Format = DXGI_FORMAT_BC5_SNORM;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_BC5S;
                    isSupportFormat   = true;
                }
                break;

            case FOURCC_DX10:
                {
                }
                break;

            case 36: // D3DFMT_A16B16G16R16
                {
                    // DXGI_FORMAT_R16G61B16A16_UNORM
                }
                break;

            case 110: // D3DFMT_W16W16V16U16
                {
                    // DXGI_FORMAT_R16G16B16A16_SNORM;
                }
                break;

            case 111: // D3DFMT_R16F
                {
                    // DXGI_FORMAT_R16_FLOAT
                    resTexture.Format = DXGI_FORMAT_R16_FLOAT;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_R16_FLOAT;
                    isSupportFormat   = true;
                }
                break;

            case 112: // D3DFMT_G16R16F
                {
                    // DXGI_FORMAT_R16G16_FLOAT
                    resTexture.Format = DXGI_FORMAT_R16G16_FLOAT;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_G16R16_FLOAT;
                    isSupportFormat   = true;
                }
                break;

            case 113: // D3DFMT_A16B16G16R16F
                {
                    // DXGI_FORMAT_R16G16B16A16_FLOAT
                    resTexture.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_A16B16G16R16_FLOAT;
                    isSupportFormat   = true;
                }
                break;

            case 114: // D3DFMT_R32F
                {
                    // DXGI_FORMAT_R32_FLOAT
                    resTexture.Format = DXGI_FORMAT_R32_FLOAT;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_R32_FLOAT;
                    isSupportFormat   = true;
                }
                break;

            case 115: // D3DFMT_G32R32F
                {
                    // DXGI_FORMAT_R32G32_FLOAT
                    resTexture.Format = DXGI_FORMAT_R32G32_FLOAT;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_G32R32_FLOAT;
                    isSupportFormat   = true;
                }
                break;

            case 116: // D3DFMT_A32B32G32R32F
                {
                    // DXGI_FORMAT_R32G32B32A32_FLOAT
                    resTexture.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                    nativeFormat      = NATIVE_TEXTURE_FORMAT_A32B32G32R32_FLOAT;
                    isSupportFormat   = true;
                }
                break;

            default:
                {
                    isSupportFormat = false;
                }
                break;
            }
        }
        else if ( ddsd->pixelFormat.flags & DDPF_RGB )
        {
            switch( ddsd->pixelFormat.bpp )
            {
            case 32:
                {
                    if ( CheckMask( ddsd->pixelFormat, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 ) )
                    {
                        // A8 R8 G8 B8
                        isSupportFormat   = true;
                        resTexture.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                        nativeFormat      = NATIVE_TEXTURE_FORMAT_ARGB_8888;
                    }

                    if ( CheckMask( ddsd->pixelFormat, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 ) )
                    {
                        // A8 B8 G8 R8
                        isSupportFormat   = true;
                        resTexture.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                        nativeFormat      = NATIVE_TEXTURE_FORMAT_ABGR_8888;
                    }

                    if ( CheckMask( ddsd->pixelFormat, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 ) )
                    {
                        // X8 R8 G8 B8
                        isSupportFormat   = true;
                        resTexture.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                        nativeFormat      = NATIVE_TEXTURE_FORMAT_XRGB_8888;
                    }

                    if ( CheckMask( ddsd->pixelFormat, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000 ) )
                    {
                        // X8 B8 G8 R8
                        isSupportFormat = true;
                        resTexture.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                        nativeFormat = NATIVE_TEXTURE_FORMAT_XBGR_8888;
                    }

                #if 0
                    //if ( CheckMask( ddsd->pixelFormat, 0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000 ) )
                    //{
                    //    // R10 G10 B10 A2
                    //    /* NOT_SUPPORT */
                    //}

                    //if ( CheckMask( ddsd->pixelFormat, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 ) )
                    //{
                    //    // R16 G16
                    //    /* NOT_SUPPORT */
                    //}
                    //if ( CheckMask( ddsd->pixelFormat, 0xffffffff, 0x00000000,0x00000000,0x00000000 ) )
                    //{
                    //    // R32
                    //    /* NOT_SUPPORT */
                    //}
                #endif
                }
                break;

            case 24:
                {
                #if 0
                    /* NOT_SUPPORT */
                #endif
                }
                break;

            case 16:
                {
                #if 0
                    //if ( CheckMask( ddsd->pixelFormat, 0x7c00, 0x03e0, 0x001f, 0x8000 ) )
                    //{
                    //    // B5 G5 R5 A1
                    //    /* NOT_SUPPORT */
                    //}

                    //if ( CheckMask( ddsd->pixelFormat, 0xf800, 0x07e0, 0x001f, 0x0000 ) )
                    //{
                    //    // B5 G6 R5
                    //    /* NOT_SUPPORT */
                    //}
                #endif
                }
                break;
            }
        }
        else if ( ddsd->pixelFormat.flags & DDPF_LUMINANCE )
        {
            switch( ddsd->pixelFormat.bpp )
            {
            case 8:
                {
                    if ( CheckMask( ddsd->pixelFormat, 0x000000ff, 0x00000000, 0x00000000, 0x00000000 ) )
                    {
                        // R8
                        isSupportFormat   = true;
                        nativeFormat      = NATIVE_TEXTURE_FORMAT_R8;
                        resTexture.Format = DXGI_FORMAT_R8_UNORM;
                    }
                }
                break;

            case 16:
                {
                #if 0
                    //if ( CheckMask( ddsd->pixelFormat, 0x0000ffff, 0x00000000, 0x00000000, 0x00000000 ) )
                    //{
                    //    // R16
                    //    /* NOT_SUPPORT */
                    //}

                    //if ( CheckMask( ddsd->pixelFormat, 0x000000ff, 0x00000000, 0x00000000, 0x0000ff00 ) )
                    //{
                    //    // R8 G8
                    //    /* NOT_SUPPORT */
                    //}
                #endif
                }
                break;
            }
        }
        else if ( ddsd->pixelFormat.flags & DDPF_ALPHA )
        {
            if ( 8 == ddsd->pixelFormat.bpp )
            {
                // A8
                isSupportFormat   = true;
                nativeFormat      = NATIVE_TEXTURE_FORMAT_A8;
                resTexture.Format = DXGI_FORMAT_R8_UNORM;
            }
        }
    }

    // サポートフォーマットがあったかチェック.
    if ( !isSupportFormat )
    {
        // エラーログ出力.
        ELOG( "Error : Unsupported Format." );

        // 異常終了.
        return false;
    }

    // ピクセルデータのサイズを算出.
    size_t pixelSize = bufferSize - offset;

    // ピクセルデータのメモリを確保.
    unsigned char* pPixelData = new unsigned char [pixelSize];

    // NULLチェック.
    if ( pPixelData == nullptr )
    {
        // エラーログ出力.
        ELOG( "Error : Memory Allocate Failed." );

        // 異常終了.
        return false;
    }

    memcpy( pPixelData, pCur, sizeof(unsigned char) * pixelSize );

    // リトルエンディアンなのでピクセルの並びを補正.
    switch( nativeFormat )
    {
        // 一括読み込みでくるっているので修正.
        case NATIVE_TEXTURE_FORMAT_ARGB_8888:
        case NATIVE_TEXTURE_FORMAT_XRGB_8888:
        {
            for( size_t i=0; i<pixelSize; i+=4 )
            {
                // BGRA -> RGBA
                unsigned char R = pPixelData[ i + 0 ];
                unsigned char B = pPixelData[ i + 2 ];
                pPixelData[ i + 0 ] = B;
                pPixelData[ i + 2 ] = R;
            }
        }
        break;
    }

    size_t byteOffset = 0;
    size_t idx    = 0;
    size_t skipMip = 0;

    // リソースデータのメモリを確保.
    resTexture.pResources = new SubResource[ resTexture.MipMapCount * resTexture.SurfaceCount ];

    for ( size_t j=0; j<resTexture.SurfaceCount; ++j )
    {
        size_t w = width;
        size_t h = height;
        size_t d = depth;

        // 各ミップレベルごとに処理.
        for( size_t i=0; i<resTexture.MipMapCount; ++i )
        {
            size_t rowBytes = 0;
            size_t numRows  = 0;
            size_t numBytes = 0;

            // ブロック圧縮フォーマットの場合.
            if ( ( nativeFormat == NATIVE_TEXTURE_FORMAT_BC1 )
              || ( nativeFormat == NATIVE_TEXTURE_FORMAT_BC2 )
              || ( nativeFormat == NATIVE_TEXTURE_FORMAT_BC3 )
              || ( nativeFormat == NATIVE_TEXTURE_FORMAT_BC4U )
              || ( nativeFormat == NATIVE_TEXTURE_FORMAT_BC4S )
              || ( nativeFormat == NATIVE_TEXTURE_FORMAT_BC5U )
              || ( nativeFormat == NATIVE_TEXTURE_FORMAT_BC5S ) )
            {
                size_t bcPerBlock = 0;
                size_t blockWide  = 0;
                size_t blockHigh  = 0;

                // BC1, BC4の場合.
                if ( ( nativeFormat == NATIVE_TEXTURE_FORMAT_BC1 )
                  || ( nativeFormat == NATIVE_TEXTURE_FORMAT_BC4S )
                  || ( nativeFormat == NATIVE_TEXTURE_FORMAT_BC4U ) )
                { bcPerBlock = 8; }
                // BC2, BC3, BC5, BC6H, BC7の場合.
                else
                { bcPerBlock = 16; }

                blockWide = Max< size_t >( 1, ( w + 3 ) / 4 );
                blockHigh = Max< size_t >( 1, ( h + 3 ) / 4 );

                // 一行のバイト数.
                rowBytes = blockWide * bcPerBlock;

                // 行数.
                numRows  = blockHigh;
            }
            // ブロック圧縮フォーマット以外の場合.
            else
            {
                // 1ピクセル当たりのビット数.
                size_t bpp = GetBitPerPixel( nativeFormat );

                // 一行のバイト数.
                rowBytes = ( w * bpp + 7 ) / 8;

                // 行数.
                numRows  = h;
            }

            // データ数 = (1行当たりのバイト数) * 行数.
            numBytes = rowBytes * numRows;

            // リソースデータを設定.
            resTexture.pResources[ idx ].Width      = uint32_t( w );
            resTexture.pResources[ idx ].Height     = uint32_t( h );
            resTexture.pResources[ idx ].Pitch      = uint32_t( rowBytes );
            resTexture.pResources[ idx ].SlicePitch = uint32_t( numBytes );
            resTexture.pResources[ idx ].pPixels    = pPixelData + byteOffset;

            // オフセットをカウントアップ.
            if ( resTexture.Depth != 0 )
            {
                byteOffset += numBytes * d;
            }
            else
            {
                byteOffset += numBytes;
            }

            // 横幅，縦幅を更新.
            w = w >> 1;
            h = h >> 1;
            d = d >> 1;

            // クランプ処理.
            if ( w == 0 ) { w = 1; }
            if ( h == 0 ) { h = 1; }
            if ( d == 0 ) { d = 1; }

            // インデックスをカウントアップ.
            idx++;
        }
    }

    // 正常終了.
    return true;
}
#endif

//-------------------------------------------------------------------------------------------------
//      ファイルからテクスチャを生成します.
//-------------------------------------------------------------------------------------------------
bool CreateResTextureFromFile( const char* filename, a3d::ResTexture& resTexture )
{
    if ( filename == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    FILE* pFile;
    char magic[4];

    // ファイルを開きます.
    errno_t err = fopen_s( &pFile, filename, "rb" );

    // ファイルオープンチェック.
    if ( err != 0 )
    {
        ELOG( "Error : File Open Failed. filename = %s", filename );
        return false;
    }

    // マジックを読み込む.
    fread( magic, sizeof(char), 4, pFile);

    bool isDDS = true;

    // マジックをチェック.
    if ( ( magic[0] != 'D' )
      || ( magic[1] != 'D' )
      || ( magic[2] != 'S' )
      || ( magic[3] != ' ' ) )
    {
        isDDS = false;
    }

    // ファイルを閉じる.
    fclose( pFile );

    if ( isDDS )
    { return CreateResTextureFromDDSFile( filename, resTexture ); }

    return CreateResTextureFromWICFile( filename, resTexture );
}

#if 0
//-------------------------------------------------------------------------------------------------
//      メモリストリームからテクスチャを生成します.
//-------------------------------------------------------------------------------------------------
bool CreateResTextureFromMemory( const uint8_t* pBinary, const uint32_t bufferSize, ResTexture& resTexture )
{
    if ( pBinary == nullptr || bufferSize < 4 )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    bool isDDS = true;
    if ( ( pBinary[0] != 'D' )
      || ( pBinary[1] != 'D' )
      || ( pBinary[2] != 'S' )
      || ( pBinary[3] != ' ' ) )
    {
        isDDS = false;
    }

    if ( isDDS )
    { return CreateResTextureFromDDSMemory( pBinary, bufferSize, resTexture ); }

    return CreateResTextureFromWICMemory( pBinary, bufferSize, resTexture );
}
#endif


} // namespace a3dx
