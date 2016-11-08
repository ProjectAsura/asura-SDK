//--------------------------------------------------------------------------------------------------
// File : asdxTarga.cpp
// Desc : Targa Module.
// Copyright(c) Project Asura. All right reserved.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------------------------------------
#include <cstdio>
#include <cstring>
#include <cassert>
#include <new>
#include "SampleTarga.h"


#ifndef ELOG
#define ELOG( x, ... )  fprintf_s( stderr, "[File: %s, Line:%d] " x, __FILE__, __LINE__, ##__VA_ARGS__ )
#endif//ELOG


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//! @brief      8Bitインデックスカラー形式を解析します.
//!
//! @param[in]      pColorMap       カラーマップです.
//-------------------------------------------------------------------------------------------------
void Parse8Bits( FILE* pFile, uint32_t size, uint8_t* pColorMap, uint8_t* pPixels )
{
    uint8_t color = 0;
    for( uint32_t i=0; i<size; ++i )
    {
        color = (uint8_t)fgetc( pFile );
        pPixels[ i * 3 + 2 ] = pColorMap[ color * 3 + 0 ];
        pPixels[ i * 3 + 1 ] = pColorMap[ color * 3 + 1 ];
        pPixels[ i * 3 + 0 ] = pColorMap[ color * 3 + 2 ];
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      16Bitフルカラー形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse16Bits( FILE* pFile, uint32_t size, uint8_t* pPixels )
{
    for( uint32_t i=0; i<size; ++i )
    {
        uint16_t color = fgetc( pFile ) + ( fgetc( pFile ) << 8 );
        pPixels[ i * 3 + 0 ] = (uint8_t)(( ( color & 0x7C00 ) >> 10 ) << 3);
        pPixels[ i * 3 + 1 ] = (uint8_t)(( ( color & 0x03E0 ) >>  5 ) << 3);
        pPixels[ i * 3 + 2 ] = (uint8_t)(( ( color & 0x001F ) >>  0 ) << 3);
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      24Bitフルカラー形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse24Bits( FILE* pFile, uint32_t size, uint8_t* pPixels )
{
    for( uint32_t i=0; i<size; ++i )
    {
        pPixels[ i * 4 + 2 ] = (uint8_t)fgetc( pFile );
        pPixels[ i * 4 + 1 ] = (uint8_t)fgetc( pFile );
        pPixels[ i * 4 + 0 ] = (uint8_t)fgetc( pFile );
        pPixels[ i * 4 + 3 ] = 255;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      32Bitフルカラー形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse32Bits( FILE* pFile, uint32_t size, uint8_t* pPixels )
{
    for( uint32_t i=0; i<size; ++i )
    {
        pPixels[ i * 4 + 2 ] = (uint8_t)fgetc( pFile );
        pPixels[ i * 4 + 1 ] = (uint8_t)fgetc( pFile );
        pPixels[ i * 4 + 0 ] = (uint8_t)fgetc( pFile );
        pPixels[ i * 4 + 3 ] = (uint8_t)fgetc( pFile );
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief     8Bitグレースケール形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse8BitsGrayScale( FILE* pFile, uint32_t size, uint8_t* pPixels )
{
    for( uint32_t i=0; i<size; ++i )
    {
        pPixels[ i ] = (uint8_t)fgetc( pFile );
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      16Bitグレースケール形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse16BitsGrayScale( FILE* pFile, uint32_t size, uint8_t* pPixels )
{
    for( uint32_t i=0; i<size; ++i )
    {
        pPixels[ i * 2 + 0 ] = (uint8_t)fgetc( pFile );
        pPixels[ i * 2 + 1 ] = (uint8_t)fgetc( pFile );
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      8BitRLE圧縮インデックスカラー形式を解析します.
//!
//! @param[in]  pColorMap       カラーマップです.
//-------------------------------------------------------------------------------------------------
void Parse8BitsRLE( FILE* pFile, uint8_t* pColorMap, uint32_t size, uint8_t* pPixels )
{
    uint32_t count  = 0;
    uint8_t  color  = 0;
    uint8_t  header = 0;
    uint8_t* ptr    = pPixels;

    while( ptr < pPixels + size )   // size = width * height * 3.
    {
        header = (uint8_t)fgetc( pFile );
        count = 1 + ( header & 0x7F );

        if ( header & 0x80 )
        {
            color = (uint8_t)fgetc( pFile );

            for( uint32_t i=0; i<count; ++i, ptr+=3 )
            {
                ptr[ 0 ] = pColorMap[ color * 3 + 2 ];
                ptr[ 1 ] = pColorMap[ color * 3 + 1 ];
                ptr[ 2 ] = pColorMap[ color * 3 + 0 ];
            }
        }
        else
        {
            for( uint32_t i=0; i<count; ++i, ptr+=3 )
            {
                color = (uint8_t)fgetc( pFile );

                ptr[ 0 ] = pColorMap[ color * 3 + 2 ];
                ptr[ 1 ] = pColorMap[ color * 3 + 1 ];
                ptr[ 2 ] = pColorMap[ color * 3 + 0 ];
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      16BitRLE圧縮フルカラー形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse16BitsRLE( FILE* pFile, uint32_t size, uint8_t* pPixels )
{
    uint32_t count  = 0;
    uint16_t color  = 0;
    uint8_t  header = 0;
    uint8_t* ptr    = pPixels;

    while( ptr < pPixels + size )   // size = width * height * 3.
    {
        header = (uint8_t)fgetc( pFile );
        count = 1 + ( header & 0x7F );

        if ( header & 0x80 )
        {
            color = fgetc( pFile ) + ( fgetc( pFile ) << 8 ); 

            for( uint32_t i=0; i<count; ++i, ptr+=3 )
            {
                ptr[ 0 ] = (uint8_t)(( ( color & 0x7C00 ) >> 10 ) << 3);
                ptr[ 1 ] = (uint8_t)(( ( color & 0x03E0 ) >>  5 ) << 3);
                ptr[ 2 ] = (uint8_t)(( ( color & 0x001F ) >>  0 ) << 3);
            }
        }
        else
        {
            for( uint32_t i=0; i<count; ++i, ptr+=3 )
            {
                color = fgetc( pFile ) + ( fgetc( pFile ) << 8 );

                ptr[ 0 ] = (uint8_t)(( ( color & 0x7C00 ) >> 10 ) << 3);
                ptr[ 1 ] = (uint8_t)(( ( color & 0x03E0 ) >>  5 ) << 3);
                ptr[ 2 ] = (uint8_t)(( ( color & 0x001F ) >>  0 ) << 3);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      24BitRLE圧縮フルカラー形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse24BitsRLE( FILE* pFile, uint32_t size, uint8_t* pPixels )
{
    uint32_t count    = 0;
    uint8_t  color[3] = { 0, 0, 0 };
    uint8_t  header   = 0;
    uint8_t* ptr      = pPixels;

    while( ptr < pPixels + size )   // size = width * height * 3.
    {
        header = (uint8_t)fgetc( pFile );
        count = 1 + ( header & 0x7F );

        if ( header & 0x80 )
        {
            fread( color, sizeof(uint8_t), 3, pFile );

            for( uint32_t i=0; i<count; ++i, ptr+=3 )
            {
                ptr[ 0 ] = color[ 2 ];
                ptr[ 1 ] = color[ 1 ];
                ptr[ 2 ] = color[ 0 ];
            }
        }
        else
        {
            for( uint32_t i=0; i<count; ++i, ptr+=3 )
            {
                ptr[ 2 ] = (uint8_t)fgetc( pFile );
                ptr[ 1 ] = (uint8_t)fgetc( pFile );
                ptr[ 0 ] = (uint8_t)fgetc( pFile );
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      32BitRLE圧縮フルカラー形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse32BitsRLE( FILE* pFile, uint32_t size, uint8_t* pPixels )
{
    uint32_t count    = 0;
    uint8_t  color[4] = { 0, 0, 0, 0 };
    uint8_t  header   = 0;
    uint8_t* ptr      = pPixels;

    while( ptr < pPixels + size )   // size = width * height * 4.
    {
        header = (uint8_t)fgetc( pFile );
        count = 1 + ( header & 0x7F );

        if ( header & 0x80 )
        {
            fread( color, sizeof(uint8_t), 4, pFile );

            for( uint32_t i=0; i<count; ++i, ptr+=4 )
            {
                ptr[ 0 ] = color[ 2 ];
                ptr[ 1 ] = color[ 1 ];
                ptr[ 2 ] = color[ 0 ];
                ptr[ 3 ] = color[ 3 ];
            }
        }
        else
        {
            for( uint32_t i=0; i<count; ++i, ptr+=4 )
            {
                ptr[ 2 ] = (uint8_t)fgetc( pFile );
                ptr[ 1 ] = (uint8_t)fgetc( pFile );
                ptr[ 0 ] = (uint8_t)fgetc( pFile );
                ptr[ 3 ] = (uint8_t)fgetc( pFile );
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      8BitRLE圧縮グレースケール形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse8BitsGrayScaleRLE( FILE* pFile, uint32_t size, uint8_t* pPixles )
{
    uint32_t count  = 0;
    uint8_t  color  = 0;
    uint8_t  header = 0;
    uint8_t* ptr    = pPixles;

    while( ptr < pPixles + size ) // size = width * height
    {
        header = (uint8_t)fgetc( pFile );
        count = 1 + ( header & 0x7F );

        if ( header & 0x80 )
        {
            color = (uint8_t)fgetc( pFile );

            for( uint32_t i=0; i<count; ++i, ptr++ )
            { (*ptr) = color; }
        }
        else
        {
            for( uint32_t i=0; i<count; ++i, ptr++ )
            { (*ptr) = (uint8_t)fgetc( pFile ); }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      16BitRLE圧縮グレースケール形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse16BitsGrayScaleRLE( FILE* pFile, uint32_t size, uint8_t* pPixles )
{
    uint32_t count  = 0;
    uint8_t  color  = 0;
    uint8_t  alpha  = 0;
    uint8_t  header = 0;
    uint8_t* ptr    = pPixles;

    while( ptr < pPixles + size ) // size = width * height * 2
    {
        header = (uint8_t)fgetc( pFile );
        count = 1 + ( header & 0x7F );

        if ( header & 0x80 )
        {
            color = (uint8_t)fgetc( pFile );
            alpha = (uint8_t)fgetc( pFile );

            for( uint32_t i=0; i<count; ++i, ptr+=2 )
            {
                ptr[ 0 ] = color;
                ptr[ 1 ] = alpha;
            }
        }
        else
        {
            for( uint32_t i=0; i<count; ++i, ptr+=2 )
            {
                ptr[ 0 ] = (uint8_t)fgetc( pFile );
                ptr[ 1 ] = (uint8_t)fgetc( pFile );
            }
        }
    }
}

} // namespace /* anonymous */


///////////////////////////////////////////////////////////////////////////////////////////////////
// Targa class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Targa::Targa()
: m_Width       ( 0 )
, m_Height      ( 0 )
, m_BitPerPixel ( 0 )
, m_Format      ( TGA_FORMAT_NONE )
, m_pPixels     ( nullptr )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Targa::~Targa()
{ Release(); }

//-------------------------------------------------------------------------------------------------
//      ファイルから読み込みします.
//-------------------------------------------------------------------------------------------------
bool Targa::Load( const wchar_t* filename )
{
    // 引数チェック.
    if ( filename == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    FILE* pFile;

    // ファイルを開く.
    auto err = _wfopen_s( &pFile, filename, L"rb" );
    if ( err != 0 )
    {
        ELOG( "Error : File Open Failed." );
        return false;
    }

    // フッターを読み込み.
    TGA_FOOTER footer;
    long offset = sizeof(footer);
    fseek( pFile, -offset, SEEK_END );
    fread( &footer, sizeof(footer), 1, pFile );

    // ファイルマジックをチェック.
    if ( strcmp( footer.Tag, "TRUEVISION-XFILE." ) != 0 &&
         strcmp( footer.Tag, "TRUEVISION-TARGA." ) != 0 )
    {
        ELOG( "Error : Invalid File Format." );
        fclose( pFile );
        return false;
    }

    // 拡張データがある場合は読み込み.
    if ( footer.OffsetExt != 0 )
    {
        TGA_EXTENSION extension;

        fseek( pFile, footer.OffsetExt, SEEK_SET );
        fread( &extension, sizeof(extension), 1, pFile );
    }

    // ディベロッパーエリアがある場合.
    if ( footer.OffsetDev != 0 )
    {
        /* NOT IMPLEMENT */
    }

    // ファイル先頭に戻す.
    fseek( pFile, 0, SEEK_SET );

    // ヘッダデータを読み込む.
    TGA_HEADER header;
    fread( &header, sizeof(header), 1, pFile );

    // フォーマット判定.
    uint32_t bytePerPixel;
    switch( header.Format )
    {
    // 該当なし.
    case TGA_FORMAT_NONE:
        {
            ELOG( "Error : Invalid Format." );
            fclose( pFile );
            return false;
        }
        break;

    // グレースケール
    case TGA_FORMAT_GRAYSCALE:
    case TGA_FORMAT_RLE_GRAYSCALE:
        { 
            if ( header.BitPerPixel == 8 )
            { bytePerPixel = 1; }
            else
            { bytePerPixel = 2; }
        }
        break;

    // カラー.
    case TGA_FORMAT_INDEXCOLOR:
    case TGA_FORMAT_FULLCOLOR:
    case TGA_FORMAT_RLE_INDEXCOLOR:
    case TGA_FORMAT_RLE_FULLCOLOR:
        {
            if ( header.BitPerPixel <= 24 )
            { bytePerPixel = 3; }
            else
            { bytePerPixel = 4; }
        }
        break;

    // 上記以外.
    default:
        {
            ELOG( "Error : Unsupported Format." );
            fclose( pFile );
            return false;
        }
        break;
    }

    // IDフィールドサイズ分だけオフセットを移動させる.
    fseek( pFile, header.IdFieldLength, SEEK_CUR );

    // ピクセルサイズを決定してメモリを確保.
    auto size = header.Width * header.Height * bytePerPixel;
    m_pPixels = new (std::nothrow) uint8_t [ size ];
    if ( m_pPixels == nullptr )
    {
        ELOG( "Error : Out Of Memory." );
        fclose( pFile );
        return false;
    }

    // カラーマップを持つかチェック.
    uint8_t* pColorMap = nullptr;
    if ( header.HasColorMap )
    {
        // カラーマップサイズを算出.
        uint32_t colorMapSize = header.ColorMapEntry * ( header.ColorMapEntrySize >> 3 );

        // メモリを確保.
        pColorMap = new (std::nothrow) uint8_t [ colorMapSize ];
        if ( pColorMap == nullptr )
        {
            ELOG( "Error : Out Of Memory." );
            delete [] m_pPixels;
            m_pPixels = nullptr;
            fclose( pFile );
            return false;
        }

        // がばっと読み込む.
        fread( pColorMap, sizeof(uint8_t), colorMapSize, pFile );
    }

    // 幅・高さ・ビットの深さ・ハッシュキーを設定.
    m_Width       = header.Width;
    m_Height      = header.Height;
    m_BitPerPixel = bytePerPixel * 8;
    m_Format      = static_cast<TGA_FORMAT_TYPE>( header.Format );

    // フォーマットに合わせてピクセルデータを解析する.
    switch( header.Format )
    {
    // パレット.
    case TGA_FORMAT_INDEXCOLOR:
        { Parse8Bits( pFile, m_Width * m_Height, pColorMap, m_pPixels ); }
        break;

    // フルカラー.
    case TGA_FORMAT_FULLCOLOR:
        {
            switch( header.BitPerPixel )
            {
            case 16:
                { Parse16Bits( pFile, m_Width * m_Height, m_pPixels ); }
                break;

            case 24:
                { Parse24Bits( pFile, m_Width * m_Height, m_pPixels ); }
                break;

            case 32:
                { Parse32Bits( pFile, m_Width * m_Height, m_pPixels ); }
                break;
            }
        }
        break;

    // グレースケール.
    case TGA_FORMAT_GRAYSCALE:
        {
            if ( header.BitPerPixel == 8 )
            { Parse8BitsGrayScale( pFile, m_Width * m_Height, m_pPixels ); }
            else
            { Parse16BitsGrayScale( pFile, m_Width * m_Height, m_pPixels ); }
        }
        break;

    // パレットRLE圧縮.
    case TGA_FORMAT_RLE_INDEXCOLOR:
        { Parse8BitsRLE( pFile, pColorMap, m_Width * m_Height * 3, m_pPixels ); }
        break;

    // フルカラーRLE圧縮.
    case TGA_FORMAT_RLE_FULLCOLOR:
        {
            switch( header.BitPerPixel )
            {
            case 16:
                { Parse16BitsRLE( pFile, m_Width * m_Height * 3, m_pPixels ); }
                break;

            case 24:
                { Parse24BitsRLE( pFile, m_Width * m_Height * 3, m_pPixels ); }
                break;

            case 32:
                { Parse32BitsRLE( pFile, m_Width * m_Height * 4, m_pPixels ); }
                break;
            }
        }
        break;

    // グレースケールRLE圧縮.
    case TGA_FORMAT_RLE_GRAYSCALE:
        {
            if ( header.BitPerPixel == 8 )
            { Parse8BitsGrayScaleRLE( pFile, m_Width * m_Height, m_pPixels ); }
            else
            { Parse16BitsGrayScaleRLE( pFile, m_Width * m_Height * 2, m_pPixels ); }
        }
        break;
    }

    // 不要なメモリを解放.
    delete [] pColorMap;
    pColorMap = nullptr;

    // ファイルを閉じる.
    fclose( pFile );

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      メモリを解放します.
//-------------------------------------------------------------------------------------------------
void Targa::Release()
{
    if (m_pPixels != nullptr)
    {
        delete [] m_pPixels;
        m_pPixels = nullptr;
    }

    m_Width       = 0;
    m_Height      = 0;
    m_BitPerPixel = 0;
    m_Format      = TGA_FORMAT_NONE;
}

//-------------------------------------------------------------------------------------------------
//      画像の横幅を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Targa::GetWidth() const
{ return m_Width; }

//-------------------------------------------------------------------------------------------------
//      画像の縦幅を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Targa::GetHeight() const
{ return m_Height; }

//-------------------------------------------------------------------------------------------------
//      1ピクセルあたりのビット数を取得します.
//-------------------------------------------------------------------------------------------------
uint8_t Targa::GetBitPerPixel() const
{ return m_BitPerPixel; }

//-------------------------------------------------------------------------------------------------
//      1ピクセルあたりのバイト数を取得します.
//-------------------------------------------------------------------------------------------------
uint8_t Targa::GetBytePerPixel() const
{ return m_BitPerPixel / 8; }

//-------------------------------------------------------------------------------------------------
//      画像形式を取得します.
//-------------------------------------------------------------------------------------------------
TGA_FORMAT_TYPE Targa::GetFormat() const
{ return m_Format; }

//-------------------------------------------------------------------------------------------------
//      ピクセルデータを取得します.
//-------------------------------------------------------------------------------------------------
const uint8_t* Targa::GetPixels() const
{ return m_pPixels; }

