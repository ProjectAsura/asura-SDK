//-------------------------------------------------------------------------------------------------
// File : a3dx_app.h
// Desc : Application.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <a3d.h>
#include <a3dx_events.h>


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// IApp interface
///////////////////////////////////////////////////////////////////////////////////////////////////
struct IApp
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual ~IApp()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      解放処理を行います.
    //---------------------------------------------------------------------------------------------
    virtual void Release() = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      メインループを続行するかどうか.
    //!
    //! @retval true    メインループを続行します.
    //! @retval false   メインループを終了します.
    //---------------------------------------------------------------------------------------------
    virtual bool IsLoop() = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      終了要求を出します.
    //---------------------------------------------------------------------------------------------
    virtual void PostQuit() = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      ウィンドウの横幅を取得します.
    //!
    //! @return     ウィンドウの横幅を返却します.
    //---------------------------------------------------------------------------------------------
    virtual uint32_t GetWidth() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      ウィンドウの縦幅を取得します.
    //!
    //! @return     ウィンドウの縦幅を返却します.
    //---------------------------------------------------------------------------------------------
    virtual uint32_t GetHeight() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      インスタンスハンドルを取得します.
    //!
    //! @return     インスタンスハンドルを返却します.
    //---------------------------------------------------------------------------------------------
    virtual void* GetInstanceHandle() const = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      ウィンドウハンドルを取得します.
    //!
    //! @return     ウィンドウハンドルを返却します.
    //---------------------------------------------------------------------------------------------
    virtual void* GetWindowHandle() const = 0;
};

//-------------------------------------------------------------------------------------------------
//! @brief      アプリケーションを生成します.
//!
//! @param[in]      pAllocator  アロケータです.
//! @param[in]      width       ウィンドウの横幅です.
//! @param[in]      height      ウィンドウの縦幅です.
//! @param[in]      mouseFunc   マウスコールバック関数です.
//! @param[in]      keyFunc     キーボードコールバック関数です.
//! @param[in]      resizeFunc  リサイズコールバック関数です.
//! @param[in]      typingFunc  タイピングコールバック関数です.
//! @param[out]     ppApp       アプリケーションの格納先です.
//! @retval true    生成に成功.
//! @retval false   生成に失敗.
//-------------------------------------------------------------------------------------------------
bool CreateApp(
    IAllocator* pAlloator,
    uint32_t    width,
    uint32_t    height,
    OnMouse     mouseFunc,
    OnKey       keyFunc,
    OnResize    resizeFunc,
    OnTyping    typingFunc,
    IApp**      ppApp);

} // namespace a3d
