//-------------------------------------------------------------------------------------------------
// File : a3dQueue.cpp
// Desc : Command Queue.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Queue class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Queue::Queue()
: m_RefCount        (1)
, m_pDevice         (nullptr)
, m_MaxSubmitCount  (0)
, m_SubmitIndex     (0)
, m_Frequency       (0)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Queue::~Queue()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool Queue::Init(IDevice* pDevice, COMMANDLIST_TYPE type, uint32_t maxSubmitCount)
{
    if (pDevice == nullptr)
    { return false; }

    // NOTE : Deviceから呼ばれるので，参照カウントを増やしてまうと
    // Device が解放されなくなるので AddRef() しないこと!!
    m_pDevice = static_cast<Device*>(pDevice);

    m_MaxSubmitCount = maxSubmitCount;
    m_SubmitIndex = 0;

    m_pCommandLists = new CommandList* [m_MaxSubmitCount];
    if (m_pCommandLists == nullptr)
    { return false; }

    memset(m_pCommandLists, 0, sizeof(CommandList*) * m_MaxSubmitCount);

    auto pD3D11Device = m_pDevice->GetD3D11Device();
    A3D_ASSERT(pD3D11Device != nullptr);

    {
        D3D11_QUERY_DESC desc = {};
        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;

        auto hr = pD3D11Device->CreateQuery(&desc, &m_pQuery);
        if ( FAILED(hr) )
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Queue::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    m_pDevice->GetD3D11DeviceContext()->ClearState();

    if (m_pCommandLists != nullptr)
    {
        delete[] m_pCommandLists;
        m_pCommandLists = nullptr;
    }

    SafeRelease(m_pQuery);
    SafeRelease(m_pDevice);

    m_SubmitIndex    = 0;
    m_MaxSubmitCount = 0;
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void Queue::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void Queue::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Queue::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void Queue::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef();}
}

