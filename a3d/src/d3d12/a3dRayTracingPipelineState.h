﻿//-------------------------------------------------------------------------------------------------
// File : a3dRayTracingPipelineState.h
// Desc : Ray Tracing Pipeline State.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// RayTracingPipelineState class
///////////////////////////////////////////////////////////////////////////////////////////////////
class RayTracingPipelineState : public IPipelineState, public BaseAllocator
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
    //! @param[in]      pDevice             デバイスです.
    //! @param[in]      pDesc               構成設定です.
    //! @param[out]     ppPipelineState     パイプラインステートの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(
        IDevice*                            pDevice, 
        const RayTracingPipelineStateDesc*  pDesc,
        IPipelineState**                    ppPipelineState);

    //---------------------------------------------------------------------------------------------
    //! @brief      参照カウンタを増やします.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY AddRef() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      解放処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Release() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      参照カウンタを取得します.
    //! 
    //! @return     参照カウンタを返却します.
    //---------------------------------------------------------------------------------------------
    uint32_t A3D_APIENTRY GetCount() const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイスを取得します.
    //! 
    //! @param[out]     ppDevice        デバイスの格納先です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY GetDevice(IDevice** ppDevice) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      パイプラインステートタイプを取得します.
    //! 
    //! @return     パイプラインステートタイプを返却します.
    //---------------------------------------------------------------------------------------------
    PIPELINE_STATE_TYPE A3D_APIENTRY GetType() const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      キャッシュデータを取得します.
    //! 
    //! @param[out]     ppBlob      キャッシュデータの格納先です.
    //! @retval true    取得に成功.
    //! @retval false   取得に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY GetCachedBlob(IBlob** ppBlob) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      描画コマンドを発行します.
    //! 
    //! @param[in]      pCommandList    コマンドリストです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Issue(ICommandList* pCommandList);

    //---------------------------------------------------------------------------------------------
    //! @brief      ディスクリプタセットレイアウトを取得します.
    //! 
    //! @return     ディスクリプタセットレイアウトを返却します.
    //---------------------------------------------------------------------------------------------
    DescriptorSetLayout* GetDescriptorSetLayout() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>   m_RefCount;
    Device*                 m_pDevice;
    DescriptorSetLayout*    m_pGlobalLayout;
    DescriptorSetLayout*    m_pLocalLayout;
    ID3D12StateObject*      m_pStateObject;

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    RayTracingPipelineState();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    ~RayTracingPipelineState();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理です.
    //! 
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      pDesc       構成設定です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool Init(IDevice* pDevice, const RayTracingPipelineStateDesc* pDesc);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理です.
    //---------------------------------------------------------------------------------------------
    void Term();

    RayTracingPipelineState             (const RayTracingPipelineState&) = delete;
    RayTracingPipelineState& operator = (const RayTracingPipelineState&) = delete;
};

} // namespace a3d