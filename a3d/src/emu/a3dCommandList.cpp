//-------------------------------------------------------------------------------------------------
// File : a3dCommandList.cpp
// Desc : Command Buffer Emulation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// CommandList class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
CommandList::CommandList()
: m_RefCount(1)
, m_pDevice (nullptr)
, m_Type    (COMMANDLIST_TYPE_DIRECT)
, m_Buffer  ()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
CommandList::~CommandList()
{ SafeRelease(m_pDevice); }

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void CommandList::AddRef() 
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void CommandList::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t CommandList::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void CommandList::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      レンダーターゲットビューをクリアします.
//-------------------------------------------------------------------------------------------------
void CommandList::ClearRenderTargetView
(
    IRenderTargetView*      pRenderTargetView,
    const ClearColorValue&  clearValue
)
{
    if (pRenderTargetView == nullptr)
    { return; }

    ImCmdClearRenderTargetView cmd = {};
    cmd.Type                = CMD_CLEAR_RENDER_TARGET_VIEW;
    cmd.pRenderTargetView   = pRenderTargetView;
    cmd.ClearColor[0]       = clearValue.R;
    cmd.ClearColor[1]       = clearValue.G;
    cmd.ClearColor[2]       = clearValue.B;
    cmd.ClearColor[3]       = clearValue.A;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      深度ステンシルビューをクリアします.
//-------------------------------------------------------------------------------------------------
void CommandList::ClearDepthStencilView
(
    IDepthStencilView*              pDepthStencilView,
    const ClearDepthStencilValue&   clearValue
)
{
    if (pDepthStencilView == nullptr)
    { return; }

    ImCmdClearDepthStencilView cmd = {};
    cmd.Type                = CMD_CLEAR_DEPTH_STENCIL_VIEW;
    cmd.pDepthStencilView   = pDepthStencilView;
    cmd.EnableClearDepth    = clearValue.EnableClearDepth;
    cmd.EnableClearStencil  = clearValue.EnableClearStencil;
    cmd.ClearDepth          = clearValue.Depth;
    cmd.ClearStencil        = clearValue.Stencil;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      コマンドの記録を開始します.
//-------------------------------------------------------------------------------------------------
void CommandList::Begin()
{
    m_Buffer.Reset();

    ImCmdBegin cmd = {};
    cmd.Type = (m_Type == COMMANDLIST_TYPE_DIRECT) ? CMD_BEGIN : CMD_SUB_BEGIN;
    
    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      フレームバッファを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::BeginFrameBuffer
(
    uint32_t            renderTargetViewCount,
    IRenderTargetView** pRenderTargetViews,
    IDepthStencilView*  pDepthStencilView
)
{
    if (pRenderTargetViews == nullptr && pDepthStencilView == nullptr)
    { return; }

    ImCmdBeginFrameBuffer cmd = {};
    cmd.Type         = CMD_BEGIN_FRAME_BUFFER;
    cmd.RenderTargetViewCount = REQUEST_OPLOCK_CURRENT_VERSION;

    auto maxCount = (renderTargetViewCount >= 8) ? 8 : renderTargetViewCount;
    for(auto i=0u; i<maxCount; ++i)
    { cmd.pRenderTargetView[i] = pRenderTargetViews[i]; }
    cmd.pDepthStencilView = pDepthStencilView;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      フレームバッファを解除します.
//-------------------------------------------------------------------------------------------------
void CommandList::EndFrameBuffer()
{
    ImCmdBase cmd = {};
    cmd.Type = CMD_END_FRAME_BUFFER;
    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      ブレンド定数を設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetBlendConstant(const float blendConstant[4])
{
    ImCmdSetBlendConstant cmd = {};
    cmd.Type = CMD_SET_BLEND_CONSTANT;
    memcpy( cmd.BlendConstant, blendConstant, sizeof(cmd.BlendConstant) );

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      ステンシル参照値を設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetStencilReference(uint32_t stencilRef)
{
    ImCmdSetStencilReference cmd = {};
    cmd.Type = CMD_SET_STENCIL_REFERENCE;
    cmd.StencilReference = stencilRef;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      ビューポートを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetViewports(uint32_t count, Viewport* pViewports)
{
    if (count == 0 || pViewports == nullptr)
    { return; }

    ImCmdSetViewports cmd = {};
    cmd.Type  = CMD_SET_VIEWPORTS;
    cmd.Count = count;
    memcpy(cmd.Viewports, pViewports, sizeof(Viewport) * count);

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      シザー矩形を設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetScissors(uint32_t count, Rect* pScissors)
{
    if (count == 0 || pScissors == nullptr)
    { return; }

    ImCmdSetScissors cmd = {};
    cmd.Type  = CMD_SET_SCISSORS;
    cmd.Count = count;
    memcpy(cmd.Rects, pScissors, sizeof(Rect) * count);

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      パイプラインステートを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetPipelineState(IPipelineState* pPipelineState)
{
    ImCmdSetPipelineState cmd = {};
    cmd.Type            = CMD_SET_PIPELINESTATE;
    cmd.pPipelineState  = pPipelineState;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタセットを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetDescriptorSet(IDescriptorSet* pDescriptorSet)
{
    if (pDescriptorSet == nullptr)
    { return; }

    auto pWrapDescriptorSet = static_cast<DescriptorSet*>(pDescriptorSet);
    ImCmdSetDescriptorSet cmd = {};
    pWrapDescriptorSet->MakeCommand(&cmd);

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      頂点バッファを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetVertexBuffers
(
    uint32_t    startSlot,
    uint32_t    count,
    IBuffer**   ppResources,
    uint64_t*   pOffsets
)
{
    if (ppResources == nullptr)
    { return; }

    ImCmdSetVertexBuffers cmd = {};
    cmd.Type        = CMD_SET_VERTEX_BUFFERS;
    cmd.StartSlot   = startSlot;
    cmd.Count       = count;
    memcpy(cmd.pBuffers, ppResources, sizeof(IBuffer) * count);
    cmd.HasOffset   = (pOffsets != nullptr);
    if (pOffsets != nullptr)
    { memcpy(cmd.Offsets, pOffsets, sizeof(uint64_t) * count); }

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      インデックスバッファを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetIndexBuffer(IBuffer* pResource, uint64_t offset)
{
    ImCmdSetIndexBuffer cmd = {};
    cmd.Type    = CMD_SET_INDEX_BUFFER;
    cmd.pBuffer = pResource;
    cmd.Offset  = offset;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      テクスチャバリアを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::TextureBarrier
(
    ITexture*       pResource,
    RESOURCE_STATE  prevState,
    RESOURCE_STATE  nextState
)
{
    if (prevState == nextState)
    { return; }

    ImCmdTextureBarrier cmd = {};
    cmd.Type        = CMD_TEXTURE_BARRIER;
    cmd.pResource   = pResource;
    cmd.PrevState   = prevState;
    cmd.NextState   = nextState;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      バッファバリアを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::BufferBarrier
(
    IBuffer*        pResource,
    RESOURCE_STATE  prevState,
    RESOURCE_STATE  nextState
)
{
    if (prevState == nextState)
    { return; }

    ImCmdBufferBarrier cmd = {};
    cmd.Type        = CMD_BUFFER_BARRIER;
    cmd.pResource   = pResource;
    cmd.PrevState   = prevState;
    cmd.NextState   = nextState;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      インスタンス描画します.
//-------------------------------------------------------------------------------------------------
void CommandList::DrawInstanced
(
    uint32_t    vertexCount,
    uint32_t    instanceCount,
    uint32_t    firstVertex,
    uint32_t    firstInstance
)
{
    ImCmdDrawInstanced cmd = {};
    cmd.Type            = CMD_DRAW_INSTANCED;
    cmd.VertexCount     = vertexCount;
    cmd.InstanceCount   = instanceCount;
    cmd.FirstVertex     = firstVertex;
    cmd.FirstInstance   = firstInstance;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      インデックス付きでインスタンス描画します.
//-------------------------------------------------------------------------------------------------
void CommandList::DrawIndexedInstanced
(
    uint32_t    indexCount,
    uint32_t    instanceCount,
    uint32_t    firstIndex,
    int         vertexOffset,
    uint32_t    firstInstance
)
{
    ImCmdDrawIndexedInstanced cmd = {};
    cmd.Type            = CMD_DRAW_INDEXED_INSTANCED;
    cmd.IndexCount      = indexCount;
    cmd.InstanceCount   = instanceCount;
    cmd.FirstIndex      = firstIndex;
    cmd.VertexOffset    = vertexOffset;
    cmd.FirstInstance   = firstInstance;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      コンピュートシェーダを起動します.
//-------------------------------------------------------------------------------------------------
void CommandList::DispatchCompute(uint32_t x, uint32_t y, uint32_t z)
{
    ImCmdDispatch cmd = {};
    cmd.Type    = CMD_DISPATCH_COMPUTE;
    cmd.X       = x;
    cmd.Y       = y;
    cmd.Z       = z;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      メッシュシェーダを起動します.
//-------------------------------------------------------------------------------------------------
void CommandList::DispatchMesh(uint32_t x, uint32_t y, uint32_t z)
{
    ImCmdDispatch cmd = {};
    cmd.Type = CMD_DISPATCH_MESH;
    cmd.X = x;
    cmd.Y = y;
    cmd.Z = z;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      インダイレクトコマンドを実行します.
//-------------------------------------------------------------------------------------------------
void CommandList::ExecuteIndirect
(
    ICommandSet*    pCommandSet,
    uint32_t        maxCommandCount,
    IBuffer*        pArgumentBuffer,
    uint64_t        argumentBufferOffset,
    IBuffer*        pCounterBuffer,
    uint64_t        counterBufferOffset
)
{
    if (pCommandSet == nullptr || pArgumentBuffer == nullptr)
    { return; }

    ImCmdExecuteIndirect cmd = {};
    cmd.Type                    = CMD_EXECUTE_INDIRECT;
    cmd.pCommandSet             = pCommandSet;
    cmd.MaxCommandCount         = maxCommandCount;
    cmd.pArgumentBuffer         = pArgumentBuffer;
    cmd.ArgumentBufferOffset    = argumentBufferOffset;
    cmd.pCounterBuffer          = pCounterBuffer;
    cmd.CounterBufferOffset     = counterBufferOffset;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      クエリを開始します.
//-------------------------------------------------------------------------------------------------
void CommandList::BeginQuery(IQueryPool* pQuery, uint32_t index)
{
    if (pQuery == nullptr)
    { return; }

    ImCmdBeginQuery cmd = {};
    cmd.Type    = CMD_BEGIN_QUERY;
    cmd.pQuery  = pQuery;
    cmd.Index   = index;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      クエリを終了します.
//-------------------------------------------------------------------------------------------------
void CommandList::EndQuery(IQueryPool* pQuery, uint32_t index)
{
    if (pQuery == nullptr)
    { return; }

    ImCmdEndQuery cmd = {};
    cmd.Type    = CMD_END_QUERY;
    cmd.pQuery  = pQuery;
    cmd.Index   = index;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      クエリを解決します.
//-------------------------------------------------------------------------------------------------
void CommandList::ResolveQuery
(
    IQueryPool* pQuery,
    uint32_t    startIndex,
    uint32_t    queryCount,
    IBuffer*    pDstBuffer,
    uint64_t    dstOffset
)
{
    if (pQuery == nullptr)
    { return; }

    ImCmdResolveQuery cmd = {};
    cmd.Type        = CMD_RESOLVE_QUERY;
    cmd.pQuery      = pQuery;
    cmd.StartIndex  = startIndex;
    cmd.QueryCount  = queryCount;
    cmd.pDstBuffer  = pDstBuffer;
    cmd.DstOffset   = dstOffset;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      クエリをリセットします.
//-------------------------------------------------------------------------------------------------
void CommandList::ResetQuery(IQueryPool* pQuery)
{
    if (pQuery == nullptr)
    { return; }

    ImCmdResetQuery cmd = {};
    cmd.Type    = CMD_RESET_QUERY;
    cmd.pQuery  = pQuery;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      テクスチャをコピーします.
//-------------------------------------------------------------------------------------------------
void CommandList::CopyTexture
(
    ITexture*       pDstResource,
    ITexture*       pSrcResource
)
{
    if (pDstResource == nullptr || pSrcResource == nullptr)
    { return; }

    ImCmdCopyTexture cmd = {};
    cmd.Type        = CMD_COPY_TEXTURE;
    cmd.pDstTexture = pDstResource;
    cmd.pSrcTexture = pSrcResource;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      バッファをコピーします.
//-------------------------------------------------------------------------------------------------
void CommandList::CopyBuffer(IBuffer* pDstResource, IBuffer* pSrcResource)
{
    if (pDstResource == nullptr || pSrcResource == nullptr)
    { return; }

    ImCmdCopyBuffer cmd = {};
    cmd.Type       = CMD_COPY_BUFFER;
    cmd.pDstBuffer = pDstResource;
    cmd.pSrcBuffer = pSrcResource;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      領域を指定してテクスチャをコピーします.
//-------------------------------------------------------------------------------------------------
void CommandList::CopyTextureRegion
(
    ITexture*       pDstResource,
    uint32_t        dstSubresource,
    Offset3D        dstOffset,
    ITexture*       pSrcResource,
    uint32_t        srcSubresource,
    Offset3D        srcOffset,
    Extent3D        srcExtent
)
{
    if (pDstResource == nullptr || pSrcResource == nullptr)
    { return; }

    ImCmdCopyTextureRegion cmd = {};
    cmd.Type            = CMD_COPY_TEXTURE_REGION;
    cmd.pDstResource    = pDstResource;
    cmd.DstSubresource  = dstSubresource;
    cmd.DstOffset       = dstOffset;
    cmd.pSrcResource    = pSrcResource;
    cmd.SrcSubresource  = srcSubresource;
    cmd.SrcOffset       = srcOffset;
    cmd.SrcExtent       = srcExtent;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      領域を指定してバッファをコピーします.
//-------------------------------------------------------------------------------------------------
void CommandList::CopyBufferRegion
(
    IBuffer*    pDstBuffer,
    uint64_t    dstOffset,
    IBuffer*    pSrcBuffer,
    uint64_t    srcOffset,
    uint64_t    byteCount
)
{
    if (pDstBuffer == nullptr || pSrcBuffer == nullptr)
    { return; }

    ImCmdCopyBufferRegion cmd = {};
    cmd.Type        = CMD_COPY_BUFFER_REGION;
    cmd.pDstBuffer  = pDstBuffer;
    cmd.DstOffset   = dstOffset;
    cmd.pSrcBuffer  = pSrcBuffer;
    cmd.SrcOffset   = srcOffset;
    cmd.ByteCount   = byteCount;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      バッファからテクスチャにコピーします.
//-------------------------------------------------------------------------------------------------
void CommandList::CopyBufferToTexture
(
    ITexture*       pDstTexture,
    uint32_t        dstSubresource,
    Offset3D        dstOffset,
    IBuffer*        pSrcBuffer,
    uint64_t        srcOffset
)
{
    if (pDstTexture == nullptr || pSrcBuffer == nullptr)
    { return; }

    ImCmdCopyBufferToTexture cmd = {};
    cmd.Type            = CMD_COPY_BUFFER_TO_TEXTURE;
    cmd.pDstTexture     = pDstTexture;
    cmd.DstSubresource  = dstSubresource;
    cmd.DstOffset       = dstOffset;
    cmd.pSrcBuffer      = pSrcBuffer;
    cmd.SrcOffset       = srcOffset;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      テクスチャからバッファにコピーします.
//-------------------------------------------------------------------------------------------------
void CommandList::CopyTextureToBuffer
(
    IBuffer*        pDstBuffer,
    uint64_t        dstOffset,
    ITexture*       pSrcTexture,
    uint32_t        srcSubresource,
    Offset3D        srcOffset,
    Extent3D        srcExtent
)
{
    if (pDstBuffer == nullptr || pSrcTexture == nullptr)
    { return; }

    ImCmdCopyTextureToBuffer cmd = {};
    cmd.Type            = CMD_COPY_TEXTURE_TO_BUFFER;
    cmd.pDstBuffer      = pDstBuffer;
    cmd.DstOffset       = dstOffset;
    cmd.pSrcTexture     = pSrcTexture;
    cmd.SrcSubresource  = srcSubresource;
    cmd.SrcOffset       = srcOffset;
    cmd.SrcExtent       = srcExtent;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      サブリソースを解決します.
//-------------------------------------------------------------------------------------------------
void CommandList::ResolveSubresource
(
    ITexture*       pDstResource,
    uint32_t        dstSubresource,
    ITexture*       pSrcResource,
    uint32_t        srcSubresource
)
{
    if (pDstResource == nullptr || pSrcResource == nullptr)
    { return; }

    ImCmdResolveSubresource cmd = {};
    cmd.Type            = CMD_RESOLVE_SUBRESOURCE;
    cmd.pDstResource    = pDstResource;
    cmd.DstSubresource  = dstSubresource;
    cmd.pSrcResource    = pSrcResource;
    cmd.SrcSubresource  = srcSubresource;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      バンドルを実行します.
//-------------------------------------------------------------------------------------------------
void CommandList::ExecuteBundle(ICommandList* pCommandList)
{
    if (pCommandList == nullptr)
    { return; }

    auto pWrapCommandList = reinterpret_cast<CommandList*>(pCommandList);
    A3D_ASSERT(pWrapCommandList != nullptr);

    m_Buffer.Append(pWrapCommandList->GetCommandBuffer());
}

//-------------------------------------------------------------------------------------------------
//      デバッグマーカーをプッシュします.
//-------------------------------------------------------------------------------------------------
void CommandList::PushMarker(const char* tag)
{
    if (tag == nullptr)
    { return; }

    ImCmdPushMarker cmd = {};
    cmd.Type = CMD_PUSH_MARKER;
    strcpy_s( cmd.Tag, tag );

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      デバッグマーカーをポップします.
//-------------------------------------------------------------------------------------------------
void CommandList::PopMarker()
{
    ImCmdPopMarker cmd = {};
    cmd.Type = CMD_POP_MARKER;

    m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      定数バッファを更新します.
//-------------------------------------------------------------------------------------------------
bool CommandList::UpdateConstantBuffer(IBuffer* pBuffer, size_t offset, size_t size, const void* pData)
{
    if (pBuffer == nullptr || size == 0 || pData == nullptr)
    { return false; }

    ImCmdUpdateConstantBuffer cmd = {};
    cmd.Type    = CMD_UPDATE_CONSTANT_BUFFER;
    cmd.pBuffer = pBuffer;
    cmd.Offset  = offset;
    cmd.Size    = size;

    m_Buffer.Push(&cmd, sizeof(cmd));
    m_Buffer.Push(pData, size);
    return true;
}

//-------------------------------------------------------------------------------------------------
//      コマンドの記録を終了します.
//-------------------------------------------------------------------------------------------------
void CommandList::End()
{
    ImCmdEnd cmd = {};
    cmd.Type = (m_Type == COMMANDLIST_TYPE_DIRECT) ? CMD_END : CMD_SUB_END;

    m_Buffer.Push(&cmd, sizeof(cmd));
    m_Buffer.Close();
}

//-------------------------------------------------------------------------------------------------
//      コマンドバッファを取得します.
//-------------------------------------------------------------------------------------------------
const CommandBuffer* CommandList::GetCommandBuffer() const
{ return &m_Buffer; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool CommandList::Create
(
    IDevice*                pDevice,
    const CommandListDesc*  pDesc,
    ICommandList**          ppCommandList
)
{
    if (pDevice == nullptr || pDesc == nullptr || ppCommandList == nullptr)
    { return false; }

    if (pDesc->BufferSize == 0)
    { return false; }

    auto instance = new CommandList();
    if ( instance == nullptr )
    { return false; }

    instance->m_pDevice = pDevice;
    instance->m_pDevice->AddRef();
    instance->m_Type = pDesc->Type;
    
    if (!instance->m_Buffer.Init(pDesc->BufferSize))
    {
        SafeRelease(instance);
        return false;
    }

    *ppCommandList = instance;
    return true;
}

} // namespace a3d
