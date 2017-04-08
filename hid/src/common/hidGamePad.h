//-------------------------------------------------------------------------------------------------
// File : hidGamePad.h
// Desc : Game Pad State.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <hid.h>


namespace hid {

///////////////////////////////////////////////////////////////////////////////////////////////////
// GamePadState class
///////////////////////////////////////////////////////////////////////////////////////////////////
class GamePadState : IGamePadState
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    int         m_Index;
    bool        m_IsConnected;
    float       m_StickLX;
    float       m_StickLY;
    float       m_StickRX;
    float       m_StickRY;
    uint32_t    m_PressedButtons;
    uint32_t    m_CurrButtons;
    uint32_t    m_PrevButtons;

    //=============================================================================================
    // public methods.
    //=============================================================================================
    GamePadState()
    : m_Index           (-1)
    , m_IsConnected     (false)
    , m_StickLX         (0.0f)
    , m_StickLY         (0.0f)
    , m_StickRX         (0.0f)
    , m_StickRY         (0.0f)
    , m_PressedButtons  (0)
    , m_CurrButtons     (0)
    , m_PrevButtons     (0)
    { /* DO_NOTHING */ }

    ~GamePadState()
    { /* DO_NOTHING */ }

    uint32_t GetIndex() const
    { return m_Index; }

    bool IsConnected() const
    { return m_IsConnected; }

    bool IsDown( PAD_BUTTON value ) const 
    { return ( m_PressedButtons & value ) > 0; }

    bool IsPush( PAD_BUTTON value ) const
    { return ( m_CurrButtons & value ) > 0; }

    float GetStickLX() const
    { return m_StickLX; }

    float GetStickLY() const
    { return m_StickLY; }

    float GetStickRX() const
    { return m_StickRX; }

    float GetStickRY() const
    { return m_StickRY; }
};


} // namespace
