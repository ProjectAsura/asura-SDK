﻿//-------------------------------------------------------------------------------------------------
// File : a3dDevice.h
// Desc : Device Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

//-------------------------------------------------------------------------------------------------
// Forward Declarations.
//-------------------------------------------------------------------------------------------------
class Queue;


///////////////////////////////////////////////////////////////////////////////////////////////////
// Device class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API Device : public IDevice, public BaseAllocator
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
    //! @param[in]      pDesc       構成設定です.
    //! @param[out]     ppDevice    デバイスの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(
        const DeviceDesc*   pDesc,
        IDevice**           ppDevice);

    //---------------------------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //!
    //! @return     構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    DeviceDesc A3D_APIENTRY GetDesc() const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイス情報を取得します.
    //!
    //! @return     デバイス情報を返却します.
    //---------------------------------------------------------------------------------------------
    DeviceInfo A3D_APIENTRY GetInfo() const override;

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
    //! @brief      グラフィックスキューを取得します.
    //!
    //! @param[out]     ppQueue     グラフィックスキューの格納先です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY GetGraphicsQueue(IQueue** ppQueue) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      コンピュートキューを取得します.
    //!
    //! @parma[out]     ppQueue     コンピュートキューの格納先です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY GetComputeQueue(IQueue** ppQueue) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      コピーキューを取得します.
    //!
    //! @param[out]     ppQueue     コピーキューの格納先です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY GetCopyQueue(IQueue** ppQueue) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      GPUタイムスタンプが増分する頻度(Hz単位)を取得します.
    //!
    //! @return     GPUタイムスタンプが増分する頻度を返却します.
    //---------------------------------------------------------------------------------------------
    uint64_t A3D_APIENTRY GetTimeStampFrequency() const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドリストを生成します.
    //!
    //! @param[in]      pDesc               構成設定です.
    //! @param[out]     ppCommandList       コマンドリストの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateCommandList(
        const CommandListDesc*  pDesc,
        ICommandList**          ppCommandList) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      スワップチェインを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppSwapChain     スワップチェインの格納先です.
    //! @reval true     生成に成功.
    //! @reval false    生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateSwapChain(
        const SwapChainDesc*    pDesc,
        ISwapChain**            ppSwapChain) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      バッファを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppResource      リソースの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateBuffer(
        const BufferDesc*     pDesc,
        IBuffer**             ppResource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      テクスチャを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppResource      リソースの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateTexture(
        const TextureDesc*     pDesc,
        ITexture**             ppResource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      レンダーターゲットビューを生成します.
    //!
    //! @param[in]      pTexture                テクスチャです.
    //! @param[in]      pDesc                   構成設定です.
    //! @param[out]     ppRenderTargetView      レンダーターゲットビューの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateRenderTargetView(
        ITexture*               pTexture,
        const TargetViewDesc*   pDesc,
        IRenderTargetView**     ppRenderTargetView) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      深度ステンシルビューを生成します.
    //! 
    //! @param[in]      pTexture            テクスチャです.
    //! @param[in]      pDesc               構成設定です.
    //! @param[out]     ppDepthStencilView  深度ステンシルビューの格納先です.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateDepthStencilView(
        ITexture*               pTexture,
        const TargetViewDesc*   pDesc,
        IDepthStencilView**     ppDepthStencilView) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      シェーダリソースビューを生成します.
    //! 
    //! @param[in]      pResource               リソースです.
    //! @param[in]      pDesc                   構成設定です.
    //! @param[out]     ppShaderResourceView    シェーダリソースビューの格納先です.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateShaderResourceView(
        IResource*                      pResource,
        const ShaderResourceViewDesc*   pDesc,
        IShaderResourceView**           ppShaderResourceView) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      定数バッファビューを生成します.
    //!
    //! @param[in]      pBuffer         バッファです.
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppBufferView    バッファビューの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateConstantBufferView(
        IBuffer*                        pBuffer,
        const ConstantBufferViewDesc*   pDesc,
        IConstantBufferView**           ppBufferView) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      アンオーダードアクセスビューを生成します.
    //!
    //! @param[in]      pResource       リソースです.
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppStorageView   ストレージビューの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateUnorderedAccessView(
        IResource*                      pResource,
        const UnorderedAccessViewDesc*  pDesc,
        IUnorderedAccessView**          ppStorageView) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      サンプラーを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppSamplers      サンプラーの格納先です.
    //! @retval true    生成に成功.
    //! @retval fasle   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateSampler(
        const SamplerDesc*      pDesc,
        ISampler**              ppSampler) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      グラフィックスパイプラインを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppPipelineState パイプラインステートの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateGraphicsPipeline(
        const GraphicsPipelineStateDesc*    pDesc,
        IPipelineState**                    ppPipelineState) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      コンピュートパイプラインを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppPipelineState パイプラインステートの格納先です.
    //! @reval true     生成に成功.
    //! @reval false    生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateComputePipeline(
        const ComputePipelineStateDesc*     pDesc,
        IPipelineState**                    ppPipelineState) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      ジオメトリパイプラインを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppPipelineState パイプラインステートの格納先です.
    //! @reval true     生成に成功.
    //! @reval false    生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateGeometryPipeline(
        const GeometryPipelineStateDesc*    pDesc,
        IPipelineState**                    ppPipelineState) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      ディスクリプタセットレイアウトを生成します.
    //!
    //! @param[in]      pDesc                   構成設定です.
    //! @param[out]     ppDescriptorSetLayout   ディスクリプタセットレイアウトの格納先です.
    //! @reval true     生成に成功.
    //! @reval false    生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateDescriptorSetLayout(
        const DescriptorSetLayoutDesc*  pDesc,
        IDescriptorSetLayout**          ppDescriptorSetLayout) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      クエリプールを生成します.
    //!
    //! @parma[in]      pDesc           構成設定です.
    //! @param[out]     ppQueryPool     クエリプールの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateQueryPool(
        const QueryPoolDesc*    pDesc,
        IQueryPool**            ppQueryPool) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドセットを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppCommandSet    コマンドセットの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateCommandSet(
        const CommandSetDesc* pDesc,
        ICommandSet**         ppCommandSet) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      フェンスを生成します.
    //!
    //! @param[out]     ppFence         フェンスの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateFence(IFence** ppFence) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      アイドル状態になるまで待機します.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY WaitIdle() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイスを取得します.
    //!
    //! @return     デバイスを返却します.
    //---------------------------------------------------------------------------------------------
    ID3D11Device* A3D_APIENTRY GetD3D11Device() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイスコンテキストを取得します.
    //!
    //! @return     デバイスコンテキストを返却します.
    //---------------------------------------------------------------------------------------------
    ID3D11DeviceContext* A3D_APIENTRY GetD3D11DeviceContext() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      DXGIファクトリー3を取得します.
    //!
    //! @return     DXGIファクトリー3を返却します.
    //---------------------------------------------------------------------------------------------
    IDXGIFactory* A3D_APIENTRY GetDXGIFactory() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      DXGIアダプター2を取得します.
    //!
    //! @return     デフォルトアダプタを返却します.
    //---------------------------------------------------------------------------------------------
    IDXGIAdapter* A3D_APIENTRY GetDXGIAdapter() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      DXGIアウトプット3を取得します.
    //!
    //! @return     デフォルトディスプレイを返却します.
    //---------------------------------------------------------------------------------------------
    IDXGIOutput* A3D_APIENTRY GetDXGIOutput() const;

