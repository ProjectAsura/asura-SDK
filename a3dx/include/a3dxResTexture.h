﻿//-------------------------------------------------------------------------------------------------
// File : a3dxResTexture.h
// Desc : Texture Resource.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <vector>
#include <a3d.h>


namespace a3dx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResSurface structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResSurface
{
    uint32_t                Width;          //!< 横幅です.
    uint32_t                Height;         //!< 縦幅です.
    uint32_t                RowPitch;       //!< 1行あたりのデータサイズ(バイト単位)です.
    uint32_t                SlicePitch;     //!< 1スライスあたりのデータサイズ(バイト単位)です.
    std::vector<uint8_t>    Pixel;          //!< ピクセルデータです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResTexture structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResTexture
{
    a3d::RESOURCE_DIMENSION Dimension;          //!< 次元です.
    uint32_t                Width;              //!< 横幅です.
    uint32_t                Height;             //!< 縦幅です.
    uint32_t                DepthOrArraySize;   //!< 奥行または配列数です.
    uint32_t                MipLevels;          //!< ミップレベル数です.
    a3d::RESOURCE_FORMAT    Format;             //!< ピクセルフォーマットです.
    std::vector<ResSurface> Surfaces;           //!< サーフェイスリソースです.
};

//-------------------------------------------------------------------------------------------------
//! @brief      テクスチャリソースを読み込みます.
//!
//! @param[in]      filename        ファイルパスです.
//! @param[in]      resource        リソースの格納先です.
//! @retval true    読み込みに成功.
//! @retval false   読み込みに失敗.
//-------------------------------------------------------------------------------------------------
bool LoadResTexture(const char* filename, ResTexture& resource);

//-------------------------------------------------------------------------------------------------
//! @brief      テクスチャリソースを書き込みます.
//!
//! @param[in]      filename        ファイルパスです.
//! @param[in]      resource        書き出すリソースです.
//! @retval true    書き出しに成功.
//! @retval false   書き出しに失敗.
//-------------------------------------------------------------------------------------------------
bool SaveResTexture(const char* filename, const ResTexture& resource);

} // namespace a3dx