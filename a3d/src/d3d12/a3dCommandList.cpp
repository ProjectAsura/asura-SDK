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


namespace {

D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE ToNativeCopyMode(a3d::ACCELERATION_STRUCTURE_COPY_MODE mode)
{
    static const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE kMode[] = {
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_CLONE,
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_COMPACT,
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_SERIALIZE,
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_DESERIALIZE
    };

    return kMode[mode];
}

} // namespace


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
    {
        A3D_LOG("Erorr : Invalid Argument.");
        return false;
    }

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
        {
            A3D_LOG("Error : ID3D12Device::CreateCommandAllocator() Failed. errcode = 0x%x", hr);
            return false;
        }

        hr = pNativeDevice->CreateCommandList( 0, type, m_pCommandAllocator, nullptr, IID_PPV_ARGS(&m_pCommandList) );
        if ( FAILED(hr) )
        {
            A3D_LOG("Error : ID3D12Device::CreateCommandList() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    m_pCommandList->Close();
    m_Type = listType;

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
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetName(const char* name)
{
    m_Name = name;
    m_pCommandList->SetPrivateData(WKPDID_D3DDebugObjectName, m_Name.size(), m_Name.c_str());
}

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* CommandList::GetName() const
{ return m_Name.c_str(); }

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
void CommandList::BeginFrameBuffer
(
    uint32_t                        renderTargetViewCount,
    IRenderTargetView**             pRenderTargetViews,
    IDepthStencilView*              pDepthStencilView,
    uint32_t                        clearColorCount,
    const ClearColorValue*          pClearColors,
    const ClearDepthStencilValue*   pClearDepthStencil
)
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[MAX_RTV_COUNT] = {};
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};

    if (pRenderTargetViews != nullptr) {
        for(auto i=0u; i<renderTargetViewCount; ++i)
        {
            auto pWrapperRTV = static_cast<const RenderTargetView*>(pRenderTargetViews[i]);
            A3D_ASSERT(pWrapperRTV != nullptr);
            rtvHandles[i] = pWrapperRTV->GetDescriptor()->GetHandleCPU();
        }
    }
    if (pDepthStencilView != nullptr)
    {
        auto pWrapperDSV = static_cast<const DepthStencilView*>(pDepthStencilView);
        A3D_ASSERT(pWrapperDSV != nullptr);
        dsvHandle = pWrapperDSV->GetDescriptor()->GetHandleCPU();
    }

    m_pCommandList->OMSetRenderTargets(
        renderTargetViewCount,
        (pRenderTargetViews != nullptr) ? rtvHandles : nullptr,
        FALSE,
        (pDepthStencilView != nullptr) ? &dsvHandle : nullptr);

    if (pClearColors != nullptr && clearColorCount > 0)
    {
        for(auto i=0u; i<clearColorCount; ++i)
        {
            auto index = pClearColors[i].SlotIndex;
            float colors[4] = { 
                pClearColors[i].R,
                pClearColors[i].G,
                pClearColors[i].B,
                pClearColors[i].A
            };
            m_pCommandList->ClearRenderTargetView(rtvHandles[index], colors, 0, nullptr);
        }
    }

    if (pClearDepthStencil != nullptr)
    {
        D3D12_CLEAR_FLAGS flags = {};
        if (pClearDepthStencil->EnableClearDepth)
        { flags |= D3D12_CLEAR_FLAG_DEPTH; }
        if (pClearDepthStencil->EnableClearStencil)
        { flags |= D3D12_CLEAR_FLAG_STENCIL; }

        m_pCommandList->ClearDepthStencilView(
            dsvHandle,
            flags,
            pClearDepthStencil->Depth,
            pClearDepthStencil->Stencil,
            0, nullptr);
    }
}

//-------------------------------------------------------------------------------------------------
//      フレームバッファを解除します.
//-------------------------------------------------------------------------------------------------
void CommandList::EndFrameBuffer()
{ m_pCommandList->OMSetRenderTargets(0, nullptr, FALSE, nullptr); }

