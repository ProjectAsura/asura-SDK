﻿//-------------------------------------------------------------------------------------------------
// File : SampleGui.h
// Desc : ImGui Helper.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <a3d.h>
#include <chrono>
#include <SampleApp.h>
#include <SampleMath.h>
#include <imgui.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// GuiMgr class
///////////////////////////////////////////////////////////////////////////////////////////////////
class GuiMgr
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================

    //=============================================================================================
    // public methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      シングルトンインスタンスを取得します.
    //---------------------------------------------------------------------------------------------
    static GuiMgr& GetInstance();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化を行います.
    //!
    //! @param[in]      pDevice         デバイスです.
    //! @param[in]      pApp            アプリです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool Init(a3d::IDevice* pDevice, a3d::IFrameBuffer* pFrameBuffer, IApp* pApp);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void Term();

    //---------------------------------------------------------------------------------------------
    //! @brief      バッファを入れ替えます.
    //---------------------------------------------------------------------------------------------
    void SwapBuffers();

    //---------------------------------------------------------------------------------------------
    //! @brief      描画コマンドを作成します.
    //!
    //! @param[in]      pCommandList        コマンドリストです.
    //---------------------------------------------------------------------------------------------
    void Issue(a3d::ICommandList* pCommandList);

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    static GuiMgr   s_Instance;
    const uint32_t  MaxPrimitiveCount = 6 * 1024;
    a3d::IDevice*               m_pDevice;              //!< デバイスです.
    a3d::IBuffer*               m_pVB[2];               //!< 頂点バッファです.
    a3d::IBuffer*               m_pIB[2];               //!< インデックスバッファです.
    a3d::IBuffer*               m_pConstantBuffer;      //!< 定数バッファです.
    a3d::IBufferView*           m_pConstantView;        //!< 定数バッファビューです.
    size_t                      m_SizeVB[2];            //!< 頂点バッファのサイズです.
    size_t                      m_SizeIB[2];            //!< インデックスバッファのサイズです.
    a3d::ISampler*              m_pSampler;             //!< サンプラーです.
    a3d::ITexture*              m_pTexture;             //!< テクスチャです.
    a3d::ITextureView*          m_pTextureView;         //!< テクスチャビューです.
    a3d::IDescriptorSetLayout*  m_pDescriptorSetLayout; //!< ディスクリプタレイアウトです.
    a3d::IDescriptorSet*        m_pDescriptorSet;       //!< ディスクリプタセットです.
    a3d::IPipelineState*        m_pPipelineState;       //!< パイプラインステートです.
    a3d::ICommandList*          m_pCommandList;         //!< コマンドリストです.
    Mat4*                       m_pProjection;          //!< 射影行列です.
    int                         m_BufferIndex;          //!< バッファ番号です.
    IApp*                       m_pApp;                 //!< アプリケーションです.
    std::chrono::system_clock::time_point m_LastTime;

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    GuiMgr();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    ~GuiMgr();

    //---------------------------------------------------------------------------------------------
    //! @brief      マウスコールバックです.
    //!
    //! @param[in]      x               マウスカーソルのX座標です.
    //! @param[in]      y               マウスカーソルのY座標です.
    //! @param[in]      whleelDelta     マウスホイールの移動量です.
    //! @param[in]      isDownL         マウス左ボタンが押されていれば true.
    //! @param[in]      isDownM         マウス中ボタンが押されていれば true.
    //! @param[in]      isDownR         マウス右ボタンが押されていれば true.
    //---------------------------------------------------------------------------------------------
    void OnMouse(int x, int y, int wheelDelta, bool isDownL, bool isDownM, bool isDownR);

    //---------------------------------------------------------------------------------------------
    //! @brief      キーボードコールバックです.
    //!
    //! @param[in]      keyCode         キーコードです.
    //! @param[in]      isKeyDown       キーが押されていれば true.
    //! @param[in]      isAltDown       ALTキーが押されていれば true.
    //---------------------------------------------------------------------------------------------
    void OnKeyboard(uint32_t keyCode, bool isKeyDown, bool isAltDown);

    //---------------------------------------------------------------------------------------------
    //! @brief      文字入力コールバックです.
    //---------------------------------------------------------------------------------------------
    void OnChar(uint32_t keyCode);

    //---------------------------------------------------------------------------------------------
    //! @brief      描画コマンドを作成します.
    //---------------------------------------------------------------------------------------------
    void OnDraw(ImDrawData* pData);
    static void Draw(ImDrawData* pData);

    GuiMgr          (const GuiMgr&) = delete;
    void operator = (const GuiMgr&) = delete;
};
