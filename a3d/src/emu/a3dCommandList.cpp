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
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_Type        (COMMANDLIST_TYPE_DIRECT)
, m_Buffer      ()
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
//      定数バッファを更新します.
//-------------------------------------------------------------------------------------------------
bool CommandList::UpdateConstantBuffer(IBuffer* pBuffer, size_t offset, size_t size, const void* pData)
{
    if (pBuffer == nullptr || size == 0 || pData == nullptr)
    { return false; }

    ImCmdUpdateConstantBuffer cmd = {};
    cmd.Id      = CMD_UPDATE_CONSTANT_BUFFER;
    cmd.pBuffer = pBuffer;
    cmd.Offset  = offset;
    cmd.Size    = size;

    m_Buffer.Push(&cmd, sizeof(cmd));
    m_Buffer.Push(pData, size);
    return true;
}

//-------------------------------------------------------------------------------------------------
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetName(const char* name)
{ m_Name = name; }

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* CommandList::GetName() const
{ return m_Name.c_str(); }

//-------------------------------------------------------------------------------------------------
//      コマンドバッファを取得します.
//-------------------------------------------------------------------------------------------------
const CommandBuffer* CommandList::GetCommandBuffer() const
{ return &m_Buffer; }

//-------------------------------------------------------------------------------------------------
//      コマンドの記録を開始します.
//-------------------------------------------------------------------------------------------------
void ICommandList::Begin()
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    pThis->m_Buffer.Reset();

    ImCmdBegin cmd = {};
    cmd.Id = (pThis->m_Type == COMMANDLIST_TYPE_DIRECT) ? CMD_BEGIN : CMD_SUB_BEGIN;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      フレームバッファを設定します.
