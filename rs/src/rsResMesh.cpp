//-------------------------------------------------------------------------------------------------
// File : rsResMesh.cpp
// Desc : Mesh Resource.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "rsResMesh.h"
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
    uint32_t    ColorCount;         //!< 頂点カラー数です.
    uint32_t    BoneCount;          //!< ボーン数です.
    uint32_t    MorphMeshCount;     //!< モーフメッシュ数です.
    uint32_t    MaterialId;         //!< マテリアルIDです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MSH_BONE structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MSH_BONE
{
    MSH_STRING  Tag;                //!< タグです.
    float       BindPose[16];       //!< バインドポーズ行列です.
    uint32_t    WeightCount;        //!< ウェイト数です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MSH_MORPH_MESH structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MSH_MORPH_MESH
{
    uint32_t    PositionCount;      //!< 位置座標です.
    uint32_t    NormalCount;        //!< 法線ベクトル数です.
    uint32_t    TangentCount;       //!< 接線ベクトル数です.
    uint32_t    CountCount;         //!< 頂点カラー数です.
    uint32_t    TexCoordCount;      //!< テクスチャ座標数です.
};

} // namespace


namespace rs {

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
        { fread( &resource[i].Position[idx], sizeof(fnd::Vector3), 1, pFile ); }

        resource[i].TexCoord.resize(section.TexCoordCount);
        for(auto idx=0u; idx<section.TangentCount; ++idx)
        { fread( &resource[i].TexCoord[idx], sizeof(fnd::Vector2), 1, pFile ); }

        resource[i].Normal.resize(section.NormalCount);
        for(auto idx=0u; idx<section.NormalCount; ++idx)
        { fread( &resource[i].Normal[idx], sizeof(fnd::Vector3), 1, pFile ); }

        resource[i].Tangent.resize(section.TangentCount);
        for(auto idx=0u; idx<section.TangentCount; ++idx)
        { fread( &resource[i].Tangent[idx], sizeof(fnd::Vector3), 1, pFile ); }

        resource[i].Color.resize(section.ColorCount);
        for(auto idx=0u; idx<section.ColorCount; ++idx)
        { fread( &resource[i].Color[idx], sizeof(fnd::Vector4), 1, pFile); }

        resource[i].Bone.resize(section.BoneCount);
        for(auto idx=0u; idx<section.BoneCount; ++idx)
        {
            MSH_BONE bone = {};
            fread(&bone, sizeof(bone), 1, pFile);
            resource[i].Bone[idx].Tag         = std::string(bone.Tag.Value);
            resource[i].Bone[idx].BindPose    = fnd::Matrix(bone.BindPose);
            resource[i].Bone[idx].Weight.resize(bone.WeightCount);

            for(auto w=0u; w<bone.WeightCount; ++w)
            { fread(&resource[i].Bone[idx].Weight[w], sizeof(ResVertexWeight), 1, pFile); }
        }

        resource[i].MorphMesh.resize(section.MorphMeshCount);
        for(auto idx=0u; idx<section.MorphMeshCount; ++idx)
        {
            MSH_MORPH_MESH info = {};
            fread(&info, sizeof(info), 1, pFile);

            resource[i].MorphMesh[idx].Position.resize(info.PositionCount);
            for(auto j=0u; j<info.PositionCount; ++j)
            { fread(resource[i].MorphMesh[idx].Position[j], sizeof(fnd::Vector3), 1, pFile); }

            resource[i].MorphMesh[idx].Normal.resize(info.NormalCount);
            for(auto j=0u; j<info.NormalCount; ++j)
            { fread(resource[i].MorphMesh[idx].Normal[j], sizeof(fnd::Vector3), 1, pFile); }

            resource[i].MorphMesh[idx].Tangent.resize(info.TangentCount);
            for(auto j=0u; j<info.TangentCount; ++j)
            { fread(resource[i].MorphMesh[idx].Tangent[j], sizeof(fnd::Vector3), 1, pFile); }

            resource[i].MorphMesh[idx].Color.resize(info.CountCount);
            for(auto j=0u; j<info.CountCount; ++j)
            { fread(resource[i].MorphMesh[idx].Color[j], sizeof(fnd::Vector4), 1, pFile); }

            resource[i].MorphMesh[idx].TexCoord.resize(info.TexCoordCount);
            for(auto j=0u; j<info.TexCoordCount; ++j)
            { fread(resource[i].MorphMesh[idx].TexCoord[j], sizeof(fnd::Vector2), 1, pFile); }
        }

