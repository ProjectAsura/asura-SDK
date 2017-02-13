//-------------------------------------------------------------------------------------------------
// File : a3dxResMaterial.cpp
// Desc : Material Resource.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "a3dxResMaterial.h"


namespace {

///////////////////////////////////////////////////////////////////////////////////////////////////
// MAT_STRING structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MAT_STRING
{ 
    char Value[256];        //!< NULL終端文字列です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MAT_FILE_HEADER structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MAT_FILE_HEADER
{
    uint8_t     Tag[4];             //!< ファイルタグです.
    uint32_t    MaterialCount;      //!< マテリアル数です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MAT_DATA_HEADER structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MAT_DATA_HEADER
{
    MAT_STRING      Tag;            //!< タグです.
    uint32_t        ElementCount;   //!< 要素数です.
    uint32_t        TextureCount;   //!< テクスチャ数です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MAT_ELEMENT structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MAT_ELEMENT
{
    MAT_STRING      Tag;            //!< タグです.
    uint32_t        Type;           //!< データ型です.
    uint32_t        Size;           //!< データサイズです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MAT_TEXTURE structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MAT_TEXTURE
{
    MAT_STRING      Tag;            //!< タグです.
    MAT_STRING      Path;           //!< ファイルパスです.
    uint8_t         AddressU;       //!< テクスチャU座標の解決方法です.
    uint8_t         AddressV;       //!< テクスチャV座標の解決方法です.
};

} // namespace


namespace a3dx {

//-------------------------------------------------------------------------------------------------
//      マテリアルリソースを読み込みます.
//-------------------------------------------------------------------------------------------------
bool LoadResMaterial(const char* filename, std::vector<ResMaterial>& resource)
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

    MAT_FILE_HEADER header = {};
    fread(&header, sizeof(header), 1, pFile);

    if (header.Tag[0] != 'M' ||
        header.Tag[1] != 'A' ||
        header.Tag[2] != 'T' ||
        header.Tag[3] != '\0')
    {
        fclose(pFile);
        return false;
    }

    resource.resize(header.MaterialCount);

    for(auto i=0u; i<header.MaterialCount; ++i)
    {
        MAT_DATA_HEADER section = {};
        fread(&section, sizeof(section), 1, pFile);

        resource[i].Tag = std::string(section.Tag.Value);

        resource[i].Elements.resize(section.ElementCount);
        for(auto idx=0u; idx<section.ElementCount; ++idx)
        {
            MAT_ELEMENT element = {};
            fread(&element, sizeof(element), 1, pFile);

            resource[i].Elements[idx].Tag  = std::string(element.Tag.Value);
            resource[i].Elements[idx].Type = RESMAT_ELEMENT_TYPE(element.Type);
            resource[i].Elements[idx].Value.resize(element.Size);

            auto ptr = resource[i].Elements[idx].Value.data();
            fread(ptr, element.Size, 1, pFile);
        }

        resource[i].Textures.resize(section.TextureCount);
        for(auto idx=0u; idx<section.TextureCount; ++idx)
        {
            MAT_TEXTURE texture = {};
            fread(&texture, sizeof(texture), 1, pFile);

            resource[i].Textures[idx].Tag      = std::string(texture.Tag.Value);
            resource[i].Textures[idx].Path     = std::string(texture.Path.Value);
            resource[i].Textures[idx].AddressU = a3d::TEXTURE_ADDRESS_MODE(texture.AddressU);
            resource[i].Textures[idx].AddressV = a3d::TEXTURE_ADDRESS_MODE(texture.AddressV);
        }
    }

    fclose(pFile);

    return true;
}

//-------------------------------------------------------------------------------------------------
//      マテリアルリソースを書き込みます.
//-------------------------------------------------------------------------------------------------
bool SaveResMaterial(const char* filename, const std::vector<ResMaterial>& resource)
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

    MAT_FILE_HEADER header = {};
    header.Tag[0]        = 'M';
    header.Tag[1]        = 'A';
    header.Tag[2]        = 'T';
    header.Tag[3]        = '\0';
    header.MaterialCount = uint32_t(resource.size());
    fwrite(&header, sizeof(header), 1, pFile);

    for(size_t i=0; i<resource.size(); ++i)
    {
        MAT_DATA_HEADER section = {};
        strcpy_s( section.Tag.Value, resource[i].Tag.c_str() );
        section.ElementCount = uint32_t(resource[i].Elements.size());
        section.TextureCount = uint32_t(resource[i].Textures.size());
        fwrite(&section, sizeof(section), 1, pFile);

        for(size_t idx=0; idx<resource[i].Elements.size(); ++idx)
        {
            MAT_ELEMENT element = {};
            strcpy_s( element.Tag.Value, resource[i].Elements[idx].Tag.c_str() );
            element.Type = uint32_t(element.Type);
            element.Size = uint32_t(resource[i].Elements[idx].Value.size());
            fwrite(&element, sizeof(element), 1, pFile);

            auto ptr = resource[i].Elements[idx].Value.data();
            fwrite(ptr, resource[i].Elements[idx].Value.size(), 1, pFile);
        }

        for(size_t idx=0; idx<resource[i].Textures.size(); ++idx)
        {
            MAT_TEXTURE texture = {};
            strcpy_s( texture.Tag.Value,  resource[i].Textures[idx].Tag.c_str() );
            strcpy_s( texture.Path.Value, resource[i].Textures[idx].Path.c_str() );
            texture.AddressU = uint32_t( resource[i].Textures[idx].AddressU );
            texture.AddressV = uint32_t( resource[i].Textures[idx].AddressV );
            fwrite(&texture, sizeof(texture), 1, pFile);
        }
    }

    fclose(pFile);

    return true;
}

} // namespace a3dx
