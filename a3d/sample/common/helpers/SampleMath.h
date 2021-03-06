﻿//-------------------------------------------------------------------------------------------------
// File : SampleMath.h
// Desc : Sample Math Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cmath>


//-------------------------------------------------------------------------------------------------
// Constant Values.
//-------------------------------------------------------------------------------------------------
static constexpr float F_PI = 3.1415926535897932384626433832795f;


//-------------------------------------------------------------------------------------------------
//! @brief      ラジアンに変換します.
//!
//! @param[in]      degree      度数法での角度.
//! @return     ラジアン単位に変換した角度を返却します.
//-------------------------------------------------------------------------------------------------
inline float ToRadian( float degree ) noexcept
{ return degree * ( F_PI / 180.0f ); }

//-------------------------------------------------------------------------------------------------
//! @brief      度に変換します.
//!
//! @param[in]      radian      弧度法での角度.
//! @return     度単位に変換した角度を返却します.
//-------------------------------------------------------------------------------------------------
inline float ToDegree( float radian ) noexcept
{ return radian * ( 180.0f / F_PI ); }


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vec2 structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Vec2
{
    float x;    //!< X成分.
    float y;    //!< Y成分.

    //---------------------------------------------------------------------------------------------
    //! @brief      デフォルトコンストラクタです
    //---------------------------------------------------------------------------------------------
    Vec2() = default;

    //---------------------------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //!
    //! @param[in]      nx      X成分.
    //! @param[in]      ny      Y成分.
    //---------------------------------------------------------------------------------------------
    Vec2(float nx, float ny)
    : x(nx)
    , y(ny)
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      コピーコンストラクタです.
    //!
    //! @param[in]      value       コピー元の値です.
    //---------------------------------------------------------------------------------------------
    Vec2(const Vec2& value)
    : x(value.x)
    , y(value.y)
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      加算します.
    //!
    //! @param[in]      value       加算するベクトル値.
    //! @return     加算結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec2 operator + (const Vec2& value) const
    {
        return Vec2(
            x + value.x,
            y + value.y);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      減算します.
    //!
    //! @param[in]      value       減算するベクトル値.
    //! @return     減算結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec2 operator - (const Vec2& value) const
    {
        return Vec2(
            x - value.x,
            y - value.y);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      スカラー乗算します.
    //!
    //! @param[in]      value       乗算するスカラー値.
    //! @return     スカラー乗算した結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec2 operator * (float value) const
    {
        return Vec2(
            x * value,
            y * value);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      加算代入します.
    //!
    //! @param[in]      value       加算するベクトル値.
    //! @returnn    加算結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec2& operator += (const Vec2& value)
    {
        x += value.x;
        y += value.y;
        return *this;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      減算代入します.
    //!
    //! @param[in]      value       減算するベクトル値.
    //! @return     減算結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec2& operator -= (const Vec2& value)
    {
        x -= value.x;
        y -= value.y;
        return *this;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      スカラー乗算代入します.
    //!
    //! @param[in]      value       乗算するスカラー値.
    //! @return     スカラー乗算した結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec2& operator *= (float value)
    {
        x *= value;
        y *= value;
        return *this;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      内積を求めます.
    //!
    //! @param[in]      lhs     左オペランド
    //! @param[in]      rhs     右オペランド.
    //! @return     内積を返却します.
    //---------------------------------------------------------------------------------------------
    static float Dot(const Vec2& lhs, const Vec2& rhs)
    {
        return 
            lhs.x * rhs.x 
          + lhs.y * rhs.y;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      ベクトルを正規化します.
    //!
    //! @param[in]      value       正規化するベクトル値
    //! @return     正規化したベクトルを返却します.
    //---------------------------------------------------------------------------------------------
    static Vec2 Normalize(const Vec2& value)
    {
        auto mag = std::sqrt( Dot(value, value) );
        return Vec2(
            value.x / mag,
            value.y / mag);
    }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vec3 structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Vec3
{
    float x;    //!< X成分.
    float y;    //!< Y成分.
    float z;    //!< Z成分.

    //---------------------------------------------------------------------------------------------
    //! @brief      デフォルトコンストラクタです
    //---------------------------------------------------------------------------------------------
    Vec3() = default;

    //---------------------------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //!
    //! @param[in]      nx      X成分.
    //! @param[in]      ny      Y成分.
    //! @param[in]      nz      Z成分.
    //---------------------------------------------------------------------------------------------
    Vec3(float nx, float ny, float nz)
    : x(nx)
    , y(ny)
    , z(nz)
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      コピーコンストラクタです.
    //!
    //! @param[in]      value       コピー元の値です.
    //---------------------------------------------------------------------------------------------
    Vec3(const Vec3& value)
    : x(value.x)
    , y(value.y)
    , z(value.z)
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      加算します.
    //!
    //! @param[in]      value       加算するベクトル値.
    //! @return     加算結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec3 operator + (const Vec3& value) const
    {
        return Vec3(
            x + value.x,
            y + value.y,
            z + value.z);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      減算します.
    //!
    //! @param[in]      value       減算するベクトル値.
    //! @return     減算結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec3 operator - (const Vec3& value) const
    {
        return Vec3(
            x - value.x,
            y - value.y,
            z - value.z);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      スカラー乗算します.
    //!
    //! @param[in]      value       乗算するスカラー値.
    //! @return     スカラー乗算した結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec3 operator * (float value) const
    {
        return Vec3(
            x * value,
            y * value,
            z * value);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      加算代入します.
    //!
    //! @param[in]      value       加算するベクトル値.
    //! @returnn    加算結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec3& operator += (const Vec3& value)
    {
        x += value.x;
        y += value.y;
        z += value.z;
        return *this;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      減算代入します.
    //!
    //! @param[in]      value       減算するベクトル値.
    //! @return     減算結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec3& operator -= (const Vec3& value)
    {
        x -= value.x;
        y -= value.y;
        z -= value.z;
        return *this;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      スカラー乗算代入します.
    //!
    //! @param[in]      value       乗算するスカラー値.
    //! @return     スカラー乗算した結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec3& operator *= (float value)
    {
        x *= value;
        y *= value;
        z *= value;
        return *this;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      内積を求めます.
    //!
    //! @param[in]      lhs     左オペランド
    //! @param[in]      rhs     右オペランド.
    //! @return     内積を返却します.
    //---------------------------------------------------------------------------------------------
    static float Dot(const Vec3& lhs, const Vec3& rhs)
    {
        return 
            lhs.x * rhs.x 
          + lhs.y * rhs.y 
          + lhs.z * rhs.z;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      外積を求めます.
    //!
    //! @param[in]      lhs     左オペランド
    //! @param[in]      rhs     右オペランド
    //! @return     外積を返却します.
    //---------------------------------------------------------------------------------------------
    static Vec3 Cross(const Vec3& lhs, const Vec3& rhs)
    {
        return Vec3( 
            ( lhs.y * rhs.z ) - ( lhs.z * rhs.y ),
            ( lhs.z * rhs.x ) - ( lhs.x * rhs.z ),
            ( lhs.x * rhs.y ) - ( lhs.y * rhs.x ));
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      ベクトルを正規化します.
    //!
    //! @param[in]      value       正規化するベクトル値
    //! @return     正規化したベクトルを返却します.
    //---------------------------------------------------------------------------------------------
    static Vec3 Normalize(const Vec3& value)
    {
        auto mag = std::sqrt( Dot(value, value) );
        return Vec3(
            value.x / mag,
            value.y / mag,
            value.z / mag);
    }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vec4 structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Vec4
{
    float x;    //!< X成分です.
    float y;    //!< Y成分です.
    float z;    //!< Z成分です.
    float w;    //!< W成分です.

    //---------------------------------------------------------------------------------------------
    //! @brief      デフォルトコンストラクタです.
    //---------------------------------------------------------------------------------------------
    Vec4() = default;

    //---------------------------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //!
    //! @param[in]      nx      X成分です.
    //! @param[in]      ny      Y成分です.
    //! @param[in]      nz      Z成分です.
    //! @param[in]      nw      W成分です.
    //---------------------------------------------------------------------------------------------
    Vec4(float nx, float ny, float nz, float nw)
    : x(nx)
    , y(ny)
    , z(nz)
    , w(nw)
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      コピーコンストラクタです.
    //!
    //! @param[in]      value       コピー元の値です.
    //---------------------------------------------------------------------------------------------
    Vec4(const Vec4& value)
    : x(value.x)
    , y(value.y)
    , z(value.z)
    , w(value.w)
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      加算します.
    //!
    //! @param[in]      value       加算するベクトル値です.
    //! @return     加算結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec4 operator + (const Vec4& value) const
    {
        return Vec4(
            x + value.x,
            y + value.y,
            z + value.z,
            w + value.w);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      減算します.
    //!
    //! @param[in]      value       減算するベクトル値です.
    //! @return     減算結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec4 operator - (const Vec4& value) const
    {
        return Vec4(
            x - value.x,
            y - value.y,
            z - value.z,
            w - value.w);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      スカラー乗算します.
    //!
    //! @param[in]      value       スカラー乗算する値です.
    //! @return     スカラー乗算した結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec4 operator * (float value) const
    {
        return Vec4(
            x * value,
            y * value,
            z * value,
            w * value);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      加算代入します.
    //!
    //! @param[in]      value       加算するベクトル値です.
    //! @return     加算代入した結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec4& operator += (const Vec4& value)
    {
        x += value.x;
        y += value.y;
        z += value.z;
        w += value.w;
        return *this;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      減算代入します.
    //!
    //! @param[in]      value       減算するベクトル値です.
    //! @return     減算代入した結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec4& operator -= (const Vec4& value)
    {
        x -= value.x;
        y -= value.y;
        z -= value.z;
        w -= value.w;
        return *this;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      スカラー乗算代入します.
    //!
    //! @param[in]      value       スカラー乗算する値です.
    //! @return     スカラー乗算代入した結果を返却します.
    //---------------------------------------------------------------------------------------------
    Vec4& operator *= (float value)
    {
        x *= value;
        y *= value;
        z *= value;
        w *= value;
        return *this;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      内積を求めます.
    //!
    //! @param[in]      lhs     左オペランド.
    //! @param[in]      rhs     右オペランド.
    //! @return     内積を返却します.
    //---------------------------------------------------------------------------------------------
    static float Dot(const Vec4& lhs, const Vec4& rhs)
    {
        return 
            lhs.x * rhs.x 
          + lhs.y * rhs.y 
          + lhs.z * rhs.z
          + lhs.w * rhs.w;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      ベクトルを正規化します.
    //!
    //! @param[in]      value       正規化するベクトル値です.
    //! @return     正規化したベクトルを返却します.
    //---------------------------------------------------------------------------------------------
    static Vec4 Normalize(const Vec4& value)
    {
        auto mag = std::sqrt( Dot(value, value) );
        return Vec4(
            value.x / mag,
            value.y / mag,
            value.z / mag,
            value.w / mag);
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Mat4 structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Mat4
{
    Vec4 row[4];    //!< 行ベクトルです.

    //---------------------------------------------------------------------------------------------
    //! @brief      デフォルトコンストラクタです.
    //---------------------------------------------------------------------------------------------
    Mat4() = default;

    //---------------------------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //!
    //! @param[in]      m11         1行1列成分です.
    //! @param[in]      m12         1行2列成分です.
    //! @param[in]      m13         1行3列成分です.
    //! @param[in]      m14         1行4列成分です.
    //! @param[in]      m21         2行1列成分です.
    //! @param[in]      m22         2行2列成分です.
    //! @param[in]      m23         2行3列成分です.
    //! @param[in]      m24         2行4列成分です.
    //! @param[in]      m11         3行1列成分です.
    //! @param[in]      m32         3行2列成分です.
    //! @param[in]      m33         3行3列成分です.
    //! @param[in]      m34         3行4列成分です.
    //! @param[in]      m41         4行1列成分です.
    //! @param[in]      m42         4行2列成分です.
    //! @param[in]      m43         4行3列成分です.
    //! @param[in]      m44         4行4列成分です.
    //---------------------------------------------------------------------------------------------
    Mat4(
        float m11, float m12, float m13, float m14,
        float m21, float m22, float m23, float m24,
        float m31, float m32, float m33, float m34,
        float m41, float m42, float m43, float m44)
    {
        row[0] = Vec4(m11, m12, m13, m14);
        row[1] = Vec4(m21, m22, m23, m24);
        row[2] = Vec4(m31, m32, m33, m34);
        row[3] = Vec4(m41, m42, m43, m44);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //!
    //! @param[in]      r0      1行目のベクトル.
    //! @param[in]      r1      2行目のベクトル.
    //! @param[in]      r2      3行目のベクトル.
    //! @param[in]      r3      4行目のベクトル.
    //---------------------------------------------------------------------------------------------
    Mat4(const Vec4& r0, const Vec4& r1, const Vec4& r2, const Vec4 r3)
    {
        row[0] = r0;
        row[1] = r1;
        row[2] = r2;
        row[3] = r3;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      スカラー乗算します.
    //!
    //! @param[in]      value       スカラー乗算する値です.
    //! @return     スカラー乗算した結果を返却します.
    //---------------------------------------------------------------------------------------------
    Mat4 operator * (float value) const
    {
        return Mat4(
            row[0] * value,
            row[1] * value,
            row[2] * value,
            row[3] * value);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      乗算します.
    //!
    //! @param[in]      value       乗算する行列です.
    //! @return     乗算結果を返却します.
    //---------------------------------------------------------------------------------------------
    Mat4 operator * (const Mat4& value) const
    {
        return Mat4(
            ( row[0].x * value.row[0].x ) + ( row[0].y * value.row[1].x ) + ( row[0].z * row[2].x ) + ( row[0].w * value.row[3].x ),
            ( row[0].x * value.row[0].y ) + ( row[0].y * value.row[1].y ) + ( row[0].z * row[2].y ) + ( row[0].w * value.row[3].y ),
            ( row[0].x * value.row[0].z ) + ( row[0].y * value.row[1].z ) + ( row[0].z * row[2].z ) + ( row[0].w * value.row[3].z ),
            ( row[0].x * value.row[0].w ) + ( row[0].y * value.row[1].w ) + ( row[0].z * row[2].w ) + ( row[0].w * value.row[3].w ),

            ( row[1].x * value.row[0].x ) + ( row[1].y * value.row[1].x ) + ( row[1].z * row[2].x ) + ( row[1].w * value.row[3].x ),
            ( row[1].x * value.row[0].y ) + ( row[1].y * value.row[1].y ) + ( row[1].z * row[2].y ) + ( row[1].w * value.row[3].y ),
            ( row[1].x * value.row[0].z ) + ( row[1].y * value.row[1].z ) + ( row[1].z * row[2].z ) + ( row[1].w * value.row[3].z ),
            ( row[1].x * value.row[0].w ) + ( row[1].y * value.row[1].w ) + ( row[1].z * row[2].w ) + ( row[1].w * value.row[3].w ),

            ( row[2].x * value.row[2].x ) + ( row[2].y * value.row[1].x ) + ( row[2].z * row[2].x ) + ( row[2].w * value.row[3].x ),
            ( row[2].x * value.row[2].y ) + ( row[2].y * value.row[1].y ) + ( row[2].z * row[2].y ) + ( row[2].w * value.row[3].y ),
            ( row[2].x * value.row[2].z ) + ( row[2].y * value.row[1].z ) + ( row[2].z * row[2].z ) + ( row[2].w * value.row[3].z ),
            ( row[2].x * value.row[2].w ) + ( row[2].y * value.row[1].w ) + ( row[2].z * row[2].w ) + ( row[2].w * value.row[3].w ),

            ( row[3].x * value.row[3].x ) + ( row[3].y * value.row[1].x ) + ( row[3].z * row[2].x ) + ( row[3].w * value.row[3].x ),
            ( row[3].x * value.row[3].y ) + ( row[3].y * value.row[1].y ) + ( row[3].z * row[2].y ) + ( row[3].w * value.row[3].y ),
            ( row[3].x * value.row[3].z ) + ( row[3].y * value.row[1].z ) + ( row[3].z * row[2].z ) + ( row[3].w * value.row[3].z ),
            ( row[3].x * value.row[3].w ) + ( row[3].y * value.row[1].w ) + ( row[3].z * row[2].w ) + ( row[3].w * value.row[3].w ));
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      単位行列を取得します.
    //!
    //! @return     単位行列を返却します.
    //---------------------------------------------------------------------------------------------
    static Mat4 Identity()
    {
        return Mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      拡大縮小行列を取得します.
    //!
    //! @param[in]      x       X成分の拡大縮小値.
    //! @param[in]      y       Y成分の拡大縮小値.
    //! @param[in]      z       Z成分の拡大縮小値.
    //! @return     拡大縮小行列を返却します.
    //---------------------------------------------------------------------------------------------
    static Mat4 Scale(float x, float y, float z)
    {
        return Mat4(
            x, 0.0f, 0.0f, 0.0f,
            0.0f, y, 0.0f, 0.0f,
            0.0f, 0.0f, z, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      平行移動行列を取得します.
    //!
    //! @param[in]      x       X成分の平行移動量.
    //! @param[in]      y       Y成分の平行移動量.
    //! @param[in]      z       Z成分の平行移動量.
    //! @return     平行移動行列を返却します.
    //---------------------------------------------------------------------------------------------
    static Mat4 Translation(float x, float y, float z)
    {
        return Mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            x, y, z, 1.0f);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      X軸周りの回転行列を取得します.
    //!
    //! @param[in]      rad     弧度法での回転角度.
    //! @return     回転行列を返却します.
    //---------------------------------------------------------------------------------------------
    static Mat4 RotateX(float rad)
    {
        auto cosRad = std::cos(rad);
        auto sinRad = std::sin(rad);
        return Mat4(
            1.0f,   0.0f,   0.0f,   0.0f,
            0.0f,   cosRad, sinRad, 0.0f,
            0.0f,  -sinRad, cosRad, 0.0f,
            0.0f,   0.0f,   0.0f,   1.0f );
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      Y軸周りの回転行列を取得します.
    //!
    //! @param[in]      rad     弧度法での回転角度.
    //! @return     回転行列を返却します.
    //---------------------------------------------------------------------------------------------
    static Mat4 RotateY(float rad)
    {
        auto cosRad = std::cos(rad);
        auto sinRad = std::sin(rad);

        return Mat4(
            cosRad, 0.0f,  -sinRad, 0.0f,
            0.0f,   1.0f,   0.0f,   0.0f,
            sinRad, 0.0f,   cosRad, 0.0f,
            0.0f,   0.0f,   0.0f,   1.0f );
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      Z軸周りの回転行列を取得します.
    //!
    //! @param[in]      rad     弧度法での回転角度.
    //! @return     回転行列を返却します.
    //---------------------------------------------------------------------------------------------
    static Mat4 RotateZ(float rad)
    {
        auto cosRad = std::cos(rad);
        auto sinRad = std::sin(rad);

        return Mat4( 
            cosRad, sinRad, 0.0f, 0.0f,
           -sinRad, cosRad, 0.0f, 0.0f,
            0.0f,   0.0f,   1.0f, 0.0f,
            0.0f,   0.0f,   0.0f, 1.0f );
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      ビュー行列を取得します.
    //!
    //! @param[in]      pos     カメラ位置です.
    //! @param[in]      at      注視点です.
    //! @param[in]      up      上向きベクトルです.
    //! @return     ビュー行列を返却します.
    //---------------------------------------------------------------------------------------------
    static Mat4 LookAt(const Vec3& pos, const Vec3& at, const Vec3& up)
    {
        auto zaxis = pos - at;
        zaxis = Vec3::Normalize(zaxis);

        auto xaxis = Vec3::Cross(up, zaxis);
        xaxis = Vec3::Normalize(xaxis);

        auto yaxis = Vec3::Cross(zaxis, xaxis);
        yaxis = Vec3::Normalize(yaxis);

        return Mat4(
            xaxis.x, yaxis.x, zaxis.x, 0.0f,
            xaxis.y, yaxis.y, zaxis.y, 0.0f,
            xaxis.z, yaxis.z, zaxis.z, 0.0f,
            -Vec3::Dot(xaxis, pos),
            -Vec3::Dot(yaxis, pos),
            -Vec3::Dot(zaxis, pos),
            1.0f);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      透視投影行列を取得します.
    //!
    //! @param[in]      fov         弧度法での垂直画角です.
    //! @param[in]      aspect      アスペクト比です.
    //! @param[in]      minZ        ニアクリップ平面までの距離です.
    //! @param[in]      maxZ        ファークリップ平面までの距離です.
    //! @return     透視投影行列を返却します.
    //---------------------------------------------------------------------------------------------
    static Mat4 PersFov(float fov, float aspect, float minZ, float maxZ)
    {
        auto sinFov = std::sin(0.5f * fov);
        auto cosFov = std::cos(0.5f * fov);
        auto height = cosFov / sinFov;
        auto width  = height / aspect;
        auto range  = maxZ / (minZ - maxZ);

        return Mat4(
            width, 0.0f, 0.0f, 0.0f,
            0.0f, height, 0.0f, 0.0f,
            0.0f, 0.0f, range, -1.0f,
            0.0f, 0.0f, range * minZ, 0.0f);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      正射影行列を取得します.
    //!
    //! @param [in]     left        ビューボリュームの最小X値.
    //! @param [in]     right       ビューボリュームの最大X値.
    //! @param [in]     top         ビューボリュームの最小Y値.
    //! @param [in]     bottom      ビューボリュームの最大Y値.
    //! @param [in]     minZ        近クリップ平面までの距離.
    //! @param [in]     maxZ        遠クリップ平面までの距離.
    //! @return     正射影行列を返却します.
    //---------------------------------------------------------------------------------------------
    static Mat4 Ortho(float left, float right, float top, float bottom, float minZ, float maxZ)
    {
        auto width  = right  - left;
        auto height = top    - bottom;
        auto depth  = maxZ   - minZ;

        return Mat4(
            2.0f / width, 0.0f, 0.0f, 0.0f,
            0.0f, 2.0f / height, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f / depth, 0.0f,
            0.0f, 0.0f, minZ / depth, 1.0f);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      転置行列を取得します.
    //!
    //! @return     value       転置する行列です.
    //! @return     転置行列を返却します.
    //---------------------------------------------------------------------------------------------
    static Mat4 Transpose(const Mat4& value)
    {
        return Mat4(
            value.row[0].x, value.row[1].x, value.row[2].x, value.row[3].x,
            value.row[0].y, value.row[1].y, value.row[2].y, value.row[3].y,
            value.row[0].z, value.row[1].z, value.row[2].z, value.row[3].z,
            value.row[0].w, value.row[1].w, value.row[2].w, value.row[3].w);
    }
};