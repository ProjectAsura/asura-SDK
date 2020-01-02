//-----------------------------------------------------------------------------
// File : a3dxResTexture.cpp
// Desc : Texture Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <a3dxResTexture.h>
#include <cstdio>


namespace {

///////////////////////////////////////////////////////////////////////////////
// TexHeader structure
///////////////////////////////////////////////////////////////////////////////
struct TexHeader
{
    uint32_t Magic;             //!< ' XET'
    uint32_t Dimension;         //!< テクスチャ次元.
    uint32_t Width;             //!< 横幅.
    uint32_t Height;            //!< 縦幅.
    uint32_t DepthOrArraySize;  //!< 奥行または配列サイズ.
    uint32_t MipLevels;         //!< ミップレベル数.
    uint32_t Format;            //!< ピクセルフォーマット.
    uint32_t SurfaceCount;      //!< サーフェイス数.
};

///////////////////////////////////////////////////////////////////////////////
// SurfaceDesc structure
///////////////////////////////////////////////////////////////////////////////
struct SurfaceDesc
{
    uint32_t MipIndex;
    uint32_t RowPitch;
    uint32_t SlicePitch;
};

} // namespace


namespace a3d {

///////////////////////////////////////////////////////////////////////////////
// ResTexture class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ResTexture::ResTexture()
: Dimension         (RESOURCE_DIMENSION_TEXTURE1D)
, Width             (0)
, Height            (0)
, DepthOrArraySize  (0)
, MipLevels         (0)
, Format            (RESOURCE_FORMAT_UNKNOWN)
, SurfaceCount      (0)
, Surfaces          (nullptr)
, m_pAllocator      (nullptr)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ResTexture::~ResTexture()
{ Dispose(); }

//-----------------------------------------------------------------------------
//      MAPファイルからテクスチャを読み込みます.
//-----------------------------------------------------------------------------
bool ResTexture::Load(IAllocator* pAllocator, const char* path)
{
    FILE* pFile = nullptr;

    pFile = fopen(path, "rb");
    if (pFile == nullptr)
    { return false; }

    TexHeader header = {};
    fread(&header, sizeof(header), 1, pFile);

    if (header.Magic != '\0XET')
    {
        fclose(pFile);
        return false;
    }

    Surfaces = static_cast<ResSurface*>(pAllocator->Alloc(sizeof(ResSurface) * header.SurfaceCount, 4));
    if (Surfaces == nullptr)
    {
        fclose(pFile);
        return false;
    }

    m_pAllocator = pAllocator;

    Dimension           = RESOURCE_DIMENSION(header.Dimension);
    Width               = header.Width;
    Height              = header.Height;
    DepthOrArraySize    = header.DepthOrArraySize;
    MipLevels           = header.MipLevels;
    Format              = RESOURCE_FORMAT(header.Format);
    SurfaceCount        = header.SurfaceCount;

    for(auto i=0u; i<header.SurfaceCount; ++i)
    {
        SurfaceDesc surface;
        fread(&surface, sizeof(surface), 1, pFile);

        Surfaces[i].MipIndex    = surface.MipIndex;
        Surfaces[i].RowPitch    = surface.RowPitch;
        Surfaces[i].SlicePitch  = surface.SlicePitch;

        Surfaces[i].Pixels = static_cast<uint8_t*>(pAllocator->Alloc(surface.SlicePitch, 4));
        if (Surfaces[i].Pixels == nullptr)
        {
            Dispose();
            return false;
        }

        fread(Surfaces[i].Pixels, surface.SlicePitch, 1, pFile);
    }

    fclose(pFile);
    return true;
}

//-----------------------------------------------------------------------------
//      MAPファイルにテクスチャを書き込みます.
//-----------------------------------------------------------------------------
bool ResTexture::Save(const char* path)
{
    FILE* pFile;

    pFile = fopen(path, "wb");
    if (pFile == nullptr)
    { return false; }

    TexHeader header;
    header.Magic            = '\0XET';
    header.Dimension        = uint32_t(Dimension);
    header.Width            = Width;
    header.Height           = Height;
    header.DepthOrArraySize = DepthOrArraySize;
    header.MipLevels        = MipLevels;
    header.Format           = uint32_t(Format);
    header.SurfaceCount     = SurfaceCount;

    for(auto i=0u; i<SurfaceCount; ++i)
    {
        SurfaceDesc surface;
        surface.MipIndex    = Surfaces[i].MipIndex;
        surface.RowPitch    = Surfaces[i].RowPitch;
        surface.SlicePitch  = Surfaces[i].SlicePitch;

        fwrite(&surface, sizeof(surface), 1, pFile);

        fwrite(Surfaces[i].Pixels, surface.SlicePitch, 1, pFile);
    }

    fclose(pFile);
    return true;
}

//-----------------------------------------------------------------------------
//      テクスチャを破棄します.
//-----------------------------------------------------------------------------
void ResTexture::Dispose()
{
    Dimension           = RESOURCE_DIMENSION_UNKNOWN;
    Width               = 0;
    Height              = 0;
    DepthOrArraySize    = 0;
    MipLevels           = 0;
    Format              = RESOURCE_FORMAT_UNKNOWN;

    if (m_pAllocator != nullptr)
    {
        for(auto i=0u; i<SurfaceCount; ++i)
        {
            auto ptr = Surfaces[i].Pixels;
            Surfaces[i].Pixels = nullptr;
            m_pAllocator->Free(ptr);
        }

        {
            auto ptr = Surfaces;
            m_pAllocator->Free(ptr);
        }
    }

    SurfaceCount = 0;
    Surfaces     = nullptr;
    m_pAllocator = nullptr;
}

} // namespace a3d
