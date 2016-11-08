﻿//-------------------------------------------------------------------------------------------------
// File : a3dDevice.h
// Desc : Device Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Device class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API Device : IDevice
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
    static bool A3D_APIENTRY Create(const DeviceDesc* pDesc, IDevice** ppDevice);

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
    //! @brief      コマンドリストを生成します.
    //!
    //! @param[in]      commandListType     コマンドリストタイプです.
    //! @param[in]      pOption             オプション情報を指定します.
    //! @param[out]     ppCommandList       コマンドリストの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateCommandList(
        COMMANDLIST_TYPE    commandListType,
        void*               pPlatformData,
        ICommandList**      ppCommandList) override;

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
    //! @brief      フレームバッファを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppFrameBuffer   フレームバッファの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗. 
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateFrameBuffer(
        const FrameBufferDesc*  pDesc,
        IFrameBuffer**          ppFrameBuffer) override;

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
    //! @brief      ファクトリを取得します.
    //!
    //! @return     ファクトリを返却します.
    //---------------------------------------------------------------------------------------------
    IDXGIFactory1* A3D_APIENTRY GetDXGIFactory() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイスを取得します.
    //!
    //! @return     デバイスを返却します.
    //---------------------------------------------------------------------------------------------
    ID3D12Device* A3D_APIENTRY GetD3D12Device() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      ディスクリプタヒープを取得します.
    //!
    //! @param[in]      index       ヒープ番号です.
    //! @return     ディスクリプタヒープを返却します.
    //---------------------------------------------------------------------------------------------
    DescriptorHeap* A3D_APIENTRY GetDescriptorHeap(uint32_t index);

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>   m_RefCount;             //!< 参照カウントです.
    IDXGIFactory1*          m_pFactory;             //!< ファクトリです.
    ID3D12Device*           m_pDevice;              //!< デバイスです.
    DeviceDesc              m_Desc;                 //!< 構成設定です.
    DeviceInfo              m_Info;                 //!< デバイス情報です.
    DescriptorHeap          m_DescriptorHeap[4];    //!< ディスクリプタヒープです.
    IQueue*                 m_pGraphicsQueue;       //!< グラフィックスキューです.
    IQueue*                 m_pComputeQueue;        //!< コンピュートキューです.
    IQueue*                 m_pCopyQueue;           //!< コピーキューです.

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
