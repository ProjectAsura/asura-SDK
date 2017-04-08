//-------------------------------------------------------------------------------------------------
// File : hidMouse.cpp
// Desc : Mouse State.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <common/hidMouse.h>
#include <windows.h>


namespace {

hid::MouseState g_MouseState = {};

}

namespace hid {

bool GetMouseState(void* pWindowHandle, IMouseState** ppMouseState)
{
    auto hWnd = static_cast<HWND>(pWindowHandle);
    g_MouseState.m_BufferIndex = 1 - g_MouseState.m_BufferIndex;
    g_MouseState.m_PrevCursorX = g_MouseState.m_CursorX;
    g_MouseState.m_PrevCursorY = g_MouseState.m_CursorY;

    POINT pt;
    GetCursorPos( &pt );
    ScreenToClient( hWnd, &pt );

    g_MouseState.m_CursorX = int( pt.x );
    g_MouseState.m_CursorY = int( pt.y );

    g_MouseState.m_Button[ g_MouseState.m_BufferIndex ][ MOUSE_BUTTON_L ]  = ( GetAsyncKeyState( VK_LBUTTON )  & 0x8000 ) ? true : false;
    g_MouseState.m_Button[ g_MouseState.m_BufferIndex ][ MOUSE_BUTTON_R ]  = ( GetAsyncKeyState( VK_RBUTTON )  & 0x8000 ) ? true : false;
    g_MouseState.m_Button[ g_MouseState.m_BufferIndex ][ MOUSE_BUTTON_M ]  = ( GetAsyncKeyState( VK_MBUTTON )  & 0x8000 ) ? true : false;
    g_MouseState.m_Button[ g_MouseState.m_BufferIndex ][ MOUSE_BUTTON_X1 ] = ( GetAsyncKeyState( VK_XBUTTON1 ) & 0x8000 ) ? true : false;
    g_MouseState.m_Button[ g_MouseState.m_BufferIndex ][ MOUSE_BUTTON_X2 ] = ( GetAsyncKeyState( VK_XBUTTON2 ) & 0x8000 ) ? true : false;

    return true;
}

} // namespace hid