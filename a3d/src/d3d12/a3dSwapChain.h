﻿//-------------------------------------------------------------------------------------------------
// File : a3dSwapChain.h
// Desc : SwapChain Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// SwapChain class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API SwapChain : ISwapChain, BaseAllocator
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
    //! @param[in]      pDevice         デバイスです.
    //! @param[in]      pQueue          キューです.
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppSwapChain     スワップチェインの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(
        IDevice*                pDevice, 
        IQueue*                 pQueue, 
        const SwapChainDesc*    pDesc,
        ISwapChain**            ppSwapChain);

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
    //! @brief      構成設定を取得します.
    //!
    //! @return     構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    SwapChainDesc A3D_APIENTRY GetDesc() const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      画面に表示します.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Present() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      現在のバッファ番号を取得します.
    //!
    //! @return     現在のバッファ番号を返却します.
    //---------------------------------------------------------------------------------------------
    uint32_t A3D_APIENTRY GetCurrentBufferIndex() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      指定されたバッファを取得します.
    //!
    //! @param[in]      index       バッファ番号です.
    //! @param[out]     ppResource  リソースの格納先です.
    //! @retval true    バッファの取得に成功.
    //! @retval false   バッファの取得に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY GetBuffer(uint32_t index, ITexture** ppResource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      スワップチェインを取得します.
    //!
    //! @return     スワップチェインを返却します.
    //---------------------------------------------------------------------------------------------
    IDXGISwapChain3* A3D_APIENTRY GetDXGISwapChain() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>   m_RefCount;     //!< 参照カウンタです.
    IDevice*                m_pDevice;      //!< デバイスです.
    SwapChainDesc           m_Desc;         //!< 構成設定です.
    IDXGISwapChain3*        m_pSwapChain;   //!< スワップチェインです.
    ITexture**              m_pBuffers;     //!< バッファです.

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY SwapChain();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //--------------------------------------------------------------------------------------------
    A3D_APIENTRY ~SwapChain();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      pQueue      キューです.
    //! @param[in]      pDesc       構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(IDevice* pDevice, IQueue* pQueue, const SwapChainDesc* pDesc);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    SwapChain       (const SwapChain&) = delete;
    void operator = (const SwapChain&) = delete;
};


} // namespace a3d