#if defined(A3D_FOR_WINDOWS10)
    //---------------------------------------------------------------------------------------------
    //! @brief      DXGIファクトリー5を取得します.
    //!
    //! @return     DXGIファクトリー5を返却します.
    //---------------------------------------------------------------------------------------------
    IDXGIFactory5* A3D_APIENTRY GetDXGIFactory5() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      DXGIアダプター2を取得します.
    //!
    //! @return     デフォルトアダプタを返却します.
    //---------------------------------------------------------------------------------------------
    IDXGIAdapter3* A3D_APIENTRY GetDXGIAdapter3() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      DXGIアウトプット4を取得します.
    //!
    //! @return     デフォルトディスプレイを返却します.
    //---------------------------------------------------------------------------------------------
    IDXGIOutput4* A3D_APIENTRY GetDXGIOutput4() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      HDRディスプレイをサポートしているかどうかチェックします.
    //!
    //! @param[in]      region      ウィンドウの表示領域.
    //! @retval true    サポートしています.
    //! @retval false   非サポートです.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CheckDisplayHDRSupport(RECT region);

#endif//defined(A3D_FOR_WINDOWS10)

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>   m_RefCount;             //!< 参照カウントです.
    DeviceDesc              m_Desc;                 //!< 構成設定です.
    DeviceInfo              m_Info;                 //!< デバイス情報です.
    Queue*                  m_pGraphicsQueue;       //!< グラフィックスキューです.
    Queue*                  m_pComputeQueue;        //!< コンピュートキューです.
    Queue*                  m_pCopyQueue;           //!< コピーキューです.
    IDXGIFactory*           m_pFactory;             //!< ファクトリーです.
    IDXGIAdapter*           m_pAdapter;             //!< アダプターです.
    IDXGIOutput*            m_pOutput;              //!< アウトプットです.
    ID3D11Device*           m_pDevice;              //!< デバイスです.
    ID3D11DeviceContext*    m_pDeviceContext;       //!< デバイスコンテキストです.
    D3D_FEATURE_LEVEL       m_FeatureLevel;         //!< 機能レベル.
    uint64_t                m_TimeStampFrequency;   //!< GPUタイムスタンプの更新頻度(Hz単位)です.
#if defined(A3D_FOR_WINDOWS10)
    IDXGIFactory5*          m_pFactory5;            //!< ファクトリ5です.
    IDXGIAdapter4*          m_pAdapter3;            //!< アダプター3です.
    IDXGIOutput6*           m_pOutput4;             //!< アウトプット4です.
#endif

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY Device();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY ~Device();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @parma[in]      pDesc       構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(const DeviceDesc* pDesc);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    Device          (const Device&) = delete;
    void operator = (const Device&) = delete;
};

} // namespace a3d
