﻿//-------------------------------------------------------------------------------------------------
// File : hid.h
// Desc : Human Interface Device Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstdint>


namespace hid {

///////////////////////////////////////////////////////////////////////////////////////////////////
// PAD_BUTTON enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum PAD_BUTTON
{
    PAD_DIR_UP      = 0x00001,    //!< 方向パッド上.
    PAD_DIR_DOWN    = 0x00002,    //!< 方向パッド下.
    PAD_DIR_LEFT    = 0x00004,    //!< 方向パッド左.
    PAD_DIR_RIGHT   = 0x00008,    //!< 方向パッド右.
    PAD_CENTER_R    = 0x00010,    //!< XInput:STARTボタン, NX:Homeボタン,        PS4:Optionボタン.
    PAD_CENTER_L    = 0x00020,    //!< XInput:BACKボタン,  NX:キャプチャーボタン, PS4:Shareボタン.
    PAD_THUMB_L     = 0x00040,    //!< 左アナログスティックボタン.
    PAD_THUMB_R     = 0x00080,    //!< 右アナログスティックボタン.
    PAD_SHOULDER_L  = 0x00100,    //!< XInput:LBボタン,   NX:Lボタン,  PS4:L1ボタン.
    PAD_SHOULDER_R  = 0x00200,    //!< XInput:RBボタン,   NX:Rボタン,  PS4:R1ボタン.
    PAD_BTN_DOWN    = 0x01000,    //!< XInput:Aボタン,    NX:Bボタン,  PS4:×ボタン.
    PAD_BTN_RIGHT   = 0x02000,    //!< XInput:Bボタン,    NX:Aボタン,  PS4:〇ボタン.
    PAD_BTN_LEFT    = 0x04000,    //!< XInput:Xボタン,    NX:Yボタン,  PS4:□ボタン.
    PAD_BTN_UP      = 0x08000,    //!< XInput:Yボタン,    NX:Xボタン,  PS4:△ボタン.
    PAD_TRIGGER_L   = 0x10000,    //!< XInput:左トリガー, NX:ZLボタン, PS4:L2ボタン.
    PAD_TRIGGER_R   = 0x20000,    //!< XInput:右トリガー, NX:ZRボタン, PS4:R2ボタン.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// KEY_CODE enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum KEY_CODE
{
    KEY_RETURN = 0x80,      //!< リターンキーです.
    KEY_TAB,                //!< Tabキーです.
    KEY_ESC,                //!< Escapeキーです.
    KEY_BACK,               //!< BackSpaceキーです.
    KEY_SHIFT,              //!< Shiftキーです.
    KEY_CONTROL,            //!< Controlキーです.
    KEY_ALT,                //!< Altキーです.
    KEY_F1,                 //!< F1キーです.
    KEY_F2,                 //!< F2キーです.
    KEY_F3,                 //!< F3キーです.
    KEY_F4,                 //!< F4キーです.
    KEY_F5,                 //!< F5キーです.
    KEY_F6,                 //!< F6キーです.
    KEY_F7,                 //!< F7キーです.
    KEY_F8,                 //!< F8キーです.
    KEY_F9,                 //!< F9キーです.
    KEY_F10,                //!< F10キーです.
    KEY_F11,                //!< F11キーです.
    KEY_F12,                //!< F12キーです.
    KEY_UP,                 //!< ↑キーです.
    KEY_DOWN,               //!< ↓キーです.
    KEY_LEFT,               //!< ←キーです.
    KEY_RIGHT,              //!< →キーです.
    KEY_NUM0,               //!< Num0キーです.
    KEY_NUM1,               //!< Num1キーです.
    KEY_NUM2,               //!< Num2キーです.
    KEY_NUM3,               //!< Num3キーです.
    KEY_NUM4,               //!< Num4キーです.
    KEY_NUM5,               //!< Num5キーです.
    KEY_NUM6,               //!< Num6キーです.
    KEY_NUM7,               //!< Num7キーです.
    KEY_NUM8,               //!< Num8キーです.
    KEY_NUM9,               //!< Num9キーです.
    KEY_INSERT,             //!< Insertキーです.
    KEY_DELETE,             //!< Deleteキーです.
    KEY_HOME,               //!< Homeキーです.
    KEY_END,                //!< Endキーです.
    KEY_PAGE_UP,            //!< PageUpキーです.
    KEY_PAGE_DOWN,          //!< PageDownキーです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MOUSE_BUTTON enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum MOUSE_BUTTON
{
    MOUSE_BUTTON_L = 0x0,       //!< 左ボタンです.
    MOUSE_BUTTON_R,             //!< 右ボタンです.
    MOUSE_BUTTON_M,             //!< 中ボタンです.
    MOUSE_BUTTON_X1,            //!< サイドボタン1です.
    MOUSE_BUTTON_X2,            //!< サイドボタン2です.
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// IGamePadState interface
///////////////////////////////////////////////////////////////////////////////////////////////////
struct IGamePadState
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual ~IGamePadState()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      プレイヤー番号を取得します.
    //!
    //! @return     プレイヤー番号を返却します.
    //---------------------------------------------------------------------------------------------
    virtual uint32_t GetIndex() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      接続されているかどうかチェックします.
    //!
    //! @retval true    接続中です.
    //! @retval false   切断状態です.
    //---------------------------------------------------------------------------------------------
    virtual bool IsConnected() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      ボタンが押されたかどうかチェックします.
    //!
    //! @param[in]      value       判定するボタンです.
    //! @retval true    ボタンが押されています.
    //! @retval false   ボタンは押されていません.
    //! @memo       フラグ切り替え向け用途として使用してください.
    //---------------------------------------------------------------------------------------------
    virtual bool IsDown( PAD_BUTTON value ) const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      ボタンが押しっぱなしかどうかチェックします.
    //!
    //! @param[in]      value       判定するボタンで巣.
    //! @retval true    ボタンが押され続けています.
    //! @retval false   ボタンは押され続けていません.
    //! @memo       現在のフレームで押されている場合に true を返します.
    //---------------------------------------------------------------------------------------------
    virtual bool IsPush( PAD_BUTTON value ) const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      正規化された左アナログスティックのX成分を取得します.
    //!
    //! @return     正規化された左アナログスティックのX成分を返却します.
    //---------------------------------------------------------------------------------------------
    virtual float GetStickLX() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      正規化された左アナログスティックのY成分を取得します.
    //!
    //! @return     正規化された左アナログスティックのY成分を返却します.
    //---------------------------------------------------------------------------------------------
    virtual float GetStickLY() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      正規化された右アナログスティックのX成分を取得します.
    //!
    //! @return     正規化された右アナログスティックのX成分を返却します.
    //---------------------------------------------------------------------------------------------
    virtual float GetStickRX() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      正規化された右アナログスティックのY成分を取得します.
    //!
    //! @return     正規化された右アナログスティックのY成分を返却します.
    //---------------------------------------------------------------------------------------------
    virtual float GetStickRY() const = 0;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// IKeyboardState interface
///////////////////////////////////////////////////////////////////////////////////////////////////
struct IKeyboardState
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual ~IKeyboardState()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      ボタンが押されたかどうかチェックします.
    //!
    //! @param[in]      value       キーコードです(指定可能な値は　ASCII + KEYCODE列挙体です).
    //! @retval true    ボタンが押されています.
    //! @retval false   ボタンは押されていません.
    //! @memo       前のフレームのキー状態と現在のフレームのキー状態が異なる場合にtrueを返します.
    //!             フラグの切り替え用途などに使用します.
    //---------------------------------------------------------------------------------------------
    virtual bool IsDown( uint32_t value ) const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      ボタンが押されっぱなしかどうかチェックします.
    //!
    //! @param[in]      value       キーコードです(指定可能な値は　ASCII + KEYCODE列挙体です).
    //! @return     キーが押されている場合はtrueを返します.
    //! @memo       現在のフレームでキーが押されている場合にtrueを返します.
    //---------------------------------------------------------------------------------------------
    virtual bool IsPush( uint32_t value ) const = 0;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// IMouseState interface
///////////////////////////////////////////////////////////////////////////////////////////////////
struct IMouseState
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual ~IMouseState()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      現在フレームのマウスカーソルのX座標を取得します.
    //!
    //! @return     現在フレームのマウスカーソルのX座標を返却します.
    //---------------------------------------------------------------------------------------------
    virtual int GetCursorX() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      現在フレームのマウスカーソルのY座標を取得します.
    //!
    //! @return     現在フレームのマウスカーソルのY座標を返却します.
    //---------------------------------------------------------------------------------------------
    virtual int GetCursorY() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      前フレームのマウスカーソルのX座標を取得します.
    //!
    //! @return     前フレームのマウスカーソルのX座標を返却します.
    //---------------------------------------------------------------------------------------------
    virtual int GetPrevCursorX() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      前フレームのマウスカーソルのY座標を取得します.
    //!
    //! @return     前フレームのマウスカーソルのY座標を返却します.
    //---------------------------------------------------------------------------------------------
    virtual int GetPrevCursorY() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      マウスカーソルのX方向の移動差分を取得します.
    //!
    //! @return     マウスカーソルのX方向の移動差分を返却します.
    //---------------------------------------------------------------------------------------------
    virtual int GetCursorDiffX() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      マウスカーソルのY方向の移動差分を取得します.
    //!
    //! @return     マウスカーソルのY方向の移動差分を返却します.
    //---------------------------------------------------------------------------------------------
    virtual int GetCursorDiffY() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      ボタンが押されたかどうかチェックします.
    //!
    //! @param[in]      value      ボタン.
    //! @return     ボタンが押されている場合はtrueを返します.
    //! @memo       前のフレームのボタン状態と現在のフレームのボタン状態が異なる場合にtrueを返します.
    //!             フラグの切り替え用途などに使用します.   
    //---------------------------------------------------------------------------------------------
    virtual bool IsDown( MOUSE_BUTTON value ) const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      ボタンが押されっぱなしかどうかチェックします.
    //!
    //! @param[in]      value      ボタン.
    //! @return     ボタンが押された場合はtrueを返します.そうでない場合はfalseを返却します.
    //! @memo       現在のフレームでボタンが押されている場合にtrueを返します.
    //---------------------------------------------------------------------------------------------
    virtual bool IsPush( MOUSE_BUTTON value ) const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      ボタンが押されたかどうかチェックします.
    //!
    //! @param[in]      value      ボタン.
    //! @return     ボタンが押されている場合はtrueを返します.
    //! @memo       前のフレームのボタン状態と現在のフレームのボタン状態が異なる場合にtrueを返します.
    //!             フラグの切り替え用途などに使用します.
    //---------------------------------------------------------------------------------------------
    virtual bool IsDrag( MOUSE_BUTTON value ) const = 0;
};

//-------------------------------------------------------------------------------------------------
//! @brief      指定されたパッドを震わせます.
//!
//! @param[in]      index       プレイヤー番号.
//! @param[in]      leftMoter   左のモーターの振動量(有効値:0.0 ～ 1.0).
//! @param[in]      rightMoter  右のモーターの振動量(有効値:0.0 ～ 1.0).
//-------------------------------------------------------------------------------------------------
void SetGamePadVibrate(uint32_t index, float leftMoter, float rightMoter);

//-------------------------------------------------------------------------------------------------
//! @brief      ゲームパッドステートを取得します.
//!
//! @param[in]      index           プレイヤー番号です.
//! @param[out]     ppGamePad       ゲームパッドステートの格納先です.
//! @retval true    取得に成功.
//! @retval false   取得に失敗.
//-------------------------------------------------------------------------------------------------
bool GetGamePadState(uint32_t index, IGamePadState** ppGamePad);

//-------------------------------------------------------------------------------------------------
//! @brief      マウスステートを取得します.
//!
//! @param[out]     ppMouse         マウスステートの格納先です.
//! @retval true    取得に成功.
//! @retval false   取得に失敗.
//-------------------------------------------------------------------------------------------------
bool GetMouseState(IMouseState** ppMouse, void* pWindowHandle = nullptr);

//-------------------------------------------------------------------------------------------------
//! @brief      キーボードステートを取得します.
//!
//! @param[out]     ppKeyboard      キーボードステートの各右脳先です.
//! @retval true    取得に成功.
//! @retval false   取得に失敗.
//-------------------------------------------------------------------------------------------------
bool GetKeyboardState(IKeyboardState** ppKeyboard);

} // namespace hid
