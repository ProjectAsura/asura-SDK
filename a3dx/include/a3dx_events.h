//-------------------------------------------------------------------------------------------------
// File : a3dx_events.h
// Desc : Event Definitions.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstdint>


namespace a3d {

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
// MouseEventArg structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MouseEventArg
{
    int     CursorX;        //!< マウスカーソル(X座標)
    int     CursorY;        //!< マウスカーソル(Y座標)
    int     WheelDelta;     //!< マウスホイール.
    bool    IsDownL;        //!< 左ボタンが押されたら true.
    bool    IsDownM;        //!< 中ボタンが押されたら true.
    bool    IsDownR;        //!< 右ボタンが押されたら true.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// KeyEventArg structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct KeyEventArg
{
    bool        IsKeyDown;      //!< キーが押されたら true.
    bool        IsAltDown;      //!< ALTキーが押されたら true.
    uint32_t    KeyCode;        //!< キーコード.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResizeEventArg structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResizeEventArg
{
    uint32_t    Width;          //!< ウィンドウの横幅.
    uint32_t    Height;         //!< ウィンドウの縦幅.
    float       AspectRatio;    //!< ウィンドウのアスペクト比.
};

//-------------------------------------------------------------------------------------------------
// Callbacks.
//-------------------------------------------------------------------------------------------------
typedef void (*OnMouse) (const MouseEventArg&  arg);
typedef void (*OnKey)   (const KeyEventArg&    arg);
typedef void (*OnResize)(const ResizeEventArg& arg);
typedef void (*OnTyping)(uint32_t keycode);

} // namespace a3d
