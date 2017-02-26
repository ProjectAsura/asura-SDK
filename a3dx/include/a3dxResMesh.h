﻿//-------------------------------------------------------------------------------------------------
// File : a3dxResMesh.h
// Desc : Mesh Resource.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <vector>
#include <a3dxMath.h>


namespace a3dx {

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
    Matrix                       BindPose;       //!< バインドポーズ行列です.
    std::vector<ResVertexWeight> Weight;         //!< 頂点の重みです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResMorphMesh structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResMorphMesh
{
    std::vector<Vector3>    Position;   //!< 位置座標.
    std::vector<Vector3>    Normal;     //!< 法線ベクトル.
    std::vector<Vector3>    Tangent;    //!< 接線ベクトル.
    std::vector<Vector4>    Color;      //!< 頂点カラー.
    std::vector<Vector2>    TexCoord;   //!< テクスチャ座標.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResMesh structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResMesh
{
    std::string                 Tag;            //!< タグです.
    std::vector<Vector3>        Position;       //!< 位置座標です.
    std::vector<Vector2>        TexCoord;       //!< テクスチャ座標です.
    std::vector<Vector3>        Normal;         //!< 法線ベクトルです.
    std::vector<Vector3>        Tangent;        //!< 接線ベクトルです.
    std::vector<Vector4>        Color;          //!< 頂点カラーです.
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

} // namespace a3dx
