//----------------------------------------------------------------------------
// File : a3dDescSettings.cpp
// Desc : Desc Structure Settings.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>


namespace a3d {

//-----------------------------------------------------------------------------
//      ログ出力を行います.
//-----------------------------------------------------------------------------
void OutputLog(const char* format, ...)
{
    #if A3D_IS_WIN
        char temp[2048] = {};
        va_list va;
        va_start(va, format);
        vsprintf_s( temp, format, va );
        va_end(va);

        OutputDebugStringA(temp);
        printf_s( "%s", temp );
    #else
        va_list va;
        va_start(va, format);
        vprintf(format, va);
        va_end(va);
    #endif
}

} // namespace a3d