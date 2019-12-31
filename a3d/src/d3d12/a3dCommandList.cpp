//-------------------------------------------------------------------------------------------------
// File : a3dCommandList.cpp
// Desc : CommandList Implmentation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#if A3D_IS_WIN
    #include <pix_win.h>
#else
    #include <pix.h>
#endif


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// CommandList class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
CommandList::CommandList()
: m_RefCount            (1)
, m_pDevice             (nullptr)
, m_pCommandAllocator   (nullptr)
, m_pCommandList        (nullptr)
, m_pFrameBuffer        (nullptr)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
CommandList::~CommandList()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool CommandList::Init(IDevice* pDevice, COMMANDLIST_TYPE listType)
{
    if (pDevice == nullptr)
    { return false; }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    {
        D3D12_COMMAND_LIST_TYPE type;
        switch( listType )
        {
        case COMMANDLIST_TYPE_DIRECT:
            type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            break;

        case COMMANDLIST_TYPE_BUNDLE:
            type = D3D12_COMMAND_LIST_TYPE_BUNDLE;
            break;

        case COMMANDLIST_TYPE_COPY:
            type = D3D12_COMMAND_LIST_TYPE_COPY;
            break;

        case COMMANDLIST_TYPE_COMPUTE:
            type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            break;
        }

        auto hr = pNativeDevice->CreateCommandAllocator( type, IID_PPV_ARGS(&m_pCommandAllocator) );
        if ( FAILED(hr) )
        { return false; }

        hr = pNativeDevice->CreateCommandList( 0, type, m_pCommandAllocator, nullptr, IID_PPV_ARGS(&m_pCommandList) );
        if ( FAILED(hr) )
        { return false; }
    }

    m_pCommandList->Close();

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void CommandList::Term()
{
    SafeRelease(m_pCommandList);
    SafeRelease(m_pCommandAllocator);
    SafeRelease(m_pDevice);
}

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
//      コマンドリストの記録を開始します.
//-------------------------------------------------------------------------------------------------
void CommandList::Begin()
{
    m_pCommandAllocator->Reset();
    m_pCommandList->Reset(m_pCommandAllocator, nullptr);
    m_pFrameBuffer = nullptr;

    auto heapBuf = m_pDevice->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    auto heapSmp = m_pDevice->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
   
    uint32_t count = 0;
    ID3D12DescriptorHeap* pHeaps[2] = {};
    if (heapBuf->GetHandleCount() > 0)
    {
        pHeaps[count] = heapBuf->GetD3D12DescriptorHeap();
        count++;
    }

    if (heapSmp->GetHandleCount() > 0)
    {
        pHeaps[count] = heapSmp->GetD3D12DescriptorHeap();
        count++;
    }

    if (count == 0)
    {
        return;
    }

    m_pCommandList->SetDescriptorHeaps(count, pHeaps);
}

//-------------------------------------------------------------------------------------------------
//      フレームバッファを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::BeginFrameBuffer(IFrameBuffer* pBuffer)
{
    if (pBuffer == nullptr)
    { return; }

    auto pWrapFrameBuffer = static_cast<FrameBuffer*>(pBuffer);
    A3D_ASSERT(pWrapFrameBuffer != nullptr);

    // 同じバッファであればコマンドを出さない.
    if (m_pFrameBuffer == pWrapFrameBuffer)
    { return; }

    pWrapFrameBuffer->Bind(this);
    m_pFrameBuffer = pWrapFrameBuffer;
}

//-------------------------------------------------------------------------------------------------
//      フレームバッファを解除します.
//-------------------------------------------------------------------------------------------------
void CommandList::EndFrameBuffer()
{
    m_pCommandList->OMSetRenderTargets(0, nullptr, FALSE, nullptr);
}

//-------------------------------------------------------------------------------------------------
//      フレームバッファをクリアします.
//-------------------------------------------------------------------------------------------------
void CommandList::ClearFrameBuffer
(
    uint32_t                        clearColorCount,
    const ClearColorValue*          pClearColors,
    const ClearDepthStencilValue*   pClearDepthStencil
)
{
    if (clearColorCount == 0 && pClearColors == nullptr && pClearDepthStencil == nullptr)
    { return; }

    if (m_pFrameBuffer == nullptr)
    { return; }

    m_pFrameBuffer->Clear(this, clearColorCount, pClearColors, pClearDepthStencil);
}

//-------------------------------------------------------------------------------------------------
//      ブレンド定数を設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetBlendConstant(const float blendConstant[4])
{ m_pCommandList->OMSetBlendFactor(blendConstant); }

//-------------------------------------------------------------------------------------------------
//      ステンシル参照値を設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetStencilReference(uint32_t stencilRef)
{ m_pCommandList->OMSetStencilRef(stencilRef); }

//-------------------------------------------------------------------------------------------------
//      ビューポートを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetViewports(uint32_t count, Viewport* pViewports)
{
    if (count == 0 || pViewports == nullptr)
    { return; }

    auto pNativeViewports = reinterpret_cast<D3D12_VIEWPORT*>(pViewports);

    m_pCommandList->RSSetViewports(count, pNativeViewports);
}

//-------------------------------------------------------------------------------------------------
//      シザー矩形を設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetScissors(uint32_t count, Rect* pScissors)
{
    if (count == 0 || pScissors == nullptr)
    { return; }

    D3D12_RECT rects[D3D12_VIEWPORT_AND_SCISSORRECT_MAX_INDEX] = {};
    for(auto i=0u; i<count; ++i)
    {
        rects[i].left   = pScissors[i].Offset.X;
        rects[i].top    = pScissors[i].Offset.Y;
        rects[i].right  = pScissors[i].Offset.X + pScissors[i].Extent.Width;
        rects[i].bottom = pScissors[i].Offset.Y + pScissors[i].Extent.Height;
    }

    m_pCommandList->RSSetScissorRects(count, rects);
}

//-------------------------------------------------------------------------------------------------
//      パイプラインステートを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetPipelineState(IPipelineState* pPipelineState)
{
    if (pPipelineState == nullptr)
    { return; }

    auto pWrapPipelineState = static_cast<PipelineState*>(pPipelineState);
    A3D_ASSERT(pWrapPipelineState != nullptr);

    pWrapPipelineState->Issue(this);
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタセットを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetDescriptorSet(IDescriptorSet* pDescriptorSet)
{
    if (pDescriptorSet == nullptr)
    { return; }

    auto pWrapDescriptorSet = static_cast<DescriptorSet*>(pDescriptorSet);
    pWrapDescriptorSet->Bind(this);
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
    if (count == 0 || ppResources == nullptr)
    { return; }

    D3D12_VERTEX_BUFFER_VIEW views[D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = {};

    for(auto i=0u; i<count; ++i)
    {
        auto pWrapResource = static_cast<Buffer*>(ppResources[i]);
        A3D_ASSERT(pWrapResource != nullptr);

        auto pNativeResource = pWrapResource->GetD3D12Resource();
        A3D_ASSERT(pNativeResource != nullptr);

        auto desc = pWrapResource->GetDesc();

        uint64_t offset = 0;
        if (pOffsets != nullptr)
        { offset = pOffsets[i]; }

        views[i].BufferLocation = pNativeResource->GetGPUVirtualAddress() + offset;
        views[i].SizeInBytes    = uint32_t(desc.Size);
        views[i].StrideInBytes  = desc.Stride;
    }

    m_pCommandList->IASetVertexBuffers(startSlot, count, views);
}

//-------------------------------------------------------------------------------------------------
//      インデックスバッファを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetIndexBuffer
(
    IBuffer*   pResource,
    uint64_t   offset
)
{
    if (pResource == nullptr)
    { return; }

    auto pWrapResource = static_cast<Buffer*>(pResource);
    A3D_ASSERT( pWrapResource != nullptr );

    auto pNativeResource = pWrapResource->GetD3D12Resource();
    A3D_ASSERT( pNativeResource != nullptr );

    D3D12_INDEX_BUFFER_VIEW view = {};
    view.BufferLocation = pNativeResource->GetGPUVirtualAddress() + offset;
    view.Format         = (pWrapResource->GetDesc().Stride == sizeof(uint16_t))
                          ? DXGI_FORMAT_R16_UINT 
                          : DXGI_FORMAT_R32_UINT;
    view.SizeInBytes    = static_cast<uint32_t>(pWrapResource->GetDesc().Size);

    m_pCommandList->IASetIndexBuffer( &view );
}

//-------------------------------------------------------------------------------------------------
//      リソースバリアを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::TextureBarrier
(
    ITexture*       pResource,
    RESOURCE_STATE  prevState,
    RESOURCE_STATE  nextState
)
{
    if (pResource == nullptr)
    { return; }

    auto pWrapResource = static_cast<Texture*>(pResource);
    A3D_ASSERT(pWrapResource != nullptr);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = pWrapResource->GetD3D12Resource();
    barrier.Transition.StateBefore = ToNativeState(prevState);
    barrier.Transition.StateAfter  = ToNativeState(nextState);
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    m_pCommandList->ResourceBarrier(1, &barrier);
}

//-------------------------------------------------------------------------------------------------
//      リソースバリアを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::BufferBarrier
(
    IBuffer*        pResource,
    RESOURCE_STATE  prevState,
    RESOURCE_STATE  nextState
)
{
    if (pResource == nullptr)
    { return; }

    auto pWrapResource = static_cast<Buffer*>(pResource);
    A3D_ASSERT(pWrapResource != nullptr);

    auto heapType = pWrapResource->GetDesc().HeapProperty.Type;

    // D3D12では UPLOAD と READBACK はリソースステートを仕様上変更できないため，
    // 実行せずに正常終了させる.
    if (heapType == HEAP_TYPE_UPLOAD || heapType == HEAP_TYPE_READBACK)
    { return;}

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = pWrapResource->GetD3D12Resource();
    barrier.Transition.StateBefore = ToNativeState(prevState);
    barrier.Transition.StateAfter  = ToNativeState(nextState);
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    m_pCommandList->ResourceBarrier(1, &barrier);
}

//-------------------------------------------------------------------------------------------------
//      インスタンス描画します.
//-------------------------------------------------------------------------------------------------
void CommandList::DrawInstanced
(
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance
)
{ m_pCommandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance); }

//-------------------------------------------------------------------------------------------------
//      インデックスバッファを用いてインスタンス描画します.
//-------------------------------------------------------------------------------------------------
void CommandList::DrawIndexedInstanced
(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int      vertexOffset,
    uint32_t firstInstance
)
{
    m_pCommandList->DrawIndexedInstanced(
        indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

//-------------------------------------------------------------------------------------------------
//      スレッドグループからコマンドリストを実行します.
//-------------------------------------------------------------------------------------------------
void CommandList::Dispatch(uint32_t x, uint32_t y, uint32_t z)
{ m_pCommandList->Dispatch(x, y, z); }

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
    if (pCommandSet == nullptr || maxCommandCount == 0 || pArgumentBuffer == nullptr)
    { return; }

    auto pWrapCommandSet = static_cast<CommandSet*>(pCommandSet);
    A3D_ASSERT(pWrapCommandSet != nullptr);

    auto pNativeCommandSignature = pWrapCommandSet->GetD3D12CommandSignature();
    A3D_ASSERT(pNativeCommandSignature != nullptr);

    auto pWrapArgumentBuffer = static_cast<Buffer*>(pArgumentBuffer);
    A3D_ASSERT(pWrapArgumentBuffer != nullptr);

    auto pNativeArgumentResource = pWrapArgumentBuffer->GetD3D12Resource();
    A3D_ASSERT(pNativeArgumentResource != nullptr);

    ID3D12Resource* pNativeCounterResource = nullptr;
    if (pCounterBuffer != nullptr)
    {
        auto pWrapCouterBuffer = static_cast<Buffer*>(pCounterBuffer);
        A3D_ASSERT(pWrapCouterBuffer != nullptr);

        pNativeCounterResource = pWrapCouterBuffer->GetD3D12Resource();
        A3D_ASSERT(pNativeCounterResource != nullptr);
    }

    m_pCommandList->ExecuteIndirect(
        pNativeCommandSignature,
        maxCommandCount,
        pNativeArgumentResource,
        argumentBufferOffset,
        pNativeCounterResource, 
        counterBufferOffset);
}

//-------------------------------------------------------------------------------------------------
//      クエリを開始します.
//-------------------------------------------------------------------------------------------------
void CommandList::BeginQuery(IQueryPool* pQuery, uint32_t index)
{
    if (pQuery == nullptr)
    { return; }

    auto pWrapQueryPool = static_cast<QueryPool*>(pQuery);
    A3D_ASSERT(pWrapQueryPool != nullptr);

    m_pCommandList->BeginQuery(
        pWrapQueryPool->GetD3D12QueryHeap(),
        pWrapQueryPool->GetD3D12QueryType(),
        index);
}

//-------------------------------------------------------------------------------------------------
//      クエリを終了します.
//-------------------------------------------------------------------------------------------------
void CommandList::EndQuery(IQueryPool* pQuery, uint32_t index)
{
    if (pQuery == nullptr)
    { return; }

    auto pWrapQueryPool = static_cast<QueryPool*>(pQuery);
    A3D_ASSERT(pWrapQueryPool != nullptr);

    m_pCommandList->EndQuery(
        pWrapQueryPool->GetD3D12QueryHeap(),
        pWrapQueryPool->GetD3D12QueryType(),
        index);
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
    if (pQuery == nullptr || queryCount == 0 || pDstBuffer == nullptr)
    { return; }

    auto pWrapQueryPool = static_cast<QueryPool*>(pQuery);
    A3D_ASSERT(pWrapQueryPool != nullptr);

    auto pNativeQueryPool = pWrapQueryPool->GetD3D12QueryHeap();
    A3D_ASSERT(pNativeQueryPool != nullptr);

    auto type = pWrapQueryPool->GetD3D12QueryType();

    auto pWrapBuffer = static_cast<Buffer*>(pDstBuffer);
    A3D_ASSERT(pWrapBuffer != nullptr);

    auto pNativeResource = pWrapBuffer->GetD3D12Resource();
    A3D_ASSERT(pNativeResource != nullptr);

    m_pCommandList->ResolveQueryData(
        pNativeQueryPool,
        type,
        startIndex,
        queryCount,
        pNativeResource,
        dstOffset );
}

//-------------------------------------------------------------------------------------------------
//      バッファをコピーします.
//-------------------------------------------------------------------------------------------------
void CommandList::CopyBuffer(IBuffer* pDst, IBuffer* pSrc)
{
    if (pDst == nullptr || pSrc == nullptr)
    { return; }

    auto pWrapDst = static_cast<Buffer*>(pDst);
    auto pWrapSrc = static_cast<Buffer*>(pSrc);
    A3D_ASSERT(pWrapDst != nullptr);
    A3D_ASSERT(pWrapSrc != nullptr);

    m_pCommandList->CopyResource(pWrapDst->GetD3D12Resource(), pWrapSrc->GetD3D12Resource());
}

//-------------------------------------------------------------------------------------------------
//      テクスチャをコピーします.
//-------------------------------------------------------------------------------------------------
void CommandList::CopyTexture
(
    ITexture*       pDst,
    ITexture*       pSrc
)
{
    if (pDst == nullptr || pSrc == nullptr)
    { return; }

    auto pWrapDst = static_cast<Texture*>(pDst);
    auto pWrapSrc = static_cast<Texture*>(pSrc);
    A3D_ASSERT(pWrapDst != nullptr);
    A3D_ASSERT(pWrapSrc != nullptr);

    m_pCommandList->CopyResource(pWrapDst->GetD3D12Resource(), pWrapSrc->GetD3D12Resource());
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

    auto pWrapDst = static_cast<Texture*>(pDstResource);
    auto pWrapSrc = static_cast<Texture*>(pSrcResource);
    A3D_ASSERT(pWrapDst != nullptr);
    A3D_ASSERT(pWrapSrc != nullptr);

    D3D12_TEXTURE_COPY_LOCATION src = {};
    src.pResource        = pWrapSrc->GetD3D12Resource();
    src.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    src.SubresourceIndex = srcSubresource;

    D3D12_TEXTURE_COPY_LOCATION dst = {};
    dst.pResource        = pWrapDst->GetD3D12Resource();
    dst.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex = dstSubresource;

    D3D12_BOX box = {};
    box.left   = srcOffset.X;
    box.top    = srcOffset.Y;
    box.front  = srcOffset.Z;
    box.right  = srcExtent.Width;
    box.bottom = srcExtent.Height;
    box.back   = srcExtent.Depth;

    m_pCommandList->CopyTextureRegion(
        &dst, dstOffset.X, dstOffset.Y, dstOffset.Z,
        &src, &box);
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

    auto pWrapDst = static_cast<Buffer*>(pDstBuffer);
    auto pWrapSrc = static_cast<Buffer*>(pSrcBuffer);
    A3D_ASSERT(pWrapDst != nullptr);
    A3D_ASSERT(pWrapSrc != nullptr);

    m_pCommandList->CopyBufferRegion(
        pWrapDst->GetD3D12Resource(),
        dstOffset,
        pWrapSrc->GetD3D12Resource(),
        srcOffset,
        byteCount);
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

    auto pWrapDst = static_cast<Texture*>(pDstTexture);
    auto pWrapSrc = static_cast<Buffer*>(pSrcBuffer);
    A3D_ASSERT(pWrapDst != nullptr);
    A3D_ASSERT(pWrapSrc != nullptr);

    auto pWrapDevice = static_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    auto nativeDstDesc = pWrapDst->GetD3D12Resource()->GetDesc();

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint;
    pNativeDevice->GetCopyableFootprints(
        &nativeDstDesc,
        0,
        1,
        srcOffset,
        &footPrint,
        nullptr,
        nullptr,
        nullptr);

    D3D12_TEXTURE_COPY_LOCATION src = {};
    src.pResource       = pWrapSrc->GetD3D12Resource();
    src.Type            = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src.PlacedFootprint = footPrint;

    D3D12_TEXTURE_COPY_LOCATION dst = {};
    dst.pResource        = pWrapDst->GetD3D12Resource();
    dst.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex = dstSubresource;

    m_pCommandList->CopyTextureRegion(&dst, dstOffset.X, dstOffset.Y, dstOffset.Z, &src, nullptr);
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

    auto pWrapDst = static_cast<Buffer*>(pDstBuffer);
    auto pWrapSrc = static_cast<Texture*>(pSrcTexture);
    A3D_ASSERT(pWrapDst != nullptr);
    A3D_ASSERT(pWrapSrc != nullptr);

    auto srcDesc = pWrapSrc->GetDesc();

    auto rowPitch   = static_cast<uint64_t>(srcDesc.Width * ToByte(srcDesc.Format));
    auto slicePitch = rowPitch * srcDesc.Height;

    auto offset = srcOffset.X + srcOffset.Y * rowPitch + srcOffset.Z * slicePitch;

    auto rowBytes = static_cast<uint64_t>(srcExtent.Width * ToByte(srcDesc.Format));
    auto byteCount = rowBytes 
                   + rowBytes * srcExtent.Height
                   + rowBytes * srcExtent.Height * srcExtent.Depth;

    m_pCommandList->CopyBufferRegion(
        pWrapDst->GetD3D12Resource(),
        dstOffset,
        pWrapSrc->GetD3D12Resource(),
        offset,
        byteCount);
}

//-------------------------------------------------------------------------------------------------
//      マルチサンプリングされたリソースをマルチサンプリングされていないリソースにコピーします
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

    auto pWrapDst = static_cast<Texture*>(pDstResource);
    auto pWrapSrc = static_cast<Texture*>(pSrcResource);
    A3D_ASSERT(pWrapDst != nullptr);
    A3D_ASSERT(pWrapSrc != nullptr);

    auto desc = pWrapDst->GetDesc();
    auto dstFormat = ToNativeFormat(desc.Format);

    m_pCommandList->ResolveSubresource(
        pWrapDst->GetD3D12Resource(),
        dstSubresource,
        pWrapSrc->GetD3D12Resource(),
        srcSubresource,
        dstFormat
    );
}

//-------------------------------------------------------------------------------------------------
//      バンドルを実行します.
//-------------------------------------------------------------------------------------------------
void CommandList::ExecuteBundle(ICommandList* pCommandList)
{
    if (pCommandList == nullptr)
    { return; }

    auto pWrapCommandList = static_cast<CommandList*>(pCommandList);
    A3D_ASSERT(pWrapCommandList != nullptr);

    auto pNativeCommandList = pWrapCommandList->GetD3D12GraphicsCommandList();
    A3D_ASSERT(pNativeCommandList != nullptr);

    m_pCommandList->ExecuteBundle(pNativeCommandList);
}

//-------------------------------------------------------------------------------------------------
//      デバッグマーカーをプッシュします.
//-------------------------------------------------------------------------------------------------
void CommandList::PushMarker(const char* tag)
{ PIXBeginEvent(m_pCommandList, 0, tag); }

//-------------------------------------------------------------------------------------------------
//      デバッグマーカーをポップします.
//-------------------------------------------------------------------------------------------------
void CommandList::PopMarker()
{ PIXEndEvent(); }

//-------------------------------------------------------------------------------------------------
//      コマンドリストの記録を終了します.
//-------------------------------------------------------------------------------------------------
void CommandList::End()
{
    m_pCommandList->Close();
    m_pFrameBuffer = nullptr;
}

//-------------------------------------------------------------------------------------------------
//      コマンドアロケータを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12CommandAllocator* CommandList::GetD3D12Allocator() const
{ return m_pCommandAllocator; }

//-------------------------------------------------------------------------------------------------
//      グラフィックスコマンドリストを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12GraphicsCommandList* CommandList::GetD3D12GraphicsCommandList() const
{ return m_pCommandList; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool CommandList::Create
(
    IDevice*         pDevice, 
    COMMANDLIST_TYPE listType,
    ICommandList**   ppComandList
)
{
    if (pDevice == nullptr || ppComandList == nullptr)
    { return false; }

    auto instance = new CommandList;
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, listType))
    {
        SafeRelease(instance);
        return false;
    }

    *ppComandList = instance;
    return true;
}

} // namespace a3d
