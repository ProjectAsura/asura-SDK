﻿//-------------------------------------------------------------------------------------------------
// File : a3dFence.h
// Desc : Fence Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Fence class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API Fence : public IFence, public BaseAllocator
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      生成処理を行います.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @param[out]     ppFence     フェンスの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(IDevice* pDevice, IFence** ppFence);

    //---------------------------------------------------------------------------------------------
    //! @brief      参照カウントを増やします.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY AddRef() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      解放処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Release() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      参照カウントを取得します.
    //!
    //! @return     参照カウントを返却します.
    //---------------------------------------------------------------------------------------------
    uint32_t A3D_APIENTRY GetCount() const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイスを取得します.
    //!
    //! @param[out]     ppDevice        デバイスの格納先です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY GetDevice(IDevice** ppDevice) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      シグナル状態かどうかチェックします.
    //!
    //! @retval true    シグナル状態です.
    //! @retval false   非シグナル状態です.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY IsSignaled() const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      完了を待機します.
    //!
    //! @param[int]     timeout     タイムアウト時間です.
    //! @retval true    処理完了です.
    //! @retval false   処理未完了です.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Wait(uint32_t timeoutMsec) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      フェンスを取得します.
    //!
    //! @return     フェンスを返却します.
    //---------------------------------------------------------------------------------------------
    ID3D12Fence* A3D_APIENTRY GetD3D12Fence() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      フェンスカウンターを取得します.
    //!
    //! @return     フェンスカウンターを返却します.
    //---------------------------------------------------------------------------------------------
    uint32_t A3D_APIENTRY GetFenceValue() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      イベントを取得します.
    //!
    //! @return     イベントを返却します.
    //---------------------------------------------------------------------------------------------
    HANDLE A3D_APIENTRY GetEvent() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      フェンスカウンターを進めます.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY AdvanceValue();

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>   m_RefCount;         //!< 参照カウンタです.
    Device*                 m_pDevice;          //!< デバイスです.
    ID3D12Fence*            m_pFence;           //!< フェンスです.
    uint32_t                m_CurrentValue;     //!< 現在のフェンスカウンターです.
    uint32_t                m_PreviousValue;    //!< 以前のフェンスカウンターです.
    HANDLE                  m_Event;            //!< イベントです.

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY Fence();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY ~Fence();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(IDevice* pDevice);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    Fence           (const Fence&) = delete;
    void operator = (const Fence&) = delete;
};

} // namespace a3d

