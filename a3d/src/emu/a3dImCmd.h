﻿//-------------------------------------------------------------------------------------------------
// File : a3dImCmd.h
// Desc : Intermediate Command
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
//! @enum   CMD_TYPE
//! @brief  コマンドタイプです.
///////////////////////////////////////////////////////////////////////////////////////////////////
enum CMD_TYPE
{
    CMD_BEGIN = 0,                      //!< ICommandList::Begin() For COMMANDLIST_TYPE_DIRECT
    CMD_SUB_BEGIN,                      //!< ICommandList::Begin() For COMMANDLIST_TYPE_BUNDLE
    CMD_SET_FRAME_BUFFER,               //!< ICommandList::SetFrameBuffer()
    CMD_CLEAR_FRAME_BUFFER,             //!< ICommandList::ClearFrameBuffer()
    CMD_SET_BLEND_CONSTANT,             //!< ICommandList::SetBlendConstant()
    CMD_SET_STENCIL_REFERENCE,          //!< ICommandList::SetStencilReference()
    CMD_SET_VIEWPORTS,                  //!< ICommandList::SetViewports()
    CMD_SET_SCISSORS,                   //!< ICommandList::SetScissors()
    CMD_SET_PIPELINESTATE,              //!< ICommandList::SetPipelineState()
    CMD_SET_DESCRIPTORSET,              //!< ICommandList::SetDescriptorSet()
    CMD_SET_VERTEX_BUFFERS,             //!< ICommandList::SetVertexBuffers()
    CMD_SET_INDEX_BUFFER,               //!< ICommandList::SetIndexBuffer()
    CMD_TEXTURE_BARRIER,                //!< ICommandList::TextureBarrier()
    CMD_BUFFER_BARRIER,                 //!< ICommandList::BufferBarrier()
    CMD_DRAW_INSTANCED,                 //!< ICommandList::DrawInstanced()
    CMD_DRAW_INDEXED_INSTANCED,         //!< ICommandList::DrawIndexedInstanced()
    CMD_DISPATCH,                       //!< ICommandList::Dispatch()
    CMD_EXECUTE_INDIRECT,               //!< ICommandList::ExecuteIndirect()
    CMD_BEGIN_QUERY,                    //!< ICommandList::BeginQuery()
    CMD_END_QUERY,                      //!< ICommandList::EndQuery()
    CMD_RESOLVE_QUERY,                  //!< ICommandList::ResolveQuery()
    CMD_COPY_TEXTURE,                   //!< ICommandList::CopyTexture()
    CMD_COPY_BUFFER,                    //!< ICommandList::CopyBuffer()
    CMD_COPY_TEXTURE_REGION,            //!< ICommandList::CopyTextureRegion()
    CMD_COPY_BUFFER_REGION,             //!< ICommandList::CopyBufferRegion()
    CMD_COPY_BUFFER_TO_TEXTURE,         //!< ICommandList::CopyBufferToTexture()
    CMD_COPY_TEXTURE_TO_BUFFER,         //!< ICommandList::CopyTextureToBuffer()
    CMD_RESOLVE_SUBRESOURCE,            //!< ICommandList::ResolveSubresource()
    CMD_EXECUTE_BUNDLE,                 //!< ICommandList::ExecuteBundle()
    CMD_SUB_END,                        //!< ICommandList::End() For COMMANDLIST_TYPE_BUNDLE
    CMD_END,                            //!< ICommandList::End() For COMMANDLIST_TYPE_DIRECT
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdBase    structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdBase 
{
    CMD_TYPE    Type;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdBegin structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdBegin : ImCmdBase
{ /* NOTHING */ };

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdSetFrameBuffer structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdSetFrameBuffer : ImCmdBase
{
    IFrameBuffer*   pFrameBuffer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdClearFrameBuffer structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdClearFrameBuffer : ImCmdBase
{
    uint32_t                ClearColorCount;
    bool                    HasDepth;
    ClearColorValue         ClearColors[8];
    ClearDepthStencilValue  ClearDepthStencil;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdSetBlendConstant structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdSetBlendConstant : ImCmdBase
{
    float       BlendConstant[4];
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdSetStencilReference structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdSetStencilReference : ImCmdBase
{
    uint32_t    StencilReference;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdSetViewports structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdSetViewports : ImCmdBase
{
    uint32_t    Count;
    Viewport    Viewports[16];
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdSetScissors structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdSetScissors : ImCmdBase
{
    uint32_t    Count;
    Rect        Rects[16];
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImdCmdSetPipelineState sturcture
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdSetPipelineState : ImCmdBase
{
    IPipelineState* pPipelineState;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdSetDescriptorSet structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdSetDescriptorSet : ImCmdBase
{
    IDescriptorSet*     pDescriptorSet;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdSetVertexBuffers structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdSetVertexBuffers : ImCmdBase
{
    uint32_t    StartSlot;
    uint32_t    Count;
    IBuffer*    pBuffers[32];
    bool        HasOffset;
    uint64_t    Offsets[32];
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImdCmdSetIndexBuffer structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdSetIndexBuffer : ImCmdBase
{
    IBuffer*    pBuffer;
    uint64_t    Offset;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdTextureBarrier structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdTextureBarrier : ImCmdBase
{
    ITexture*       pResource;
    RESOURCE_STATE  NextState;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdBufferBarrier structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdBufferBarrier : ImCmdBase
{
    IBuffer*        pResource;
    RESOURCE_STATE  NextState;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdDrawInstanced structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdDrawInstanced : ImCmdBase
{
    uint32_t        VertexCount;
    uint32_t        InstanceCount;
    uint32_t        FirstVertex;
    uint32_t        FirstInstance;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdDrawIndexedInstance structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdDrawIndexedInstanced : ImCmdBase
{
    uint32_t        IndexCount;
    uint32_t        InstanceCount;
    uint32_t        FirstIndex;
    int             VertexOffset;
    uint32_t        FirstInstance;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdDispatch structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdDispatch : ImCmdBase
{
    uint32_t        X;
    uint32_t        Y;
    uint32_t        Z;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdExecuteIndirect structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdExecuteIndirect : ImCmdBase
{
    ICommandSet*    pCommandSet;
    uint32_t        MaxCommandCount;
    IBuffer*        pArgumentBuffer;
    uint64_t        ArgumentBufferOffset;
    IBuffer*        pCounterBuffer;
    uint64_t        CounterBufferOffset;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdBeginQuery structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdBeginQuery : ImCmdBase
{
    IQueryPool*     pQuery;
    uint32_t        Index;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdEndQuery structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdEndQuery : ImCmdBase
{
    IQueryPool*     pQuery;
    uint32_t        Index;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdResolveQuery structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdResolveQuery : ImCmdBase
{
    IQueryPool*     pQuery;
    uint32_t        StartIndex;
    uint32_t        QueryCount;
    IBuffer*        pDstBuffer;
    uint64_t        DstOffset;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdCopyTexture structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdCopyTexture : ImCmdBase
{
    ITexture*       pDstTexture;
    ITexture*       pSrcTexture;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdCopyBuffer structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdCopyBuffer : ImCmdBase
{
    IBuffer*        pDstBuffer;
    IBuffer*        pSrcBuffer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdCopyTextureRegion structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdCopyTextureRegion : ImCmdBase
{
    ITexture*       pDstResource;
    uint32_t        DstSubresource;
    Offset3D        DstOffset;
    ITexture*       pSrcResource;
    uint32_t        SrcSubresource;
    Offset3D        SrcOffset;
    Extent3D        SrcExtent;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ICmCmdCopyBufferrRegion structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdCopyBufferRegion : ImCmdBase
{
    IBuffer*        pDstBuffer;
    uint64_t        DstOffset;
    IBuffer*        pSrcBuffer;
    uint64_t        SrcOffset;
    uint64_t        ByteCount;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdCopyBufferToTexture structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdCopyBufferToTexture : ImCmdBase
{
    ITexture*       pDstTexture;
    uint32_t        DstSubresource;
    Offset3D        DstOffset;
    IBuffer*        pSrcBuffer;
    uint64_t        SrcOffset;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdCopyTextureToBuffer structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdCopyTextureToBuffer : ImCmdBase
{
    IBuffer*        pDstBuffer;
    uint64_t        DstOffset;
    ITexture*       pSrcTexture;
    uint32_t        SrcSubresource;
    Offset3D        SrcOffset;
    Extent3D        SrcExtent;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdResolveSubresource structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdResolveSubresource : ImCmdBase
{
    ITexture*       pDstResource;
    uint32_t        DstSubresource;
    ITexture*       pSrcResource;
    uint32_t        SrcSubresource;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ImCmdEnd structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ImCmdEnd : ImCmdBase
{ /* NOTHING */ };

} // namespace a3d
