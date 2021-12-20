//-------------------------------------------------------------------------------------------------
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
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // EXTENSION enum
    ///////////////////////////////////////////////////////////////////////////////////////////////
    enum EXTENSION
    {
        EXT_KHR_PUSH_DESCRIPTOR = 0,            // VK_KHR_push_descriptor
        EXT_KHR_DESCRIPTOR_UPDATE_TEMPLATE,     // VK_KHR_descriptor_upate_template
        EXT_AMD_DRAW_INDIRECT_COUNT,            // VK_AMD_draw_indirect_count
        EXT_DEBUG_MARKER,                       // VK_EXT_debug_marker
        EXT_HDR_METADATA,                       // VK_EXT_hdr_metadata
        EXT_KHR_GET_EMEMORY_REQUIREMENT2,       // VK_KHR_get_memory_requirement2   (for VK_KHR_ray_tracing).
        EXT_KHR_MAINTENANCE3,                   // VK_KHR_maintenance3              (for VK_EXT_descriptor_indexing).
        EXT_DESCRIPTOR_INDEXING,                // VK_EXT_descriptor_indexing       (for VK_KHR_ray_tracing).
        EXT_KHR_BUFFER_DEVICE_ADDRESS,          // VK_KHR_buffer_device_address     (for VK_KHR_ray_tracing).
        EXT_KHR_DEFERRED_HOST_OPERATION,        // VK_KHR_deferred_host_operation   (for VK_KHR_ray_tracing).
        EXT_KHR_PIPELINE_LIBRARY,               // VK_KHR_pipeline_library          (for VK_KHR_ray_tracing).
        EXT_KHR_ACCELERATION_STRUCTURE,         // VK_KHR_acceleration_structure
        EXT_KHR_RAY_TRACING,                    // VK_KHR_ray_tracing
        EXT_KHR_RAY_QUERY,                      // VK_KHR_ray_query
        EXT_NV_MESH_SHADER,                     // VK_NV_mesh_shader
        EXT_KHR_DYNAMIC_RENDERING,              // VK_KHR_dynamic_rendering
        EXT_COUNT,
    };

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
    //! @param[out]     ppQueue     コンピュートキューの格納先です.
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
    //! @param[out]     ppResource      バッファの格納先です.
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
    //! @param[out]     ppResource      テクスチャの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateTexture(
        const TextureDesc*     pDesc,
        ITexture**             ppResource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      レンダーターゲットビューを生成します.
    //! 
    //! @param[in]      pTexture            テクスチャです.
    //! @param[in]      pDesc               構成設定です.
    //! @param[out]     ppRenderTargetView  レンダーターゲットビューの格納先です.
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
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateDepthStencilView(
        ITexture*               pTexture,
        const TargetViewDesc*   pDesc,
        IDepthStencilView**     ppDepthStencilView) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      定数バッファビューを生成します.
    //!
    //! @param[in]      pBuffer                 バッファです.
    //! @param[in]      pDesc                   構成設定です.
    //! @param[out]     ppConstantBufferView    バッファビューの格納先です.
    //! @retval true    生成に成功.
    //! @retval fasle   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateConstantBufferView(
        IBuffer*                        pBuffer,
        const ConstantBufferViewDesc*   pDesc,
        IConstantBufferView**           ppConstantBufferView) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      シェーダリソースビューを生成します.
    //! 
    //! @param[in]      pResource               リソースです
    //! @param[in]      pDesc                   構成設定です.
    //! @param[out]     ppShaderResourceView    シェーダリソースビューの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateShaderResourceView(
        IResource*                      pResource,
        const ShaderResourceViewDesc*   pDesc,
        IShaderResourceView**           ppShaderResourceView) override;

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
        const SamplerDesc*  pDesc,
        ISampler**          ppSampler) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      グラフィックスパイプラインを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppPipelineState パイプラインステートの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateGraphicsPipeline(
        const GraphicsPipelineStateDesc* pDesc,
        IPipelineState**                 ppPipelineState) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      コンピュートパイプラインを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppPipelineState パイプラインステートの格納先です.
    //! @reval true     生成に成功.
    //! @reval false    生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateComputePipeline(
        const ComputePipelineStateDesc* pDesc,
        IPipelineState**                ppPipelineState) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      メッシュパイプラインを生成します.
    //!
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppPipelineState パイプラインステートの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateMeshShaderPipeline(
        const MeshShaderPipelineStateDesc*  pDesc,
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
        const CommandSetDesc*   pDesc,
        ICommandSet**           ppCommandSet) override;

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
    //! @brief      インスタンスを取得します.
    //!
    //! @return     インスタンスを返却します.
    //---------------------------------------------------------------------------------------------
    VkInstance A3D_APIENTRY GetVulkanInstance() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイスを取得します.
    //!
    //! @return     デバイスを返却します.
    //---------------------------------------------------------------------------------------------
    VkDevice A3D_APIENTRY GetVulkanDevice() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      物理デバイス数を取得します.
    //!
    //! @return     物理デバイス数を返却します.
    //---------------------------------------------------------------------------------------------
    uint32_t A3D_APIENTRY GetVulkanPhysicalDeviceCount() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      物理デバイスを取得します.
    //!
    //! @param[in]      index       取得するインデックスです.
    //! @return     物理デバイスを返却します.
    //---------------------------------------------------------------------------------------------
    VkPhysicalDevice A3D_APIENTRY GetVulkanPhysicalDevice(uint32_t index) const;

    //---------------------------------------------------------------------------------------------
    //! @brief      物理デバイスメモリプロパティを取得します.
    //!
    //! @param[in]      index       取得するインデックスです.
    //! @return     物理デバイスメモリプロパティを返却します.
    //---------------------------------------------------------------------------------------------
    VkPhysicalDeviceMemoryProperties A3D_APIENTRY GetVulkanPhysicalDeviceMemoryProperties(uint32_t index) const;

    //---------------------------------------------------------------------------------------------
    //! @brief      物理デバイスプロパティを取得します.
    //!
    //! @param[in]      index       取得するインデックスです.
    //! @return     物理デバイスプロパティを返却します.
    //---------------------------------------------------------------------------------------------
    VkPhysicalDeviceProperties A3D_APIENTRY GetVulkanPhysicalDeviceProperties(uint32_t index) const;

    //---------------------------------------------------------------------------------------------
    //! @brief      ディスクリプタプールを生成します.
    //!
    //! @param[in]      maxSet      最大ディスクリプタセット生成数です.
    //! @param[out]     pPool       ディスクリプタプールの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CreateVulkanDescriptorPool(uint32_t maxSet, VkDescriptorPool* pPool);

    //---------------------------------------------------------------------------------------------
    //! @brief      拡張機能をサポートしているかどうか?
    //!
    //! @param[in]      value       拡張機能.
    //! @retval true    サポート.
    //! @retval false   非サポート.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY IsSupportExtension(EXTENSION value) const;

    //---------------------------------------------------------------------------------------------
    //! @brief      アロケータを取得します.
    //!
    //! @return     アロケータを返却します.
    //---------------------------------------------------------------------------------------------
    VmaAllocator GetAllocator() const;

