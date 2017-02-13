//-------------------------------------------------------------------------------------------------
// File : a3dxResMaterial.h
// Desc : Material Resource.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <string>
#include <vector>
#include <a3d.h>


namespace a3dx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// RESMAT_ELEMENT_TYPE
///////////////////////////////////////////////////////////////////////////////////////////////////
enum RESMAT_ELEMENT_TYPE
{
    RESMAT_ELEMENT_TYPE_BOOL = 0,       //!< bool型です.
    RESMAT_ELEMENT_TYPE_SINT8,          //!< int8_t 型です.
    RESMAT_ELEMENT_TYPE_UINT8,          //!< uint8_t 型です.
    RESMAT_ELEMENT_TYPE_SINT16,         //!< int16_t 型です.
    RESMAT_ELEMENT_TYPE_UINT16,         //!< uint16_t 型です.
    RESMAT_ELEMENT_TYPE_SINT32,         //!< int32_t 型です.
    RESMAT_ELEMENT_TYPE_UINT32,         //!< uint32_t 型です.
    RESMAT_ELEMENT_TYPE_SINT64,         //!< int64_t 型です.
    RESMAT_ELEMENT_TYPE_UINT64,         //!< uint64_t 型です.
    RESMAT_ELEMENT_TYPE_FLOAT16,        //!< half 型です.
    RESMAT_ELEMENT_TYPE_FLOAT32,        //!< float 型です.
    RESMAT_ELEMENT_TYPE_FLOAT64,        //!< double 型です.
    RESMAT_ELEMENT_TYPE_VECTOR2,        //!< a3dx::Vector2 型です.
    RESMAT_ELEMENT_TYPE_VECTOR3,        //!< a3dx::Vector3 型です.
    RESMAT_ELEMENT_TYPE_VECTOR4,        //!< a3dx::Vector4 型です.
    RESMAT_ELEMENT_TYPE_QUATERNION,     //!< a3dx::Quaternion 型です.
    RESMAT_ELEMENT_TYPE_MATRIX,         //!< a3dx::Matrix 型です.
    RESMAT_ELEMENT_TYPE_STRING,         //!< char [256] 型です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResMatElement structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResMatElement
{
    std::string             Tag;        //!< タグです.
    RESMAT_ELEMENT_TYPE     Type;       //!< タイプです.
    std::vector<uint8_t>    Value;      //!< 要素の値です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResMatTexture structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResMatTexture
{
    std::string                 Tag;        //!< タグです.
    std::string                 Path;       //!< ファイルパスです.
    a3d::TEXTURE_ADDRESS_MODE   AddressU;   //!< テクスチャU座標の解決方法です.
    a3d::TEXTURE_ADDRESS_MODE   AddressV;   //!< テクスチャV座標の解決方法です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResMaterial structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResMaterial
{
    std::string                     Tag;        //!< タグです.
    std::vector<ResMatElement>      Elements;   //!< マテリアル要素の定義です.
    std::vector<ResMatTexture>      Textures;   //!< テクスチャデータです.
};

//-------------------------------------------------------------------------------------------------
//! @brief      マテリアルリソースを読み込みます.
//!
//! @param[in]      filename        ファイルパスです.
//! @param[out]     resource        リソースの格納先です.
//! @retval true    読み込みに成功.
//! @retval false   読み込みに失敗.
//-------------------------------------------------------------------------------------------------
bool LoadResMaterial(const char* filename, std::vector<ResMaterial>& resource);

//-------------------------------------------------------------------------------------------------
//! @brief      マテリアルリソースを書き込みます.
//!
//! @param[in]      filename        ファイルパスです.
//! @param[in]      resource        書き出しするリソースです.
//! @retval true    書き込みに成功.
//! @retval false   書き込みに失敗.
//-------------------------------------------------------------------------------------------------
bool SaveResMaterial(const char* filename, const std::vector<ResMaterial>& resource);

} // namespace a3dx
