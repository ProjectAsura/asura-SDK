﻿//-------------------------------------------------------------------------------------------------
// File : a3dQueryPool.h
// Desc : QueryPool Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// QueryPool class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API QueryPool : IQueryPool, BaseAllocator
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
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppQueryPool     クエリプールの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(
        IDevice*                pDevice,
        const QueryPoolDesc*    pDesc,
        IQueryPool**            ppQueryPool);

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
    QueryPoolDesc A3D_APIENTRY GetDesc() const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      クエリヒープを取得します.
    //!
    //! @return     クエリヒープを返却します.
    //---------------------------------------------------------------------------------------------
    ID3D12QueryHeap* A3D_APIENTRY GetD3D12QueryHeap() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      クエリヒープタイプを取得します.
    //!
    //! @return     クエリヒープタイプを返却します.
    //---------------------------------------------------------------------------------------------
    D3D12_QUERY_TYPE A3D_APIENTRY GetD3D12QueryType() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>   m_RefCount;         //!< 参照カウンタです.
    IDevice*                m_pDevice;          //!< デバイスです.
    QueryPoolDesc           m_Desc;             //!< 構成設定です.
    ID3D12QueryHeap*        m_pQueryHeap;       //!< クエリヒープです.
    D3D12_QUERY_TYPE        m_QueryType;        //!< クエリタイプです.

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY QueryPool();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY ~QueryPool();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      pDesc       構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(IDevice* pDevice, const QueryPoolDesc* pDesc);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    QueryPool       (const QueryPool&) = delete;
    void operator = (const QueryPool&) = delete;
};


} // namespace a3d
