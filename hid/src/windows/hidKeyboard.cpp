//-------------------------------------------------------------------------------------------------
// File : hidKeyboard.h
// Desc : Keyboard State.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <common/hidKeyboard.h>
#include <windows.h>


namespace {

hid::KeyboardState g_KeyboardState = {};

uint32_t ConvertKey( uint32_t keyCode )
{
    auto key = keyCode;
    auto result = 0u;

    auto isAscii = ( 0x20 <= key && key <= 0x7e );
    if ( isAscii )
    { return key; }

    switch( key )
    {
        case hid::KEY_RETURN:  { result = VK_RETURN; }     break;
        case hid::KEY_TAB:     { result = VK_TAB; }        break;
        case hid::KEY_ESC:     { result = VK_ESCAPE; }     break;
        case hid::KEY_BACK:    { result = VK_BACK; }       break;
        case hid::KEY_SHIFT:   { result = VK_SHIFT; }      break;
        case hid::KEY_CONTROL: { result = VK_CONTROL; }    break;
        case hid::KEY_ALT:     { result = VK_MENU; }       break;
        case hid::KEY_F1:      { result = VK_F1; }         break;
        case hid::KEY_F2:      { result = VK_F2; }         break;
        case hid::KEY_F3:      { result = VK_F3; }         break;
        case hid::KEY_F4:      { result = VK_F4; }         break;
        case hid::KEY_F5:      { result = VK_F5; }         break;
        case hid::KEY_F6:      { result = VK_F6; }         break;
        case hid::KEY_F7:      { result = VK_F7; }         break;
        case hid::KEY_F8:      { result = VK_F8; }         break;
        case hid::KEY_F9:      { result = VK_F9; }         break;
        case hid::KEY_F10:     { result = VK_F10; }        break;
        case hid::KEY_F11:     { result = VK_F11; }        break;
        case hid::KEY_F12:     { result = VK_F12; }        break;
        case hid::KEY_UP:      { result = VK_UP; }         break;
        case hid::KEY_DOWN:    { result = VK_DOWN; }       break;
        case hid::KEY_LEFT:    { result = VK_LEFT; }       break;
        case hid::KEY_RIGHT:   { result = VK_RIGHT; }      break;
        case hid::KEY_NUM0:    { result = VK_NUMPAD0; }    break;
        case hid::KEY_NUM1:    { result = VK_NUMPAD1; }    break;
        case hid::KEY_NUM2:    { result = VK_NUMPAD2; }    break;
        case hid::KEY_NUM3:    { result = VK_NUMPAD3; }    break;
        case hid::KEY_NUM4:    { result = VK_NUMPAD4; }    break;
        case hid::KEY_NUM5:    { result = VK_NUMPAD5; }    break;
        case hid::KEY_NUM6:    { result = VK_NUMPAD6; }    break;
        case hid::KEY_NUM7:    { result = VK_NUMPAD7; }    break;
        case hid::KEY_NUM8:    { result = VK_NUMPAD8; }    break;
        case hid::KEY_NUM9:    { result = VK_NUMPAD9; }    break;
        case hid::KEY_INSERT:  { result = VK_INSERT; }     break;
        case hid::KEY_DELETE:  { result = VK_DELETE; }     break;
        case hid::KEY_HOME:    { result = VK_HOME; }       break;
        case hid::KEY_END:     { result = VK_END; }        break;
    }

    return result;
}

} 

namespace hid {

bool GetKeyboardState(IKeyboardState** ppKeyboardState)
{
    if (*ppKeyboardState == nullptr)
    { return false; }

    g_KeyboardState.m_BuffferIndex = 1 - g_KeyboardState.m_BuffferIndex;

    uint8_t keys[KeyboardState::MaxKeys];
    ::GetKeyboardState( keys );
    for(auto i=0u; i<KeyboardState::MaxKeys; ++i)
    {
        auto idx = ConvertKey( i );
        g_KeyboardState.m_Keys[g_KeyboardState.m_BuffferIndex][idx] = (keys[i] & 0x80) != 0;
    }

    *ppKeyboardState = reinterpret_cast<IKeyboardState*>(&g_KeyboardState);
    return true;
}

}