﻿//-------------------------------------------------------------------------------------------------
// File : fndHash.h
// Desc : Hash Key.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstdint>


namespace fnd {

//-------------------------------------------------------------------------------------------------
//! @brief      ハッシュ値を求めます.
//!
//! @param[in]      count       バッファサイズです.
//! @param[in]      pBuffer     バッファです.
//! @return     FNV-1アルゴリズムにより求めたハッシュ値を返却します.
//-------------------------------------------------------------------------------------------------
inline size_t CalcHash(size_t count, const uint8_t* pBuffer)
{
    auto ret = 2166136261;

    for(size_t i=0; i<count; ++i)
    { ret = (16777619 * ret) ^ pBuffer[i]; }

    return ret;
}

} // namespace fnd
