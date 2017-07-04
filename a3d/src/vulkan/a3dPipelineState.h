﻿//-------------------------------------------------------------------------------------------------
// File : a3dPipelineState.h
// Desc : Pipeline State Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// PipelineState class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API PipelineState : public IPipelineState, public BaseAllocator
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
    //! @brief      グラフィックスパイプラインとして生成します.
    //!
    //! @param[in]      pDevice             デバイスです.
    //! @param[in]      pDesc               構成設定です.
    //! @param[out]     ppPipelineState     パイプラインステートの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY CreateAsGraphics(
        IDevice*                         pDevice, 
        const GraphicsPipelineStateDesc* pDesc,
        IPipelineState**                 ppPipelineState);

    //---------------------------------------------------------------------------------------------
    //! @brief      コンピュートパイプラインとして生成します.
    //!
    //! @param[in]      pDevice             デバイスです.
    //! @param[in]      pDesc               構成設定です.
    //! @param[in]      ppPipelineState     パイプラインステートの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY CreateAsCompute(
        IDevice*                        pDevice,
        const ComputePipelineStateDesc* pDesc,
        IPipelineState**                ppPipelineState);

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
    //! @brief      キャッシュデータを取得します.
    //!
    //! @param[out]     ppBlob      キャッシュデータの角の先です.
    //! @retval true    取得に成功.
    //! @retval false   取得に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY GetCachedBlob(IBlob** ppBlob) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      パイプラインステートを取得します.
    //!
    //! @return     パイプラインステートを返却します.
    //---------------------------------------------------------------------------------------------
    VkPipeline A3D_APIENTRY GetVulkanPipeline() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      パイプラインバインドポイントを取得します.
    //!
    //! @return     パイプラインバインドポイントを返却します.
    //---------------------------------------------------------------------------------------------
    VkPipelineBindPoint A3D_APIENTRY GetVulkanPipelineBindPoint() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>   m_RefCount;             //!< 参照カウンタです.
    Device*                 m_pDevice;              //!< デバイスです.
    VkPipeline              m_PipelineState;        //!< パイプラインステートです.
    VkPipelineBindPoint     m_BindPoint;            //!< バインドポイントです.
    VkPipelineCache         m_PipelineCache;        //!< パイプラインキャッシュです.
    VkRenderPass            m_RenderPass;           //!< レンダーパスです.

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY PipelineState();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY ~PipelineState();

    //---------------------------------------------------------------------------------------------
    //! @brief      グラフィックスパイプラインとして初期化します.
    //!
    //! @param[in]      pDevice         デバイスです.
    //! @parma[in]      pDesc           構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY InitAsGraphics(IDevice* pDevice, const GraphicsPipelineStateDesc* pDesc);

    //---------------------------------------------------------------------------------------------
    //! @brief      コンピュートパイプランとして初期化します.
    //!
    //! @param[in]      pDevice         デバイスです.
    //! @param[in]      pDesc           構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY InitAsCompute(IDevice* pDevice, const ComputePipelineStateDesc* pDesc);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    PipelineState   (const PipelineState&) = delete;
    void operator = (const PipelineState&) = delete;
};

} // namespace a3d
