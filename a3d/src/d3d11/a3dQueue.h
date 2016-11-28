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
class A3D_API Queue : IQueue, BaseAllocator
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
    //! @param[in]      type            コマンドリストタイプです.
    //! @param[in]      maxSubmitCount  最大サブミット数です.
    //! @param[out]     ppQueue         キューの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(
        IDevice*            pDevice,
        COMMANDLIST_TYPE    type,
        uint32_t            maxSubmitCount,
        IQueue**            ppQueue);

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

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>       m_RefCount;         //!< 参照カウンタです.
    std::mutex                  m_Mutex;            //!< ミューテックスです.
    IDevice*                    m_pDevice;          //!< デバイスです.
    uint32_t                    m_MaxSubmitCount;   //!< 最大サブミット数です.
    uint32_t                    m_SubmitIndex;      //!< サブミット番号です.
    CommandList**               m_pCommandLists;    //!< コマンドリストです.
    ID3D11Query*                m_pQuery;           //!< クエリです.
    uint64_t                    m_Frequency;        //!< GPU周期です.

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
    //! @param[in]      pDevice         デバイスです.
    //! @param[in]      type            コマンドリストタイプです.
    //! @param[in]      maxSubmitCount  最大サブミット数です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(
        IDevice*            pDevice,
        COMMANDLIST_TYPE    type,
        uint32_t            maxSubmitCount);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドを解析します.
    //---------------------------------------------------------------------------------------------
    void ParseCmd();

    Queue           (const Queue&) = delete;
    void operator = (const Queue&) = delete;
};


} // namespace a3d
