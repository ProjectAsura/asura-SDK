//-------------------------------------------------------------------------------------------------
// File : SampleUtil.cpp
// Desc : Sample Utility.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "SampleUtil.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>


//-------------------------------------------------------------------------------------------------
//      シェーダバイナリを読み込みます.
//-------------------------------------------------------------------------------------------------
bool LoadShaderBinary(const char* filename, a3d::ShaderBinary& result )
{
    FILE* pFile = nullptr;

    // ファイルを開きます.
    #if A3D_IS_WIN
        auto err = fopen_s( &pFile, filename, "rb" );
        if ( err != 0 )
        { return false; }
    #else
        pFile = fopen( filename, "r" );
        if ( pFile == nullptr )
        { return false; }
    #endif

    // ファイルサイズ産出.
    auto curpos = ftell(pFile);
    fseek(pFile, 0, SEEK_END);
    auto endpos = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    auto size = endpos - curpos;

    // メモリ確保.
    auto buf = malloc(size);
    if ( buf == nullptr )
    {
        fclose( pFile );
        return false;
    }

    // ファイル読み込み.
    fread( buf, 1, size, pFile );

    // データを設定.
    result.pByteCode    = buf;
    result.ByteCodeSize = size;
    result.EntryPoint   = "main";

    // ファイルを閉じます.
    fclose( pFile );

    // 正常終了.
    return true;
}


//-------------------------------------------------------------------------------------------------
//      シェーダバイナリを破棄します.
//-------------------------------------------------------------------------------------------------
void DisposeShaderBinary(a3d::ShaderBinary& binary)
{
    if (binary.pByteCode != nullptr)
    { free(binary.pByteCode); }

    memset(&binary, 0, sizeof(binary));
}