//-------------------------------------------------------------------------------------------------
void ICommandList::BeginFrameBuffer
(
    uint32_t                        renderTargetViewCount,
    IRenderTargetView**             pRenderTargetViews,
    IDepthStencilView*              pDepthStencilView,
    uint32_t                        clearColorCount,
    const ClearColorValue*          pClearColors,
    const ClearDepthStencilValue*   pClearDepthStencil
)
{
    if (pRenderTargetViews == nullptr && pDepthStencilView == nullptr)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdBeginFrameBuffer cmd = {};
    cmd.Id         = CMD_BEGIN_FRAME_BUFFER;
    cmd.RenderTargetViewCount = REQUEST_OPLOCK_CURRENT_VERSION;

    auto maxCount = (renderTargetViewCount >= 8) ? 8 : renderTargetViewCount;
    for(auto i=0u; i<maxCount; ++i)
    { cmd.pRenderTargetView[i] = pRenderTargetViews[i]; }
    cmd.pDepthStencilView = pDepthStencilView;

    cmd.ClearColorCount = clearColorCount;
    for(auto i=0u; i<clearColorCount; ++i)
    { cmd.ClearColors[i] = pClearColors[i]; }

    if (pClearDepthStencil != nullptr)
    { cmd.ClearDepthStencil = *pClearDepthStencil; }

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      フレームバッファを解除します.
//-------------------------------------------------------------------------------------------------
void ICommandList::EndFrameBuffer()
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdBase cmd = {};
    cmd.Id = CMD_END_FRAME_BUFFER;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      加速機構を構築します.
//-------------------------------------------------------------------------------------------------
void ICommandList::BuildAccelerationStructure(IAccelerationStructure* pAS)
{
    if (pAS == nullptr)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdBuildAccelerationStructure cmd = {};
    cmd.Id  = CMD_BUILD_ACCELERATION_STRUCTURE;
    cmd.pAS = pAS;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      ブレンド定数を設定します.
//-------------------------------------------------------------------------------------------------
void ICommandList::SetBlendConstant(const float blendConstant[4])
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdSetBlendConstant cmd = {};
    cmd.Id = CMD_SET_BLEND_CONSTANT;
    memcpy( cmd.BlendConstant, blendConstant, sizeof(cmd.BlendConstant) );

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      ステンシル参照値を設定します.
//-------------------------------------------------------------------------------------------------
void ICommandList::SetStencilReference(uint32_t stencilRef)
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdSetStencilReference cmd = {};
    cmd.Id                  = CMD_SET_STENCIL_REFERENCE;
    cmd.StencilReference    = stencilRef;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      ビューポートを設定します.
//-------------------------------------------------------------------------------------------------
void ICommandList::SetViewports(uint32_t count, Viewport* pViewports)
{
    if (count == 0 || pViewports == nullptr)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdSetViewports cmd = {};
    cmd.Id      = CMD_SET_VIEWPORTS;
    cmd.Count   = count;
    memcpy(cmd.Viewports, pViewports, sizeof(Viewport) * count);

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      シザー矩形を設定します.
//-------------------------------------------------------------------------------------------------
void ICommandList::SetScissors(uint32_t count, Rect* pScissors)
{
    if (count == 0 || pScissors == nullptr)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdSetScissors cmd = {};
    cmd.Id      = CMD_SET_SCISSORS;
    cmd.Count   = count;
    memcpy(cmd.Rects, pScissors, sizeof(Rect) * count);

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      パイプラインステートを設定します.
//-------------------------------------------------------------------------------------------------
void ICommandList::SetPipelineState(IPipelineState* pPipelineState)
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdSetPipelineState cmd = {};
    cmd.Id              = CMD_SET_PIPELINESTATE;
    cmd.pPipelineState  = pPipelineState;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      頂点バッファを設定します.
//-------------------------------------------------------------------------------------------------
void ICommandList::SetVertexBuffers
(
    uint32_t    startSlot,
    uint32_t    count,
    IBuffer**   ppResources,
    uint64_t*   pOffsets
)
{
    if (ppResources == nullptr)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdSetVertexBuffers cmd = {};
    cmd.Id          = CMD_SET_VERTEX_BUFFERS;
    cmd.StartSlot   = startSlot;
    cmd.Count       = count;
    memcpy(cmd.pBuffers, ppResources, sizeof(IBuffer) * count);
    cmd.HasOffset   = (pOffsets != nullptr);
    if (pOffsets != nullptr)
    { memcpy(cmd.Offsets, pOffsets, sizeof(uint64_t) * count); }

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      インデックスバッファを設定します.
//-------------------------------------------------------------------------------------------------
void ICommandList::SetIndexBuffer(IBuffer* pResource, uint64_t offset)
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdSetIndexBuffer cmd = {};
    cmd.Id      = CMD_SET_INDEX_BUFFER;
    cmd.pBuffer = pResource;
    cmd.Offset  = offset;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      32bit定数を設定します.
//-------------------------------------------------------------------------------------------------
void ICommandList::SetConstants(uint32_t count, const void* pValues, uint32_t offset)
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdSetConstants cmd = {};
    cmd.Id      = CMD_SET_CONSTANTS;
    cmd.Count   = count;
    cmd.Offset  = offset;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
    pThis->m_Buffer.Push(pValues, count * 4);
}

//-------------------------------------------------------------------------------------------------
//      定数バッファビューを設定します.
//-------------------------------------------------------------------------------------------------
void ICommandList::SetView(uint32_t index, IConstantBufferView* const pView)
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdSetCBV cmd = {};
    cmd.Id      = CMD_SET_CBV;
    cmd.Index   = index;
    cmd.pView   = pView;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを設定します.
//-------------------------------------------------------------------------------------------------
void ICommandList::SetView(uint32_t index, IShaderResourceView* const pView)
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdSetSRV cmd = {};
    cmd.Id      = CMD_SET_SRV;
    cmd.Index   = index;
    cmd.pView   = pView;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      アンオーダードアクセスビューを設定します.
//-------------------------------------------------------------------------------------------------
void ICommandList::SetView(uint32_t index, IUnorderedAccessView* const pView)
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdSetUAV cmd = {};
    cmd.Id      = CMD_SET_UAV;
    cmd.Index   = index;
    cmd.pView   = pView;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      サンプラーを設定します.
//-------------------------------------------------------------------------------------------------
void ICommandList::SetSampler(uint32_t index, ISampler* const pSampler)
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdSetSampler cmd = {};
    cmd.Id          = CMD_SET_SAMPLER;
    cmd.Index       = index;
    cmd.pSampler    = pSampler;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      テクスチャバリアを設定します.
//-------------------------------------------------------------------------------------------------
void ICommandList::TextureBarrier
(
    ITexture*       pResource,
    RESOURCE_STATE  prevState,
    RESOURCE_STATE  nextState
)
{
    if (prevState == nextState)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdTextureBarrier cmd = {};
    cmd.Id          = CMD_TEXTURE_BARRIER;
    cmd.pResource   = pResource;
    cmd.PrevState   = prevState;
    cmd.NextState   = nextState;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      バッファバリアを設定します.
//-------------------------------------------------------------------------------------------------
void ICommandList::BufferBarrier
(
    IBuffer*        pResource,
    RESOURCE_STATE  prevState,
    RESOURCE_STATE  nextState
)
{
    if (prevState == nextState)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdBufferBarrier cmd = {};
    cmd.Id          = CMD_BUFFER_BARRIER;
    cmd.pResource   = pResource;
    cmd.PrevState   = prevState;
    cmd.NextState   = nextState;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      インスタンス描画します.
//-------------------------------------------------------------------------------------------------
void ICommandList::DrawInstanced
(
    uint32_t    vertexCount,
    uint32_t    instanceCount,
    uint32_t    firstVertex,
    uint32_t    firstInstance
)
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdDrawInstanced cmd = {};
    cmd.Id              = CMD_DRAW_INSTANCED;
    cmd.VertexCount     = vertexCount;
    cmd.InstanceCount   = instanceCount;
    cmd.FirstVertex     = firstVertex;
    cmd.FirstInstance   = firstInstance;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      インスタンスを間接描画します.
//-------------------------------------------------------------------------------------------------
void ICommandList::DrawInstancedIndirect
(
    uint32_t    maxCommandCount,
    IBuffer*    pArgumentBuffer,
    uint64_t    argumentBufferOffset,
    IBuffer*    pCounterBuffer,
    uint64_t    counterBufferOffset
)
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdDrawInstancedIndirect cmd = {};
    cmd.Id                      = CMD_DRAW_INSTANCED_INDIRECT;
    cmd.maxCommandCount         = maxCommandCount;
    cmd.pArgumentBuffer         = pArgumentBuffer;
    cmd.argumentBufferOffset    = argumentBufferOffset;
    cmd.pCounterBuffer          = pCounterBuffer;
    cmd.counterBufferOffset     = counterBufferOffset;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      インデックス付きでインスタンス描画します.
//-------------------------------------------------------------------------------------------------
void ICommandList::DrawIndexedInstanced
(
    uint32_t    indexCount,
    uint32_t    instanceCount,
    uint32_t    firstIndex,
    int         vertexOffset,
    uint32_t    firstInstance
)
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdDrawIndexedInstanced cmd = {};
    cmd.Id              = CMD_DRAW_INDEXED_INSTANCED;
    cmd.IndexCount      = indexCount;
    cmd.InstanceCount   = instanceCount;
    cmd.FirstIndex      = firstIndex;
    cmd.VertexOffset    = vertexOffset;
    cmd.FirstInstance   = firstInstance;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      インデックス付きでインスタンスを間接描画します.
//-------------------------------------------------------------------------------------------------
void ICommandList::DrawIndexedInstancedIndirect
(
    uint32_t    maxCommandCount,
    IBuffer*    pArgumentBuffer,
    uint64_t    argumentBufferOffset,
    IBuffer*    pCounterBuffer,
    uint64_t    counterBufferOffset

)
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdDrawIndexedInstancedIndirect cmd = {};
    cmd.Id                      = CMD_DRAW_INDEXED_INSTANCED_INDIRECT;
    cmd.maxCommandCount         = maxCommandCount;
    cmd.pArgumentBuffer         = pArgumentBuffer;
    cmd.argumentBufferOffset    = argumentBufferOffset;
    cmd.pCounterBuffer          = pCounterBuffer;
    cmd.counterBufferOffset     = counterBufferOffset;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      コンピュートシェーダを起動します.
//-------------------------------------------------------------------------------------------------
void ICommandList::DispatchCompute(uint32_t x, uint32_t y, uint32_t z)
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdDispatch cmd = {};
    cmd.Id  = CMD_DISPATCH_COMPUTE;
    cmd.X   = x;
    cmd.Y   = y;
    cmd.Z   = z;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      コンピュートシェーダを間接起動します.
//-------------------------------------------------------------------------------------------------
void ICommandList::DispatchComputeIndirect
(
    uint32_t    maxCommandCount,
    IBuffer*    pArgumentBuffer,
    uint64_t    argumentBufferOffset,
    IBuffer*    pCounterBuffer,
    uint64_t    counterBufferOffset

)
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdDispatchComputeIndirect cmd = {};
    cmd.Id                      = CMD_DISPATCH_COMPUTE_INDIRECT;
    cmd.maxCommandCount         = maxCommandCount;
    cmd.pArgumentBuffer         = pArgumentBuffer;
    cmd.argumentBufferOffset    = argumentBufferOffset;
    cmd.pCounterBuffer          = pCounterBuffer;
    cmd.counterBufferOffset     = counterBufferOffset;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      メッシュシェーダを起動します.
//-------------------------------------------------------------------------------------------------
void ICommandList::DispatchMesh(uint32_t x, uint32_t y, uint32_t z)
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdDispatch cmd = {};
    cmd.Id  = CMD_DISPATCH_MESH;
    cmd.X   = x;
    cmd.Y   = y;
    cmd.Z   = z;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      メッシュシェーダを間接起動します.
//-------------------------------------------------------------------------------------------------
void ICommandList::DispatchMeshIndirect
(
    uint32_t    maxCommandCount,
    IBuffer*    pArgumentBuffer,
    uint64_t    argumentBufferOffset,
    IBuffer*    pCounterBuffer,
    uint64_t    counterBufferOffset

)
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdDispatchMeshIndirect cmd = {};
    cmd.Id                      = CMD_DISPATCH_MESH_INDIRECT;
    cmd.maxCommandCount         = maxCommandCount;
    cmd.pArgumentBuffer         = pArgumentBuffer;
    cmd.argumentBufferOffset    = argumentBufferOffset;
    cmd.pCounterBuffer          = pCounterBuffer;
    cmd.counterBufferOffset     = counterBufferOffset;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      レイトレーシングパイプラインを起動します.
//-------------------------------------------------------------------------------------------------
void ICommandList::TraceRays(const TraceRayArguments* pArgs)
{
    if (pArgs == nullptr)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdTraceRays cmd = {};
    cmd.Id   = CMD_TRACE_RAYS;
    cmd.Args = *pArgs;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      クエリを開始します.
//-------------------------------------------------------------------------------------------------
void ICommandList::BeginQuery(IQueryPool* pQuery, uint32_t index)
{
    if (pQuery == nullptr)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdBeginQuery cmd = {};
    cmd.Id      = CMD_BEGIN_QUERY;
    cmd.pQuery  = pQuery;
    cmd.Index   = index;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      クエリを終了します.
//-------------------------------------------------------------------------------------------------
void ICommandList::EndQuery(IQueryPool* pQuery, uint32_t index)
{
    if (pQuery == nullptr)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdEndQuery cmd = {};
    cmd.Id      = CMD_END_QUERY;
    cmd.pQuery  = pQuery;
    cmd.Index   = index;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      クエリを解決します.
//-------------------------------------------------------------------------------------------------
void ICommandList::ResolveQuery
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

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdResolveQuery cmd = {};
    cmd.Id          = CMD_RESOLVE_QUERY;
    cmd.pQuery      = pQuery;
    cmd.StartIndex  = startIndex;
    cmd.QueryCount  = queryCount;
    cmd.pDstBuffer  = pDstBuffer;
    cmd.DstOffset   = dstOffset;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      クエリをリセットします.
//-------------------------------------------------------------------------------------------------
void ICommandList::ResetQuery(IQueryPool* pQuery)
{
    if (pQuery == nullptr)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdResetQuery cmd = {};
    cmd.Id      = CMD_RESET_QUERY;
    cmd.pQuery  = pQuery;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      テクスチャをコピーします.
//-------------------------------------------------------------------------------------------------
void ICommandList::CopyTexture
(
    ITexture*       pDstResource,
    ITexture*       pSrcResource
)
{
    if (pDstResource == nullptr || pSrcResource == nullptr)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdCopyTexture cmd = {};
    cmd.Id          = CMD_COPY_TEXTURE;
    cmd.pDstTexture = pDstResource;
    cmd.pSrcTexture = pSrcResource;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      バッファをコピーします.
//-------------------------------------------------------------------------------------------------
void ICommandList::CopyBuffer(IBuffer* pDstResource, IBuffer* pSrcResource)
{
    if (pDstResource == nullptr || pSrcResource == nullptr)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdCopyBuffer cmd = {};
    cmd.Id          = CMD_COPY_BUFFER;
    cmd.pDstBuffer = pDstResource;
    cmd.pSrcBuffer = pSrcResource;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      領域を指定してテクスチャをコピーします.
//-------------------------------------------------------------------------------------------------
void ICommandList::CopyTextureRegion
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

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdCopyTextureRegion cmd = {};
    cmd.Id              = CMD_COPY_TEXTURE_REGION;
    cmd.pDstResource    = pDstResource;
    cmd.DstSubresource  = dstSubresource;
    cmd.DstOffset       = dstOffset;
    cmd.pSrcResource    = pSrcResource;
    cmd.SrcSubresource  = srcSubresource;
    cmd.SrcOffset       = srcOffset;
    cmd.SrcExtent       = srcExtent;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      領域を指定してバッファをコピーします.
//-------------------------------------------------------------------------------------------------
void ICommandList::CopyBufferRegion
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

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdCopyBufferRegion cmd = {};
    cmd.Id          = CMD_COPY_BUFFER_REGION;
    cmd.pDstBuffer  = pDstBuffer;
    cmd.DstOffset   = dstOffset;
    cmd.pSrcBuffer  = pSrcBuffer;
    cmd.SrcOffset   = srcOffset;
    cmd.ByteCount   = byteCount;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      バッファからテクスチャにコピーします.
//-------------------------------------------------------------------------------------------------
void ICommandList::CopyBufferToTexture
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

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdCopyBufferToTexture cmd = {};
    cmd.Id              = CMD_COPY_BUFFER_TO_TEXTURE;
    cmd.pDstTexture     = pDstTexture;
    cmd.DstSubresource  = dstSubresource;
    cmd.DstOffset       = dstOffset;
    cmd.pSrcBuffer      = pSrcBuffer;
    cmd.SrcOffset       = srcOffset;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      テクスチャからバッファにコピーします.
//-------------------------------------------------------------------------------------------------
void ICommandList::CopyTextureToBuffer
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

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdCopyTextureToBuffer cmd = {};
    cmd.Id              = CMD_COPY_TEXTURE_TO_BUFFER;
    cmd.pDstBuffer      = pDstBuffer;
    cmd.DstOffset       = dstOffset;
    cmd.pSrcTexture     = pSrcTexture;
    cmd.SrcSubresource  = srcSubresource;
    cmd.SrcOffset       = srcOffset;
    cmd.SrcExtent       = srcExtent;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      加速機構をコピーします.
//-------------------------------------------------------------------------------------------------
void ICommandList::CopyAccelerationStructure
(
    IAccelerationStructure*             pDstAS,
    IAccelerationStructure*             pSrcAS,
    ACCELERATION_STRUCTURE_COPY_MODE    mode
)
{
    if (pDstAS == nullptr || pSrcAS == nullptr)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdCopyAccelerationStructure cmd = {};
    cmd.Id      = CMD_COPY_ACCELERATION_STRUCTURE;
    cmd.pDstAS  = pDstAS;
    cmd.pSrcAS  = pSrcAS;
    cmd.Mode    = mode;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      サブリソースを解決します.
//-------------------------------------------------------------------------------------------------
void ICommandList::ResolveSubresource
(
    ITexture*       pDstResource,
    uint32_t        dstSubresource,
    ITexture*       pSrcResource,
    uint32_t        srcSubresource
)
{
    if (pDstResource == nullptr || pSrcResource == nullptr)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdResolveSubresource cmd = {};
    cmd.Id              = CMD_RESOLVE_SUBRESOURCE;
    cmd.pDstResource    = pDstResource;
    cmd.DstSubresource  = dstSubresource;
    cmd.pSrcResource    = pSrcResource;
    cmd.SrcSubresource  = srcSubresource;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      バンドルを実行します.
//-------------------------------------------------------------------------------------------------
void ICommandList::ExecuteBundle(ICommandList* pCommandList)
{
    if (pCommandList == nullptr)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    auto pWrapCommandList = reinterpret_cast<CommandList*>(pCommandList);
    A3D_ASSERT(pWrapCommandList != nullptr);

    pThis->m_Buffer.Append(pWrapCommandList->GetCommandBuffer());
}

//-------------------------------------------------------------------------------------------------
//      デバッグマーカーをプッシュします.
//-------------------------------------------------------------------------------------------------
void ICommandList::PushMarker(const char* tag)
{
    if (tag == nullptr)
    { return; }

    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdPushMarker cmd = {};
    cmd.Id = CMD_PUSH_MARKER;
    strcpy_s( cmd.Tag, tag );

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      デバッグマーカーをポップします.
//-------------------------------------------------------------------------------------------------
void ICommandList::PopMarker()
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdPopMarker cmd = {};
    cmd.Id = CMD_POP_MARKER;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
}

//-------------------------------------------------------------------------------------------------
//      コマンドの記録を終了します.
//-------------------------------------------------------------------------------------------------
void ICommandList::End()
{
    auto pThis = static_cast<CommandList*>(this);
    A3D_ASSERT(pThis != nullptr);

    ImCmdEnd cmd = {};
    cmd.Id = (pThis->m_Type == COMMANDLIST_TYPE_DIRECT) ? CMD_END : CMD_SUB_END;

    pThis->m_Buffer.Push(&cmd, sizeof(cmd));
    pThis->m_Buffer.Close();
}

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
    instance->m_Type    = pDesc->Type;
    
    if (!instance->m_Buffer.Init(pDesc->BufferSize))
    {
        SafeRelease(instance);
        return false;
    }

    *ppCommandList = instance;
    return true;
}

} // namespace a3d
