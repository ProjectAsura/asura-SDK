//-------------------------------------------------------------------------------------------------
// File : hidGamePad.cpp
// Desc : Game Pad State.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <common/hidGamePad.h>
#include <windows.h>
#include <Xinput.h>


namespace {

hid::GamePadState g_PadState[4] = {};

inline float Max(float a, float b)
{ return (a > b) ? a : b; }

} // namespace


namespace hid {

bool GetGamePadState(uint32_t index, IGamePadState** ppGamePad)
{
    if (index >= 4)
    { return false; }

    XINPUT_STATE state;
    memset(&state, 0, sizeof(state));

    auto ret = XInputGetState(index, &state);
    if (ret == ERROR_SUCCESS)
    { g_PadState[index].m_IsConnected = true; }
    else
    {
        GamePadState state = {};
        g_PadState[index] = state;
        return false;
    }

    int16_t stickLX = state.Gamepad.sThumbLX;
    int16_t stickLY = state.Gamepad.sThumbLY;
    int16_t stickRX = state.Gamepad.sThumbRX;
    int16_t stickRY = state.Gamepad.sThumbRY;

    if (stickLX <  XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
        stickLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
    { stickLX = 0; }

    if (stickLY <  XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
        stickLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
    { stickLY = 0; }

    if (stickRX <  XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
        stickRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
    { stickRX = 0; }

    if (stickRY <  XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
        stickRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
    { stickRY = 0; }

    const float divide = 32767.0f;
    g_PadState[index].m_StickLX         = Max( -1.0f, float(stickLX) / divide );
    g_PadState[index].m_StickLY         = Max( -1.0f, float(stickLY) / divide );
    g_PadState[index].m_StickRX         = Max( -1.0f, float(stickRX) / divide );
    g_PadState[index].m_StickRY         = Max( -1.0f, float(stickRY) / divide );
    g_PadState[index].m_PrevButtons     = g_PadState[index].m_CurrButtons;
    g_PadState[index].m_CurrButtons     = state.Gamepad.wButtons;
    g_PadState[index].m_PressedButtons  = (g_PadState[index].m_PrevButtons ^ g_PadState[index].m_CurrButtons) & g_PadState[index].m_CurrButtons;

    *ppGamePad = reinterpret_cast<IGamePadState*>(&g_PadState[index]);
    return true;
}

} // namespace hid



