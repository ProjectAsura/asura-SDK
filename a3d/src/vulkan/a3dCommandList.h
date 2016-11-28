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
class FrameBuffer;


///////////////////////////////////////////////////////////////////////////////////////////////////
// CommandList class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API CommandList : ICommandList, BaseAllocator
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
    //! @param[in]      queueType       キュータイプです.
    //! @param[in]      listType        リストタイプです.
    //! @param[in]      pOption         プラットフォーム依存データです.
    //! @param[out]     ppCommandList   コマンドリストの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(
        IDevice*         pDevice,
        COMMANDLIST_TYPE listType,
        const void*      pOption,
        ICommandList**   ppCommandList);

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
    uint32_t A3D_APIENTRY GetCount() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイスを取得します.
    //!
    //! @param[out]     ppDevice        デバイスの格納先です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY GetDevice(IDevice** ppDevice);

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドリストの記録を開始します.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Begin() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      フレームバッファを設定します.
    //!
    //! @param[in]      pBuffer     フレームバッファです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetFrameBuffer(IFrameBuffer* pBuffer) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      フレームバッファをクリアします.
    //!
    //! @param[in]      clearColorCount     クリアカラー数です.
    //! @param[in]      pClearColors        クリアカラーの配列です.
    //! @param[in]      pClearDepthStencil  クリア深度ステンシルです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY ClearFrameBuffer(
        uint32_t                        clearColorCount,
        const ClearColorValue*          pClearColors,
        const ClearDepthStencilValue*   pClearDepthStencil) override;

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
    //! @brief      ディスクリプタセットレイアウトを設定します.
    //!
    //! @param[in]      pDescriptorSetLayout    ディスクリプタセットレイアウトです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetDescriptorSetLayout(IDescriptorSetLayout* pDescriptorSetLayout) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      ディスクリプタセットを設定します.
    //!
    //! @param[in]      pDescriptorSet      ディスクリプタセットです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetDescriptorSet(IDescriptorSet* pDescriptorSet) override;

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
    //! @brief      リソースバリアを設定します.
    //!
    //! @param[in]      pResource       リソースです.
    //! @param[in]      nextState       次の状態です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY TextureBarrier(
        ITexture*       pResource,
        RESOURCE_STATE  nextState) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      リソースバリアを設定します.
    //!
    //! @param[in]      pResource       リソースです.
    //! @param[in]      nextState       次の状態です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY BufferBarrier(
        IBuffer*        pResource,
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
    //! @brief      スレッド グループからコマンドリストを実行します
    //!
    //! @param[in]      x       x 方向にディスパッチしたグループの数
    //! @param[in]      y       y 方向にディスパッチしたグループの数
    //! @param[in]      z       z 方向にディスパッチしたグループの数
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Dispatch(uint32_t x, uint32_t y, uint32_t z) override;

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
        ITexture*   pDstResource,
        uint32_t    dstSubresource,
        Offset3D    dstOffset,
        ITexture*   pSrcResource,
        uint32_t    srcSubresource,
        Offset3D    srcOffset,
        Extent3D    srcExtent) override;

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
        ITexture*   pDstTexture,
        uint32_t    dstSubresource,
        Offset3D    dstOffset,
        IBuffer*    pSrcBuffer,
        uint64_t    srcOffset) override;

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
        IBuffer*    pDstBuffer,
        uint64_t    dstOffset,
        ITexture*   pSrcTexture,
        uint32_t    srcSubresource,
        Offset3D    srcOffset,
        Extent3D    srcExtent) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      マルチサンプリングされたリソースをマルチサンプリングされていないリソースにコピーします
    //!
    //! @param[in]      pDstResource        コピー先のリソースです
    //! @param[in]      dstSubresource      コピー先のサブリソースです.
    //! @param[in]      pSrcResource        コピー元のリソースです。マルチサンプリングされている必要があります.
    //! @param[in]      srcSubresource      コピー元のサブリソースです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY ResolveSubresource(
        ITexture*   pDstResource,
        uint32_t    dstSubresource,
        ITexture*   pSrcResource,
        uint32_t    srcSubresource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      バンドルを実行します.
    //!
    //! @param[in]      pCommandList        実行するバンドルです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY ExecuteBundle(ICommandList* pCommandList) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドリストの記録を終了します.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY End() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドを実行し，完了を待機します.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Flush();

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドプールを取得します.
    //!
    //! @return     コマンドプールを返却します.
    //---------------------------------------------------------------------------------------------
    VkCommandPool A3D_APIENTRY GetVulkanCommandPool() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドバッファを取得します.
    //!
    //! @return     コマンドバッファを返却します.
    //---------------------------------------------------------------------------------------------
    VkCommandBuffer A3D_APIENTRY GetVulkanCommandBuffer() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>       m_RefCount;             //!< 参照カウントです.
    IDevice*                    m_pDevice;              //!< デバイスです.
    VkCommandPool               m_CommandPool;          //!< コマンドプールです.
    VkCommandBuffer             m_CommandBuffer;        //!< コマンドバッファです.
    FrameBuffer*                m_pFrameBuffer;         //!< バインドされているフレームバッファです.

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
    //! @param[in]      pOption     プラットフォーム依存データです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(IDevice* pDevice, COMMANDLIST_TYPE listType, const void* pOption);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    CommandList     (const CommandList&) = delete;
    void operator = (const CommandList&) = delete;
};


} // namepsace a3d