//-------------------------------------------------------------------------------------------------
//      コマンドリストを登録します.
//-------------------------------------------------------------------------------------------------
bool Queue::Submit( ICommandList* pCommandList )
{
    A3D_UNUSED(pCommandList);
    std::lock_guard<std::mutex> locker(m_Mutex);

    if (m_SubmitIndex + 1 >= m_MaxSubmitCount)
    { return false; }

    m_pCommandLists[m_SubmitIndex] = static_cast<CommandList*>(pCommandList);
    m_SubmitIndex++;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      登録したコマンドリストを実行します.
//-------------------------------------------------------------------------------------------------
void Queue::Execute( IFence* pFence )
{
    auto pD3D11DeviceContext = m_pDevice->GetD3D11DeviceContext();
    A3D_ASSERT(pD3D11DeviceContext != nullptr);

    // Freuencyを取得.
    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT data;
    if (pD3D11DeviceContext->GetData(m_pQuery, &data, sizeof(data), 0) != S_FALSE)
    { m_Frequency = data.Frequency; }

    pD3D11DeviceContext->Begin(m_pQuery);

    ParseCmd();
    m_SubmitIndex = 0;

    if (pFence != nullptr)
    {
        auto pWrapFence = reinterpret_cast<Fence*>(pFence);
        A3D_ASSERT(pWrapFence != nullptr);

        pD3D11DeviceContext->End(pWrapFence->GetD3D11Query());
    }

    pD3D11DeviceContext->End(m_pQuery);
}

//-------------------------------------------------------------------------------------------------
//      コマンドの実行が完了するまで待機します.
//-------------------------------------------------------------------------------------------------
void Queue::WaitIdle()
{
    auto pD3D11DeviceContext = m_pDevice->GetD3D11DeviceContext();
    A3D_ASSERT(pD3D11DeviceContext != nullptr);

    while(pD3D11DeviceContext->GetData(m_pQuery, nullptr, 0, 0) == S_FALSE)
    { /* DO_NOTHING */ }
}

//-------------------------------------------------------------------------------------------------
//      コマンドを解析します.
//-------------------------------------------------------------------------------------------------
void Queue::ParseCmd()
{
    auto pDeviceContext = m_pDevice->GetD3D11DeviceContext();
    A3D_ASSERT(pDeviceContext != nullptr);

    bool end = false;
    FrameBuffer*     pActiveFrameBuffer   = nullptr;
    DescriptorSet*   pActiveDescriptorSet = nullptr;
    float            blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    uint32_t         stencilRef     = 0;

    for(auto i=0u; i<m_SubmitIndex; ++i)
    {
        auto pCmd = m_pCommandLists[i]->GetCommandBuffer()->GetBuffer();

        while(pCmd != m_pCommandLists[i]->GetCommandBuffer()->GetCmdPtr() || !end)
        {
            auto type = reinterpret_cast<ImCmdBase*>(pCmd)->Type; 

            // 中間コマンドからネイティブに変換します.
            switch(type)
            {
            case CMD_BEGIN:
                {
                    auto cmd = reinterpret_cast<ImCmdBegin*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);
                    pCmd += sizeof(ImCmdBegin);
                }
                break;

            case CMD_SUB_BEGIN:
                {
                    auto cmd = reinterpret_cast<ImCmdBegin*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);
                    /* DO_NOTHING */
                    pCmd += sizeof(ImCmdBegin);
                }
                break;

            case CMD_SET_FRAME_BUFFER:
                {
                    auto cmd = reinterpret_cast<ImCmdSetFrameBuffer*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    pActiveFrameBuffer = static_cast<FrameBuffer*>(cmd->pFrameBuffer);
                    if (pActiveFrameBuffer != nullptr)
                    { pActiveFrameBuffer->Bind(pDeviceContext); }
                    else
                    {
                        pDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
                        pActiveFrameBuffer = nullptr;
                    }

                    pCmd += sizeof(ImCmdSetFrameBuffer);
                }
                break;

            case CMD_CLEAR_FRAME_BUFFER:
                {
                    auto cmd = reinterpret_cast<ImCmdClearFrameBuffer*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    if (pActiveFrameBuffer != nullptr)
                    {
                        pActiveFrameBuffer->Clear(
                            pDeviceContext,
                            cmd->ClearColorCount,
                            cmd->ClearColors,
                            ((cmd->HasDepth) ? &cmd->ClearDepthStencil : nullptr));
                    }

                    pCmd += sizeof(ImCmdClearFrameBuffer);
                }
                break;

            case CMD_SET_BLEND_CONSTANT:
                {
                    auto cmd = reinterpret_cast<ImCmdSetBlendConstant*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    memcpy( blendFactor, cmd->BlendConstant, sizeof(blendFactor) );

                    pCmd += sizeof(ImCmdSetBlendConstant);
                }
                break;

            case CMD_SET_STENCIL_REFERENCE:
                {
                    auto cmd = reinterpret_cast<ImCmdSetStencilReference*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    stencilRef = cmd->StencilReference;

                    pCmd += sizeof(ImCmdSetStencilReference);
                }
                break;

            case CMD_SET_VIEWPORTS:
                {
                    auto cmd = reinterpret_cast<ImCmdSetViewports*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    D3D11_VIEWPORT viewports[16];
                    for(auto i=0u; i<cmd->Count; ++i)
                    {
                        viewports[i].TopLeftX   = cmd->Viewports[i].X;
                        viewports[i].TopLeftY   = cmd->Viewports[i].Y;
                        viewports[i].Width      = cmd->Viewports[i].Width;
                        viewports[i].Height     = cmd->Viewports[i].Height;
                        viewports[i].MinDepth   = cmd->Viewports[i].MinDepth;
                        viewports[i].MaxDepth   = cmd->Viewports[i].MaxDepth;
                    }

                    pDeviceContext->RSSetViewports(cmd->Count, viewports);

                    pCmd += sizeof(ImCmdSetViewports);
                }
                break;

            case CMD_SET_SCISSORS:
                {
                    auto cmd = reinterpret_cast<ImCmdSetScissors*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    D3D11_RECT rects[16];
                    for(auto i=0u; i<cmd->Count; ++i)
                    {
                        rects[i].left   = cmd->Rects[i].Offset.X;
                        rects[i].right  = cmd->Rects[i].Offset.X + cmd->Rects[i].Extent.Width;
                        rects[i].top    = cmd->Rects[i].Offset.Y;
                        rects[i].bottom = cmd->Rects[i].Offset.Y + cmd->Rects[i].Extent.Height;
                    }

                    pDeviceContext->RSSetScissorRects(cmd->Count, rects);

                    pCmd += sizeof(ImCmdSetScissors);
                }
                break;

            case CMD_SET_PIPELINESTATE:
                {
                    auto cmd = reinterpret_cast<ImCmdSetPipelineState*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pPipelineState = static_cast<PipelineState*>(cmd->pPipelineState);
                    pPipelineState->Bind(pDeviceContext, blendFactor, stencilRef);

                    pCmd += sizeof(ImCmdSetPipelineState);
                }
                break;

            case CMD_SET_DESCRIPTORSET:
                {
                    auto cmd = reinterpret_cast<ImCmdSetDescriptorSet*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);
                    pActiveDescriptorSet = static_cast<DescriptorSet*>(cmd->pDescriptorSet);
                    pActiveDescriptorSet->Bind(pDeviceContext);
                    pActiveDescriptorSet->UpdateSubreosurce(pDeviceContext);
                    pCmd += sizeof(ImCmdSetDescriptorSet);
                }
                break;

            case CMD_SET_VERTEX_BUFFERS:
                {
                    auto cmd = reinterpret_cast<ImCmdSetVertexBuffers*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    ID3D11Buffer* pBuffers[32];
                    uint32_t strides[32];
                    uint32_t offsets[32];

                    for(auto i=0u; i<cmd->Count; ++i)
                    {
                        auto pWrapBuffer = static_cast<Buffer*>(cmd->pBuffers[i]);
                        A3D_ASSERT(pWrapBuffer != nullptr);

                        pBuffers[i] = pWrapBuffer->GetD3D11Buffer();
                        strides [i] = pWrapBuffer->GetDesc().Stride;
                        offsets [i] = static_cast<uint32_t>(cmd->Offsets[i]);
                    }

                    pDeviceContext->IASetVertexBuffers(
                        cmd->StartSlot,
                        cmd->Count,
                        pBuffers,
                        strides,
                        offsets);

                    pCmd += sizeof(ImCmdSetVertexBuffers);
                }
                break;

            case CMD_SET_INDEX_BUFFER:
                {
                    auto cmd = reinterpret_cast<ImCmdSetIndexBuffer*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pWrapBuffer = static_cast<Buffer*>(cmd->pBuffer);
                    auto format = pWrapBuffer->GetDesc().Stride == sizeof(uint16_t)
                                    ? DXGI_FORMAT_R16_UINT
                                    : DXGI_FORMAT_R32_UINT;

                    pDeviceContext->IASetIndexBuffer(
                        pWrapBuffer->GetD3D11Buffer(),
                        format,
                        static_cast<uint32_t>(cmd->Offset));

                    pCmd += sizeof(ImCmdSetIndexBuffer);
                }
                break;

            case CMD_TEXTURE_BARRIER:
                {
                    auto cmd = reinterpret_cast<ImCmdTextureBarrier*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pTexture = static_cast<Texture*>(cmd->pResource);
                    pTexture->SetState(cmd->NextState);

                    pDeviceContext->Flush();

                    pCmd += sizeof(ImCmdTextureBarrier);
                }
                break;

            case CMD_BUFFER_BARRIER:
                {
                    auto cmd = reinterpret_cast<ImCmdBufferBarrier*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pBuffer = reinterpret_cast<Buffer*>(cmd->pResource);
                    pBuffer->SetState(cmd->NextState);

                    pDeviceContext->Flush();

                    pCmd += sizeof(ImCmdBufferBarrier);
                }
                break;

            case CMD_DRAW_INSTANCED:
                {
                    auto cmd = reinterpret_cast<ImCmdDrawInstanced*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    pDeviceContext->DrawInstanced(
                        cmd->VertexCount,
                        cmd->InstanceCount,
                        cmd->FirstVertex,
                        cmd->FirstInstance);

                    pCmd += sizeof(ImCmdDrawIndexedInstanced);
                }
                break;

            case CMD_DRAW_INDEXED_INSTANCED:
                {
                    auto cmd = reinterpret_cast<ImCmdDrawIndexedInstanced*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    pDeviceContext->DrawIndexedInstanced(
                        cmd->IndexCount,
                        cmd->InstanceCount,
                        cmd->FirstIndex,
                        cmd->VertexOffset,
                        cmd->FirstInstance);

                    pCmd += sizeof(ImCmdDrawIndexedInstanced);
                }
                break;

            case CMD_DISPATCH:
                {
                    auto cmd = reinterpret_cast<ImCmdDispatch*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    pDeviceContext->Dispatch(
                        cmd->X,
                        cmd->Y,
                        cmd->Z);

                    pCmd += sizeof(ImCmdDispatch);
                }
                break;

            case CMD_EXECUTE_INDIRECT:
                {
                    auto cmd = reinterpret_cast<ImCmdExecuteIndirect*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pWrapCommandSet = static_cast<CommandSet*>(cmd->pCommandSet);
                    A3D_ASSERT(pWrapCommandSet != nullptr);

                    auto& desc = pWrapCommandSet->GetDesc();

                    auto pWrapArgumentBuffer = static_cast<Buffer*>(cmd->pArgumentBuffer);
                    A3D_ASSERT(pWrapArgumentBuffer != nullptr);

                    auto pNativeArgumentBuffer = pWrapArgumentBuffer->GetD3D11Buffer();
                    A3D_ASSERT(pNativeArgumentBuffer != nullptr);

                    uint32_t* pCounters = nullptr;
                    if (cmd->pCounterBuffer != nullptr)
                    { pCounters = static_cast<uint32_t*>(cmd->pCounterBuffer->Map()); }

                    auto offset = static_cast<uint32_t>(cmd->ArgumentBufferOffset);
                    for(auto i=0u; i<desc.ArgumentCount; ++i)
                    {
                        auto count = cmd->MaxCommandCount;
                        if (pCounters != nullptr)
                        { count = (pCounters[i] < cmd->MaxCommandCount ) ? pCounters[i] : cmd->MaxCommandCount; }

                        switch(desc.pArguments[i])
                        {
                        case INDIRECT_ARGUMENT_TYPE_DRAW:
                            { pDeviceContext->DrawInstancedIndirect(pNativeArgumentBuffer, offset); }
                            break;

                        case INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED:
                            { pDeviceContext->DrawIndexedInstancedIndirect(pNativeArgumentBuffer, offset); }
                            break;

                        case INDIRECT_ARGUMENT_TYPE_DISPATCH:
                            { pDeviceContext->DispatchIndirect(pNativeArgumentBuffer, offset); }
                            break;
                        }

                        offset += desc.ByteStride;
                    }

                    pCmd += sizeof(ImCmdExecuteIndirect);
                }
                break;

            case CMD_BEGIN_QUERY:
                {
                    auto cmd = reinterpret_cast<ImCmdBeginQuery*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pQuery = static_cast<QueryPool*>(cmd->pQuery);
                    auto pD3D11Query = pQuery->GetD3D11Query(cmd->Index);

                    pDeviceContext->Begin(pD3D11Query);
                    pCmd += sizeof(ImCmdBeginQuery);
                }
                break;

            case CMD_END_QUERY:
                {
                    auto cmd = reinterpret_cast<ImCmdEndQuery*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pQuery = static_cast<QueryPool*>(cmd->pQuery);
                    auto pD3D11Query = pQuery->GetD3D11Query(cmd->Index);

                    pDeviceContext->End(pD3D11Query);

                    pCmd += sizeof(ImCmdBeginQuery);
                }
                break;

            case CMD_RESOLVE_QUERY:
                {
                    auto cmd = reinterpret_cast<ImCmdResolveQuery*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pWrapQuery = static_cast<QueryPool*>(cmd->pQuery);
                    A3D_ASSERT(pWrapQuery != nullptr);
                    auto queryType = pWrapQuery->GetDesc().Type;

                    auto pWrapBuffer = static_cast<Buffer*>(cmd->pDstBuffer);
                    A3D_ASSERT(pWrapBuffer != nullptr);

                    auto pDstPtr = static_cast<uint8_t*>(pWrapBuffer->Map()) + cmd->DstOffset;

                    for(auto i=cmd->StartIndex; i<cmd->QueryCount; ++i)
                    {
                        switch(queryType)
                        {
                        case QUERY_TYPE_OCCLUSION:
                            {
                                UINT64 data = 0;
                                pDeviceContext->GetData(pWrapQuery->GetD3D11Query(i), &data, sizeof(data), 0);
                                memcpy(pDstPtr, &data, sizeof(data));
                                pDstPtr += sizeof(data);
                            }
                            break;

                        case QUERY_TYPE_TIMESTAMP:
                            {
                                UINT64 data = 0;
                                pDeviceContext->GetData(pWrapQuery->GetD3D11Query(i), &data, sizeof(data), 0);
                                memcpy(pDstPtr, &data, sizeof(data));
                                pDstPtr += sizeof(data);
                            }
                            break;

                        case QUERY_TYPE_PIPELINE_STATISTICS:
                            {
                                D3D11_QUERY_DATA_PIPELINE_STATISTICS data = {};
                                pDeviceContext->GetData(pWrapQuery->GetD3D11Query(i), &data, sizeof(data), 0);

                                PipelineStatistics convert = {};
                                convert.IAVertices      = data.IAVertices;
                                convert.IAPrimitives    = data.IAPrimitives;
                                convert.VSInvocations   = data.VSInvocations;
                                convert.GSInvocations   = data.GSInvocations;
                                convert.GSPrimitives    = data.GSPrimitives;
                                convert.CInvocations    = data.CInvocations;
                                convert.CPrimitives     = data.CPrimitives;
                                convert.PSInvocations   = data.PSInvocations;
                                convert.HSInvocations   = data.HSInvocations;
                                convert.DSInvocations   = data.DSInvocations;
                                convert.CSInvocations   = data.CSInvocations;

                                memcpy(pDstPtr, &convert, sizeof(convert));
                                pDstPtr += sizeof(convert);
                            }
                            break;
                        }
                    }

                    pWrapBuffer->Unmap();

                    pCmd += sizeof(ImCmdResolveQuery);
                }
                break;
        
            case CMD_COPY_TEXTURE:
                {
                    auto cmd = reinterpret_cast<ImCmdCopyTexture*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pDstTexture = static_cast<Texture*>(cmd->pDstTexture);
                    auto pSrcTexture = static_cast<Texture*>(cmd->pSrcTexture);
                    A3D_ASSERT(pDstTexture != nullptr);
                    A3D_ASSERT(pSrcTexture != nullptr);

                    pDeviceContext->CopyResource(
                        pDstTexture->GetD3D11Resource(),
                        pSrcTexture->GetD3D11Resource());

                    pCmd += sizeof(ImCmdCopyTexture);
                }
                break;

            case CMD_COPY_BUFFER:
                {
                    auto cmd = reinterpret_cast<ImCmdCopyBuffer*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pDstBuffer = static_cast<Buffer*>(cmd->pDstBuffer);
                    auto pSrcBuffer = static_cast<Buffer*>(cmd->pSrcBuffer);
                    A3D_ASSERT(pDstBuffer != nullptr);
                    A3D_ASSERT(pSrcBuffer != nullptr);

                    pDeviceContext->CopyResource(
                        pDstBuffer->GetD3D11Buffer(),
                        pSrcBuffer->GetD3D11Buffer());

                    pCmd += sizeof(ImCmdCopyBuffer);
                }
                break;

            case CMD_COPY_TEXTURE_REGION:
                {
                    auto cmd = reinterpret_cast<ImCmdCopyTextureRegion*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pDstTexture = static_cast<Texture*>(cmd->pDstResource);
                    auto pSrcTexture = static_cast<Texture*>(cmd->pSrcResource);
                    A3D_ASSERT(pDstTexture != nullptr);
                    A3D_ASSERT(pSrcTexture != nullptr);

                    D3D11_BOX box = {};
                    box.left    = cmd->SrcOffset.X;
                    box.right   = cmd->SrcOffset.X + cmd->SrcExtent.Width;
                    box.top     = cmd->SrcOffset.Y;
                    box.bottom  = cmd->SrcOffset.Y + cmd->SrcExtent.Height;
                    box.front   = cmd->SrcOffset.Z;
                    box.back    = cmd->SrcOffset.Z + cmd->SrcExtent.Depth;

                    pDeviceContext->CopySubresourceRegion(
                        pDstTexture->GetD3D11Resource(),
                        cmd->DstSubresource,
                        cmd->DstOffset.X,
                        cmd->DstOffset.Y,
                        cmd->DstOffset.Z,
                        pSrcTexture->GetD3D11Resource(),
                        cmd->SrcSubresource,
                        &box);

                    pCmd += sizeof(ImCmdCopyTextureRegion);
                }
                break;

            case CMD_COPY_BUFFER_REGION:
                {
                    auto cmd = reinterpret_cast<ImCmdCopyBufferRegion*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pDstBuffer = static_cast<Buffer*>(cmd->pDstBuffer);
                    auto pSrcBuffer = static_cast<Buffer*>(cmd->pSrcBuffer);
                    A3D_ASSERT(pDstBuffer != nullptr);
                    A3D_ASSERT(pSrcBuffer != nullptr);

                    D3D11_MAPPED_SUBRESOURCE dstMap = {};
                    D3D11_MAPPED_SUBRESOURCE srcMap = {};

                    pDeviceContext->Map(pSrcBuffer->GetD3D11Buffer(), 0, D3D11_MAP_READ, 0, &srcMap);
                    pDeviceContext->Map(pDstBuffer->GetD3D11Buffer(), 0, D3D11_MAP_WRITE, 0, &dstMap);

                    auto pSrcPtr = static_cast<uint8_t*>(srcMap.pData) + cmd->SrcOffset;
                    auto pDstPtr = static_cast<uint8_t*>(dstMap.pData) + cmd->DstOffset;
                    memcpy(pDstPtr, pSrcPtr, static_cast<size_t>(cmd->ByteCount));

                    pDeviceContext->Unmap(pSrcBuffer->GetD3D11Buffer(), 0);
                    pDeviceContext->Unmap(pDstBuffer->GetD3D11Buffer(), 0);

                    pCmd += sizeof(ImCmdCopyBufferRegion);
                }
                break;

            case CMD_COPY_BUFFER_TO_TEXTURE:
                {
                    auto cmd = reinterpret_cast<ImCmdCopyBufferToTexture*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pDstTexture = static_cast<Texture*>(cmd->pDstTexture);
                    auto pSrcBuffer  = static_cast<Buffer*>(cmd->pSrcBuffer);
                    A3D_ASSERT(pDstTexture != nullptr);
                    A3D_ASSERT(pSrcBuffer  != nullptr);

                    auto& dstDesc = pDstTexture->GetDesc();

                    auto subResourceLayout = pDstTexture->GetSubresourceLayout(cmd->DstSubresource);
                    auto pSrcPtr = static_cast<uint8_t*>(pSrcBuffer->Map()) + cmd->SrcOffset;

                    D3D11_BOX dstBox = {};
                    dstBox.left     = cmd->DstOffset.X;
                    dstBox.right    = dstDesc.Width;
                    dstBox.top      = cmd->DstOffset.Y;
                    dstBox.bottom   = dstDesc.Height;
                    dstBox.front    = cmd->DstOffset.Z;
                    dstBox.back     = dstDesc.DepthOrArraySize;

                    pDeviceContext->UpdateSubresource(
                        pDstTexture->GetD3D11Resource(),
                        cmd->DstSubresource,
                        &dstBox,
                        pSrcPtr,
                        static_cast<uint32_t>(subResourceLayout.RowPitch),
                        static_cast<uint32_t>(subResourceLayout.SlicePitch));

                    pCmd += sizeof(ImCmdCopyBufferToTexture);
                }
                break;

            case CMD_COPY_TEXTURE_TO_BUFFER:
                {
                    auto cmd = reinterpret_cast<ImCmdCopyTextureToBuffer*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pDstBuffer  = static_cast<Buffer*>(cmd->pDstBuffer);
                    auto pSrcTexture = static_cast<Texture*>(cmd->pSrcTexture);
                    A3D_ASSERT(pDstBuffer  != nullptr);
                    A3D_ASSERT(pSrcTexture != nullptr);

                    D3D11_MAPPED_SUBRESOURCE srcMap = {};

                    pDeviceContext->Map(
                        pSrcTexture->GetD3D11Resource(),
                        cmd->SrcSubresource,
                        D3D11_MAP_READ,
                        0,
                        &srcMap);

                    D3D11_BOX dstBox = {};
                    dstBox.left     = static_cast<uint32_t>(cmd->DstOffset);
                    dstBox.right    = static_cast<uint32_t>(pDstBuffer->GetDesc().Size);
                    dstBox.top      = 0;
                    dstBox.bottom   = 1;
                    dstBox.front    = 0;
                    dstBox.back     = 1;

                    pDeviceContext->UpdateSubresource(
                        pDstBuffer->GetD3D11Buffer(),
                        0,
                        &dstBox,
                        srcMap.pData,
                        srcMap.RowPitch,
                        srcMap.DepthPitch);

                    pCmd += sizeof(ImCmdCopyTextureToBuffer);
                }
                break;

            case CMD_RESOLVE_SUBRESOURCE:
                {
                    auto cmd = reinterpret_cast<ImCmdResolveSubresource*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pDstTexture = static_cast<Texture*>(cmd->pDstResource);
                    auto pSrcTexture = static_cast<Texture*>(cmd->pSrcResource);
                    A3D_ASSERT(pDstTexture != nullptr);
                    A3D_ASSERT(pSrcTexture != nullptr);

                    pDeviceContext->ResolveSubresource(
                        pDstTexture->GetD3D11Resource(),
                        cmd->DstSubresource,
                        pSrcTexture->GetD3D11Resource(),
                        cmd->SrcSubresource,
                        ToNativeFormat(pDstTexture->GetDesc().Format));

                    pCmd += sizeof(ImCmdResolveSubresource);
                }
                break;

            case CMD_SUB_END:
                {
                    auto cmd = reinterpret_cast<ImCmdEnd*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);
                    /* DO_NOTHING */
                    pCmd += sizeof(ImCmdEnd);
                }
                break;

            case CMD_END:
                {
                    auto cmd = reinterpret_cast<ImCmdEnd*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);
                    end = true;
                    pCmd += sizeof(ImCmdEnd);
                    pDeviceContext->Flush();
                }
                break;
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Queue::Create
(
    IDevice*            pDevice,
    COMMANDLIST_TYPE    type,
    uint32_t            maxSubmitCount,
    IQueue**            ppQueue
)
{
    auto instance = new Queue();
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, type, maxSubmitCount))
    {
        SafeRelease(instance);
        return false;
    }

    *ppQueue = instance;
    return true;
}

} // namespace a3d
