﻿//-------------------------------------------------------------------------------------------------
// File : a3dCommandList.h
// Desc : CommandList Implmentation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

//-------------------------------------------------------------------------------------------------
// Forward Declarations.
//-------------------------------------------------------------------------------------------------
class DescriptorSetLayout;


///////////////////////////////////////////////////////////////////////////////////////////////////
// CommandList class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API CommandList : public ICommandList, public BaseAllocator
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
    //! @param[out]     ppCommandList   コマンドリストの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(
        IDevice*                pDevice,
        const CommandListDesc*  pDesc,
        ICommandList**          ppCommandList);

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
    //! @brief      コマンドリストの記録を開始します.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Begin() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      フレームバッファを設定します.
    //!
    //! @param[in]      pBuffer     フレームバッファです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY BeginFrameBuffer(
        uint32_t                        renderTargetViewCount,
        IRenderTargetView**             pRenderTargetViews,
        IDepthStencilView*              pDepthStencilView,
        uint32_t                        clearColorCount,
        const ClearColorValue*          pClearColors,
        const ClearDepthStencilValue*   pClearDepthStencil) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      フレームバッファを解除します.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY EndFrameBuffer() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      高速化機構を構築します.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY BuildAccelerationStructure(IAccelerationStructure* pAS) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      ブレンド定数を設定します.
    //!
    //! @param[in]      blendConstant       ブレンド定数です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetBlendConstant(const float blendConstant[4]) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      ステンシル参照値を設定します.
    //!
    //! @param[in]      stencilRef          ステンシル参照値です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetStencilReference(uint32_t stencilRef) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      ビューポートを設定します.
    //!
    //! @param[in]      count       ビューポートの数です.
    //! @param[in]      pViewports  ビューポートの配列です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetViewports(uint32_t count, Viewport* pViewports) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      シザー矩形を設定します.
    //!
    //! @param[in]      count       シザー矩形の数です.
    //! @param[in]      pScissors   シザー矩形の配列です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetScissors(uint32_t count, Rect* pScissors) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      パイプラインステートを設定します.
    //!
    //! @param[in]      pPipelineState      パイプラインステートです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetPipelineState(IPipelineState* pPipelineState) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      頂点バッファを設定します.
    //!
    //! @param[in]      startSlot       開始スロット番号です.
    //! @param[in]      count           頂点バッファ数です.
    //! @param[in]      ppResoruces     頂点バッファの配列です.
    //! @param[in]      pOffsets        オフセットの配列です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetVertexBuffers(
        uint32_t    startSlot,
        uint32_t    count,
        IBuffer**   ppResources,
        uint64_t*   pOffsets) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      インデックスバッファを設定します.
    //!
    //! @param[in]      pResource       設定するインデックスバッファです.
    //! @param[in]      offset          インデックスオフセットです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetIndexBuffer(
        IBuffer*    pResource,
        uint64_t    offset) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      定数バッファビューを設定します.
    //!
    //! @param[in]      index       レイアウト番号です.
    //! @param[in]      pResource   設定するリソースです.
    //! @note       設定したバッファは ICommandList::SetDescriptorSet() 呼び出し時に反映されます.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetView(uint32_t index, IConstantBufferView* const pResource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      シェーダリソースビューを設定します.
    //!
    //! @param[in]      index       レイアウト番号です.
    //! @param[in]      pResource   設定するリソースです.
    //! @note       設定したテクスチャは ICommandList::SetDescriptorSet() 呼び出し時に反映されます.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetView(uint32_t index, IShaderResourceView* const pResource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      アンオーダードアクセスビューを設定します.
    //!
    //! @param[in]      index       レイアウト番号です.
    //! @param[in]      pResource   設定するリソースです.
    //! @note       設定したストレージは ICommandList::SetDescriptorSet() 呼び出し時に反映されます.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetView(uint32_t index, IUnorderedAccessView* const pResource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      サンプラーを設定します.
    //!
    //! @param[in]      index       レイアウト番号です.
    //! @param[in]      pSampler    設定するサンプラーです.
    //! @note       設定したサンプラーは ICommandList::SetDescriptorSet() 呼び出し時に反映されます.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetSampler(uint32_t index, ISampler* const pSampler) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      リソースバリアを設定します.
    //!
    //! @param[in]      pResource       リソースです.
    //! @param[in]      prevState       変更前の状態です.
    //! @param[in]      nextState       変更後の状態です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY TextureBarrier(
        ITexture*       pResource,
        RESOURCE_STATE  prevState,
        RESOURCE_STATE  nextState) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      リソースバリアを設定します.
    //!
    //! @param[in]      pResource       リソースです.
    //! @param[in]      prevState       変更前の状態です.
    //! @param[in]      nextState       変更後の状態です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY BufferBarrier(
        IBuffer*        pResource,
        RESOURCE_STATE  prevState,
        RESOURCE_STATE  nextState) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      インスタンス描画します.
    //!
    //! @param[in]      vertexCount     頂点数です.
    //! @param[in]      instanceCount   描画するインスタンス数です.
    //! @param[in]      firstVertex     最初に描画する頂点へのオフセットです.
    //! @param[in]      firstInstance   最初に描画するインスタンスへのオフセットです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY DrawInstanced(
        uint32_t vertexCount,
        uint32_t instanceCount, 
        uint32_t firstVertex, 
        uint32_t firstInstance) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      インデックスバッファを用いてインスタンス描画します.
    //!
    //! @param[in]      indexCount      インデックス数です.
    //! @param[in]      instanceCount   描画するインスタンス数です.
    //! @param[in]      firstIndex      最初に描画するインデックスへのオフセットです.
    //! @param[in]      vertexOffset    最初に描画する頂点へのオフセットです.
    //! @param[in]      firstInstance   最初に描画するインスタンスへのオフセットです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY DrawIndexedInstanced(
        uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t fistIndex,
        int      vertexOffset,
        uint32_t firstInstance) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      コンピュートシェーダを起動します.
    //!
    //! @param[in]      x       x 方向にディスパッチしたグループの数
    //! @param[in]      y       y 方向にディスパッチしたグループの数
    //! @param[in]      z       z 方向にディスパッチしたグループの数
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY DispatchCompute(uint32_t x, uint32_t y, uint32_t z) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      メッシュシェーダを起動します.
    //!
    //! @param[in]      x       x 方向にディスパッチしたグループの数
    //! @param[in]      y       y 方向にディスパッチしたグループの数
    //! @param[in]      z       z 方向にディスパッチしたグループの数
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY DispatchMesh(uint32_t x, uint32_t y, uint32_t z) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      レイトレーシングパイプラインを起動します.
    //! 
    //! @param[in]      pArgs       起動引数.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY TraceRays(const TraceRayArguments* pArgs) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      インダイレクトコマンドを実行します.
    //!
    //! @param[in]      pCommandSet             インダイレクトコマンドセットです.
    //! @param[in]      maxCommandCount         コマンドの最大実行回数です.
    //! @param[in]      pArgumentBuffers        引数バッファです.
    //! @param[in]      argumentBufferOffset    引数バッファのオフセットです.
    //! @param[in]      pCounterBuffer          カウンターバッファです.
    //! @param[in]      counterBufferOffset     カウンタバッファのオフセットです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY ExecuteIndirect(
        ICommandSet* pCommandSet,
        uint32_t     maxCommandCount,
        IBuffer*     pArgumentBuffer,
        uint64_t     argumentBufferOffset,
        IBuffer*     pCounterBuffer,
        uint64_t     counterBufferOffset) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      クエリを開始します.
    //!
    //! @param[in]      pQuery      設定するクエリプールです.
    //! @param[in]      index       クエリ番号です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY BeginQuery(IQueryPool* pQuery, uint32_t index) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      クエリを終了します.
    //!
    //! @param[in]      pQuery      設定するクエリプールです.
    //! @param[in]      index       クエリ番号です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY EndQuery(IQueryPool* pQuery, uint32_t index) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      クエリを解決します.
    //!
    //! @param[in]      pQuery          解決するクエリプールです
    //! @param[in]      startIndex      解決するクエリのオフセットです.
    //! @param[in]      queryCount      解決するクエリ数です.
    //! @param[in]      pDstBuffer      書き込み先のバッファです.
    //! @param[in]      dstOffset       書き込み先のバッファのオフセットです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY ResolveQuery(
        IQueryPool* pQuery,
        uint32_t    startIndex,
        uint32_t    queryCount,
        IBuffer*    pDstBuffer,
        uint64_t    dstOffset) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      クエリをリセットします.
    //!
    //! @param[in]      pQuery          リセットするクエリプールです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY ResetQuery(IQueryPool* pQuery) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      テクスチャをコピーします.
    //!
    //! @param[in]      pDstResource        コピー先のリソースです.
    //! @param[in]      pSrcResource        コピー元のリソースです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY CopyTexture(ITexture* pDstResource, ITexture* pSrcResource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      バッファをコピーします.
    //!
    //! @param[in]      pDstResource        コピー先のリソースです.
    //! @param[in]      pSrcResource        コピー元のリソースです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY CopyBuffer(IBuffer* pDstResource, IBuffer* pSrcResource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      領域を指定してテクスチャをコピーします.
    //!
    //! @param[in]      pDstResource        コピー先のテクスチャです.
    //! @param[in]      dstSubresource      コピー先のサブリソースです.
    //! @param[in]      dstOffset           コピー先の領域です.
    //! @param[in]      pSrcResource        コピー元のテクスチャです.
    //! @param[in]      srcSubresource      コピー元のサブリソースです.
    //! @param[in]      srcOffset           コピー元の領域です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY CopyTextureRegion(
        ITexture*       pDstResource,
        uint32_t        dstSubresource,
        Offset3D        dstOffset,
        ITexture*       pSrcResource,
        uint32_t        srcSubresource,
        Offset3D        srcOffset,
        Extent3D        srcExtent) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      領域を指定してバッファをコピーします.
    //!
    //! @param[in]      pDstBuffer      コピー先のバッファです.
    //! @param[in]      dstOffset       コピー先のバッファオフセットです
    //! @param[in]      pSrcBuffer      コピー元のバッファです.
    //! @param[in]      srcOffset       コピー元のバッファオフセットです.
    //! @param[in]      byteCount       コピーするバイト数です.
    //--------------------------------------------------------------------------------------------
    void A3D_APIENTRY CopyBufferRegion(
        IBuffer*    pDstBuffer,
        uint64_t    dstOffset,
        IBuffer*    pSrcBuffer,
        uint64_t    srcOffset,
        uint64_t    byteCount ) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      領域を指定してバッファからテクスチャにコピーします.
    //!
    //! @param[in]      pDstTexture         コピー先のテクスチャです.
    //! @param[in]      dstSubresource      コピー先のサブリソースです.
    //! @param[in]      dstOffset           コピー先のオフセットです
    //! @param[in]      pSrcBuffer          コピー元のバッファです.
    //! @param[in]      srcOffset           コピー元のオフセットです
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY CopyBufferToTexture(
        ITexture*       pDstTexture,
        uint32_t        dstSubresource,
        Offset3D        dstOffset,
        IBuffer*        pSrcBuffer,
        uint64_t        srcOffset) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      領域を指定してテクスチャからバッファにコピーします.
    //!
    //! @param[in]      pDstBuffer          コピー先のバッファです
    //! @param[in]      dstOffset           コピー先のオフセットです
    //! @param[in]      pSrcTexture         コピー元のテクスチャです
    //! @param[in]      srcSubresource      コピー元のサブリソースです
    //! @param[in]      srcOffset           コピー元のオフセットです
    //! @param[in]      srcExtent           コピーする大きさです
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY CopyTextureToBuffer(
        IBuffer*        pDstBuffer,
        uint64_t        dstOffset,
        ITexture*       pSrcTexture,
        uint32_t        srcSubresource,
        Offset3D        srcOffset,
        Extent3D        srcExtent) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      高速化機構をコピーします.
    //! 
    //! @param[in]      pDstAS      コピー先の高速化機構.
    //! @param[in]      pSrcAS      コピー元の高速化機構.
    //! @param[in]      mode        コピーモード.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY CopyAccelerationStructure(
        IAccelerationStructure*             pDstAS,
        IAccelerationStructure*             pSrcAS,
        ACCELERATION_STRUCTURE_COPY_MODE    mode) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      マルチサンプリングされたリソースをマルチサンプリングされていないリソースにコピーします
    //!
    //! @param[in]      pDstResource        コピー先のリソースです
    //! @param[in]      dstSubresource      コピー先のサブリソースです.
    //! @param[in]      pSrcResource        コピー元のリソースです。マルチサンプリングされている必要があります.
    //! @param[in]      srcSubresource      コピー元のサブリソースです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY ResolveSubresource(
        ITexture*       pDstResource,
        uint32_t        dstSubresource,
        ITexture*       pSrcResource,
        uint32_t        srcSubresource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      バンドルを実行します.
    //!
    //! @param[in]      pCommandList        実行するバンドルです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY ExecuteBundle(ICommandList* pCommandList) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバッグマーカーをプッシュします.
    //!
    //! @param[in]          tag         タグです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY PushMarker(const char* tag) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバッグマーカーをポップします.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY PopMarker() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドリストの記録を終了します.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY End() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      定数バッファを更新します.
    //!
    //! @param[in]      pBuffer     更新するバッファです.
    //! @param[in]      offset      バッファのオフセットです(バイト単位).
    //! @param[in]      size        書き込みサイズです(バイト単位).
    //! @param[in]      pData       書き込みデータです.
    //! @retval true    更新に成功.
    //! @retval false   更新に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY UpdateConstantBuffer(
        IBuffer*    pBuffer,
        size_t      offset,
        size_t      size, 
        const void* pData);

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドを実行し，完了を待機します.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Flush();

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドプールを取得します.
    //!
    //! @return     コマンドプールを返却します.
    //---------------------------------------------------------------------------------------------
    VkCommandPool A3D_APIENTRY GetVkCommandPool() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドバッファを取得します.
    //!
    //! @return     コマンドバッファを返却します.
    //---------------------------------------------------------------------------------------------
    VkCommandBuffer A3D_APIENTRY GetVkCommandBuffer() const;

