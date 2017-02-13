//-------------------------------------------------------------------------------------------------
// File : a3dxResMesh.cpp
// Desc : Mesh Resource.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "a3dxResMesh.h"
#include <cstdio>


namespace {

///////////////////////////////////////////////////////////////////////////////////////////////////
// MSH_STRING structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MSH_STRING
{
    char    Value[256];     //!< NULL終端文字列です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MSH_FILE_HEADER structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MSH_FILE_HEADER
{
    uint8_t     Tag[4];         //!< タグです.
    uint32_t    MeshCount;      //!< メッシュ数です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MSH_STRING strcuture
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MSH_DATA_HEADER
{
    MSH_STRING  Tag;                //!< タグです.
    uint32_t    PositionCount;      //!< 位置座標数です.
    uint32_t    TexCoordCount;      //!< テクスチャ座標数です.
    uint32_t    NormalCount;        //!< 法線ベクトル数です.
    uint32_t    TangentCount;       //!< 接線ベクトル数です.
    uint32_t    BoneIndexCount;     //!< ボーンインデックス数です.
    uint32_t    BoneWeightCount;    //!< ボーンウェイト数です.
    uint32_t    BoneCount;          //!< ボーン数です.
    MSH_STRING  MaterialTag;        //!< マテリアルタグです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MSH_BONE structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MSH_BONE
{
    MSH_STRING  Tag;                //!< タグです.
    uint32_t    ParentId;           //!< 親ボーン番号です.
    float       BindPose[16];       //!< バインドポーズ行列です.
    float       InvBindPose[16];    //!< 逆バインドポーズ行列です.
};

} // namespace


namespace a3dx {

//-------------------------------------------------------------------------------------------------
//      メッシュリソースを読み込みます.
//-------------------------------------------------------------------------------------------------
bool LoadResMesh(const char* filename, std::vector<ResMesh>& resource)
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

    MSH_FILE_HEADER header = {};
    fread( &header, sizeof(header), 1, pFile );

    if (header.Tag[0] != 'M' ||
        header.Tag[1] != 'S' ||
        header.Tag[2] != 'H' ||
        header.Tag[3] != '\0' )
    {
        fclose(pFile);
        return false;
    }

    resource.resize(header.MeshCount);

    for(auto i=0u; i<header.MeshCount; ++i)
    {
        MSH_DATA_HEADER section = {};
        fread( &section, sizeof(section), 1, pFile );

        resource[i].Tag = std::string(section.Tag.Value);

        resource[i].Position.resize(section.PositionCount);
        for(auto idx=0u; idx<section.PositionCount; ++idx)
        { fread( &resource[i].Position[idx], sizeof(Vector3), 1, pFile ); }

        resource[i].TexCoord.resize(section.TexCoordCount);
        for(auto idx=0u; idx<section.TangentCount; ++idx)
        { fread( &resource[i].TexCoord[idx], sizeof(Vector2), 1, pFile ); }

        resource[i].Normal.resize(section.NormalCount);
        for(auto idx=0u; idx<section.NormalCount; ++idx)
        { fread( &resource[i].Normal[idx], sizeof(Vector3), 1, pFile ); }

        resource[i].Tangent.resize(section.TangentCount);
        for(auto idx=0u; idx<section.TangentCount; ++idx)
        { fread( &resource[i].Tangent[idx], sizeof(Vector3), 1, pFile ); }

        resource[i].BoneIndex.resize(section.BoneIndexCount);
        for(auto idx=0u; idx<section.BoneIndexCount; ++idx)
        { fread( &resource[i].BoneIndex[idx], sizeof(ResBoneIndex), 1, pFile ); }

        resource[i].BoneWeight.resize(section.BoneWeightCount);
        for(auto idx=0u; idx<section.BoneWeightCount; ++idx)
        { fread( &resource[i].BoneWeight[idx], sizeof(Vector4), 1, pFile); }

        resource[i].Bone.resize(section.BoneCount);
        for(auto idx=0u; idx<section.BoneCount; ++idx)
        {
            MSH_BONE bone = {};
            fread(&bone, sizeof(bone), 1, pFile);
            resource[i].Bone[idx].Tag         = std::string(bone.Tag.Value);
            resource[i].Bone[idx].ParentId    = bone.ParentId;
            resource[i].Bone[idx].BindPose    = Matrix(bone.BindPose);
            resource[i].Bone[idx].InvBindPose = Matrix(bone.InvBindPose);
        }

        resource[i].MaterialTag = std::string(section.MaterialTag.Value);
    }

    fclose(pFile);

    return true;
}

//-------------------------------------------------------------------------------------------------
//      メッシュリソースを書き込みます.
//-------------------------------------------------------------------------------------------------
bool SaveResMesh(const char* filename, const std::vector<ResMesh>& resource)
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

    MSH_FILE_HEADER header = {};
    header.Tag[0]    = 'M';
    header.Tag[1]    = 'S';
    header.Tag[2]    = 'H';
    header.Tag[3]    = '\0';
    header.MeshCount = uint32_t(resource.size());

    fwrite(&header, sizeof(header), 1, pFile);

    for(size_t i=0; i<resource.size(); ++i)
    {
        MSH_DATA_HEADER section = {};
        strcpy_s( section.Tag.Value, resource[i].Tag.c_str() );
        section.PositionCount    = uint32_t(resource[i].Position    .size());
        section.TangentCount     = uint32_t(resource[i].TexCoord    .size());
        section.NormalCount      = uint32_t(resource[i].Normal      .size());
        section.TangentCount     = uint32_t(resource[i].Tangent     .size());
        section.BoneIndexCount   = uint32_t(resource[i].BoneIndex   .size());
        section.BoneWeightCount  = uint32_t(resource[i].BoneWeight  .size());
        section.BoneCount        = uint32_t(resource[i].Bone        .size());
        strcpy_s( section.MaterialTag.Value, resource[i].MaterialTag.c_str() );

        fwrite(&section, sizeof(section), 1, pFile);

        for(size_t idx=0; idx<resource[i].Position.size(); ++idx)
        { fwrite(&resource[i].Position[idx], sizeof(Vector3), 1, pFile); }

        for(size_t idx=0; idx<resource[i].TexCoord.size(); ++idx)
        { fwrite(&resource[i].TexCoord[idx], sizeof(Vector2), 1, pFile); }

        for(size_t idx=0; idx<resource[i].Normal.size(); ++idx)
        { fwrite(&resource[i].Normal[idx], sizeof(Vector3), 1, pFile); }

        for(size_t idx=0; idx<resource[i].Tangent.size(); ++idx)
        { fwrite(&resource[i].Tangent[idx], sizeof(Vector3), 1, pFile); }

        for(size_t idx=0; idx<resource[i].BoneIndex.size(); ++idx)
        { fwrite(&resource[i].BoneIndex[idx], sizeof(ResBoneIndex), 1, pFile); }

        for(size_t idx=0; idx<resource[i].BoneWeight.size(); ++idx)
        { fwrite(&resource[i].BoneWeight[idx], sizeof(Vector4), 1, pFile); }

        MSH_STRING matTag = {};
        strcpy_s( matTag.Value, resource[i].MaterialTag.c_str() );
        fwrite(&matTag, sizeof(matTag), 1, pFile);
    }

    fclose(pFile);
    return true;
}

} // namespace a3dx
