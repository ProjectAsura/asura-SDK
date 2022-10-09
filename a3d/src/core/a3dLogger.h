//----------------------------------------------------------------------------
// File : a3dLogger.h
// Desc : Logger
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdarg>


namespace a3d {

//-----------------------------------------------------------------------------
//! @brief      ログ出力を行います.
//! 
//! @param[in]      format      フォーマット.
//-----------------------------------------------------------------------------
void OutputLog(const char* format, ...);

} // namespace a3d


#if defined(DEBUG) || defined(_DEBUG)
    #define A3D_LOG(x, ...)     a3d::OutputLog("[File: %s, Line: %d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define A3D_LOG(x, ...)     ((void)0)
#endif