private:
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // DescriptorInfo union
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct DescriptorInfo
    {
        union
        {
            VkDescriptorImageInfo   Image;          //!< イメージ情報です.
            VkDescriptorBufferInfo  Buffer;         //!< バッファ情報です.
            VkBufferView            BufferView;     //!< バッファビューです.
        };
    };

    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>       m_RefCount;                     //!< 参照カウントです.
    Device*                     m_pDevice;                      //!< デバイスです.
    VkCommandPool               m_CommandPool;                  //!< コマンドプールです.
    VkCommandBuffer             m_CommandBuffer;                //!< コマンドバッファです.
    bool                        m_BindRenderPass;               //!< レンダーパスバインド中か?
    bool                        m_DirtyDescriptor;              //!< ディスクリプタダーティフラグ.
    DescriptorSetLayout*        m_pDescriptorSetLayout;         //!< ディスクリプタセットレイアウトです.
    DescriptorInfo              m_DescriptorInfo    [MAX_DESCRIPTOR_COUNT] = {};  //!< ディスクリプタ情報です.
    VkWriteDescriptorSet        m_WriteDescriptorSet[MAX_DESCRIPTOR_COUNT] = {};  //!< 書き込みディスクリプタセットです.
    ObjectName                  m_Name;                                           //!< デバッグ名です.

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY CommandList();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY ~CommandList();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      queueType   キュータイプです.
    //! @param[in]      listType    リストタイプです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(IDevice* pDevice, COMMANDLIST_TYPE listType);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    //---------------------------------------------------------------------------------------------
    //! @brief      ディスクリプタの更新を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY UpdateDescriptor();

    CommandList     (const CommandList&) = delete;
    void operator = (const CommandList&) = delete;
};


} // namepsace a3d
