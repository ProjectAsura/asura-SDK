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
    //! @brief      デバッグ名を設定します.
    //! 
    //! @param[in]      name        設定するデバッグ名.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetName(const char* name) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバッグ名を取得します.
    //! 
    //! @return     デバッグ名を返却します.
    //---------------------------------------------------------------------------------------------
    const char* A3D_APIENTRY GetName() const override;

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
    //! @brief      パイプラインステートを関連付けます.
    //!
    //! @param[in]      pDeviceContext      デバイスコンテキストです.
    //! @param[in]      blendFactor         ブレンド定数です.
    //! @param[in]      stencilRef          ステンシル参照値です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Bind(
        ID3D11DeviceContext*    pDeviceContext,
        const float             blendFactor[4],
        uint32_t                stencilRef);

    //---------------------------------------------------------------------------------------------
    //! @brief      ディスクリプタセットレイアウトを取得します.
    //! 
    //! @return     ディスクリプタセットレイアウトを返却します.
    //---------------------------------------------------------------------------------------------
    DescriptorSetLayout* A3D_APIENTRY GetDescriptorSetLayout() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>       m_RefCount;             //!< 参照カウンタです.
    Device*                     m_pDevice;              //!< デバイスです.
    bool                        m_IsGraphicsPipeline;   //!< グラフィックスパイプラインかどうか?
    ID3D11VertexShader*         m_pVS;                  //!< 頂点シェーダです.
    ID3D11DomainShader*         m_pDS;                  //!< ドメインシェーダです.
    ID3D11GeometryShader*       m_pGS;                  //!< ジオメトリシェーダです.
    ID3D11HullShader*           m_pHS;                  //!< ハルシェーダです.
    ID3D11PixelShader*          m_pPS;                  //!< ピクセルシェーダです.
    ID3D11ComputeShader*        m_pCS;                  //!< コンピュートシェーダです.
    ID3D11RasterizerState*      m_pRS;                  //!< ラスタライザーステートです.
    ID3D11BlendState*           m_pBS;                  //!< ブレンドステートです.
    ID3D11DepthStencilState*    m_pDSS;                 //!< 深度ステンシルステートです.
    ID3D11InputLayout*          m_pIL;                  //!< 入力レイアウトです.
    D3D11_PRIMITIVE_TOPOLOGY    m_Topology;             //!< プリミティブトポロジーです.
    DescriptorSetLayout*        m_pLayout;              //!< ディスクリプタセットレイアウトです.
    PIPELINE_STATE_TYPE         m_Type;                 //!< パイプラインタイプです.
    ObjectName                  m_Name;                 //!< デバッグ名です.

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
