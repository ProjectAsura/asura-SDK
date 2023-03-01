//-------------------------------------------------------------------------------------------------
// File : a3dQueue.cpp
// Desc : Command Queue.
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

//-------------------------------------------------------------------------------------------------
// Constant Values.
//-------------------------------------------------------------------------------------------------
constexpr uint8_t DIRTY_DESCRIPTOR  = 0x1 << 0;
constexpr uint8_t DIRTY_COMPUTE     = 0x1 << 1;

} // namespace


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
, m_pCommandLists   (nullptr)
, m_pQuery          (nullptr)
, m_Frequency       (0)
, m_pLayoutDesc     (nullptr)
, m_pCB             (nullptr)
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
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

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
        {
            A3D_LOG("Error : ID3D11Device::CreateQuery() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth              = sizeof(DWORD) * 64;
        desc.Usage                  = D3D11_USAGE_DEFAULT;
        desc.BindFlags              = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags         = 0;
        desc.MiscFlags              = 0;
        desc.StructureByteStride    = 0;

        auto hr = pD3D11Device->CreateBuffer(&desc, nullptr, &m_pCB);
        if (FAILED(hr))
        {
            A3D_LOG("Error : ID3D11Device::CreateBuffer() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

#ifdef A3D_FOR_WINDOWS10
    {
        auto hr = pD3D11Device->CreateFence(0, D3D11_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
        if (FAILED(hr))
        {
            A3D_LOG("Error : ID3D11Device::CreateFence() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_Event = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
        if (m_Event == nullptr)
        {
            A3D_LOG("Error : CreateEventEx() Failed.");
            return false;
        }
    }
#endif

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Queue::Term()
{
    if (m_pDevice == nullptr)
    { return; }

#ifdef A3D_FOR_WINDOWS10
    if (m_pFence != nullptr && m_Event != nullptr)
    { WaitIdle(); }

    if (m_Event != nullptr)
    {
        CloseHandle(m_Event);
        m_Event = nullptr;
    }
    SafeRelease(m_pFence);
#endif

    m_pDevice->GetD3D11DeviceContext()->ClearState();

    if (m_pCommandLists != nullptr)
    {
        delete[] m_pCommandLists;
        m_pCommandLists = nullptr;
    }

    SafeRelease(m_pCB);
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
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void Queue::SetName(const char* name)
{ m_Name = name; }

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* Queue::GetName() const
{ return m_Name.c_str(); }

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
//      コマンドを解析します.
//-------------------------------------------------------------------------------------------------
void Queue::ParseCmd()
{
    auto pDeviceContext = m_pDevice->GetD3D11DeviceContext();
    A3D_ASSERT(pDeviceContext != nullptr);

    bool     end = false;
    float    blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    uint32_t stencilRef     = 0;

    for(auto i=0u; i<m_SubmitIndex; ++i)
    {
        auto pCmd = m_pCommandLists[i]->GetCommandBuffer()->GetBuffer();

        while(pCmd != m_pCommandLists[i]->GetCommandBuffer()->GetCmdPtr() || !end)
        {
            auto id = reinterpret_cast<ImCmdBase*>(pCmd)->Id; 

            // 中間コマンドからネイティブに変換します.
            switch(id)
            {
            case CMD_BEGIN:
                {
                    auto cmd = reinterpret_cast<ImCmdBegin*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);
                    pCmd += sizeof(ImCmdBegin);

                    m_pLayoutDesc = nullptr;
                    for(auto i=0; i<64; ++i)
                    { m_pViews[i] = nullptr; }
                    m_DirtyView = 0;
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

            case CMD_BEGIN_FRAME_BUFFER:
                {
                    if (m_DirtyView & DIRTY_COMPUTE)
                    {
                        ResetDescriptor(pDeviceContext);
                        m_DirtyView &= ~DIRTY_COMPUTE;
                    }

                    auto cmd = reinterpret_cast<ImCmdBeginFrameBuffer*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    ID3D11RenderTargetView* pRTVs[MAX_RTV_COUNT] = {};

                    auto count = cmd->RenderTargetViewCount;
                    for(auto i=0u; i<count; ++i)
                    {
                        auto pWrapperRTV = static_cast<RenderTargetView*>(cmd->pRenderTargetView[i]);
                        pRTVs[i] = pWrapperRTV->GetD3D11RenderTargetView();
                    }

                    ID3D11DepthStencilView* pDSV = nullptr;
                    if (cmd->pDepthStencilView != nullptr)
                    {
                        auto pWrapperDSV = static_cast<DepthStencilView*>(cmd->pDepthStencilView);
                        pDSV = pWrapperDSV->GetD3D11DepthStencilView();
                    }

                    pDeviceContext->OMSetRenderTargets(count, pRTVs, pDSV);

                    count = cmd->ClearColorCount;
                    for(auto i=0u; i<count; ++i)
                    {
                        auto index = cmd->ClearColors[i].SlotIndex;
                        float colors[4] = { 
                            cmd->ClearColors[i].R,
                            cmd->ClearColors[i].G,
                            cmd->ClearColors[i].B,
                            cmd->ClearColors[i].A,
                        };
                        pDeviceContext->ClearRenderTargetView(pRTVs[index], colors);
                    }

                    if (pDSV != nullptr && (cmd->ClearDepthStencil.EnableClearDepth || cmd->ClearDepthStencil.EnableClearStencil))
                    {
                        UINT flags = 0;
                        if (cmd->ClearDepthStencil.EnableClearDepth)
                        { flags = D3D11_CLEAR_DEPTH; }
                        if (cmd->ClearDepthStencil.EnableClearStencil)
                        { flags = D3D11_CLEAR_STENCIL; }

                        pDeviceContext->ClearDepthStencilView(
                            pDSV,
                            flags,
                            cmd->ClearDepthStencil.Depth,
                            cmd->ClearDepthStencil.Stencil);
                    }

                    pCmd += sizeof(ImCmdBeginFrameBuffer);
                }
                break;

            case CMD_END_FRAME_BUFFER:
                {
                    ResetDescriptor(pDeviceContext);

                    ID3D11RenderTargetView* pNullRTVs[MAX_RTV_COUNT] = {};
                    pDeviceContext->OMSetRenderTargets(MAX_RTV_COUNT, pNullRTVs, nullptr);

                    pCmd += sizeof(ImCmdBase);
                }
                break;

            case CMD_BUILD_ACCELERATION_STRUCTURE:
                {
                    auto cmd = reinterpret_cast<ImCmdBuildAccelerationStructure*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    /* 対応するコマンドはありません. */
                    A3D_UNUSED(cmd);

                    pCmd += sizeof(ImCmdBuildAccelerationStructure);
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

                    D3D11_VIEWPORT viewports[16] = {};
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

                    D3D11_RECT rects[16] = {};
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

                    ResetDescriptor(pDeviceContext);

                    auto pLayout = pPipelineState->GetDescriptorSetLayout();
                    m_pLayoutDesc = pLayout->GetDesc();

                    pCmd += sizeof(ImCmdSetPipelineState);
                }
                break;

            case CMD_SET_VERTEX_BUFFERS:
                {
                    auto cmd = reinterpret_cast<ImCmdSetVertexBuffers*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    ID3D11Buffer*   pBuffers[32] = {};
                    uint32_t        strides [32] = {};
                    uint32_t        offsets [32] = {};

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

            case CMD_SET_CONSTANTS:
                {
                    auto cmd = reinterpret_cast<ImCmdSetConstants*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto size   = cmd->Count * 4;
                    auto offset = cmd->Offset;

                    pCmd += sizeof(ImCmdSetConstants);

                    auto pSrc = reinterpret_cast<const void*>(pCmd);
                    pCmd += size;

                    pDeviceContext->UpdateSubresource(
                        m_pCB, offset, nullptr, pSrc, size, 1);

                    auto slot = m_pLayoutDesc->Constant.ShaderRegister;

                    switch (m_pLayoutDesc->Constant.ShaderStage)
                    {
                    case SHADER_STAGE_VS: { pDeviceContext->VSSetConstantBuffers(slot, 1, &m_pCB); } break;
                    case SHADER_STAGE_DS: { pDeviceContext->DSSetConstantBuffers(slot, 1, &m_pCB); } break;
                    case SHADER_STAGE_HS: { pDeviceContext->HSSetConstantBuffers(slot, 1, &m_pCB); } break;
                    case SHADER_STAGE_PS: { pDeviceContext->PSSetConstantBuffers(slot, 1, &m_pCB); } break;
                    case SHADER_STAGE_CS: { pDeviceContext->CSSetConstantBuffers(slot, 1, &m_pCB); } break;
                    default: { /* 対応するコマンドありません. */ } break;
                    }
                }
                break;

            case CMD_SET_CBV:
                {
                    auto cmd = reinterpret_cast<ImCmdSetCBV*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    A3D_ASSERT(cmd->Index < MAX_DESCRIPTOR_COUNT);
                    m_pViews[cmd->Index] = cmd->pView;
                    m_DirtyView = true;

                    pCmd += sizeof(ImCmdSetCBV);
                }
                break;

            case CMD_SET_SRV:
                {
                    auto cmd = reinterpret_cast<ImCmdSetSRV*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    A3D_ASSERT(cmd->Index < MAX_DESCRIPTOR_COUNT);
                    m_pViews[cmd->Index] = cmd->pView;
                    m_DirtyView = true;

                    pCmd += sizeof(ImCmdSetSRV);
                }
                break;

            case CMD_SET_UAV:
                {
                    auto cmd = reinterpret_cast<ImCmdSetUAV*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    A3D_ASSERT(cmd->Index < MAX_DESCRIPTOR_COUNT);
                    m_pViews[cmd->Index] = cmd->pView;
                    m_DirtyView = true;

                    pCmd += sizeof(ImCmdSetUAV);
                }
                break;

            case CMD_SET_SAMPLER:
                {
                    auto cmd = reinterpret_cast<ImCmdSetSampler*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    A3D_ASSERT(cmd->Index < MAX_DESCRIPTOR_COUNT);
                    m_pViews[cmd->Index] = cmd->pSampler;
                    m_DirtyView = true;

                    pCmd += sizeof(ImCmdSetSampler);
                }
                break;

            case CMD_TEXTURE_BARRIER:
                {
                    auto cmd = reinterpret_cast<ImCmdTextureBarrier*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    A3D_UNUSED(cmd);
                    //auto pTexture = static_cast<Texture*>(cmd->pResource);
                    //pDeviceContext->Flush();

                    pCmd += sizeof(ImCmdTextureBarrier);
                }
                break;

            case CMD_BUFFER_BARRIER:
                {
                    auto cmd = reinterpret_cast<ImCmdBufferBarrier*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    A3D_UNUSED(cmd);
                    //auto pBuffer = reinterpret_cast<Buffer*>(cmd->pResource);
                    //pDeviceContext->Flush();

                    pCmd += sizeof(ImCmdBufferBarrier);
                }
                break;

            case CMD_DRAW_INSTANCED:
                {
                    UpdateDescriptor(pDeviceContext);

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

            case CMD_DRAW_INSTANCED_INDIRECT:
                {
                    UpdateDescriptor(pDeviceContext);

                    auto cmd = reinterpret_cast<ImCmdDrawInstancedIndirect*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pWrapArgumentBuffer = static_cast<Buffer*>(cmd->pArgumentBuffer);
                    A3D_ASSERT(pWrapArgumentBuffer != nullptr);

                    pDeviceContext->DrawInstancedIndirect(
                        pWrapArgumentBuffer->GetD3D11Buffer(),
                        UINT(cmd->argumentBufferOffset));

                    pCmd += sizeof(ImCmdDrawInstancedIndirect);
                }
                break;

            case CMD_DRAW_INDEXED_INSTANCED:
                {
                    UpdateDescriptor(pDeviceContext);

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

            case CMD_DRAW_INDEXED_INSTANCED_INDIRECT:
                {
                    UpdateDescriptor(pDeviceContext);

                    auto cmd = reinterpret_cast<ImCmdDrawIndexedInstancedIndirect*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pWrapArgumentBuffer = static_cast<Buffer*>(cmd->pArgumentBuffer);
                    A3D_ASSERT(pWrapArgumentBuffer != nullptr);

                    // agsDriverExtensionsDX11_MultiDrawIndexedIndirectCountIndirect().
                    // NvAPI_D3D11_MultiDrawIndexedInstancedIndirect
                    pDeviceContext->DrawIndexedInstancedIndirect(
                        pWrapArgumentBuffer->GetD3D11Buffer(),
                        UINT(cmd->argumentBufferOffset));

                    pCmd += sizeof(ImCmdDrawIndexedInstancedIndirect);
                }
                break;

            case CMD_DISPATCH_COMPUTE:
                {
                    UpdateDescriptor(pDeviceContext);

                    auto cmd = reinterpret_cast<ImCmdDispatch*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    pDeviceContext->Dispatch(
                        cmd->X,
                        cmd->Y,
                        cmd->Z);

                    m_DirtyView |= DIRTY_COMPUTE;

                    pCmd += sizeof(ImCmdDispatch);
                }
                break;

            case CMD_DISPATCH_COMPUTE_INDIRECT:
                {
                    UpdateDescriptor(pDeviceContext);

                    auto cmd = reinterpret_cast<ImCmdDispatchComputeIndirect*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    auto pWrapArgumentBuffer = static_cast<Buffer*>(cmd->pArgumentBuffer);
                    A3D_ASSERT(pWrapArgumentBuffer != nullptr);

                    pDeviceContext->DispatchIndirect(
                        pWrapArgumentBuffer->GetD3D11Buffer(),
                        UINT(cmd->argumentBufferOffset));

                    pCmd += sizeof(ImCmdDispatchComputeIndirect);
                }
                break;


            case CMD_DISPATCH_MESH:
                {
                    auto cmd = reinterpret_cast<ImCmdDispatch*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    /* 対応するコマンドはありません. */
                    A3D_UNUSED(cmd);

                    pCmd += sizeof(ImCmdDispatch);
                }
                break;

            case CMD_DISPATCH_MESH_INDIRECT:
                {
                    auto cmd = reinterpret_cast<ImCmdDispatchMeshIndirect*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    /* 対応するコマンドはありません. */
                    A3D_UNUSED(cmd);

                    pCmd += sizeof(ImCmdDispatchMeshIndirect);
                }
                break;

            case CMD_TRACE_RAYS:
                {
                    auto cmd = reinterpret_cast<ImCmdTraceRays*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    /* 対応するコマンドはありません. */
                    A3D_UNUSED(cmd);

                    pCmd += sizeof(ImCmdTraceRays);
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

                    pCmd += sizeof(ImCmdEndQuery);
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
                                while( pDeviceContext->GetData(pWrapQuery->GetD3D11Query(i), &data, sizeof(data), 0) != S_OK);
                                memcpy(pDstPtr, &data, sizeof(data));
                                pDstPtr += sizeof(data);
                            }
                            break;

                        case QUERY_TYPE_TIMESTAMP:
                            {
                                UINT64 data = 0;
                                while( pDeviceContext->GetData(pWrapQuery->GetD3D11Query(i), &data, sizeof(data), 0) != S_OK);
                                memcpy(pDstPtr, &data, sizeof(data));
                                pDstPtr += sizeof(data);
                            }
                            break;

                        case QUERY_TYPE_PIPELINE_STATISTICS:
                            {
                                D3D11_QUERY_DATA_PIPELINE_STATISTICS data = {};
                                while( pDeviceContext->GetData(pWrapQuery->GetD3D11Query(i), &data, sizeof(data), 0) != S_OK);

                                PipelineStatistics convert = {};
                                convert.IAVertices              = data.IAVertices;
                                convert.IAPrimitives            = data.IAPrimitives;
                                convert.VSInvocations           = data.VSInvocations;
                                convert.GSInvocations           = data.GSInvocations;
                                convert.GSPrimitives            = data.GSPrimitives;
                                convert.RasterizerInvocations   = data.CInvocations;
                                convert.RenderedPrimitives      = data.CPrimitives;
                                convert.PSInvocations           = data.PSInvocations;
                                convert.HSInvocations           = data.HSInvocations;
                                convert.DSInvocations           = data.DSInvocations;
                                convert.CSInvocations           = data.CSInvocations;

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

            case CMD_RESET_QUERY:
                {
                    /* DO_NOTHING */
                    pCmd += sizeof(ImCmdResetQuery);
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

                    auto dstDesc = pDstTexture->GetDesc();

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

                    pDeviceContext->Unmap(pSrcTexture->GetD3D11Resource(), cmd->SrcSubresource);

                    pCmd += sizeof(ImCmdCopyTextureToBuffer);
                }
                break;

            case CMD_COPY_ACCELERATION_STRUCTURE:
                {
                    auto cmd = reinterpret_cast<ImCmdCopyAccelerationStructure*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    /* 対応するコマンドはありません. */
                    A3D_UNUSED(cmd);

                    pCmd += sizeof(ImCmdCopyAccelerationStructure);
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

            case CMD_PUSH_MARKER:
                {
                    auto cmd = reinterpret_cast<ImCmdPushMarker*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    PIXBeginEvent(pDeviceContext, 0, cmd->Tag);

                    pCmd += sizeof(ImCmdPushMarker);
                }
                break;

            case CMD_POP_MARKER:
                {
                    auto cmd = reinterpret_cast<ImCmdPopMarker*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    PIXEndEvent(pDeviceContext);

                    pCmd += sizeof(ImCmdPopMarker);
                }
                break;

            case CMD_UPDATE_CONSTANT_BUFFER:
                {
                    auto cmd = reinterpret_cast<ImCmdUpdateConstantBuffer*>(pCmd);
                    A3D_ASSERT(cmd != nullptr);

                    pCmd += sizeof(ImCmdUpdateConstantBuffer);

                    auto pBuffer = static_cast<Buffer*>(cmd->pBuffer);

                    D3D11_BOX box = {};
                    box.left     = static_cast<uint32_t>(cmd->Offset);
                    box.right    = static_cast<uint32_t>(cmd->Size);
                    box.top      = 0;
                    box.bottom   = 1;
                    box.front    = 0;
                    box.back     = 1;

                    pDeviceContext->UpdateSubresource(
                        pBuffer->GetD3D11Buffer(),
                        0,
                        &box,
                        pCmd,
                        UINT(pBuffer->GetDesc().Size),
                        1);
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
                    ResetDescriptor(pDeviceContext);
                    m_pLayoutDesc = nullptr;

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
//      ディスクリプターを更新します.
//-------------------------------------------------------------------------------------------------
void Queue::UpdateDescriptor(ID3D11DeviceContext2* pContext)
{
    if (!(m_DirtyView & DIRTY_DESCRIPTOR) || m_pLayoutDesc == nullptr) 
    { return; }

    auto count = m_pLayoutDesc->EntryCount;
    if (m_pLayoutDesc->Constant.Counts > 0) {
        count++;
    }

    for(auto i=0u; i<count; ++i)
    {
        auto& entry = m_pLayoutDesc->Entries[i];

        if (entry.ShaderStage == SHADER_STAGE_VS)
        {
            switch(entry.Type)
            {
            case DESCRIPTOR_TYPE_CBV:
                {
                    auto pWrapView = static_cast<ConstantBufferView*>(m_pViews[i]);
                    auto pCBV = pWrapView->GetD3D11Buffer();
                    pContext->VSSetConstantBuffers(entry.ShaderRegister, 1, &pCBV);
                }
                break;

            case DESCRIPTOR_TYPE_SRV_T:
            case DESCRIPTOR_TYPE_SRV_B:
                {
                    auto pWrapView = static_cast<ShaderResourceView*>(m_pViews[i]);
                    auto pSRV = pWrapView->GetD3D11ShaderResourceView();
                    pContext->VSSetShaderResources(entry.ShaderRegister, 1, &pSRV);
                }
                break;

            case DESCRIPTOR_TYPE_SMP:
                {
                    auto pWrapView = static_cast<Sampler*>(m_pViews[i]);
                    auto pSmp = pWrapView->GetD3D11SamplerState();
                    pContext->VSSetSamplers(entry.ShaderRegister, 1, &pSmp);
                }
                break;
            }
        }
        else if (entry.ShaderStage == SHADER_STAGE_DS)
        {
            switch(entry.Type)
            {
            case DESCRIPTOR_TYPE_CBV:
                {
                    auto pWrapView = static_cast<ConstantBufferView*>(m_pViews[i]);
                    auto pCBV = pWrapView->GetD3D11Buffer();
                    pContext->DSSetConstantBuffers(entry.ShaderRegister, 1, &pCBV);
                }
                break;

            case DESCRIPTOR_TYPE_SRV_T:
            case DESCRIPTOR_TYPE_SRV_B:
                {
                    auto pWrapView = static_cast<ShaderResourceView*>(m_pViews[i]);
                    auto pSRV = pWrapView->GetD3D11ShaderResourceView();
                    pContext->DSSetShaderResources(entry.ShaderRegister, 1, &pSRV);
                }
                break;

            case DESCRIPTOR_TYPE_SMP:
                {
                    auto pWrapView = static_cast<Sampler*>(m_pViews[i]);
                    auto pSmp = pWrapView->GetD3D11SamplerState();
                    pContext->DSSetSamplers(entry.ShaderRegister, 1, &pSmp);
                }
                break;
            }
        }
        else if (entry.ShaderStage == SHADER_STAGE_HS)
        {
            switch(entry.Type)
            {
            case DESCRIPTOR_TYPE_CBV:
                {
                    auto pWrapView = static_cast<ConstantBufferView*>(m_pViews[i]);
                    auto pCBV = pWrapView->GetD3D11Buffer();
                    pContext->HSSetConstantBuffers(entry.ShaderRegister, 1, &pCBV);
                }
                break;

            case DESCRIPTOR_TYPE_SRV_T:
            case DESCRIPTOR_TYPE_SRV_B:
                {
                    auto pWrapView = static_cast<ShaderResourceView*>(m_pViews[i]);
                    auto pSRV = pWrapView->GetD3D11ShaderResourceView();
                    pContext->HSSetShaderResources(entry.ShaderRegister, 1, &pSRV);
                }
                break;

            case DESCRIPTOR_TYPE_SMP:
                {
                    auto pWrapView = static_cast<Sampler*>(m_pViews[i]);
                    auto pSmp = pWrapView->GetD3D11SamplerState();
                    pContext->HSSetSamplers(entry.ShaderRegister, 1, &pSmp);
                }
                break;
            }
        }
        else if (entry.ShaderStage == SHADER_STAGE_PS)
        {
            switch(entry.Type)
            {
            case DESCRIPTOR_TYPE_CBV:
                {
                    auto pWrapView = static_cast<ConstantBufferView*>(m_pViews[i]);
                    auto pCBV = pWrapView->GetD3D11Buffer();
                    pContext->PSSetConstantBuffers(entry.ShaderRegister, 1, &pCBV);
                }
                break;

            case DESCRIPTOR_TYPE_SRV_T:
            case DESCRIPTOR_TYPE_SRV_B:
                {
                    auto pWrapView = static_cast<ShaderResourceView*>(m_pViews[i]);
                    auto pSRV = pWrapView->GetD3D11ShaderResourceView();
                    pContext->PSSetShaderResources(entry.ShaderRegister, 1, &pSRV);
                }
                break;

            case DESCRIPTOR_TYPE_SMP:
                {
                    auto pWrapView = static_cast<Sampler*>(m_pViews[i]);
                    auto pSmp = pWrapView->GetD3D11SamplerState();
                    pContext->PSSetSamplers(entry.ShaderRegister, 1, &pSmp);
                }
                break;
            }
        }
        else if (entry.ShaderStage == SHADER_STAGE_CS)
        {
            switch(entry.Type)
            {
            case DESCRIPTOR_TYPE_CBV:
                {
                    auto pWrapView = static_cast<ConstantBufferView*>(m_pViews[i]);
                    auto pCBV = pWrapView->GetD3D11Buffer();
                    pContext->CSSetConstantBuffers(entry.ShaderRegister, 1, &pCBV);
                }
                break;

            case DESCRIPTOR_TYPE_SRV_T:
            case DESCRIPTOR_TYPE_SRV_B:
                {
                    auto pWrapView = static_cast<ShaderResourceView*>(m_pViews[i]);
                    auto pSRV = pWrapView->GetD3D11ShaderResourceView();
                    pContext->CSSetShaderResources(entry.ShaderRegister, 1, &pSRV);
                }
                break;

            case DESCRIPTOR_TYPE_UAV_T:
            case DESCRIPTOR_TYPE_UAV_B:
                {
                    auto pWrapView  = static_cast<UnorderedAccessView*>(m_pViews[i]);
                    auto pUAV       = pWrapView->GetD3D11UnorderedAccessView();
                    auto count      = 0u;
                    pContext->CSSetUnorderedAccessViews(entry.ShaderRegister, 1, &pUAV, &count);
                }
                break;

            case DESCRIPTOR_TYPE_SMP:
                {
                    auto pWrapView = static_cast<Sampler*>(m_pViews[i]);
                    auto pSmp = pWrapView->GetD3D11SamplerState();
                    pContext->CSSetSamplers(entry.ShaderRegister, 1, &pSmp);
                }
                break;
            }
        }
        else if (entry.ShaderStage == SHADER_STAGE_AS)
        {
            /* DO_NOTHING */
        }
        else if (entry.ShaderStage == SHADER_STAGE_MS)
        {
            /* DO_NOTHING */
        }

        if (entry.Type == DESCRIPTOR_TYPE_CBV)
        {
            auto pWrapView = static_cast<ConstantBufferView*>(m_pViews[i]);
            pWrapView->UpdateSubsource(pContext);
        }
    }

    m_DirtyView &= ~DIRTY_DESCRIPTOR;
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプターをリセットします.
//-------------------------------------------------------------------------------------------------
void Queue::ResetDescriptor(ID3D11DeviceContext2* pContext)
{
    ID3D11Buffer*               pNullCBV[1] = {};
    ID3D11ShaderResourceView*   pNullSRV[1] = {};
    ID3D11UnorderedAccessView*  pNullUAV[1] = {};
    ID3D11SamplerState*         pNullSmp[1] = {};

    if (m_pLayoutDesc == nullptr)
    { return; }

    auto count = m_pLayoutDesc->EntryCount;
    for(auto i=0u; i<count; ++i)
    {
        auto& entry = m_pLayoutDesc->Entries[i];

        if (entry.ShaderStage == SHADER_STAGE_VS)
        {
            switch(entry.Type)
            {
            case DESCRIPTOR_TYPE_CBV:
                { pContext->VSSetConstantBuffers(entry.ShaderRegister, 1, pNullCBV); }
                break;

            case DESCRIPTOR_TYPE_SRV_T:
            case DESCRIPTOR_TYPE_SRV_B:
                { pContext->VSSetShaderResources(entry.ShaderRegister, 1, pNullSRV); }
                break;
    
            case DESCRIPTOR_TYPE_SMP:
                { pContext->VSSetSamplers(entry.ShaderRegister, 1, pNullSmp); }
                break;
            }
        }
        else if (entry.ShaderStage == SHADER_STAGE_DS)
        {
            switch(entry.Type)
            {
            case DESCRIPTOR_TYPE_CBV:
                { pContext->DSSetConstantBuffers(entry.ShaderRegister, 1, pNullCBV); }
                break;

            case DESCRIPTOR_TYPE_SRV_T:
            case DESCRIPTOR_TYPE_SRV_B:
                { pContext->DSSetShaderResources(entry.ShaderRegister, 1, pNullSRV); }
                break;
    
            case DESCRIPTOR_TYPE_SMP:
                { pContext->DSSetSamplers(entry.ShaderRegister, 1, pNullSmp); }
                break;
            }
        }
        else if (entry.ShaderStage == SHADER_STAGE_HS)
        {
            switch(entry.Type)
            {
            case DESCRIPTOR_TYPE_CBV:
                { pContext->HSSetConstantBuffers(entry.ShaderRegister, 1, pNullCBV); }
                break;

            case DESCRIPTOR_TYPE_SRV_T:
            case DESCRIPTOR_TYPE_SRV_B:
                { pContext->HSSetShaderResources(entry.ShaderRegister, 1, pNullSRV); }
                break;
    
            case DESCRIPTOR_TYPE_SMP:
                { pContext->HSSetSamplers(entry.ShaderRegister, 1, pNullSmp); }
                break;
            }
        }
        else if (entry.ShaderStage == SHADER_STAGE_PS)
        {
            switch(entry.Type)
            {
            case DESCRIPTOR_TYPE_CBV:
                { pContext->PSSetConstantBuffers(entry.ShaderRegister, 1, pNullCBV); }
                break;

            case DESCRIPTOR_TYPE_SRV_T:
            case DESCRIPTOR_TYPE_SRV_B:
                { pContext->PSSetShaderResources(entry.ShaderRegister, 1, pNullSRV); }
                break;
    
            case DESCRIPTOR_TYPE_SMP:
                { pContext->PSSetSamplers(entry.ShaderRegister, 1, pNullSmp); }
                break;
            }
        }
        else if (entry.ShaderStage == SHADER_STAGE_CS)
        {
            auto dummy = 0u;
            switch(entry.Type)
            {
            case DESCRIPTOR_TYPE_CBV:
                { pContext->CSSetConstantBuffers(entry.ShaderRegister, 1, pNullCBV); }
                break;

            case DESCRIPTOR_TYPE_SRV_T:
            case DESCRIPTOR_TYPE_SRV_B:
                { pContext->CSSetShaderResources(entry.ShaderRegister, 1, pNullSRV); }
                break;

            case DESCRIPTOR_TYPE_UAV_T:
            case DESCRIPTOR_TYPE_UAV_B:
                { pContext->CSSetUnorderedAccessViews(entry.ShaderRegister, 1, pNullUAV, &dummy); }
                break;
    
            case DESCRIPTOR_TYPE_SMP:
                { pContext->CSSetSamplers(entry.ShaderRegister, 1, pNullSmp); }
                break;
            }
        }
        else if (entry.ShaderStage == SHADER_STAGE_AS)
        { /* DO_NOTHING */ }
        else if (entry.ShaderStage == SHADER_STAGE_MS)
        { /* DO_NOTHING */ }
    }
}

//-------------------------------------------------------------------------------------------------
//      コマンドリストを登録します.
//-------------------------------------------------------------------------------------------------
bool IQueue::Submit( ICommandList* pCommandList )
{
    auto pThis = static_cast<Queue*>(this);
    A3D_ASSERT(pThis != nullptr);

    LockGuard locker(&pThis->m_Lock);

    if (pThis->m_SubmitIndex + 1 >= pThis->m_MaxSubmitCount)
    { return false; }

    pThis->m_pCommandLists[pThis->m_SubmitIndex] = static_cast<CommandList*>(pCommandList);
    pThis->m_SubmitIndex++;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      登録したコマンドリストを実行します.
//-------------------------------------------------------------------------------------------------
void IQueue::Execute( IFence* pFence )
{
    auto pThis = static_cast<Queue*>(this);
    A3D_ASSERT(pThis != nullptr);

    auto pD3D11DeviceContext = pThis->m_pDevice->GetD3D11DeviceContext();
    A3D_ASSERT(pD3D11DeviceContext != nullptr);

    // Freuencyを取得.
    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT data;
    if (pD3D11DeviceContext->GetData(pThis->m_pQuery, &data, sizeof(data), 0) != S_FALSE)
    { pThis->m_Frequency = data.Frequency; }

#ifdef A3D_FOR_WINDOWS10
    if (pFence != nullptr)
    {
        auto pWrapFence = static_cast<Fence*>(pFence);
        A3D_ASSERT(pWrapFence != nullptr);

        auto pD3D11Fence = pWrapFence->GetD3D11Fence();
        auto fenceValue  = pWrapFence->GetFenceValue();
        pD3D11DeviceContext->Signal(pD3D11Fence, fenceValue);
        pWrapFence->AdvanceCount();
    }
    pThis->ParseCmd();
    pThis->m_SubmitIndex = 0;
#else
    ID3D11Query* pFecneQuery = nullptr;
    if (pFence != nullptr)
    {
        auto pWrapFence = static_cast<Fence*>(pFence);
        A3D_ASSERT(pWrapFence != nullptr);
        pFecneQuery = pWrapFence->GetD3D11Query();
    }

    pD3D11DeviceContext->Begin(pThis->m_pQuery);

    pThis->ParseCmd();
    pThis->m_SubmitIndex = 0;

    pD3D11DeviceContext->End(pThis->m_pQuery);

    if (pFecneQuery != nullptr)
    { pD3D11DeviceContext->End(pFecneQuery); }
#endif

}

//-------------------------------------------------------------------------------------------------
//      コマンドの実行が完了するまで待機します.
//-------------------------------------------------------------------------------------------------
void IQueue::WaitIdle()
{
    auto pThis = static_cast<Queue*>(this);
    A3D_ASSERT(pThis != nullptr);

    auto pD3D11DeviceContext = pThis->m_pDevice->GetD3D11DeviceContext();
    A3D_ASSERT(pD3D11DeviceContext != nullptr);

#ifdef A3D_FOR_WINDOWS10
    pThis->m_pFence->SetEventOnCompletion(1, pThis->m_Event);
    pD3D11DeviceContext->Signal(pThis->m_pFence, 1);
    WaitForSingleObject(pThis->m_Event, INFINITE);
#else
    while(pD3D11DeviceContext->GetData(pThis->m_pQuery, nullptr, 0, 0) == S_FALSE)
    { /* DO_NOTHING */ }
#endif
}

//-------------------------------------------------------------------------------------------------
//      画面に表示を行います.
//-------------------------------------------------------------------------------------------------
void IQueue::Present( ISwapChain* pSwapChain )
{
    auto pWrapSwapChain = reinterpret_cast<SwapChain*>(pSwapChain);
    if (pWrapSwapChain == nullptr)
    { return; }

    pWrapSwapChain->Present();
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
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if (!instance->Init(pDevice, type, maxSubmitCount))
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppQueue = instance;
    return true;
}

} // namespace a3d
