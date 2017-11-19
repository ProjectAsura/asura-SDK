//-------------------------------------------------------------------------------------------
// File : main.cpp
// Desc : Application Main Entry Point.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------
#include <cstring>
#include <string>
#include "ResTexture.h"


//-------------------------------------------------------------------------------------------
//! @brief      ヘルプを表示します.
//-------------------------------------------------------------------------------------------
void ShowHelp()
{
    printf_s( "//-------------------------------------------------------------------\n" );
    printf_s( "// MapConverter.exe\n" );
    printf_s( "// Copyright(c) Project Asura. All right reserved.\n" );
    printf_s( "//-------------------------------------------------------------------\n" );
    printf_s( "[使い方] MapConverter.exe -i 入力ファイル名 -o 出力ファイル名\n" );
    printf_s( "\n" );
}


//-------------------------------------------------------------------------------------------
//! @brief      メインエントリーポイントです.
//!
//! @param [in]     argc
//-------------------------------------------------------------------------------------------
int main( int argc, char** argv )
{
    std::string input_name;
    std::string output_name;

    for( int i=0; i<argc; ++i )
    {
        // 入力ファイル名.
        if( strcmp( argv[ i ], "-i" ) == 0 )
        {
            if ( i+1 <= argc )
            {
                i++;
                input_name = std::string( argv[ i ] );
            }
        }
        // 出力ファイル名.
        else if ( strcmp( argv[ i ], "-o" ) == 0 )
        {
            if ( i+1 <= argc )
            {
                i++;
                output_name = std::string( argv[ i ] );
            }
        }
        // 入出力名を同一に.
        else if ( strcmp( argv[ i ], "-io" ) == 0 )
        {
            if ( i+1 <= argc )
            {
                i++;
                input_name = std::string( argv[ i ] );

                std::string::size_type index = input_name.find_last_of( "." );
                if ( index == std::string::npos )
                {
                    output_name = input_name + ".map";
                }
                else
                {
                    output_name = input_name;
                    output_name = output_name.substr( 0, index );
                    output_name += ".map";
                }

            }
        }
        // ヘルプ表示.
        else if ( strcmp( argv[ i ], "-h" ) == 0 )
        {
            ShowHelp();
            return 0;
        }
    }

    // ファイル名が入力されたかチェック.
    if( !input_name.empty() && !output_name.empty() )
    {
        a3d::ResTexture texture;

        // テクスチャファイルをロード.
        if ( !a3dx::CreateResTextureFromFile( input_name.c_str(), texture ) )
        {
            // エラーログ出力.
            printf_s( "Error : Texture File Load Failed. filename = %s\n", input_name.c_str() );

            // 文字列をクリア.
            input_name.clear();
            output_name.clear();

            a3d::DisposeTexture(texture);

            return -1;
        }

        // MAPファイルに保存.
        if (!a3d::SaveTexture(output_name.c_str(), texture))
        {
            // エラーログ出力.
            printf_s( "Error : Binary Save Failed. filename = %s\n", output_name.c_str() );

            // 文字列をクリア.
            input_name.clear();
            output_name.clear();

            a3d::DisposeTexture(texture);

            return -1;
        }

        // 成功と表示.
        printf_s( "Info : Binary Save Success! filename = %s\n", output_name.c_str() );

        a3d::DisposeTexture(texture);
    }
    else
    {
        // ヘルプ表示.
        ShowHelp();
    }

    // 文字列をクリア.
    input_name.clear();
    output_name.clear();

    return 0;
}