//------------------------------------------------------------------------------------------------
//      加速機構を構築します.
//------------------------------------------------------------------------------------------------
void CommandList::BuildAccelerationStructure(IAccelerationStructure* pAS)
{
    if (pAS == nullptr)
    { return; }

    auto pWrapAS = static_cast<AccelerationStructure*>(pAS);
    A3D_ASSERT(pWrapAS != nullptr);

    pWrapAS->Build(m_pCommandList);
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

    if (pPipelineState->GetType() == PIPELINE_STATE_TYPE_RAYTRACING)
    {
        auto pWrapPipelineState = static_cast<RayTracingPipelineState*>(pPipelineState);
        A3D_ASSERT(pWrapPipelineState != nullptr);

        pWrapPipelineState->Issue(this);
        m_IsGraphics = false;

        m_HandleCount = pWrapPipelineState->GetDescriptorSetLayout()->GetDesc().EntryCount;
        m_DirtyDescriptor = true;
    }
    else
    {
        auto pWrapPipelineState = static_cast<PipelineState*>(pPipelineState);
        A3D_ASSERT(pWrapPipelineState != nullptr);

        pWrapPipelineState->Issue(this);
        m_IsGraphics = pWrapPipelineState->IsGraphics();

        m_HandleCount = pWrapPipelineState->GetDescriptorSetLayout()->GetDesc().EntryCount;
        m_DirtyDescriptor = true;
    }
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
    {
        D3D12_VERTEX_BUFFER_VIEW views[D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = {};
        m_pCommandList->IASetVertexBuffers(0, D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, views);
        return;
    }

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
    {
        D3D12_INDEX_BUFFER_VIEW view = {};
        m_pCommandList->IASetIndexBuffer(&view);
        return;
    }

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
//      定数バッファビューを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetView(uint32_t index, IConstantBufferView* const pResource)
{
    if (pResource == nullptr)
    { return; }

    auto pWrapView = static_cast<ConstantBufferView*>(pResource);
    A3D_ASSERT(pWrapView != nullptr);

    m_Handles[index] = pWrapView->GetDescriptor()->GetHandleGPU();
    m_DirtyDescriptor = true;
}

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetView(uint32_t index, IShaderResourceView* const pResource)
{
    if (pResource == nullptr || index >= MAX_DESCRIPTOR_COUNT)
    { return; }

    auto pWrapView = static_cast<ShaderResourceView*>(pResource);
    A3D_ASSERT(pWrapView != nullptr);

    m_Handles[index]    = pWrapView->GetDescriptor()->GetHandleGPU();
    m_DirtyDescriptor   = true;
}

//-------------------------------------------------------------------------------------------------
//      アンオーダードアクセスビューを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetView(uint32_t index, IUnorderedAccessView* const pResource)
{
    if (pResource == nullptr || index >= MAX_DESCRIPTOR_COUNT)
    { return; }

    auto pWrapView = static_cast<UnorderedAccessView*>(pResource);
    A3D_ASSERT(pWrapView != nullptr);

    m_Handles[index]    = pWrapView->GetDescriptor()->GetHandleGPU();
    m_DirtyDescriptor   = true;
}

//-------------------------------------------------------------------------------------------------
//      サンプラーを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetSampler(uint32_t index, ISampler* const pSampler)
{
    if (pSampler == nullptr || index >= MAX_DESCRIPTOR_COUNT)
    { return; }

    auto pWrapSmp = static_cast<Sampler*>(pSampler);
    A3D_ASSERT(pWrapSmp != nullptr);

    m_Handles[index]    = pWrapSmp->GetDescriptor()->GetHandleGPU();
    m_DirtyDescriptor   = true;
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
    if (pResource == nullptr || prevState == nextState)
    { return; }

    auto pWrapResource = static_cast<Texture*>(pResource);
    A3D_ASSERT(pWrapResource != nullptr);

    if (m_Type == COMMANDLIST_TYPE_COMPUTE)
    {
        if (prevState == RESOURCE_STATE_UNORDERED_ACCESS)
        {
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barrier.UAV.pResource   = pWrapResource->GetD3D12Resource();

            m_pCommandList->ResourceBarrier(1, &barrier);
        }
    }
    else
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource   = pWrapResource->GetD3D12Resource();
        barrier.Transition.StateBefore = ToNativeState(prevState);
        barrier.Transition.StateAfter  = ToNativeState(nextState);
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        m_pCommandList->ResourceBarrier(1, &barrier);
    }
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
    if (pResource == nullptr || prevState == nextState)
    { return; }

    auto pWrapResource = static_cast<Buffer*>(pResource);
    A3D_ASSERT(pWrapResource != nullptr);

    auto heapType = pWrapResource->GetDesc().HeapType;

    // D3D12では UPLOAD と READBACK はリソースステートを仕様上変更できないため，
    // 実行せずに正常終了させる.
    if (heapType == HEAP_TYPE_UPLOAD || heapType == HEAP_TYPE_READBACK)
    { return;}

    if (m_Type == COMMANDLIST_TYPE_COMPUTE)
    {
        if (prevState == RESOURCE_STATE_UNORDERED_ACCESS)
        {
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barrier.UAV.pResource   = pWrapResource->GetD3D12Resource();
        }
    }
    else
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource   = pWrapResource->GetD3D12Resource();
        barrier.Transition.StateBefore = ToNativeState(prevState);
        barrier.Transition.StateAfter  = ToNativeState(nextState);
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        m_pCommandList->ResourceBarrier(1, &barrier);
    }
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
{
    UpdateDescriptor();
    m_pCommandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
}

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
    UpdateDescriptor();
    m_pCommandList->DrawIndexedInstanced(
        indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

//-------------------------------------------------------------------------------------------------
//      コンピュートシェーダを起動します.
//-------------------------------------------------------------------------------------------------
void CommandList::DispatchCompute(uint32_t x, uint32_t y, uint32_t z)
{
    UpdateDescriptor();
    m_pCommandList->Dispatch(x, y, z);
}

//-------------------------------------------------------------------------------------------------
//      メッシュシェーダ(あるいは増幅シェーダ)を起動します.
//-------------------------------------------------------------------------------------------------
void CommandList::DispatchMesh(uint32_t x, uint32_t y, uint32_t z)
{
    UpdateDescriptor();
    m_pCommandList->DispatchMesh(x, y, z);
}

//-------------------------------------------------------------------------------------------------
//      レイトレーシングを行います.
//-------------------------------------------------------------------------------------------------
void CommandList::TraceRays(const TraceRayArguments* pArgs)
{
    if (pArgs == nullptr)
    { return; }

    D3D12_DISPATCH_RAYS_DESC desc = {};
    desc.RayGenerationShaderRecord.StartAddress = pArgs->RayGeneration.StartAddress;
    desc.RayGenerationShaderRecord.SizeInBytes  = pArgs->RayGeneration.Size;

    desc.MissShaderTable.StartAddress       = pArgs->MissShaders.StartAddress;
    desc.MissShaderTable.SizeInBytes        = pArgs->MissShaders.Size;
    desc.MissShaderTable.StrideInBytes      = pArgs->MissShaders.Stride;

    desc.HitGroupTable.StartAddress         = pArgs->HitShaders.StartAddress;
    desc.HitGroupTable.SizeInBytes          = pArgs->HitShaders.Size;
    desc.HitGroupTable.StrideInBytes        = pArgs->HitShaders.Stride;

    desc.CallableShaderTable.StartAddress   = pArgs->CallableShaders.StartAddress;
    desc.CallableShaderTable.SizeInBytes    = pArgs->CallableShaders.Size;
    desc.CallableShaderTable.StrideInBytes  = pArgs->CallableShaders.Stride;

    desc.Width  = pArgs->Width;
    desc.Height = pArgs->Height;
    desc.Depth  = pArgs->Depth;

    m_pCommandList->DispatchRays(&desc);
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

    UpdateDescriptor();

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
//      クエリをリセットします.
//-------------------------------------------------------------------------------------------------
void CommandList::ResetQuery(IQueryPool* pQuery)
{
    A3D_UNUSED(pQuery);
    /* DO_NOTHING */
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
    A3D_UNUSED(srcSubresource);

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
//      加速機構をコピーします.
//-------------------------------------------------------------------------------------------------
void CommandList::CopyAccelerationStructure
(
    IAccelerationStructure*             pDstAS,
    IAccelerationStructure*             pSrcAS,
    ACCELERATION_STRUCTURE_COPY_MODE    mode
)
{
    if (pDstAS == nullptr || pSrcAS == nullptr)
    { return; }

    auto pWrapDstAS = static_cast<AccelerationStructure*>(pDstAS);
    auto pWrapSrcAS = static_cast<AccelerationStructure*>(pSrcAS);
    A3D_ASSERT(pWrapDstAS != nullptr);
    A3D_ASSERT(pWrapSrcAS != nullptr);

    auto dstAddress = pWrapDstAS->GetD3D12Resource()->GetGPUVirtualAddress();
    auto srcAddress = pWrapSrcAS->GetD3D12Resource()->GetGPUVirtualAddress();

    m_pCommandList->CopyRaytracingAccelerationStructure(
        dstAddress,
        srcAddress,
        ToNativeCopyMode(mode));
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
    m_HandleCount       = 0;
    m_DirtyDescriptor   = false;
}

//-------------------------------------------------------------------------------------------------
//      コマンドアロケータを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12CommandAllocator* CommandList::GetD3D12Allocator() const
{ return m_pCommandAllocator; }

//-------------------------------------------------------------------------------------------------
//      グラフィックスコマンドリストを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12GraphicsCommandList6* CommandList::GetD3D12GraphicsCommandList() const
{ return m_pCommandList; }

//-------------------------------------------------------------------------------------------------
//      ディスクリプタを更新します.
//-------------------------------------------------------------------------------------------------
void CommandList::UpdateDescriptor()
{
    if (!m_DirtyDescriptor)
    { return; }

    if (m_IsGraphics)
    {
        for(auto i=0u; i<m_HandleCount; ++i)
        { m_pCommandList->SetGraphicsRootDescriptorTable(i, m_Handles[i]); }
    }
    else
    {
        for(auto i=0u; i<m_HandleCount; ++i)
        { m_pCommandList->SetComputeRootDescriptorTable(i, m_Handles[i]); }
    }

    m_DirtyDescriptor = false;
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool CommandList::Create
(
    IDevice*                pDevice, 
    const CommandListDesc*  pDesc,
    ICommandList**          ppComandList
)
{
    if (pDevice == nullptr || pDesc == nullptr || ppComandList == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new CommandList();
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if (!instance->Init(pDevice, pDesc->Type))
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppComandList = instance;
    return true;
}

} // namespace a3d
