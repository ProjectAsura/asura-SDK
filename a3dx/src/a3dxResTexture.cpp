//-------------------------------------------------------------------------------------------------
// File : a3dxResTexture.cpp
// Desc : Texture Resource.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "a3dxResTexture.h"


namespace {

///////////////////////////////////////////////////////////////////////////////////////////////////
// TEX_FILE_HEADER structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct TEX_FILE_HEADER
{
    uint8_t     Tag[4];     //!< ファイルタグです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// TEX_DATA_HEADER structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct TEX_DATA_HEADER
{
    uint32_t    Dimension;          //!< 次元です.
    uint32_t    Width;              //!< 横幅です.
    uint32_t    Height;             //!< 縦幅です.
    uint32_t    DepthOrArraySize;   //!< 奥行または配列数です.
    uint32_t    MipLevels;          //!< ミップレベル数です.
    uint32_t    Format;             //!< フォーマットです.
    uint32_t    SurfaceCount;       //!< サーフェイス数です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// TEX_SURFACE structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct TEX_SURFACE
{
    uint32_t    Width;          //!< 横幅です.
    uint32_t    Height;         //!< 縦幅です.
    uint32_t    RowPitch;       //!< 1行あたりのバイト数です.
    uint32_t    SlicePitch;     //!< 1サーフェイスあたりのバイト数です.
};

} // namespace


namespace a3dx {

//-------------------------------------------------------------------------------------------------
//      テクスチャリソースを読み込みます.
//-------------------------------------------------------------------------------------------------
bool LoadResTexture(const char* filename, ResTexture& resource)
{
    FILE* pFile = nullptr;

    #if defined(_WIN32) || defined(WIN64)
        auto ret = fopen_s( &pFile, filename, "rb");
        if (ret == 0)
        { return false; }
    #else
        pFile = fopen( filename, "r" );
        if (pFile == nullptr)
        { return false; }
    #endif

    TEX_FILE_HEADER header = {};
    fread(&header, sizeof(header), 1, pFile);

    if (header.Tag[0] != 'T' ||
        header.Tag[1] != 'E' ||
        header.Tag[2] != 'X' ||
        header.Tag[3] != '\0')
    {
        fclose(pFile);
        return false;
    }

    TEX_DATA_HEADER section = {};
    fread(&section, sizeof(section), 1, pFile);

    resource.Dimension          = a3d::RESOURCE_DIMENSION(section.Dimension);
    resource.Width              = section.Width;
    resource.Height             = section.Height;
    resource.DepthOrArraySize   = section.DepthOrArraySize;
    resource.MipLevels          = section.MipLevels;
    resource.Format             = a3d::RESOURCE_FORMAT(section.Format);
    resource.Surfaces.resize(section.SurfaceCount);

    for(auto i=0u; i<section.SurfaceCount; ++i)
    {
        TEX_SURFACE surface = {};
        fread(&surface, sizeof(surface), 1, pFile);

        resource.Surfaces[i].Width      = surface.Width;
        resource.Surfaces[i].Height     = surface.Height;
        resource.Surfaces[i].RowPitch   = surface.RowPitch;
        resource.Surfaces[i].SlicePitch = surface.SlicePitch;
        resource.Surfaces[i].Pixel.resize(surface.SlicePitch);

        auto ptr = resource.Surfaces[i].Pixel.data();
        fread(ptr, sizeof(uint8_t), surface.SlicePitch, pFile);
    }

    fclose(pFile);

    return true;
}

//-------------------------------------------------------------------------------------------------
//      テクスチャリソースを書き込みます.
//-------------------------------------------------------------------------------------------------
bool SaveResTexture(const char* filename, const ResTexture& resource)
{
    FILE* pFile = nullptr;

    #if defined(_WIN32) || defined(WIN64)
        auto ret = fopen_s( &pFile, filename, "wb");
        if (ret == 0)
        { return false; }
    #else
        pFile = fopen( filename, "w" );
        if (pFile == nullptr)
        { return false; }
    #endif

    TEX_FILE_HEADER header = {};
    header.Tag[0] = 'T';
    header.Tag[1] = 'E';
    header.Tag[2] = 'X';
    header.Tag[3] = '\0';
    fwrite(&header, sizeof(header), 1, pFile);

    TEX_DATA_HEADER section = {};
    section.Dimension        = uint32_t(resource.Dimension);
    section.Width            = resource.Width;
    section.Height           = resource.Height;
    section.DepthOrArraySize = resource.DepthOrArraySize;
    section.MipLevels        = resource.MipLevels;
    section.Format           = uint32_t(resource.Format);
    section.SurfaceCount     = uint32_t(resource.Surfaces.size());
    fwrite(&section, sizeof(section), 1, pFile);

    for(size_t i=0; i<resource.Surfaces.size(); ++i)
    {
        TEX_SURFACE surface = {};
        surface.Width       = resource.Surfaces[i].Width;
        surface.Height      = resource.Surfaces[i].Height;
        surface.RowPitch    = resource.Surfaces[i].RowPitch;
        surface.SlicePitch  = resource.Surfaces[i].SlicePitch;
        fwrite(&surface, sizeof(surface), 1, pFile);

        auto ptr = resource.Surfaces[i].Pixel.data();
        fwrite(ptr, sizeof(uint8_t), surface.SlicePitch, pFile);
    }

    fclose(pFile);

    return true;
}

} // namespace a3dx
