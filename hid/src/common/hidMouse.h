//-------------------------------------------------------------------------------------------------
// File : hidMouse.h
// Desc : Mouse State.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <hid.h>


namespace hid {

///////////////////////////////////////////////////////////////////////////////////////////////////
// MouseState
///////////////////////////////////////////////////////////////////////////////////////////////////
class MouseState : IMouseState
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    static const uint32_t ButtonCount = 5;
    int         m_CursorX;
    int         m_CursorY;
    int         m_PrevCursorX;
    int         m_PrevCursorY;
    uint32_t    m_BufferIndex;
    bool        m_Button[2][ButtonCount];

    //=============================================================================================
    // public methods.
    //=============================================================================================
    MouseState()
    : m_CursorX     (-1)
    , m_CursorY     (-1)
    , m_PrevCursorX (-1)
    , m_PrevCursorY (-1)
    , m_BufferIndex (0)
    {
        for(auto i=0u; i<ButtonCount; ++i)
        {
            m_Button[0][i] = false;
            m_Button[1][i] = false;
        }
    }

    ~MouseState()
    { /* DO_NOTHING */ }

    int GetCursorX() const
    { return m_CursorX; }

    int GetCursorY() const
    { return m_CursorY; }

    int GetPrevCursorX() const
    { return m_PrevCursorX; }

    int GetPrevCursorY() const
    { return m_PrevCursorY; }

    int GetCursorDiffX() const
    { return m_CursorX - m_PrevCursorX; }

    int GetCursorDiffY() const
    { return m_CursorY - m_PrevCursorY; }

    bool IsDown( MOUSE_BUTTON value ) const
    { return  m_Button[ m_BufferIndex ][ value ] & ( !m_Button[ 1 - m_BufferIndex ][ value ] ); }

    bool IsPush( MOUSE_BUTTON value ) const
    { return m_Button[m_BufferIndex][value]; }

    bool IsDrag( MOUSE_BUTTON value ) const
    { return m_Button[ m_BufferIndex ][ value ] & m_Button[ 1 - m_BufferIndex ][ value ]; }
};


} // namespace hid