        resource[i].MaterialId = section.MaterialId;
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
        section.PositionCount    = uint32_t(resource[i].Position .size());
        section.TangentCount     = uint32_t(resource[i].TexCoord .size());
        section.NormalCount      = uint32_t(resource[i].Normal   .size());
        section.TangentCount     = uint32_t(resource[i].Tangent  .size());
        section.ColorCount       = uint32_t(resource[i].Color    .size()); 
        section.BoneCount        = uint32_t(resource[i].Bone     .size());
        section.MorphMeshCount   = uint32_t(resource[i].MorphMesh.size());
        section.MaterialId       = resource[i].MaterialId;

        fwrite(&section, sizeof(section), 1, pFile);

        for(size_t idx=0; idx<resource[i].Position.size(); ++idx)
        { fwrite(&resource[i].Position[idx], sizeof(fnd::Vector3), 1, pFile); }

        for(size_t idx=0; idx<resource[i].TexCoord.size(); ++idx)
        { fwrite(&resource[i].TexCoord[idx], sizeof(fnd::Vector2), 1, pFile); }

        for(size_t idx=0; idx<resource[i].Normal.size(); ++idx)
        { fwrite(&resource[i].Normal[idx], sizeof(fnd::Vector3), 1, pFile); }

        for(size_t idx=0; idx<resource[i].Tangent.size(); ++idx)
        { fwrite(&resource[i].Tangent[idx], sizeof(fnd::Vector3), 1, pFile); }

        for(size_t idx=0; idx<resource[i].Color.size(); ++idx)
        { fwrite(&resource[i].Color[idx], sizeof(fnd::Vector4), 1, pFile); }

        for(size_t idx=0; idx<resource[i].Bone.size(); ++idx)
        {
            MSH_BONE info = {};
            strcpy_s( info.Tag.Value, resource[i].Bone[idx].Tag.c_str() );
            memcpy( &info.BindPose[0], &resource[i].Bone[idx].BindPose._11, sizeof(float) * 16);
            info.WeightCount = uint32_t(resource[i].Bone[idx].Weight.size());

            for(size_t w=0; w<resource[i].Bone[idx].Weight.size(); ++w)
            { fwrite(&resource[i].Bone[idx].Weight[w], sizeof(ResVertexWeight), 1, pFile); }
        }

        for(size_t idx=0; idx<resource[i].MorphMesh.size(); ++idx)
        {
            MSH_MORPH_MESH info = {};
            info.PositionCount = uint32_t(resource[i].MorphMesh[idx].Position.size());
            info.NormalCount   = uint32_t(resource[i].MorphMesh[idx].Normal  .size());
            info.TexCoordCount = uint32_t(resource[i].MorphMesh[idx].TexCoord.size());
            info.CountCount    = uint32_t(resource[i].MorphMesh[idx].Color   .size());
            info.TexCoordCount = uint32_t(resource[i].MorphMesh[idx].TexCoord.size());

            for(size_t j=0; j<resource[i].MorphMesh[idx].Position.size(); ++j)
            { fwrite(&resource[i].MorphMesh[idx].Position[j], sizeof(fnd::Vector3), 1, pFile); }

            for(size_t j=0; j<resource[i].MorphMesh[idx].Normal.size(); ++j)
            { fwrite(&resource[i].MorphMesh[idx].Normal[j], sizeof(fnd::Vector3), 1, pFile); }

            for(size_t j=0; j<resource[i].MorphMesh[idx].Tangent.size(); ++j)
            { fwrite(&resource[i].MorphMesh[idx].Tangent[j], sizeof(fnd::Vector3), 1, pFile); }

            for(size_t j=0; j<resource[i].MorphMesh[idx].Color.size(); ++j)
            { fwrite(&resource[i].MorphMesh[idx].Color[j], sizeof(fnd::Vector4), 1, pFile); }

            for(size_t j=0; j<resource[i].MorphMesh[idx].TexCoord.size(); ++j)
            { fwrite(&resource[i].MorphMesh[idx].TexCoord[j], sizeof(fnd::Vector2), 1, pFile); }
        }

        fwrite(&resource[i].MaterialId, sizeof(uint32_t), 1, pFile);
    }

    fclose(pFile);
    return true;
}

} // namespace rs
