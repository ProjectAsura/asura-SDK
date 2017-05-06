//-------------------------------------------------------------------------------------------------
// File : rsResMesh.h
// Desc : Mesh Resource.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <vector>
#include <fndMath.h>


namespace rs {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResVertexWeight structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResVertexWeight
{
    uint32_t    VertexId;       //!< 頂点番号です.
    float       Weight;         //!< 頂点の重みです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResBone structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResBone
{
    std::string                  Tag;            //!< タグです.
    fnd::Matrix                  BindPose;       //!< バインドポーズ行列です.
    std::vector<ResVertexWeight> Weight;         //!< 頂点の重みです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResMorphMesh structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResMorphMesh
{
    std::vector<fnd::Vector3>    Position;   //!< 位置座標.
    std::vector<fnd::Vector3>    Normal;     //!< 法線ベクトル.
    std::vector<fnd::Vector3>    Tangent;    //!< 接線ベクトル.
    std::vector<fnd::Vector4>    Color;      //!< 頂点カラー.
    std::vector<fnd::Vector2>    TexCoord;   //!< テクスチャ座標.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResMesh structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResMesh
{
    std::string                 Tag;            //!< タグです.
    std::vector<fnd::Vector3>   Position;       //!< 位置座標です.
    std::vector<fnd::Vector2>   TexCoord;       //!< テクスチャ座標です.
    std::vector<fnd::Vector3>   Normal;         //!< 法線ベクトルです.
    std::vector<fnd::Vector3>   Tangent;        //!< 接線ベクトルです.
    std::vector<fnd::Vector4 >  Color;          //!< 頂点カラーです.
    std::vector<ResBone>        Bone;           //!< ボーンです.
    std::vector<uint32_t>       VertexIndex;    //!< 頂点インデックスです.
    std::vector<ResMorphMesh>   MorphMesh;      //!< モーフメッシュです.
    std::uint32_t               MaterialId;     //!< マテリアルIDです.
};

//-------------------------------------------------------------------------------------------------
//! @brief      メッシュリソースを読み込みます.
//!
//! @param[in]      filename        ファイルパスです.
//! @param[out]     resource        メッシュリソースの格納先です.
//! @retval true    読み込みに成功.
//! @retval false   読み込みに失敗.
//-------------------------------------------------------------------------------------------------
bool LoadResMesh(const char* filename, std::vector<ResMesh>& resource);

//-------------------------------------------------------------------------------------------------
//! @brief      メッシュリソースを書き込みます.
//!
//! @param[in]      filename        ファイルパスです.
//! @param[in]      resource        メッシュリソースです.
//! @retval true    書き込みに成功.
//! @retval false   書き込みに失敗.
//-------------------------------------------------------------------------------------------------
bool SaveResMesh(const char* filename, const std::vector<ResMesh>& resource);

} // namespace rs
