//-------------------------------------------------------------------------------------------------
// File : a3dx_restexture.cpp
// Desc : Texture Resource.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <a3dx_restexture.h>
#include <cstdio>


namespace {

///////////////////////////////////////////////////////////////////////////////////////////////////
// MapHeader structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MapHeader
{
    uint32_t Magic;             //!< ' PAM'
    uint32_t Dimension;         //!< �e�N�X�`������.
    uint32_t Width;             //!< ����.
    uint32_t Height;            //!< �c��.
    uint32_t DepthOrArraySize;  //!< ���s�܂��͔z��T�C�Y.
    uint32_t MipLevels;         //!< �~�b�v���x����.
    uint32_t Format;            //!< �s�N�Z���t�H�[�}�b�g.
    uint32_t SurfaceCount;      //!< �T�[�t�F�C�X��.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// SurfaceDesc structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct SurfaceDesc
{
    uint32_t MipIndex;
    uint32_t RowPitch;
    uint32_t SlicePitch;
};

} // namespace


namespace a3d {

//------------------------------------------------------------------------------------------------
//      MAP�t�@�C������e�N�X�`����ǂݍ��݂܂�.
//-------------------------------------------------------------------------------------------------
bool LoadTexture(IAllocator* pAllocator, const char* path, ResTexture& texture)
{
    FILE* pFile;

    pFile = fopen(path, "rb");
    if (pFile == nullptr)
    { return false; }

    MapHeader header = {};
    fread(&header, sizeof(header), 1, pFile);

    if (header.Magic != ' PAM')
    {
        fclose(pFile);
        return false;
    }

    if (!texture.Surfaces.init(pAllocator, header.SurfaceCount))
    {
        DisposeTexture(texture);
        return false;
    }

    texture.Dimension           = RESOURCE_DIMENSION(header.Dimension);
    texture.Width               = header.Width;
    texture.Height              = header.Height;
    texture.DepthOrArraySize    = header.DepthOrArraySize;
    texture.MipLevels           = header.MipLevels;
    texture.Format              = RESOURCE_FORMAT(header.Format);

    for(auto i=0u; i<header.SurfaceCount; ++i)
    {
        SurfaceDesc surface;
        fread(&surface, sizeof(surface), 1, pFile);

        texture.Surfaces[i].MipIndex    = surface.MipIndex;
        texture.Surfaces[i].RowPitch    = surface.RowPitch;
        texture.Surfaces[i].SlicePitch  = surface.SlicePitch;

        if (!texture.Surfaces[i].Pixels.init(pAllocator, surface.SlicePitch))
        {
            DisposeTexture(texture);
            return false;
        }

        fread(texture.Surfaces[i].Pixels.data(), surface.SlicePitch, 1, pFile);
    }

    fclose(pFile);
    return true;
}

//------------------------------------------------------------------------------------------------
//      MAP�t�@�C���Ƀe�N�X�`�����������݂܂�.
//-------------------------------------------------------------------------------------------------
bool SaveTexture(const char* path, const ResTexture& texture)
{
    FILE* pFile;

    pFile = fopen(path, "wb");
    if (pFile == nullptr)
    { return false; }

    MapHeader header;
    header.Magic            = ' PAM';
    header.Dimension        = uint32_t(texture.Dimension);
    header.Width            = texture.Width;
    header.Height           = texture.Height;
    header.DepthOrArraySize = texture.DepthOrArraySize;
    header.MipLevels        = texture.MipLevels;
    header.Format           = uint32_t(texture.Format);
    header.SurfaceCount     = uint32_t(texture.Surfaces.size());

    for(auto i=0u; i<texture.Surfaces.size(); ++i)
    {
        SurfaceDesc surface;
        surface.MipIndex    = texture.Surfaces[i].MipIndex;
        surface.RowPitch    = texture.Surfaces[i].RowPitch;
        surface.SlicePitch  = texture.Surfaces[i].SlicePitch;

        fwrite(&surface, sizeof(surface), 1, pFile);

        fwrite(texture.Surfaces[i].Pixels.data(), surface.SlicePitch, 1, pFile);
    }

    fclose(pFile);
    return true;
}

//------------------------------------------------------------------------------------------------
//      �e�N�X�`����j�����܂�.
//-------------------------------------------------------------------------------------------------
void DisposeTexture(ResTexture& texture)
{
    texture.Dimension           = RESOURCE_DIMENSION_UNKNOWN;
    texture.Width               = 0;
    texture.Height              = 0;
    texture.DepthOrArraySize    = 0;
    texture.MipLevels           = 0;
    texture.Format              = RESOURCE_FORMAT_UNKNOWN;

    for(auto i=0; texture.Surfaces.size(); ++i)
    { texture.Surfaces[i].Pixels.term(); }

    texture.Surfaces.term();
}

} // namespace a3d
