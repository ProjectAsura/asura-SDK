﻿//-------------------------------------------------------------------------------------------------
// File : aut_Hash.h
// Desc : Hash Key.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstdint>


namespace aut {

//-------------------------------------------------------------------------------------------------
// Constant Values
//-------------------------------------------------------------------------------------------------
constexpr uint32_t FNV_OFFSET_BASIS_32   = 2166136261;
constexpr uint32_t FNV_PRIME_32          = 16777619;


//-------------------------------------------------------------------------------------------------
//! @brief      FNV-1 ハッシュを計算します.
//-------------------------------------------------------------------------------------------------
inline uint32_t CalcFnv1(uint32_t count, const uint8_t* pBuffer)
{
    auto ret = FNV_OFFSET_BASIS_32;
    for( auto i=0u; i<count; ++i )
    { ret = (FNV_PRIME_32 * ret) ^ pBuffer[ i ]; }
    return ret;
}

//-------------------------------------------------------------------------------------------------
//! @brief      FNV-1A ハッシュを計算します.
//-------------------------------------------------------------------------------------------------
inline uint32_t CalcFnv1a(uint32_t count, const uint8_t* pBuffer)
{
    auto ret = FNV_OFFSET_BASIS_32;
    for( auto i=0u; i<count; ++i )
    { ret = ( ret ^ pBuffer[i] ) * FNV_PRIME_32; }
    return ret;
}

} // namespace aut