private:
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // PhysicalDeviceInfo structure
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct PhysicalDeviceInfo
    {
        VkPhysicalDevice                    Device;             //!< 物理デバイスです.
        VkPhysicalDeviceMemoryProperties    MemoryProperty;     //!< 物理デバイスメモリプロパティです.
        VkPhysicalDeviceProperties          DeviceProperty;     //!< 物理デバイスプロパティです.
    };

    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>       m_RefCount;                     //!< 参照カウントです.
    VkInstance                  m_Instance;                     //!< インスタンスです.
    VkDevice                    m_Device;                       //!< デバイスです.
    DeviceDesc                  m_Desc;                         //!< 構成設定です.
    DeviceInfo                  m_Info;                         //!< デバイス情報です.
    uint32_t                    m_PhysicalDeviceCount;          //!< 物理デバイス数です.
    PhysicalDeviceInfo*         m_pPhysicalDeviceInfos;         //!< 物理デバイス情報です.
    VkDescriptorPoolCreateInfo  m_PoolCreateInfo;               //!< ディスクリプタプール生成情報です.
    Queue*                      m_pGraphicsQueue;               //!< グラフィックスキューです.
    Queue*                      m_pComputeQueue;                //!< コンピュートキューです.
    Queue*                      m_pCopyQueue;                   //!< コピーキューです.
    uint64_t                    m_TimeStampFrequency;           //!< GPUタイムスタンプの更新頻度(Hz単位)です.
    bool                        m_IsSupportExt[EXT_COUNT];      //!< 拡張機能.
    VmaAllocator                m_Allocator;                    //!< アロケータ.

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
