//-------------------------------------------------------------------------------------------------
// File : a3d.h
// Desc : Rendering Hardware Interface.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstddef>
#include <cstdint>


#ifndef A3D_TARGET_CONSOLE

    #if defined(WIN32) || defined(_WIN32)
        #ifndef A3D_PLATFORM_WIN
        #define A3D_PLATFORM_WIN    // Windowsプラットフォームです.
        #endif//A3D_PLATFORM_WIN
    #endif//defined(WIN32) || defined(_WIN32)

    #ifdef WIN64
        #ifndef A3D_PLATFORM_WIN
        #define A3D_PLATFORM_WIN    // Windowsプラットフォームです.
        #endif//A3D_PLATFORM_WIN
    #endif//defined(WIN64)

    #if defined(A3D_PLATFORM_WIN)
        #define A3D_IS_WIN      (1)
    #endif

#else
    #define A3D_IS_WIN      (0)

#endif// A3D_TARGET_CONSOLE


#ifndef A3D_API
#define A3D_API
#endif//A3D_API


#ifndef A3D_APIENTRY
    #if A3D_IS_WIN
        #define A3D_APIENTRY            __stdcall
    #else
        #define A3D_APIENTRY
    #endif//A3D_IS_WIN
#endif//A3D_APIENTRY


#ifndef A3D_UNUSED
#define A3D_UNUSED(x)                   ((void)x)
#endif//A3D_UNUSED


