﻿//-------------------------------------------------------------------------------------------------
// File : a3dQueue.h
// Desc : Command Queue.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Queue class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API Queue : public IQueue, public BaseAllocator
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    static const uint32_t       MaxBufferCount = 2;                 //!< 最大バッファ数です.

    //=============================================================================================
    // public methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      生成処理を行います.
    //!
    //! @param[in]      pDevice             デバイスです.
    //! @param[in]      familyIndex         ファミリーインデックスです.
    //! @param[in]      queueIndex          キューインデックスです.
    //! @param[in]      maxSubmitCount      最大サブミット数です.
    //! @param[out]     ppQueue             コマンドキューの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(
        IDevice*    pDevice,
        uint32_t    familyIndex,
        uint32_t    queueIndex,
        uint32_t    maxSubmitCount,
        IQueue**    ppQueue);

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
    //! @param[out]     pPDevice        デバイスの格納先です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY GetDevice(IDevice** ppDevice) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドリストを登録します.
    //!
    //! @param[in]      pCommandList        登録するコマンドリストです.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Submit( ICommandList* pCommandList ) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      登録したコマンドリストを実行します.
    //!
    //! @param[in]      pFence          フェンスです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Execute( IFence* pFence ) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドの実行が完了するまで待機します.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY WaitIdle() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      画面に表示を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Present( ISwapChain* pSwapChain ) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドキューを取得します.
    //!
    //! @return     コマンドキューを返却します.
    //---------------------------------------------------------------------------------------------
    VkQueue A3D_APIENTRY GetVkQueue() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      シグナルセマフォを取得します.
    //!
    //! @param[in]      index       バッファ番号です.
    //! @return     シグナルセマフォを返却します.
    //---------------------------------------------------------------------------------------------
    VkSemaphore A3D_APIENTRY GetVkSignalSemaphore(uint32_t index) const;

    //---------------------------------------------------------------------------------------------
    //! @brief      ウェイトセマフォを取得します.
    //!
    //! @param[in]      index       バッファ番号です.
    //! @return     ウェイトセマフォを返却します.
    //---------------------------------------------------------------------------------------------
    VkSemaphore A3D_APIENTRY GetVkWaitSemaphore(uint32_t index) const;

    //---------------------------------------------------------------------------------------------
    //! @brief      フェンスを取得します.
    //!
    //! @param[in]      index       バッファ番号です.
    //! @return     フェンスを返却します.
    //---------------------------------------------------------------------------------------------
    VkFence A3D_APIENTRY GetVkFence(uint32_t index) const;

    //---------------------------------------------------------------------------------------------
    //! @brief      ファミリーインデックスを取得します.
    //!
    //! @return     ファミリーインデックスを返却します.
    //---------------------------------------------------------------------------------------------
    uint32_t A3D_APIENTRY GetFamilyIndex() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      現在のバッファ番号を取得します.
    //!
    //! @return     現在のバッファ番号を返却します.
    //---------------------------------------------------------------------------------------------
    uint32_t A3D_APIENTRY GetCurrentBufferIndex() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      以前のバッファ番号を取得します.
    //!
    //! @return     以前のバッファ番号を返却します.
    //---------------------------------------------------------------------------------------------
    uint32_t A3D_APIENTRY GetPreviousBufferIndex() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      同期オブジェクトをリセットします.
    //!
    //! @retval true    リセットに成功.
    //! @retval false   リセットに失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY ResetSyncObject();

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>       m_RefCount;                         //!< 参照カウンタです.
    std::mutex                  m_Mutex;                            //!< ミューテックスです.
    Device*                     m_pDevice;                          //!< デバイスです.
    VkQueue                     m_Queue;                            //!< コマンドキューです.
    uint32_t                    m_SubmitIndex;                      //!< サブミット番号です.
    VkCommandBuffer*            m_pSubmitList;                      //!< コマンドバッファです.
    uint32_t                    m_FamilyIndex;                      //!< ファミリーインデックスです.
    uint32_t                    m_MaxSubmitCount;                   //!< 最大サブミット数です.
    VkSemaphore                 m_SignalSemaphore[MaxBufferCount];  //!< シグナルセマフォです.
    VkSemaphore                 m_WaitSemaphore[MaxBufferCount];    //!< ウェイトセマフォです.
    VkFence                     m_Fence[MaxBufferCount];            //!< フェンスです.
    uint32_t                    m_CurrentBufferIndex;               //!< 現在のバッファ番号です.
    uint32_t                    m_PreviousBufferIndex;              //!< 以前のバッファ番号です.

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY Queue();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY ~Queue();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice             デバイスです.
    //! @param[in]      familyIndex         ファミリーインデックスです.
    //! @param[in]      queueIndex          キューインデックスです.
    //! @param[in]      maxSubmitCount      最大サブミット数です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(
        IDevice*    pDevice,
        uint32_t    familyIndex,
        uint32_t    queueIndex,
        uint32_t    maxSubmitCount);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    Queue           (const Queue&) = delete;
    void operator = (const Queue&) = delete;
};


} // namespace a3d