//=================================================================================================
//! @name a3d   a3dライブラリの名前空間です
//=================================================================================================
namespace a3d {

//-------------------------------------------------------------------------------------------------
// Forward Declarations.
//-------------------------------------------------------------------------------------------------
struct IAllocator;
struct IBlob;
struct IDevice;
struct IDescriptorSetLayout;
struct IBuffer;
struct ITexture;
struct IConstantBufferView;
struct IShaderResourceView;
struct IUnorderedAccessView;
struct ISwapChain;
struct IColorTargetView;
struct IDepthStencilView;


//-------------------------------------------------------------------------------------------------
// Using Alias
//-------------------------------------------------------------------------------------------------
using TimeStamp   = uint64_t;       //!< タイムスタンプです.
using QuerySample = uint64_t;       //!< オクルージョンクエリのサンプル数です.


///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   INDIRECT_ARGUMENT_TYPE
//! @brief  インダイレクト引数タイプです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum INDIRECT_ARGUMENT_TYPE
{
    INDIRECT_ARGUMENT_TYPE_DRAW             = 0,    //!< 描画用引数です.
    INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED     = 1,    //!< インデックス付き描画用引数です.
    INDIRECT_ARGUMENT_TYPE_DISPATCH_COMPUTE = 2,    //!< コンピュートディスパッチ用引数です.
    INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH    = 3,    //!< メッシュディスパッチ用引数です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   RESOURCE_KIND
//! @brief  リソースタイプです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum RESOURCE_KIND
{
    RESOURCE_KIND_BUFFER = 0,       //!< バッファです.
    RESOURCE_KIND_TEXTURE,          //!< テクスチャです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   RESOURCE_DIMENSION
//! @brief  リソースの次元です.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum RESOURCE_DIMENSION
{
    RESOURCE_DIMENSION_UNKNOWN      = 0,    //!< 未知の形式です.
    RESOURCE_DIMENSION_BUFFER       = 1,    //!< バッファです.
    RESOURCE_DIMENSION_TEXTURE1D    = 2,    //!< 1次元テクスチャです.
    RESOURCE_DIMENSION_TEXTURE2D    = 3,    //!< 2次元テクスチャです.
    RESOURCE_DIMENSION_TEXTURE3D    = 4,    //!< 3次元テクスチャです.
    RESOURCE_DIMENSION_CUBEMAP      = 5,    //!< キューブマップです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   VIEW_DIMENSION
//! @brief  ビューの次元です.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum VIEW_DIMENSION
{
    VIEW_DIMENSION_BUFFER               = 0,    //!< バッファです.
    VIEW_DIMENSION_TEXTURE1D            = 1,    //!< 1次元テクスチャです.
    VIEW_DIMENSION_TEXTURE1D_ARRAY      = 2,    //!< 1次元テクスチャ配列です.
    VIEW_DIMENSION_TEXTURE2D            = 3,    //!< 2次元テクスチャです.
    VIEW_DIMENSION_TEXTURE2D_ARRAY      = 4,    //!< 2次元テクスチャ配列です.
    VIEW_DIMENSION_CUBEMAP              = 5,    //!< キューブマップです.
    VIEW_DIMENSION_CUBEMAP_ARRAY        = 6,    //!< キューブマップ配列です.
    VIEW_DIMENSION_TEXTURE3D            = 7,    //!< 3次元テクスチャです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   RESOURCE_LAYOUT
//! @brief  リソースレイアウトです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum RESOURCE_LAYOUT
{
    RESOURCE_LAYOUT_LINEAR,        //!< 線形モードです.
    RESOURCE_LAYOUT_OPTIMAL,       //!< タイリングモードです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   RESOURCE_USAGE
//! @brief  リソースの使用用途です.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum RESOURCE_USAGE
{
    RESOURCE_USAGE_RENDER_TARGET        = 0x001,   //!< レンダーターゲットとして使用します.
    RESOURCE_USAGE_DEPTH_STENCIL        = 0x002,   //!< 深度ステンシルターゲットとして使用します.
    RESOURCE_USAGE_UNORDERED_ACCESS     = 0x004,   //!< アンオーダードアクセスビューとして使用します.
    RESOURCE_USAGE_INDEX_BUFFER         = 0x008,   //!< インデックスバッファとして使用します.
    RESOURCE_USAGE_VERTEX_BUFFER        = 0x010,   //!< 頂点バッファとして使用します.
    RESOURCE_USAGE_CONSTANT_BUFFER      = 0x020,   //!< 定数バッファとして使用します.
    RESOURCE_USAGE_INDIRECT_BUFFER      = 0x040,   //!< インダイレクトバッファとして使用します.
    RESOURCE_USAGE_SHADER_RESOURCE      = 0x080,   //!< シェーダリソースビューとして使用します.
    RESOURCE_USAGE_COPY_SRC             = 0x100,   //!< コピー元として使用します.
    RESOURCE_USAGE_COPY_DST             = 0x200,   //!< コピー先として使用します.
    RESOURCE_USAGE_QUERY_BUFFER         = 0x300,   //!< クエリバッファとして使用します.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   RESOURCE_FORMAT
//! @brief  リソースフォーマットです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum RESOURCE_FORMAT
{
    RESOURCE_FORMAT_UNKNOWN                 = 0,    //!< 未知の形式です.
    RESOURCE_FORMAT_R32G32B32A32_FLOAT      = 1,    //!< RGBA 32bit 浮動小数形式です.
    RESOURCE_FORMAT_R32G32B32A32_UINT       = 2,    //!< RGBA 32bit 符号なし整数形式です.
    RESOURCE_FORMAT_R32G32B32A32_SINT       = 3,    //!< RGBA 32bit 符号つき整数形式です.
    RESOURCE_FORMAT_R16G16B16A16_FLOAT      = 4,    //!< RGBA 16bit 浮動小数形式です.
    RESOURCE_FORMAT_R16G16B16A16_UINT       = 5,    //!< RGBA 16bit 符号なし整数形式です.
    RESOURCE_FORMAT_R16G16B16A16_SINT       = 6,    //!< RGBA 16bit 符号つき整数形式です.
    RESOURCE_FORMAT_R32G32B32_FLOAT         = 7,    //!< RGB 32bit 浮動小数形式です.
    RESOURCE_FORMAT_R32G32B32_UINT          = 8,    //!< RGB 32bit 符号なし整数形式です.
    RESOURCE_FORMAT_R32G32B32_SINT          = 9,    //!< RGB 32bit 符号つき整数形式です.
    RESOURCE_FORMAT_R32G32_FLOAT            = 10,   //!< RG 32bit 浮動小数形式です.
    RESOURCE_FORMAT_R32G32_UINT             = 11,   //!< RG 32bit 符号なし整数形式です.
    RESOURCE_FORMAT_R32G32_SINT             = 12,   //!< RG 32bit 符号つき整数形式です.
    RESOURCE_FORMAT_R16G16_FLOAT            = 13,   //!< RG 16bit 浮動小数形式です.
    RESOURCE_FORMAT_R16G16_UINT             = 14,   //!< RG 16bit 符号なし整数形式です.
    RESOURCE_FORMAT_R16G16_SINT             = 15,   //!< RG 16bit 符号つき整数形式です.
    RESOURCE_FORMAT_R32_FLOAT               = 16,   //!< R 32bit 浮動小数形式です.
    RESOURCE_FORMAT_R32_UINT                = 17,   //!< R 32bit 符号なし整数形式です.
    RESOURCE_FORMAT_R32_SINT                = 18,   //!< R 32bit 符号つき整数形式です.
    RESOURCE_FORMAT_R16_FLOAT               = 19,   //!< R 16bit 浮動小数形式です.
    RESOURCE_FORMAT_R16_UINT                = 20,   //!< R 16bit 符号なし整数形式です.
    RESOURCE_FORMAT_R16_SINT                = 21,   //!< R 16bit 符号つき整数形式です.
    RESOURCE_FORMAT_R8G8B8A8_UNORM_SRGB     = 22,   //!< RGBA 8bit SRGB UNORM 形式です.
    RESOURCE_FORMAT_R8G8B8A8_UNORM          = 23,   //!< RGBA 8bit UNORM 形式です.
    RESOURCE_FORMAT_B8G8R8A8_UNORM_SRGB     = 24,   //!< BGRA 8bit SRGB UNORM 形式です.
    RESOURCE_FORMAT_B8G8R8A8_UNORM          = 25,   //!< BGRA 8bit UNORM 形式です.
    RESOURCE_FORMAT_R8G8_UNORM              = 26,   //!< RG 8bit UNORM 形式です.
    RESOURCE_FORMAT_R8_UNORM                = 27,   //!< R 8b8t UNORM 形式です.
    RESOURCE_FORMAT_D32_FLOAT               = 28,   //!< Depth 32bit 浮動小数形式です.
    RESOURCE_FORMAT_D24_UNORM_S8_UINT       = 29,   //!< Depth 24bit UNORM Stencil 8bit 符号なし整数形式です.
    RESOURCE_FORMAT_D16_UNORM               = 30,   //!< Depth 16bit UNORM 形式です.
    RESOURCE_FORMAT_BC1_UNORM_SRGB          = 31,   //!< BC1 SRGB UNORM ブロック圧縮形式です.
    RESOURCE_FORMAT_BC1_UNORM               = 32,   //!< BC1 UNORM ブロック圧縮形式です.
    RESOURCE_FORMAT_BC2_UNORM_SRGB          = 33,   //!< BC2 SRGB UNORM ブロック圧縮形式です.
    RESOURCE_FORMAT_BC2_UNORM               = 34,   //!< BC2 UNORM ブロック圧縮形式です.
    RESOURCE_FORMAT_BC3_UNORM_SRGB          = 35,   //!< BC3 SRGB UNORM ブロック圧縮形式です.
    RESOURCE_FORMAT_BC3_UNORM               = 36,   //!< BC3 UNORM ブロック圧縮形式です.
    RESOURCE_FORMAT_BC4_UNORM               = 37,   //!< BC4 UNORM ブロック圧縮形式です.
    RESOURCE_FORMAT_BC4_SNORM               = 38,   //!< BC4 SNORM ブロック圧縮形式です.
    RESOURCE_FORMAT_BC5_UNORM               = 39,   //!< BC5 UNORM ブロック圧縮形式です.
    RESOURCE_FORMAT_BC5_SNORM               = 40,   //!< BC5 SNORM ブロック圧縮形式です.
    RESOURCE_FORMAT_BC6H_UF16               = 41,   //!< BC6H 符号なし16bit浮動小数ブロック圧縮形式です.
    RESOURCE_FORMAT_BC6H_SF16               = 42,   //!< BC6H 符号つき16bit浮動小数ブロック圧縮形式です.
    RESOURCE_FORMAT_BC7_UNORM_SRGB          = 43,   //!< BC7 SRGB UNORM ブロック圧縮形式です.
    RESOURCE_FORMAT_BC7_UNORM               = 44,   //!< BC7 UNORM ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_4X4_UNORM_SRGB     = 45,   //!< ASTC SRGB 4X4ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_4X4_UNORM          = 46,   //!< ASTC 4X4ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_5X4_UNORM_SRGB     = 47,   //!< ASTC SRGB 5X4ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_5X4_UNORM          = 48,   //!< ASTC 5X4 ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_5X5_UNORM_SRGB     = 49,   //!< ASTC SRGB 5X5ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_5X5_UNORM          = 50,   //!< ASTC 5X5ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_6X5_UNORM_SRGB     = 51,   //!< ASTC SRGB 6X5ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_6X5_UNORM          = 52,   //!< ASTC 6X5ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_6X6_UNORM_SRGB     = 53,   //!< ASTC SRGB 6X6ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_6X6_UNORM          = 54,   //!< ASTC 6X6ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_8X5_UNORM_SRGB     = 55,   //!< ASTC SRGB 8X5ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_8X5_UNORM          = 56,   //!< ASTC 8X5ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_8X6_UNORM_SRGB     = 57,   //!< ASTC SRGB 8X6ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_8X6_UNORM          = 58,   //!< ASTC 8X6ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_8X8_UNORM_SRGB     = 59,   //!< ASTC SRGB 8X8ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_8X8_UNORM          = 60,   //!< ASTC 8X8ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_10X5_UNORM_SRGB    = 61,   //!< ASTC SRGB 10X5ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_10X5_UNORM         = 62,   //!< ASTC 10X5ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_10X6_UNORM_SRGB    = 63,   //!< ASTC SRGB 10X6ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_10X6_UNORM         = 64,   //!< ASTC 10X6ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_10X8_UNORM_SRGB    = 65,   //!< ASTC SRGB 10X8ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_10X8_UNORM         = 66,   //!< ASTC 10X8ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_10X10_UNORM_SRGB   = 67,   //!< ASTC SRGB 10X10ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_10X10_UNORM        = 68,   //!< ASTC 10X10ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_12X10_UNORM_SRGB   = 69,   //!< ASTC SRGB 12X10ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_12X10_UNORM        = 70,   //!< ASTC 12X10ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_12X12_UNORM_SRGB   = 71,   //!< ASTC SRGB 12X12ブロック圧縮形式です.
    RESOURCE_FORMAT_ASTC_12X12_UNORM        = 72,   //!< ASTC 12X12ブロック圧縮形式です.
    RESOURCE_FORMAT_R10G10B10A2_UNORM       = 73,   //!< RGBA 10bit, 10bit, 10bit, 2bit UNORM形式です.
    RESOURCE_FORMAT_R10G10B10A2_UINT        = 74,   //!< RGBA 10bit, 10bit, 10bit, 2bit 符号なし整数形式です.
    RESOURCE_FORMAT_R11G11B10_FLOAT         = 75,   //!< RGB 11bit, 11bit, 11bit 浮動小数形式です.
    RESOURCE_FORMAT_B16G16R16A16_FLOAT      = 76,   //!< BGRA 16bit, 16bit, 16bit, 16bit 浮動小数形式です.
    RESOURCE_FORMAT_B10G10R10A2_UNORM       = 77,   //!< BGRA 10bit, 10bit, 10bit, 2bit UNORM形式です.
    RESOURCE_FORMAT_B10G10R10A2_UINT        = 78,   //!< BGRA 10bit, 10bit, 10bit, 2bit 符号なし整数形式です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   RESOURCE_STATE
//! @brief  リソースの状態です.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum RESOURCE_STATE
{
    RESOURCE_STATE_UNKNOWN              = 0,    //!< 未定義状態です.
    RESOURCE_STATE_GENERAL              = 1,    //!< 共通状態です.
    RESOURCE_STATE_VERTEX_BUFFER        = 2,    //!< 頂点バッファです.
    RESOURCE_STATE_INDEX_BUFFER         = 3,    //!< インデックスバッファです.
    RESOURCE_STATE_CONSTANT_BUFFER      = 4,    //!< 定数バッファです.
    RESOURCE_STATE_COLOR_WRITE          = 5,    //!< カラー書き込み状態です.  
    RESOURCE_STATE_COLOR_READ           = 6,    //!< カラー読み込み状態です.
    RESOURCE_STATE_UNORDERED_ACCESS     = 7,    //!< アンオーダードアクセス状態です.
    RESOURCE_STATE_DEPTH_WRITE          = 8,    //!< 深度書き込み状態です.
    RESOURCE_STATE_DEPTH_READ           = 9,    //!< 深度読み込み状態です.
    RESOURCE_STATE_SHADER_READ          = 10,   //!< シェーダで読み込み可能な状態です.
    RESOURCE_STATE_INDIRECT_ARGUMENT    = 12,   //!< インダイレクトコマンド引数です.
    RESOURCE_STATE_COPY_DST             = 13,   //!< 転送先の状態です.
    RESOURCE_STATE_COPY_SRC             = 14,   //!< 転送元の状態です.
    RESOURCE_STATE_RESOLVE_DST          = 15,   //!< 解決先の状態です.
    RESOURCE_STATE_RESOLVE_SRC          = 16,   //!< 解決元の状態です.
    RESOURCE_STATE_PRESENT              = 17,   //!< 表示状態です.
    RESOURCE_STATE_PREDICATION          = 18,   //!< 予測状態です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   HEAP_TYPE
//! @brief  ヒープタイプです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum HEAP_TYPE
{
    HEAP_TYPE_DEFAULT   = 0,    //!< デフォルトです.
    HEAP_TYPE_UPLOAD    = 1,    //!< アップロードです.
    HEAP_TYPE_READBACK  = 2,    //!< 読み戻しです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   COMMANDLIST_TYPE
//! @brief  コマンドリストタイプです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum COMMANDLIST_TYPE
{
    COMMANDLIST_TYPE_DIRECT     = 0,    //!< 直接コマンドキューに登録可能なコマンドリストです.
    COMMANDLIST_TYPE_BUNDLE     = 1,    //!< バンドルコマンドリストです.
    COMMANDLIST_TYPE_COPY       = 2,    //!< コピーコマンドリストです.
    COMMANDLIST_TYPE_COMPUTE    = 3,    //!< コンピュートコマンドリストです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   PRIMITIVE_TOPOLOGY
//! @brief  プリミティブトポロジーです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum PRIMITIVE_TOPOLOGY
{
    PRIMITIVE_TOPOLOGY_POINTLIST            = 0,    //!< ポイントリストです.
    PRIMITIVE_TOPOLOGY_LINELIST             = 1,    //!< ラインリストです.
    PRIMITIVE_TOPOLOGY_LINESTRIP            = 2,    //!< ラインストリップです.
    PRIMITIVE_TOPOLOGY_TRIANGLELIST         = 3,    //!< トライアングルリストです.
    PRIMITVIE_TOPOLOGY_TRIANGLESTRIP        = 4,    //!< トライアングルストリップです.
    PRIMITIVE_TOPOLOGY_LINELIST_ADJ         = 5,    //!< 隣接情報付きラインリストです. 
    PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ        = 6,    //!< 隣接情報付きラインストリップです.
    PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ     = 7,    //!< 隣接情報付きトライアングルリストです.
    PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ    = 8,    //!< 隣接情報付きトライアングルストリップです.
    PRIMITIVE_TOPOLOGY_PATCHLIST            = 9,    //!< パッチリストです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   SHADER_MASK
//! @brief  シェーダマスクです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum SHADER_MASK
{
    SHADER_MASK_VS  = 0x1,        //!< 頂点シェーダステージです.
    SHADER_MASK_DS  = 0x2,        //!< ドメインシェーダステージです.
    SHADER_MASK_HS  = 0x4,        //!< ハルシェーダシェーダステージです.
    SHADER_MASK_PS  = 0x10,       //!< ピクセルシェーダステージです.
    SHADER_MASK_CS  = 0x20,       //!< コンピュートシェーダステージです.
    SHADER_MASK_AS  = 0x30,       //!< アンプリフィケーションシェーダステージです.
    SHADER_MASK_MS  = 0x40,       //!< メッシュシェーダステージです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   DESCRIPTOR_TYPE
//! @brief  ディスクリプタタイプです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum DESCRIPTOR_TYPE
{
    DESCRIPTOR_TYPE_CBV = 0,        //!< 定数バッファビューです.
    DESCRIPTOR_TYPE_SRV = 1,        //!< シェーダリソースビューです.
    DESCRIPTOR_TYPE_UAV = 2,        //!< ストレージビューです.
    DESCRIPTOR_TYPE_SMP = 3,        //!< サンプラーです.
    DESCRIPTOR_TYPE_RTV = 4,        //!< カラーターゲットビューです.
    DESCRIPTOR_TYPE_DSV = 5,        //!< 深度ステンシルビューです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   POLYGON_MODE
//! @brief  ポリゴンモードです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum POLYGON_MODE
{
    POLYGON_MODE_SOLID      = 0,    //!< ポリゴン描画です.
    POLYGON_MODE_WIREFRAME  = 1,    //!< ワイヤーフレーム描画です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   CULL_MODE
//! @brief  カリングモードです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum CULL_MODE
{
    CULL_MODE_NONE  = 0,    //!< カリングしません.
    CULL_MODE_FRONT = 1,    //!< 前面カリングします.
    CULL_MODE_BACK  = 2,    //!< 背面カリングします.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   COMPARE_OP
//! @brief  比較操作タイプです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum COMPARE_OP
{
    COMPARE_OP_NEVER    = 0,    //!< 常に失敗.
    COMPARE_OP_LESS     = 1,    //!< 入力 <  出力
    COMPARE_OP_EQUAL    = 2,    //!< 入力 == 出力
    COMPARE_OP_LEQUAL   = 3,    //!< 入力 <= 出力
    COMPARE_OP_GREATER  = 4,    //!< 入力 >  出力
    COMPARE_OP_GEQUAL   = 5,    //!< 入力 >= 出力
    COMPARE_OP_ALWAYS   = 6,    //!< 常に合格.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   STENCIL_OP
//! @brief  ステンシル操作タイプです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum STENCIL_OP
{
    STENCIL_OP_KEEP     = 0,    //!< 既存のステンシル データを保持します.
    STENCIL_OP_ZERO     = 1,    //!< ステンシル データを 0 に設定します.
    STENCIL_OP_REPLACE  = 2,    //!< ステンシル データを指定値に置き換えます.
    STENCIL_OP_INCR_SAT = 3,    //!< ステンシルの値を 1 増やし、その結果をクランプします.
    STENCIL_OP_DECR_SAT = 4,    //!< ステンシルの値を 1 減らし、その結果をクランプします.
    STENCIL_OP_INVERT   = 5,    //!< ステンシル データを反転します.
    STENCIL_OP_INCR     = 6,    //!< ステンシルの値を 1 増やし、必要に応じて結果をラップします.
    STENCIL_OP_DECR     = 7,    //!< ステンシルの値を 1 増やし、必要に応じて結果をラップします.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   BLEND_FACTOR
//! @brief  ブレンド因子タイプです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum BLEND_FACTOR
{
    BLEND_FACTOR_ZERO           = 0,    //!< データ ソースの色は黒 (0, 0, 0, 0) です。ブレンディング前の処理はありません.
    BLEND_FACTOR_ONE            = 1,    //!< データ ソースの色は白 (1, 1, 1, 1) です。ブレンディング前の処理はありません.
    BLEND_FACTOR_SRC_COLOR      = 2,    //!< データ ソースは、ピクセル シェーダーからのカラー データ (RGB) です。ブレンディング前の処理はありません.
    BLEND_FACTOR_INV_SRC_COLOR  = 3,    //!< データ ソースは、ピクセル シェーダーからのカラー データ (RGB) です。ブレンディング前の処理によってデータが反転され、1 - RGB が生成されます.
    BLEND_FACTOR_SRC_ALPHA      = 4,    //!< データ ソースは、ピクセル シェーダーからのアルファ データ (A) です。ブレンディング前の処理はありません.
    BLEND_FACTOR_INV_SRC_ALPHA  = 5,    //!< データ ソースは、ピクセル シェーダーからのアルファ データ (A) です。ブレンディング前の処理によってデータが反転され、1 - A が生成されます.
    BLEND_FACTOR_DST_ALPHA      = 6,    //!< データ ソースは、レンダー ターゲットからのアルファ データです。ブレンディング前の処理はありません.
    BLEND_FACTOR_INV_DST_ALPHA  = 7,    //!< データ ソースは、レンダー ターゲットからのアルファ データです。ブレンディング前の処理によってデータが反転され、1 - A が生成されます.
    BLEND_FACTOR_DST_COLOR      = 8,    //!< データ ソースは、レンダー ターゲットからのカラー データです。ブレンディング前の処理はありません.
    BLEND_FACTOR_INV_DST_COLOR  = 9,    //!< データ ソースは、レンダー ターゲットからのカラー データです。ブレンディング前の処理によってデータが反転され、1 - RGB が生成されます.
    BLEND_FACTOR_SRC_ALPHA_SAT  = 10,   //!< データ ソースは、ピクセル シェーダーからのアルファ データです。ブレンディング前の処理によってデータが 1 以下にクランプされます.
    BLEND_FACTOR_SRC1_COLOR     = 11,   //!< データ ソースは、ピクセル シェーダーによって出力された両方のカラー データです。ブレンディング前の処理はありません。このオプションは、デュアル ソースのカラー ブレンディングをサポートします.
    BLEND_FACTOR_INV_SRC1_COLOR = 12,   //!< データ ソースは、ピクセル シェーダーによって出力された両方のカラー データです。ブレンディング前の処理によってデータが反転され、1 - RGB が生成されます。このオプションは、デュアル ソースのカラー ブレンディングをサポートします.
    BLEND_FACTOR_SRC1_ALPHA     = 13,   //!< データ ソースは、ピクセル シェーダーによって出力されたアルファ データです。ブレンディング前の処理はありません。このオプションは、デュアル ソースのカラー ブレンディングをサポートします.
    BLEND_FACTOR_INV_SRC1_ALPHA = 14,   //!< データ ソースは、ピクセル シェーダーによって出力されたアルファ データです。ブレンディング前の処理によってデータが反転され、1 - A が生成されます。このオプションは、デュアル ソースのカラー ブレンディングをサポートします.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   BLEND_OP
//! @brief  ブレンド操作タイプです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum BLEND_OP
{
    BLEND_OP_ADD        = 0,    //!< ソース 1 とソース 2 を加算します.
    BLEND_OP_SUB        = 1,    //!< ソース 1 からソース 2 を減算します.
    BLEND_OP_REV_SUB    = 2,    //!< ソース 1 からソース 2 を減算します.
    BLEND_OP_MIN        = 3,    //!< ソース 1 とソース 2 の最小値を選択します.
    BLEND_OP_MAX        = 4,    //!< ソース 1 とソース 2 の最大値を選択します.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   LOGIC_OP
//! @brief  レンダーターゲットの論理操作タイプです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum LOGIC_OP
{
    LOGIC_OP_CLEAR      = 0,    //!< レンダーターゲットをクリアします.
    LOGIC_OP_SET        = 1,    //!< レンダーターゲットを設定します.
    LOGIC_OP_COPY       = 2,    //!< レンダーターゲットをコピーします.
    LOGIC_OP_COPY_INV   = 3,    //!< レンダーターゲットを反転してコピーを行います.
    LOGIC_OP_NOOP       = 4,    //!< レンダーターゲット上で操作は行われません.
    LOGIC_OP_INV        = 5,    //!< レンダーターゲットを反転します.
    LOGIC_OP_AND        = 6,    //!< レンダーターゲット上でAND演算を実行します.
    LOGIC_OP_NAND       = 7,    //!< レンダーターゲット上でNAND演算を実行します.
    LOGIC_OP_OR         = 8,    //!< レンダーターゲット上でOR演算を実行します.
    LOGIC_OP_NOR        = 9,    //!< レンダーターゲット上でNOR演算を実行します.
    LOGIC_OP_XOR        = 10,   //!< レンダーターゲット上でXOR演算を実行します.
    LOGIC_OP_EQUIV      = 11,   //!< レンダーターゲット上で等価演算を実行します.
    LOGIC_OP_AND_REV    = 12,   //!< レンダーターゲット上で，AND演算と逆操作を行います.
    LOGIC_OP_AND_INV    = 13,   //!< レンダーターゲット上で，AND演算と反転操作を行います.
    LOGIC_OP_OR_REV     = 14,   //!< レンダーターゲット上で，OR演算と逆操作を行います.
    LOGIC_OP_OR_INV     = 15,   //!< レンダーターゲット上で，OR演算と反転操作を行います.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   FILTER_MODE
//! @brief  テクスチャフィルタモードです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum FILTER_MODE
{
    FILTER_MODE_NEAREST = 0,    //!< 近傍フィルタです.
    FILTER_MODE_LINEAR  = 1,    //!< 線形フィルタです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   MIPMAP_MODE
//! @brief  テクスチャミップマップモードです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum MIPMAP_MODE
{
    MIPMAP_MODE_NEAREST = 0,    //!< 近傍フィルタです.
    MIPMAP_MODE_LINEAR  = 1,    //!< 線形フィルタです.
    MIPMAP_MODE_NONE    = 2,    //!< フィルタなしです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   TEXTURE_ADDRESS_MODE
//! @brief  テクスチャアドレスモードです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum TEXTURE_ADDRESS_MODE
{
    TEXTURE_ADDRESS_MODE_REPEAT         = 0,    //!< 繰り返します.
    TEXTURE_ADDRESS_MODE_MIRROR         = 1,    //!< ミラーリングします.
    TEXTURE_ADDRESS_MODE_CLAMP          = 2,    //!< クランプします.
    TEXTURE_ADDRESS_MODE_BORDER         = 3,    //!< クランプして，範囲外を境界色に設定します.
    TEXTURE_ADDRESS_MODE_MIRROR_ONCE    = 4,    //!< ミラーリングしてクランプします.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   BORDER_COLOR
//! @brief  テクスチャのボーダーカラーです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum BORDER_COLOR
{
    BORDER_COLOR_TRANSPARENT_BLACK  = 0,    //!< (0, 0, 0, 0)を設定します.
    BORDER_COLOR_OPAQUE_BLACK       = 1,    //!< (0, 0, 0, 1)を設定します.
    BORDER_COLOR_OPAQUE_WHITE       = 2,    //!< (1, 1, 1, 1)を設定します.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   QUERY_TYPE
//! @brief  クエリタイプです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum QUERY_TYPE
{
    QUERY_TYPE_OCCLUSION            = 0,    //!< 隠蔽問い合わせです.
    QUERY_TYPE_TIMESTAMP            = 1,    //!< タイムスタンプ問い合わせです.
    QUERY_TYPE_PIPELINE_STATISTICS  = 2,    //!< パイプライン統計問い合わせです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   INPUT_CLASSIFICATION
//! @brief  入力データの分類です.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum INPUT_CLASSIFICATION
{
    INPUT_CLASSIFICATION_PER_VERTEX   = 0,  //!< 頂点データ単位で入力します.
    INPUT_CLASSIFICATION_PER_INSTANCE = 1   //!< インスタンスデータ単位で入力します.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   META_DATA_TYPE
//! @brief  メタデータタイプです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum META_DATA_TYPE
{
    META_DATA_NONE          = 0,    //!< メタデータ無しです.
    META_DATA_HDR10         = 1,    //!< HDR10メタデータタイプです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   COLOR_SPACE_TYPE
//! @brief  色空間タイプです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum COLOR_SPACE_TYPE
{
    COLOR_SPACE_SRGB = 0,       //!< sRGB.
    COLOR_SPACE_BT709_170M,     //!< BT.709  SMPTE 170M
    COLOR_SPACE_BT2100_PQ,      //!< BT.2100 PQ System
    COLOR_SPACE_BT2100_HLG,     //!< BT.2100 HLG System
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Offset2D structure
//! @brief  2次元のオフセットです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Offset2D
{
    int     X;      //!< X座標のオフセットです.
    int     Y;      //!< Y座標のオフセットです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Offset3D structure
//! @brief  3次元のオフセット.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Offset3D
{
    int     X;      //!< X座標のオフセットです.
    int     Y;      //!< Y座標のオフセットです.
    int     Z;      //!< Z座標のオフセットです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Extent2D structure
//! @brief  2次元のサイズです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Extent2D
{
    uint32_t    Width;      //!< 横幅です.
    uint32_t    Height;     //!< 縦幅です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Extent3D structure
//! @brief  3次元のサイズです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Extent3D
{
    uint32_t    Width;      //!< 横幅です.
    uint32_t    Height;     //!< 縦幅です.
    uint32_t    Depth;      //!< 奥行です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Viewport structure
//! @brief  ビューポートです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Viewport
{
    float   X;              //!< 左上のX座標です.
    float   Y;              //!< 左上のY座標です.
    float   Width;          //!< 横幅です.
    float   Height;         //!< 縦幅です.
    float   MinDepth;       //!< 深度の最小値です.
    float   MaxDepth;       //!< 深度の最大値です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Rect structure
//! @brief  矩形です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Rect
{
    Offset2D    Offset;     //!< オフセットです.
    Extent2D    Extent;     //!< サイズです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ClearDepthStencilValue structure
//! @brief  深度ステンシルのクリア値です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ClearDepthStencilValue
{
    float   Depth;              //!< 深度のクリア値です.
    uint8_t Stencil;            //!< ステンシルのクリア値です.
    bool    EnableClearDepth;   //!< 深度クリア有効化フラグ.
    bool    EnableClearStencil; //!< ステンシルクリア有効フラグ.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ClearColorValue union
//! @brief  カラーのクリア値です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ClearColorValue
{
    uint32_t    SlotIndex;  //!< レンダーターゲットのスロット番号です.
    float       R;          //!< 赤成分です.
    float       G;          //!< 緑成分です.
    float       B;          //!< 青成分です.
    float       A;          //!< アルファ成分です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DrawArguments structure
//! @brief  ドローコマンドの引数です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct DrawArguments
{
    uint32_t    VertexCount;        //!< 描画する頂点数です.
    uint32_t    InstanceCount;      //!< 描画するインスタンス数です.
    uint32_t    FirstVertex;        //!< 描画する最初の頂点へのオフセットです.
    uint32_t    FirstInstance;      //!< 描画する最初のインスタンスへのオフセットです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DrawIndexedArguments structure
//! @brief  ドローインデックスドコマンドの引数です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct DrawIndexedArguments
{
    uint32_t    IndexCount;         //!< 描画するインデックス数です.
    uint32_t    InstanceCount;      //!< 描画するインスタンス数です.
    uint32_t    FirstIndex;         //!< 描画する最初のインデックスへのオフセットです.
    int         VertexOffset;       //!< 頂点オフセットです.
    uint32_t    FirstInstance;      //!< 描画する最初のインスタンスへのオフセットです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DispatchArguments structure
//! @brief  ディスパッチコマンドの引数です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct DispatchArguments
{
    uint32_t    ThreadGroupCountX;  //!< x 方向に発行するスレッドグループの数です.
    uint32_t    ThreadGroupCountY;  //!< y 方向に発行するスレッドグループの数です.
    uint32_t    ThreadGroupCountZ;  //!< z 方向の発行するスレッドグループの数です.
};

//! @brief  コンピュートシェーダディスパッチの引数です.
using DispatchComputeArguments = DispatchArguments;

//! @brief  メッシュシェーダディスパッチの引数です.
using DispatchMeshArguments = DispatchArguments;

///////////////////////////////////////////////////////////////////////////////
// CommandListDesc structure
//! @brief  コマンドリストの構成設定です.
//////////////////////////////////////////////////////////////////////////////
struct CommandListDesc
{
    COMMANDLIST_TYPE        Type;           //!< コマンドリストタイプです.
    uint64_t                BufferSize;     //!< コマンドバッファのサイズです(D3D12, VULKANの場合はゼロで問題ありません).
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// CommandSetDesc structure
//! @brief  コマンドセットの構成設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct CommandSetDesc
{
    uint32_t                    ByteStride;         //!< コマンドセットの各引数のサイズをバイト単位で指定します.
    uint32_t                    ArgumentCount;      //!< 引数タイプの数です.
    INDIRECT_ARGUMENT_TYPE*     pArguments;         //!< 引数タイプの配列です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// BufferDesc structure
//! @brief  バッファの構成設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct BufferDesc
{
    uint64_t            Size;               //!< バッファサイズです.
    uint32_t            Stride;             //!< ストライドサイズです.
    uint32_t            Usage;              //!< 使用用途です.
    RESOURCE_STATE      InitState;          //!< 初期状態です.
    HEAP_TYPE           HeapType;           //!< ヒープタイプです.
    uint32_t            Option;             //!< オプションデータです. 通常は 0 を設定します.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// TextureDesc structure
//! @brief  テクスチャの構成設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct TextureDesc
{
    RESOURCE_DIMENSION      Dimension;          //!< 次元です.
    uint32_t                Width;              //!< 横幅です.
    uint32_t                Height;             //!< 横幅です.
    uint16_t                DepthOrArraySize;   //!< 奥行または配列サイズです.
    RESOURCE_FORMAT         Format;             //!< フォーマットです.
    uint16_t                MipLevels;          //!< ミップレベル数です.
    uint32_t                SampleCount;        //!< サンプル数です.
    RESOURCE_LAYOUT         Layout;             //!< リソースレイアウトです.
    uint32_t                Usage;              //!< 使用用途です.
    RESOURCE_STATE          InitState;          //!< 初期状態です.
    HEAP_TYPE               HeapType;           //!< ヒープタイプです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// TargetViewDesc structure
//! @brief  ターゲットビューの構成設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct TargetViewDesc
{
    VIEW_DIMENSION          Dimension;          //!< 次元です.
    RESOURCE_FORMAT         Format;             //!< フォーマットです.
    uint32_t                MipSlice;           //!< ミップスライスです.
    uint32_t                MipLevels;          //!< ミップレベル数です.
    uint32_t                FirstArraySlice;    //!< 配列スライスです.
    uint32_t                ArraySize;          //!< 配列数です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ConstantBufferViewDesc structure
//! @brief  バッファビューの構成設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ConstantBufferViewDesc
{
    uint64_t            Offset;             //!< オフセットです.
    uint64_t            Range;              //!< 使用サイズです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ShaderResourceViewDesc structure
//! @brief  シェーダリソースビューの構成設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ShaderResourceViewDesc
{
    VIEW_DIMENSION          Dimension;          //!< 次元です.
    RESOURCE_FORMAT         Format;             //!< フォーマットです.
    uint32_t                MipSlice;           //!< ミップスライスです.
    uint32_t                MipLevels;          //!< ミップレベル数です.
    uint32_t                FirstElement;       //!< 配列スライスまたは最初の要素を設定します.
    uint32_t                ElementCount;       //!< 配列数または要素数を設定します.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// UnorderedAccessViewDesc structure
//! @brief  ストレージビューの構成設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct UnorderedAccessViewDesc
{
    VIEW_DIMENSION          Dimension;              //!< 次元です.
    RESOURCE_FORMAT         Format;                 //!< フォーマットです.
    uint32_t                MipSlice;               //!< ミップスライスです.
    uint32_t                MipLevels;              //!< ミップレベル数です.
    uint64_t                FirstElement;           //!< 配列スライスまたは最初の要素を設定します.
    uint32_t                ElementCount;           //!< 配列数または要素数を設定します.
    uint32_t                StructuredByteStride;   //!< 構造化バッファのストライドです(バイト単位). ローバッファとする場合はゼロを指定します.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// SampleDesc structure
//! @brief  サンプラーの構成設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct SamplerDesc
{
    FILTER_MODE             MinFilter;          //!< 縮小フィルタです.
    FILTER_MODE             MagFilter;          //!< 拡大フィルタです.
    MIPMAP_MODE             MipMapMode;         //!< ミップマップフィルタです.
    TEXTURE_ADDRESS_MODE    AddressU;           //!< 0 ～ 1 の範囲外にある u テクスチャー座標を解決するために使用されるメソッドです
    TEXTURE_ADDRESS_MODE    AddressV;           //!< 0 ～ 1 の範囲外にある v テクスチャー座標を解決するために使用されるメソッドです
    TEXTURE_ADDRESS_MODE    AddressW;           //!< 0 ～ 1 の範囲外にある w テクスチャー座標を解決するために使用されるメソッドです
    float                   MipLodBias;         //!< 計算されたミップマップ レベルからのオフセットです
    bool                    AnisotropyEnable;   //!< 異方性フィルタリングを有効化するかどうかのフラグです.
    uint32_t                MaxAnisotropy;      //!< 異方性フィルタリングが指定されている場合に使用するクランプ値です. 有効な値は 1 ~ 16 です.
    bool                    CompareEnable;      //!< サンプリングデータに対してデータを比較を有効かするかどうかのフラグです.
    COMPARE_OP              CompareOp;          //!< 比較操作です.
    float                   MinLod;             //!< アクセスをクランプするミップマップ範囲の下限です。0 は最大かつ最も詳細なミップマップ レベルを表し、レベルの値が大きくなるほど詳細でなくなります。
    float                   MaxLod;             //!< アクセスをクランプするミップマップ範囲の上限です。0 は最大かつ最も詳細なミップマップ レベルを表し、レベルの値が大きくなるほど詳細でなくなります。この値は MinLod 以上にする必要があります。
    BORDER_COLOR            BorderColor;        //!< 境界色です.

    static SamplerDesc PointClamp           ();
    static SamplerDesc PointRepeat          ();
    static SamplerDesc PointMirror          ();
    static SamplerDesc LinearClamp          ();
    static SamplerDesc LinearRepeat         ();
    static SamplerDesc LinearMirror         ();
    static SamplerDesc AnisotropicClamp     (uint32_t maxAnisotropy = 16);
    static SamplerDesc AnisotropicRepeat    (uint32_t maxAnisotropy = 16);
    static SamplerDesc AnisotropicMirror    (uint32_t maxAnisotropy = 16);
    static SamplerDesc PointClampCmp        (COMPARE_OP op = COMPARE_OP_LEQUAL);
    static SamplerDesc LinearClampCmp       (COMPARE_OP op = COMPARE_OP_LEQUAL);
    static SamplerDesc AnisotropicClampCmp  (COMPARE_OP op = COMPARE_OP_LEQUAL, uint32_t maxAinisotropy = 16);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// RasterizerState structure
//! @brief  ラスタライザ―ステートの設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct RasterizerState
{
    POLYGON_MODE        PolygonMode;                //!< ポリゴンモードです.
    CULL_MODE           CullMode;                   //!< カリングモードです.
    bool                FrontCounterClockWise;      //!< 前面を反時計周りにするかどうか.
    int                 DepthBias;                  //!< 深度バイアス.
    float               DepthBiasClamp;             //!< 深度バイアスのクランプ値です.
    float               SlopeScaledDepthBias;       //!< 深度傾斜バイアスです.
    bool                DepthClipEnable;            //!< 深度クリップを有効にするかどうか.
    bool                EnableConservativeRaster;   //!< コンサバティブラスタライゼーションを有効化にするかどうか.

    static RasterizerState CullNone ();
    static RasterizerState CullFront();
    static RasterizerState CullBack ();
    static RasterizerState Wireframe();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// StencilTestDesc structure
//! @brief  ステンシルテストの設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct StencilTestDesc
{
    STENCIL_OP      StencilFailOp;              //!< ステンシル テストで不合格となったときに実行するステンシル処理です.
    STENCIL_OP      StencilDepthFailOp;         //!< ステンシル テストに合格し、深度テストで不合格となったときに実行するステンシル処理です.
    STENCIL_OP      StencilPassOp;              //!< ステンシル テストと深度テストの両方に合格したときに実行するステンシル処理です.
    COMPARE_OP      StencilCompareOp;           //!< ステンシル データを既存のステンシル データと比較する操作です.

    static StencilTestDesc Default();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// StencilStateDesc structure
//! @brief  ステンシルステートの設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct StencilState
{
    bool            StencilTestEnable;          //!< ステンシルテストを使用可能にします.
    uint8_t         StencllReadMask;            //!< 深度ステンシル バッファーの中で、ステンシル データを読み取る部分を識別します.
    uint8_t         StencilWriteMask;           //!< 深度ステンシル バッファーの中で、ステンシル データを書き込む部分を識別します.
    StencilTestDesc FrontFace;                  //!< 法線がカメラの方向を向いているサーフェスを持つピクセルの深度テストとステンシル テストの結果を使用する方法を識別します.
    StencilTestDesc BackFace;                   //!< 法線がカメラと逆方向を向いているサーフェスを持つピクセルの深度テストとステンシル テストの結果を使用する方法を識別します.

    static StencilState Default();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DepthStencilState structure
//! @brief  深度ステートの設定.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct DepthState
{
    bool            DepthTestEnable;            //!< 深度テストを使用可能にします.
    bool            DepthWriteEnable;           //!< 深度書き込みを可能にします.
    COMPARE_OP      DepthCompareOp;             //!< 深度データの比較操作です.

    static DepthState None      ();
    static DepthState Default   (COMPARE_OP op = COMPARE_OP_LEQUAL);
    static DepthState ReadOnly  (COMPARE_OP op = COMPARE_OP_LEQUAL);
    static DepthState WriteOnly (COMPARE_OP op = COMPARE_OP_LEQUAL);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ColorBlendState structure
//! @brief  カラーブレンドステートの設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ColorBlendState
{
    bool            BlendEnable;            //!< ブレンディングを有効にします.
    BLEND_FACTOR    SrcBlend;               //!< 入力元のRGBに対する係数を指定します.
    BLEND_FACTOR    DstBlend;               //!< 出力先のRGBに対する係数を指定します.
    BLEND_OP        BlendOp;                //!< RGBのブレンディング操作です.
    BLEND_FACTOR    SrcBlendAlpha;          //!< 入力元のアルファ値に対する係数を指定します.
    BLEND_FACTOR    DstBlendAlpha;          //!< 出力元のアルファ値に対する係数を指定します.
    BLEND_OP        BlendOpAlpha;           //!< アルファ値のブレンディング操作です.
    bool            EnableWriteR;           //!< R成分への書き込みを有効にします.
    bool            EnableWriteG;           //!< G成分への書き込みを有効にします.
    bool            EnableWriteB;           //!< B成分への書き込みを有効にします.
    bool            EnableWriteA;           //!< A成分への書き込みを有効にします.

    static ColorBlendState Opaque               ();
    static ColorBlendState AlphaBlend           ();
    static ColorBlendState Additive             ();
    static ColorBlendState Substract            ();
    static ColorBlendState PremultipliedAlpha   ();
    static ColorBlendState Multiply             ();
    static ColorBlendState Screen               ();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// BlendState structure
//! @brief  ブレンドステートの設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct BlendState
{
    bool            IndependentBlendEnable; //!< RGB値とアルファ値の独立したブレンディングを有効します.
    bool            LogicOpEnable;          //!< 論理操作を有効にします.
    LOGIC_OP        LogicOp;                //!< 論理操作です.
    ColorBlendState RenderTarget[8];        //!< カラーターゲットのブレンド設定です.

    static BlendState Default();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MultiSampleState structure
//! @brief  マルチサンプルステートの設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MultiSampleState
{
    bool            EnableAlphaToCoverage;  //!< アルファトゥカバレッジを有効にします.
    bool            EnableMultiSample;      //!< マルチサンプルを有効にします.
    uint32_t        SampleCount;            //!< マルチサンプル数です.

    static MultiSampleState Default();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// TessellationState structure
//! @brief  テッセレーションステートの設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct TessellationState
{
    uint32_t        PatchControlCount;      //!< パッチ制御点の数です.

    static TessellationState Default();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// InputElementDesc structure
//! @brief  入力要素の設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct InputElementDesc
{
    const char*             SemanticName;   //!< セマンティクス名(D3D11, D3D12)
    uint32_t                SemanticIndex;  //!< セマンティクス番号(D3D11, D3D12)
    uint32_t                Location;       //!< ロケーション番号(Vulkan).
    RESOURCE_FORMAT         Format;         //!< リソースフォーマットです.
    uint32_t                StreamIndex;    //!< ストリーム番号です.
    uint32_t                OffsetInBytes;  //!< 先頭要素からオフセットです.
    INPUT_CLASSIFICATION    InputClass;     //!< 入力分類です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// InputLayoutDesc structure
//! @brief  入力レイアウトの設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct InputLayoutDesc
{
    uint32_t            ElementCount;   //!< 入力要素数です.
    InputElementDesc*   pElements;      //!< 入力要素です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorEntry structure
//! @brief  ディスクリプタエントリーです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct DescriptorEntry
{
    uint32_t            ShaderRegister;     //!< シェーダのレジスタ番号です.
    uint32_t            ShaderMask;         //!< シェーダマスクです.
    uint32_t            BindLocation;       //!< バインド番号です.
    DESCRIPTOR_TYPE     Type;               //!< ディスクリプタタイプです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorSetLayoutDesc structure
//! @brief  ディスクリプタセットレイアウトの設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct DescriptorSetLayoutDesc
{
    uint32_t            MaxSetCount;    //!< 生成可能な最大ディスクリプタセット数です.
    uint32_t            EntryCount;     //!< エントリー数です.
    DescriptorEntry     Entries[64];    //!< エントリー情報です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ShaderBinary structure
//! @brief  シェーダバイナリです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ShaderBinary
{
    const void*     pByteCode;              //!< バイトコードです.
    uint32_t        ByteCodeSize;           //!< バイトコードのサイズです(バイト単位).
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// GraphicsPipelineStateDesc structure
//! @brief  グラフィックスパイプラインステートの設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct GraphicsPipelineStateDesc
{
    IDescriptorSetLayout*   pLayout;                //!< ディスクリプタセットレイアウトです.
    ShaderBinary            VS;                     //!< 頂点シェーダです.
    ShaderBinary            PS;                     //!< ピクセルシェーダです.
    ShaderBinary            DS;                     //!< ドメインシェーダです.
    ShaderBinary            HS;                     //!< ハルシェーダです.
    BlendState              BlendState;             //!< ブレンドステートです.
    RasterizerState         RasterizerState;        //!< ラスタライザ―ステートです.
    MultiSampleState        MultiSampleState;       //!< マルチサンプルステートです.
    DepthState              DepthState;             //!< 深度ステートです.
    StencilState            StencilState;           //!< ステンシルステートです.
    TessellationState       TessellationState;      //!< テッセレーションステートです.
    InputLayoutDesc         InputLayout;            //!< 入力レイアウトステートです.
    PRIMITIVE_TOPOLOGY      PrimitiveTopology;      //!< プリミティブトポロジーです.
    uint32_t                RenderTargetCount;      //!< レンダーターゲット数です.
    RESOURCE_FORMAT         RenderTarget[8];        //!< レンダーターゲットです.
    RESOURCE_FORMAT         DepthTarget;            //!< 深度ターゲットです
    IBlob*                  pCachedPSO;             //!< パイプラインステートキャッシュです.

    static GraphicsPipelineStateDesc Default();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ComputePipelineStateDesc structure
//! @brief  コンピュートパイプラインステートの設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ComputePipelineStateDesc
{
    IDescriptorSetLayout*   pLayout;                //!< ディスクリプタセットレイアウトです.
    ShaderBinary            CS;                     //!< コンピュートシェーダです.
    IBlob*                  pCachedPSO;             //!< パイプラインステートキャッシュです.

    static ComputePipelineStateDesc Default();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MeshShaderPipelineStateDesc structure
//! @brief メッシュシェーダパイプラインステートの設定.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MeshShaderPipelineStateDesc
{
    IDescriptorSetLayout*   pLayout;                //!< ディスクリプタセットレイアウトです.
    ShaderBinary            AS;                     //!< アンプリフィケーションシェーダです.
    ShaderBinary            MS;                     //!< メッシュシェーダです.
    ShaderBinary            PS;                     //!< ピクセルシェーダです.
    BlendState              BlendState;             //!< ブレンドステートです.
    RasterizerState         RasterizerState;        //!< ラスタライザ―ステートです.
    MultiSampleState        MultiSampleState;       //!< マルチサンプルステートです.
    DepthState              DepthState;             //!< 深度ステートです.
    StencilState            StencilState;           //!< ステンシルステートです.
    uint32_t                RenderTargetCount;      //!< レンダーターゲット数です.
    RESOURCE_FORMAT         RenderTarget[8];        //!< レンダーターゲットです.
    RESOURCE_FORMAT         DepthTarget;            //!< 深度ターゲットです
    IBlob*                  pCachedPSO;             //!< パイプラインステートキャッシュです.

    static MeshShaderPipelineStateDesc Default();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// QueryPoolDesc structure
//! @brief  クエリプールの設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct QueryPoolDesc
{
    QUERY_TYPE      Type;       //!< クエリタイプです.
    uint32_t        Count;      //!< クエリ数です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// SwapChainDesc structure
//! @brief  スワップチェインの設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct SwapChainDesc
{
    Extent2D            Extent;             //!< サイズです.
    RESOURCE_FORMAT     Format;             //!< フォーマットです.
    uint32_t            MipLevels;          //!< ミップレベルです.
    uint32_t            SampleCount;        //!< マルチサンプル数です.
    uint32_t            BufferCount;        //!< バックバッファ数です.
    uint32_t            SyncInterval;       //!< 垂直同期間隔です.
    void*               InstanceHandle;     //!< インスタンスハンドルです.
    void*               WindowHandle;       //!< ウィンドウハンドルです.
    bool                EnableFullScreen;   //!< フルスクリーン化する場合は true を指定.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MetaDataHDR10 structure
//! @brief  HDR10メタデータです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MetaDataHDR10
{
    float       PrimaryR[2];                    //!< 色座標の赤値です(配列番号0がX座標, 配列番号1がY座標となります).
    float       PrimaryG[2];                    //!< 色座標の緑値です(配列番号0がX座標, 配列番号1がY座標となります).
    float       PrimaryB[2];                    //!< 色座標の青値です(配列番号0がX座標, 配列番号1がY座標となります).
    float       WhitePoint[2];                  //!< 色座標の白色点です(配列番号0がX座標, 配列番号1がY座標となります).
    float       MaxMasteringLuminance;          //!< コンテンツをマスタリングするためのディスプレイの最大ニト[nit]数です(数値の単位は1nitです).
    float       MinMasteringLuminance;          //!< コンテンツをマスタリングするためのディスプレイの最小ニト[nit]数です(数値の単位は1nitです).
    float       MaxContentLightLevel;           //!< コンテンツの最大ニト[nit]数です.
    float       MaxFrameAverageLightLevel;      //!< フレーム平均の最大ニト[nit]数です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// SubresourceLayout structure
//! @brief  サブリソースレイアウトの設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct SubresourceLayout
{
    uint64_t    Offset;         //!< オフセットです.
    uint64_t    Size;           //!< サブリソースのサイズです.
    uint64_t    RowCount;       //!< 行数です.
    uint64_t    RowPitch;       //!< 1行あたりのバイト数です.
    uint64_t    SlicePitch;     //!< 1スライスあたりのバイト数です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PipelineStatistics structure
//! @brief  パイプライン統計情報です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct PipelineStatistics
{
    uint64_t    IAVertices;            //!< 入力アセンブラによって読み込まれた頂点数です.
    uint64_t    IAPrimitives;          //!< 入力アセンブラによって読み込まれたプリミティブ数です.
    uint64_t    VSInvocations;         //!< 頂点シェーダが呼び出された回数です.
    uint64_t    GSInvocations;         //!< ジオメトリシェーダが呼び出された回数です.
    uint64_t    GSPrimitives;          //!< ジオメトリシェーダによって出力されたプリミティブ数です.
    uint64_t    RasterizerInvocations; //!< ラスタライザに送信されたプリミティブ数です.
    uint64_t    RenderedPrimitives;    //!< レンダリングされたプリミティブ数です.
    uint64_t    PSInvocations;         //!< ピクセルシェーダが呼び出された回数です.
    uint64_t    HSInvocations;         //!< ハルシェーダが呼び出された回数です.
    uint64_t    DSInvocations;         //!< ドメインシェーダが呼び出された回数です.
    uint64_t    CSInvocations;         //!< コンピュートシェーダが呼び出された回数です.
    uint64_t    ASInvocations;         //!< 増幅シェーダが呼びされた回数です.
    uint64_t    MSInvocations;         //!< メッシュシェーダが呼び出された回数です.
    uint64_t    MSPrimitives;          //!< メッシュシェーダによって出力されたプリミティブ数.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceInfo structure
//! @brief  デバイス情報です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct DeviceInfo
{
    uint32_t    ConstantBufferMemoryAlignment;  //!< 定数バッファアライメントです.
    uint32_t    MaxTargetWidth;                 //!< ターゲットの最大横幅です.
    uint32_t    MaxTargetHeight;                //!< ターゲットの最大縦幅です.
    uint32_t    MaxTargetArraySize;             //!< ターゲットの最大配列数です.
    uint32_t    MaxColorSampleCount;            //!< 最大カラーサンプル数です.
    uint32_t    MaxDepthSampleCount;            //!< 最大深度サンプル数です.
    uint32_t    MaxStencilSampleCount;          //!< 最大ステンシルサンプル数です.
    bool        SupportAsycCompute;             //!< 非同期コンピュートのサポート.
    bool        SupportRayTracing;              //!< レイトレーシングパイプラインのサポート.
    bool        SupportMeshShader;              //!< メッシュシェーダのサポート.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceDesc structure
//! @brief  デバイスの設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct DeviceDesc
{
    uint32_t        MaxShaderResourceCount;         //!< 最大シェーダリソース数です.
    uint32_t        MaxSamplerCount;                //!< 最大サンプラー数です.
    uint32_t        MaxColorTargetCount;            //!< 最大カラーターゲット数です.
    uint32_t        MaxDepthTargetCount;            //!< 最大深度ターゲット数です.
    uint32_t        MaxGraphicsQueueSubmitCount;    //!< グラフィックスキューへの最大サブミット数です.
    uint32_t        MaxComputeQueueSubmitCount;     //!< コンピュートキューへの最大サブミット数です.
    uint32_t        MaxCopyQueueSubmitCount;        //!< コピーキューへの最大サブミット数です.
    bool            EnableDebug;                    //!< デバッグモードを有効にします.
    bool            EnableCapture;                  //!< デバッガーによるフレームキャプチャーを有効にします.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// SystemDesc structure
//! @brief  システムの設定です.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct SystemDesc
{
    IAllocator*     pSystemAllocator;   //!< システムアロケータです.
    IAllocator*     pDeviceAllocator;   //!< デバイスアロケータです(通常は nullptrを指定).
};

//-------------------------------------------------------------------------------------------------
//! @brief      指定された数値の倍数に切り上げます.
//!
//! @param [in]     value   数値.
//! @param [in]     base    倍数.
//! @return     value を base の倍数に切り上げた結果を返却します.
//-------------------------------------------------------------------------------------------------
template<typename T>
T RoundUp(T value, T base)
{
    T mask = (base - 1);
    return ( value + mask ) & ~mask;
}

//-------------------------------------------------------------------------------------------------
//! @brief      指定された数値の倍数に切り下げます.
//!
//! @param [in]     value   数値.
//! @param [in]     base    倍数.
//! @return     value を base の倍数に切り下げた結果を返却します.
//-------------------------------------------------------------------------------------------------
template<typename T>
T RoundDown(T value, T base)
{ return value & ~( base - 1 ); }

//-------------------------------------------------------------------------------------------------
//! @brief      解放処理を行います.
//!
//! @param[in]      ptr     解放するインタフェースへのポインタ.
//-------------------------------------------------------------------------------------------------
template<typename T>
void SafeRelease(T*& ptr)
{ if (ptr != nullptr) { ptr->Release(); ptr = nullptr; } }

//-------------------------------------------------------------------------------------------------
//! @brief      グラフィックスシステムを初期化します.
//!
//! @param[in]      pDesc           システム設定です.
//! @retval true    初期化に成功.
//! @retval false   初期化に失敗，または初期化済みです.
//-------------------------------------------------------------------------------------------------
A3D_API bool A3D_APIENTRY InitSystem(const SystemDesc* pDesc);

//-------------------------------------------------------------------------------------------------
//! @brief      グラフィックスが初期化済みかどうかチェックします.
//!
//! @retval true    初期化済みです.
//! @retval false   未初期化です.
//-------------------------------------------------------------------------------------------------
A3D_API bool A3D_APIENTRY IsInitSystem();

//-------------------------------------------------------------------------------------------------
//! @brief      グラフィックスシステムの終了処理を行います.
//-------------------------------------------------------------------------------------------------
A3D_API void A3D_APIENTRY TermSystem();

//-------------------------------------------------------------------------------------------------
//! @brief      デバイスを生成します.
//!
//! @param[in]      pDesc       デバイス設定です.
//! @param[out]     ppDevice    デバイスの格納先です.
//! @retval true    生成に成功.
//! @retval false   生成に失敗.
//-------------------------------------------------------------------------------------------------
A3D_API bool A3D_APIENTRY CreateDevice(
    const DeviceDesc*   pDesc,
    IDevice**           ppDevice);

//-------------------------------------------------------------------------------------------------
//! @brief      バイナリラージオブジェクトを生成します.
//!
//! @param[in]      size        サイズです(バイト単位).
//! @param[out]     ppBlob      バイナリラージオブジェクトの格納先です.
//! @retval true    生成に成功.
//! @retval false   生成に失敗.
//-------------------------------------------------------------------------------------------------
A3D_API bool A3D_APIENTRY CreateBlob(size_t size, IBlob** ppBlob);


///////////////////////////////////////////////////////////////////////////////////////////////////
// IAllocator interface
//! @brief      アロケータインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct IAllocator
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual ~IAllocator()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      メモリを確保します.
    //!
    //! @param[in]      size        メモリ確保サイズです.
    //! @param[in]      alignment   メモリアライメントです.
    //! @return     確保したメモリへのポインタを返却します. メモリ確保に失敗した場合は nullptr が返却されます.
    //---------------------------------------------------------------------------------------------
    virtual void* Alloc(size_t size, size_t alignment) noexcept = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      メモリを再確保します.
    //!
    //! @param[in]      ptr         確保済みメモリの先頭ポインタです.
    //! @param[in]      size        再確保サイズです.
    //! @param[in]      alignment   メモリアライメントです.
    //! @return     確保したメモリへのポインタを返却します. メモリ確保に失敗した場合は nullptr が返却されます.
    //---------------------------------------------------------------------------------------------
    virtual void* Realloc(void* ptr, size_t size, size_t alignment) noexcept = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      メモリを解放します.
    //!
    //! @param[in]      ptr         解放するメモリへのポインタです.
    //---------------------------------------------------------------------------------------------
    virtual void Free(void* ptr) noexcept = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IReference interface
//! @brief      参照カウンタインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IReference
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IReference()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      参照カウントを増やします.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY AddRef() = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      参照カウントを減らし，カウントがゼロになったら解放します.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY Release() = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      参照カウントを取得します.
    //!
    //! @return     参照カウントを返却します.
    //---------------------------------------------------------------------------------------------
    virtual uint32_t A3D_APIENTRY GetCount() const = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IBlob interface
//! @brief      バイナリラージオブジェクトインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IBlob : public IReference
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IBlob()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      バッファポインタを取得します.
    //!
    //! @return     バッファポインタを返却します.
    //---------------------------------------------------------------------------------------------
    virtual void* A3D_APIENTRY GetBufferPointer() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      バッファサイズを取得します.
    //!
    //! @return     バッファサイズをバイト単位で返却します.
    //---------------------------------------------------------------------------------------------
    virtual uint64_t A3D_APIENTRY GetBufferSize() const = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IDeviceChild interface
//! @brief      デバイスチャイルドインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IDeviceChild : public IReference
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IDeviceChild()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイスを取得します.
    //!
    //! @param[out]     ppDevice        デバイスの格納先です.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY GetDevice(IDevice** ppDevice) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ICommandSet interface
//! @brief      コマンドセットインターフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API ICommandSet : public IDeviceChild
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~ICommandSet()
    { /* DO_NOTHING */ }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ISampler interface
//! @brief      サンプラーインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API ISampler : public IDeviceChild
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~ISampler()
    { /* DO_NOTHING */ }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IResource interface 
//! @brief      リソースインターフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IResource : public IDeviceChild
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IResource()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      メモリマッピングを行います.
    //!
    //! @return     マッピングしたメモリです.
    //---------------------------------------------------------------------------------------------
    virtual void* A3D_APIENTRY Map() = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      メモリマッピングを解除します.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY Unmap() = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      リソース種別を取得します.
    //!
    //! @return     リソース種別を返却します.
    //---------------------------------------------------------------------------------------------
    virtual RESOURCE_KIND A3D_APIENTRY GetKind() const = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IBuffer interface 
//! @brief      バッファインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IBuffer : public IResource
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IBuffer()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //!
    //! @return     構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    virtual BufferDesc A3D_APIENTRY GetDesc() const = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ITexture interface 
//! @brief      テクスチャインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API ITexture : public IResource
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~ITexture()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //!
    //! @return     構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    virtual TextureDesc A3D_APIENTRY GetDesc() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      サブリソースレイアウトを取得します.
    //!
    //! @param[in]      subresource     サブリソース番号です.
    //! @return     サブリソースレイアウトを返却します.
    //---------------------------------------------------------------------------------------------
    virtual SubresourceLayout A3D_APIENTRY GetSubresourceLayout(uint32_t subresource) const = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IConstantBufferView interface
//! @brief      定数バッファビューインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IConstantBufferView : public IDeviceChild
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IConstantBufferView()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //! 
    //! @return     構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    virtual ConstantBufferViewDesc A3D_APIENTRY GetDesc() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      バッファを取得します.
    //! 
    //! @return     バッファを返却します.
    //---------------------------------------------------------------------------------------------
    virtual IBuffer* A3D_APIENTRY GetResource() const = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IRenderTargetView interface
//! @brief      レンダーターゲットビューインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IRenderTargetView : public IDeviceChild
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IRenderTargetView()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //! 
    //! @return     構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    virtual TargetViewDesc A3D_APIENTRY GetDesc() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      テクスチャを取得します.
    //! 
    //! @return     テクスチャを返却します.
    //---------------------------------------------------------------------------------------------
    virtual ITexture* A3D_APIENTRY GetResource() const = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IDepthStencilView interface
//! @breif      深度ステンシルビューインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IDepthStencilView : public IDeviceChild
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IDepthStencilView()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //! 
    //! @return     構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    virtual TargetViewDesc A3D_APIENTRY GetDesc() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      テクスチャを取得します.
    //! 
    //! @return     テクスチャを返却します.
    //---------------------------------------------------------------------------------------------
    virtual ITexture* A3D_APIENTRY GetResource() const = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IShaderResourceView interface
//! @brief      シェーダリソースビューインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IShaderResourceView  : public IDeviceChild
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IShaderResourceView()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //! 
    //! @return     構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    virtual ShaderResourceViewDesc A3D_APIENTRY GetDesc() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      リソースを取得します
    //! 
    //! @return     リソースを返却します.
    //---------------------------------------------------------------------------------------------
    virtual IResource* A3D_APIENTRY GetResource() const = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IUnorderedAccessView interface
//! @brief      ストレージビューインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IUnorderedAccessView : public IDeviceChild
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IUnorderedAccessView()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //!
    //! @return     構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    virtual UnorderedAccessViewDesc A3D_APIENTRY GetDesc() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      リソースを取得します.
    //!
    //! @return     リソースを返却します.
    //---------------------------------------------------------------------------------------------
    virtual IResource* A3D_APIENTRY GetResource() const = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IDescriptorSet interface
//! @brief      ディスクリプタセットインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IDescriptorSet : public IDeviceChild
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IDescriptorSet()
    { /* DO_NOTHING */ }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IDescriptorSetLayout interface
//! @brief      ディスクリプタセットインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IDescriptorSetLayout : public IDeviceChild
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IDescriptorSetLayout()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      ディスクリプタセットを割り当てます.
    //!
    //! @param[out]     ppDescriptorSet     ディスクリプタセットの格納先です.
    //! @retval true    割り当てに成功.
    //! @retval false   割り当てに失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateDescriptorSet(IDescriptorSet** ppDescriptorSet) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IPipelineState interface
//! @brief      パイプラインステートインターフェイスです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IPipelineState : public IDeviceChild
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IPipelineState()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      キャッシュデータを取得します.
    //!
    //! @param[out]     ppBlob      キャッシュデータの角の先です.
    //! @retval true    取得に成功.
    //! @retval false   取得に失敗.
    //! @note       Vulkan, D3D12環境でのみサポートされます.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY GetCachedBlob(IBlob** ppBlob) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IQeuryPool interface
//! @brief      クエリプールインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IQueryPool : public IDeviceChild
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IQueryPool()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //!
    //! @return     構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    virtual QueryPoolDesc A3D_APIENTRY GetDesc() const = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IFence interface
//! @brief      フェンスインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IFence : public IDeviceChild
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IFence()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      シグナル状態かどうかをチェックします.
    //!
    //! @retval true    シグナル状態です.
    //! @retval false   非シグナル状態です.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY IsSignaled() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      完了を待機します.
    //!
    //! @param[in]      timeoutMsec     タイムアウト時間です(ミリ秒単位).
    //! @retval true    処理完了です.
    //! @retval false   処理未完了です.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY Wait(uint32_t timeoutMsec) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ICommandList interface
//! @brief      コマンドリストインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API ICommandList : public IDeviceChild
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual ~ICommandList()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドリストへの記録を開始します.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY Begin() = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      フレームバッファを設定します.
    //!
    //! @param[in]      renderTargetViewCount   レンダーターゲットビューの数です.
    //! @param[in]      pRnderTargetViews       レンダーターゲットビューの配列です.
    //! @param[in]      pDepthStencilView       深度ステンシルビューです.
    //! @param[in]      clearColorCount         クリアカラーを行う数です.
    //! @param[in]      pClearColors            カラークリア値の指定です(nullptr可).
    //! @param[in]      pClearDepthStencil      深度ステンシルビューのクリア値です(nullptr可).
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY BeginFrameBuffer(
        uint32_t                        renderTargetViewCount,
        IRenderTargetView**             pRenderTargetViews,
        IDepthStencilView*              pDepthStencilView,
        uint32_t                        clearColorCount,
        const ClearColorValue*          pClearColors,
        const ClearDepthStencilValue*   pClearDepthStencil) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      フレームバッファを解除します.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY EndFrameBuffer() = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      ブレンド定数を設定します.
    //!
    //! @param[in]      blendConstant       ブレンド定数です.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY SetBlendConstant(const float blendConstant[4]) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      ステンシル参照値を設定します.
    //!
    //! @param[in]      stencilRef          ステンシル参照値です.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY SetStencilReference(uint32_t stencilRef) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      ビューポートを設定します.
    //!
    //! @param[in]      count       設定するビューポート数です.
    //! @param[in]      pViewports  設定するビューポートの配列です.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY SetViewports(uint32_t count, Viewport* pViewports) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      シザー矩形を設定します.
    //!
    //! @param[in]      count       設定するシザー矩形の数です.
    //! @param[in]      pScissors   設定するシザー矩形の配列です.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY SetScissors(uint32_t count, Rect* pScissors) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      パイプラインステートを設定します.
    //!
    //! @param[in]      pPipelineState      設定するパイプラインステートです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY SetPipelineState(IPipelineState* pPipelineState) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      ディスクリプタセットを設定します.
    //!
    //! @param[in]      pDescriptorSet      設定するディスクリプタセットです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY SetDescriptorSet(IDescriptorSet* pDescriptorSet) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      頂点バッファを設定します.
    //!
    //! @param[in]      startSlot       開始スロット番号です.
    //! @param[in]      count           頂点バッファ数です.
    //! @param[in]      ppResources     頂点バッファの配列です.
    //! @param[in]      pOffsets        頂点オフセットの配列です.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY SetVertexBuffers(
        uint32_t    startSlot,
        uint32_t    count,
        IBuffer**   ppResources,
        uint64_t*   pOffsets) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      インデックスバッファを設定します.
    //!
    //! @param[in]      pResource       設定するインデックスバッファです.
    //! @param[in]      offset          インデックスオフセットです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY SetIndexBuffer(
        IBuffer*    pResource,
        uint64_t    offset) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      定数バッファバッファビューを設定します.
    //!
    //! @param[in]      index       レイアウト番号です.
    //! @param[in]      pView       設定するビューです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY SetView(uint32_t index, IConstantBufferView* const pView) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      シェーダリソースビューを設定します.
    //!
    //! @param[in]      index       レイアウト番号です.
    //! @param[in]      pView       設定するビューです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY SetView(uint32_t index, IShaderResourceView* const pView) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      アンオーダードアクセスビューを設定します.
    //!
    //! @param[in]      index       レイアウト番号です.
    //! @param[in]      pView       設定するビューです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY SetView(uint32_t index, IUnorderedAccessView* const pView) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      サンプラーを設定します.
    //!
    //! @param[in]      index       レイアウト番号です.
    //! @param[in]      pSampler    設定するサンプラーです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY SetSampler(uint32_t index, ISampler* const pSampler) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      リソースバリアを設定します.
    //!
    //! @param[in]      pResource       リソースです.
    //! @param[in]      prevState       変更前の状態です.
    //! @param[in]      nextState       変更後の状態です.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY TextureBarrier(
        ITexture*       pResource,
        RESOURCE_STATE  prevState,
        RESOURCE_STATE  nextState) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      リソースバリアを設定します.
    //!
    //! @param[in]      pResource       リソースです.
    //! @param[in]      prevState       変更前の状態です.
    //! @param[in]      nextState       変更後の状態です.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY BufferBarrier(
        IBuffer*        pResource,
        RESOURCE_STATE  prevState,
        RESOURCE_STATE  nextState) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      インスタンス描画します.
    //!
    //! @param[in]      vertexCount     頂点数です.
    //! @param[in]      instanceCount   描画するインスタンス数です.
    //! @param[in]      firstVertex     最初に描画する頂点へのオフセットです.
    //! @param[in]      firstInstance   最初に描画するインスタンスへのオフセットです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY DrawInstanced(
        uint32_t vertexCount,
        uint32_t instanceCount, 
        uint32_t firstVertex, 
        uint32_t firstInstance) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      インデックスバッファを用いてインスタンス描画します.
    //!
    //! @param[in]      indexCount      インデックス数です.
    //! @param[in]      instanceCount   描画するインスタンス数です.
    //! @param[in]      firstIndex      最初に描画するインデックスへのオフセットです.
    //! @param[in]      vertexOffset    最初に描画する頂点へのオフセットです.
    //! @param[in]      firstInstance   最初に描画するインスタンスへのオフセットです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY DrawIndexedInstanced(
        uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t firstIndex,
        int      vertexOffset,
        uint32_t firstInstance) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      コンピュートシェーダを起動します.
    //!
    //! @param[in]      x       x 方向にディスパッチしたグループの数
    //! @param[in]      y       y 方向にディスパッチしたグループの数
    //! @param[in]      z       z 方向にディスパッチしたグループの数
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY DispatchCompute(uint32_t x, uint32_t y, uint32_t z) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      メッシュシェーダを起動します.
    //!
    //! @param[in]      x       x 方向にディスパッチしたグループの数
    //! @param[in]      y       y 方向にディスパッチしたグループの数
    //! @param[in]      z       z 方向にディスパッチしたグループの数
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY DispatchMesh(uint32_t x, uint32_t y, uint32_t z) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      インダイレクトコマンドを実行します.
    //!
    //! @param[in]      pCommandSet             インダイレクトコマンドセットです.
    //! @param[in]      maxCommandCount         コマンドの最大実行回数です.
    //! @param[in]      pArgumentBuffer         引数バッファです.
    //! @param[in]      argumentBufferOffset    引数バッファのオフセットです.
    //! @param[in]      pCounterBuffer          カウンタバッファです.
    //! @param[in]      counterBufferOffset     カウンタバッファのオフセットです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY ExecuteIndirect(
        ICommandSet*    pCommandSet,
        uint32_t        maxCommandCount,
        IBuffer*        pArgumentBuffer,
        uint64_t        argumentBufferOffset,
        IBuffer*        pCounterBuffer,
        uint64_t        counterBufferOffset) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      クエリを開始します.
    //!
    //! @param[in]      pQuery      設定するクエリプールです.
    //! @param[in]      index       クエリ番号です.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY BeginQuery(IQueryPool* pQuery, uint32_t index) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      クエリを終了します.
    //!
    //! @param[in]      pQuery      設定するクエリプールです.
    //! @param[in]      index       クエリ番号です.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY EndQuery(IQueryPool* pQuery, uint32_t index) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      クエリを解決します.
    //!
    //! @param[in]      pQuery          解決するクエリプールです
    //! @param[in]      startIndex      解決するクエリのオフセットです.
    //! @param[in]      queryCount      解決するクエリ数です.
    //! @param[in]      pDstBuffer      書き込み先のバッファです.
    //! @param[in]      dstOffset       書き込み先のバッファのオフセットです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY ResolveQuery(
        IQueryPool* pQuery,
        uint32_t    startIndex,
        uint32_t    queryCount,
        IBuffer*    pDstBuffer,
        uint64_t    dstOffset ) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      クエリをリセットします.
    //!
    //! @param[in]      pQuery          リセットするクエリプールです.
    //! @remark     このコマンドはフレームバッファを設定する前に呼び出す必要があり，
    //!             フレームバッファ設定中には呼び出すことが出来ません
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY ResetQuery(IQueryPool* pQuery) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      テクスチャをコピーします.
    //!
    //! @param[in]      pDstResource        コピー先のリソースです.
    //! @param[in]      pSrcResource        コピー元のリソースです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY CopyTexture(ITexture* pDstResource, ITexture* pSrcResource) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      バッファをコピーします.
    //!
    //! @param[in]      pDstResource        コピー先のリソースです.
    //! @param[in]      pSrcResource        コピー元のリソースです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY CopyBuffer(IBuffer* pDstResource, IBuffer* pSrcResource) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      領域を指定してテクスチャをコピーします.
    //!
    //! @param[in]      pDstResource        コピー先のテクスチャです.
    //! @param[in]      dstSubresource      コピー先のサブリソースです.
    //! @param[in]      dstOffset           コピー先の領域です.
    //! @param[in]      pSrcResource        コピー元のテクスチャです.
    //! @param[in]      srcSubresource      コピー元のサブリソースです.
    //! @param[in]      srcOffset           コピー元の領域です.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY CopyTextureRegion(
        ITexture*       pDstResource,
        uint32_t        dstSubresource,
        Offset3D        dstOffset,
        ITexture*       pSrcResource,
        uint32_t        srcSubresource,
        Offset3D        srcOffset,
        Extent3D        srcExtent) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      領域を指定してバッファをコピーします.
    //!
    //! @param[in]      pDstBuffer      コピー先のバッファです.
    //! @param[in]      dstOffset       コピー先のバッファオフセットです
    //! @param[in]      pSrcBuffer      コピー元のバッファです.
    //! @param[in]      srcOffset       コピー元のバッファオフセットです.
    //! @param[in]      byteCount       コピーするバイト数です.
    //--------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY CopyBufferRegion(
        IBuffer*    pDstBuffer,
        uint64_t    dstOffset,
        IBuffer*    pSrcBuffer,
        uint64_t    srcOffset,
        uint64_t    byteCount ) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      領域を指定してバッファからテクスチャにコピーします.
    //!
    //! @param[in]      pDstTexture         コピー先のテクスチャです.
    //! @param[in]      dstSubresource      コピー先のサブリソースです.
    //! @param[in]      dstOffset           コピー先のオフセットです
    //! @param[in]      pSrcBuffer          コピー元のバッファです.
    //! @param[in]      srcOffset           コピー元のオフセットです
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY CopyBufferToTexture(
        ITexture*       pDstTexture,
        uint32_t        dstSubresource,
        Offset3D        dstOffset,
        IBuffer*        pSrcBuffer,
        uint64_t        srcOffset) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      領域を指定してテクスチャからバッファにコピーします.
    //!
    //! @param[in]      pDstBuffer          コピー先のバッファです
    //! @param[in]      dstOffset           コピー先のオフセットです
    //! @param[in]      pSrcTexture         コピー元のテクスチャです
    //! @param[in]      srcSubresource      コピー元のサブリソースです
    //! @param[in]      srcOffset           コピー元のオフセットです
    //! @param[in]      srcExtent           コピー元の大きさです
    //! @param[in]      srcState            コピー元のリソースステートです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY CopyTextureToBuffer(
        IBuffer*        pDstBuffer,
        uint64_t        dstOffset,
        ITexture*       pSrcTexture,
        uint32_t        srcSubresource,
        Offset3D        srcOffset,
        Extent3D        srcExtent) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      マルチサンプリングされたリソースをマルチサンプリングされていないリソースにコピーします
    //!
    //! @param[in]      pDstResource        コピー先のリソースです
    //! @param[in]      dstSubresource      コピー先のサブリソースを特定するための、ゼロから始まるインデックスです.
    //! @param[in]      pSrcResource        コピー元のリソースです。マルチサンプリングされている必要があります.
    //! @param[in]      srcSubresource      コピー元リソース内のコピー元サブリソースです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY ResolveSubresource(
        ITexture*       pDstResource,
        uint32_t        dstSubresource,
        ITexture*       pSrcResource,
        uint32_t        srcSubresource) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      バンドルを実行します.
    //!
    //! @param[in]      pCommandList        実行するバンドルです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY ExecuteBundle(ICommandList* pCommandList) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバッグマーカーをプッシュします.
    //!
    //! @param[in]          tag         タグです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY PushMarker(const char* tag) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバッグマーカーをポップします.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY PopMarker() = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドリストの記録を終了します.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY End() = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IQueue interface
//! @brief      コマンドキューインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IQueue : IDeviceChild
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IQueue()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドリストを登録します.
    //!
    //! @param[in]      pCommandList        登録するコマンドリストです.
    //! @retval true    登録に成功.
    //! @retval false   登録に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY Submit( ICommandList* pCommandList ) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      登録したコマンドリストを実行します.
    //!
    //! @param[in]      pFence          フェンスです.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY Execute( IFence* pFence ) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドリストの実行完了を待機します.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY WaitIdle() = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      画面表示を行います.
    //!
    //! @param[in]      pSwapChain      スワップチェイン.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY Present( ISwapChain* pSwapChain ) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ISwapChain interface
//! @brief      スワップチェインインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API ISwapChain : public IDeviceChild
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~ISwapChain()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //!
    //! @return     構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    virtual SwapChainDesc A3D_APIENTRY GetDesc() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      現在のバッファ番号を取得します.
    //!
    //! @return     現在のバッファ番号を返却します.
    //---------------------------------------------------------------------------------------------
    virtual uint32_t A3D_APIENTRY GetCurrentBufferIndex() = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      指定されたバッファを取得します.
    //!
    //! @param[in]      index       バッファ番号です.
    //! @param[out]     ppResource  リソースの格納先です.
    //! @retval true    バッファの取得に成功.
    //! @retval false   バッファの取得に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY GetBuffer(uint32_t index, ITexture** ppResource) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      バッファをリサイズします.
    //!
    //! @param[in]      width       リサイズする横幅.
    //! @param[in]      height      リサイズする縦幅.
    //! @retval true    リサイズに成功.
    //! @retval false   リサイズに失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY ResizeBuffers(uint32_t width, uint32_t height) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      メタデータを設定します.
    //!
    //! @param[in]      type        メタデータタイプです.
    //! @param[in]      pData       メタデータです.
    //! @retval true    メタデータの設定に成功.
    //! @retval false   メタデータの設定に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY SetMetaData(META_DATA_TYPE type, void* pData) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      フルスクリーンモードかどうかチェックします.
    //!
    //! @retval true    フルスクリーンモードです.
    //! @retval false   ウィンドウモードです.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY IsFullScreenMode() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      フルスクリーンモードを設定します.
    //!
    //! @param[in]      enable      フルスクリーンにする場合は true を，ウィンドウモードにする場合は falseを指定します.
    //! @retval true    設定に成功.
    //! @retval false   設定に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY SetFullScreenMode(bool enable) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      色空間がサポートされているかチェックします.
    //!
    //! @param[in]      type        色空間タイプです.
    //! @retval true    チェックに成功.
    //! @retval false   チェックに失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CheckColorSpaceSupport(COLOR_SPACE_TYPE type) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      色空間を設定します.
    //!
    //! @param[in]      type        色空間タイプです.
    //! @retval true    設定に成功.
    //! @retval false   設定に失敗.
    //! @note       この関数は D3D11(Windows10), D3D12のみサポートされます.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY SetColorSpace(COLOR_SPACE_TYPE type) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// IDevice interface
//! @brief      デバイスインタフェースです.
///////////////////////////////////////////////////////////////////////////////////////////////////
struct A3D_API IDevice : public IReference
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual A3D_APIENTRY ~IDevice()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //!
    //! @return     構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    virtual DeviceDesc A3D_APIENTRY GetDesc() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイス情報を取得します.
    //!
    //! @return     デバイス情報を返却します.
    //---------------------------------------------------------------------------------------------
    virtual DeviceInfo A3D_APIENTRY GetInfo() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      グラフィックスキューを取得します.
    //!
    //! @param[out]     ppQueue     グラフィックスキューの格納先です.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY GetGraphicsQueue(IQueue** ppQueue) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      コンピュートキューを取得します.
    //!
    //! @param[out]     ppQueue     コンピュートキューの格納先です.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY GetComputeQueue(IQueue** ppQueue) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      コピーキューを取得します.
    //!
    //! @param[out]     ppQueue     コピーキューの格納先です.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY GetCopyQueue(IQueue** ppQueue) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      GPUタイムスタンプが増分する頻度(Hz単位)を取得します.
    //!
    //! @return     GPUタイムスタンプが増分する頻度を返却します.
    //---------------------------------------------------------------------------------------------
    virtual uint64_t A3D_APIENTRY GetTimeStampFrequency() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドリストを生成します.
    //!
    //! @param[in]      pDesc               構成設定です.
    //! @param[out]     ppCommandList       コマンドリストの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateCommandList(
        const CommandListDesc*  pDesc,
        ICommandList**          ppCommandList) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      スワップチェインを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppSwapChain     スワップチェインの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateSwapChain(
        const SwapChainDesc*    pDesc,
        ISwapChain**            ppSwapChain) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      バッファを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppBuffer        バッファの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateBuffer(
        const BufferDesc*   pDesc,
        IBuffer**           ppBuffer) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      テクスチャを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppTexture       テクスチャの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateTexture(
        const TextureDesc*  pDesc,
        ITexture**          ppTexture) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      レンダーターゲットビューを生成します.
    //! 
    //! @param[in]      pTexture            テクスチャです.
    //! @param[in]      pDesc               構成設定です.
    //! @param[out]     ppRenderTargetView  レンダーターゲットビューの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateRenderTargetView(
        ITexture*               pTexture,
        const TargetViewDesc*   pDesc,
        IRenderTargetView**     ppRenderTargetView) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      深度ステンシルビューを生成します.
    //! 
    //! @param[in]      pTexture            テクスチャです.
    //! @param[in]      pDesc               構成設定です.
    //! @param[in]      ppDepthStencilView  深度ステンシルビューの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateDepthStencilView(
        ITexture*               pTexture,
        const TargetViewDesc*   pDesc,
        IDepthStencilView**     ppDepthStencilView) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      定数バッファビューを生成します.
    //! 
    //! @param[in]      pBuffer                 バッファです.
    //! @param[in]      pDesc                   構成設定です.
    //! @param[out]     ppConstantBufferView    定数バッファビューの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateConstantBufferView(
        IBuffer*                        pBuffer,
        const ConstantBufferViewDesc*   pDesc,
        IConstantBufferView**           ppConstantBufferView) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      シェーダリソースビューを生成します.
    //! 
    //! @param[in]      pResource               リソースです.
    //! @param[in]      pDesc                   構成設定です.
    //! @param[out]     ppShaderResourceView    シェーダリソースビューの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateShaderResourceView(
        IResource*                      pResource,
        const ShaderResourceViewDesc*   pDesc,
        IShaderResourceView**           ppShaderResourceView) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      アンオーダードアクセスビューを生成します.
    //!
    //! @param[in]      pResource       リソースです.
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppStorageView   ストレージビューの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateUnorderedAccessView(
        IResource*                      pResource,
        const UnorderedAccessViewDesc*  pDesc,
        IUnorderedAccessView**          ppUnorderedAccessView) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      サンプラーを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppSampler       サンプラーの格納先です.
    //! @retval true    生成に成功.
    //! @retval fasle   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateSampler(
        const SamplerDesc*      pDesc,
        ISampler**              ppSampler) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      グラフィックスパイプラインを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppPipelineState パイプラインステートの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateGraphicsPipeline(
        const GraphicsPipelineStateDesc*    pDesc,
        IPipelineState**                    ppPipelineState) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      コンピュートパイプラインを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppPipelineState パイプラインステートの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateComputePipeline(
        const ComputePipelineStateDesc* pDesc,
        IPipelineState**                ppPipelineState) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      メッシュシェーダパイプラインを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppPipelineState パイプラインステートの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateMeshShaderPipeline(
        const MeshShaderPipelineStateDesc*  pDesc,
        IPipelineState**                    ppPipelineState) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      ディスクリプタセットレイアウトを生成します.
    //!
    //! @param[in]      pDesc                   構成設定です.
    //! @param[out]     ppDescriptorSetLayout   ディスクリプタセットレイアウトの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateDescriptorSetLayout(
        const DescriptorSetLayoutDesc*  pDesc,
        IDescriptorSetLayout**          ppDescriptorSetLayout) = 0;


    //---------------------------------------------------------------------------------------------
    //! @brief      クエリプールを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppQueryPool     クエリプールの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateQueryPool(
        const QueryPoolDesc*    pDesc,
        IQueryPool**            ppQueryPool ) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドセットを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppCommandSet    コマンドセットの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateCommandSet(
        const CommandSetDesc* pDesc,
        ICommandSet**         ppCommandSet) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      フェンスを生成します.
    //!
    //! @param[out]     ppFence         フェンスの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    virtual bool A3D_APIENTRY CreateFence(IFence** ppFence) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      アイドル状態になるまで待機します.
    //---------------------------------------------------------------------------------------------
    virtual void A3D_APIENTRY WaitIdle() = 0;
};

//-------------------------------------------------------------------------------------------------
//! @brief      リソースフォーマットのバイト数に変換します.
//!
//! @param[in]      format      リソースフォーマットです.
//! @return     リソースフォーマットのビット数を返却します.
//-------------------------------------------------------------------------------------------------
uint32_t A3D_APIENTRY ToByte(RESOURCE_FORMAT format);

//-------------------------------------------------------------------------------------------------
//! @brief      圧縮フォーマットかどうかチェックします.
//!
//! @retval true    圧縮フォーマットです.
//! @retval false   非圧縮フォーマットです.
//-------------------------------------------------------------------------------------------------
bool A3D_APIENTRY IsCompressFormat(RESOURCE_FORMAT format);

//-------------------------------------------------------------------------------------------------
//! @brief      sRGBフォーマットかどうかチェックします.
//!
//! @retval true    sRGBフォーマットです.
//! @retval false   非sRGBフォーマットです.
//-------------------------------------------------------------------------------------------------
bool A3D_APIENTRY IsSRGBFormat(RESOURCE_FORMAT format);

//-------------------------------------------------------------------------------------------------
//! @brief      サブリソース番号を計算します.
//!
//! @param[in]      mipSlice        ミップ番号.
//! @param[in]      arraySlice      配列番号.
//! @param[in]      planeSlice      平面スライス
//! @param[in]      mipLevels       ミップレベル数.
//! @param[in]      arraySize       配列サイズ.
//! @return     サブリソース番号を返却します.
//-------------------------------------------------------------------------------------------------
uint32_t A3D_APIENTRY CalcSubresource(
    uint32_t    mipSlice,
    uint32_t    arraySlice,
    uint32_t    planeSlice,
    uint32_t    mipLevels,
    uint32_t    arraySize);

//-------------------------------------------------------------------------------------------------
//! @brief      サブリソース番号からミップ番号，配列番号を求めます.
//!
//! @param[in]      subresouce      サブリソース番号です.
//! @param[in]      mipLevels       ミップレベル数です.
//! @param[in]      arraySize       配列数です.
//! @param[out]     mipSlice        ミップ番号です.
//! @param[out]     arraySlice      配列番号です
//! @param[out]     planeSlice      平面スライスです.
//-------------------------------------------------------------------------------------------------
void A3D_APIENTRY DecomposeSubresource(
    uint32_t    subresource,
    uint32_t    mipLevels,
    uint32_t    arraySize,
    uint32_t&   mipSlice,
    uint32_t&   arraySlice,
    uint32_t&   planeSlice);

//-------------------------------------------------------------------------------------------------
//! @brief      サブリソースサイズを求めます.
//!
//! @param[in]      format          リソースフォーマットです.
//! @param[in]      width           横幅です.
//! @param[in]      height          縦幅です.
//! @param[out]     slicePitch      1スライス当たりのサイズです.
//! @param[out]     rowPitch        1行当たりのサイズです.
//! @param[out]     rowCount        行数です.
//-------------------------------------------------------------------------------------------------
void A3D_APIENTRY CalcSubresourceSize(
    RESOURCE_FORMAT format,
    uint32_t        width,
    uint32_t        height,
    uint64_t&       slicePitch,
    uint64_t&       rowPitch,
    uint64_t&       rowCount);

//-------------------------------------------------------------------------------------------------
//! @brief      サブリソースレイアウトを求めます.
//!
//! @param[in]      subresource     サブリソース番号.
//! @param[in]      format          リソースフォーマット.
//! @param[in]      width           横幅です.
//! @param[in]      height          縦幅です.
//-------------------------------------------------------------------------------------------------
SubresourceLayout A3D_APIENTRY CalcSubresourceLayout(
    uint32_t        subresource,
    RESOURCE_FORMAT format,
    uint32_t        width, 
    uint32_t        height);

} // namespace a3d
