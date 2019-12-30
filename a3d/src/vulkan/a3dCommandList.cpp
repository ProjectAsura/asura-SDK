//-------------------------------------------------------------------------------------------------
// File : a3dCommandList.cpp
// Desc : CommandList Implmentation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "a3dVulkanFunc.h"


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// CommandList class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
CommandList::CommandList()
: m_RefCount        (1)
, m_pDevice         (nullptr)
, m_CommandPool     (null_handle)
, m_CommandBuffer   (null_handle)
, m_pFrameBuffer    (nullptr)
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
    if ( pDevice == nullptr )
    { return false; }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT( pNativeDevice != null_handle );

    {
        uint32_t queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        IQueue* pQueue = nullptr;
        if (listType == COMMANDLIST_TYPE_DIRECT)
        { m_pDevice->GetGraphicsQueue(&pQueue); }
        else if (listType == COMMANDLIST_TYPE_COMPUTE)
        { m_pDevice->GetComputeQueue(&pQueue); }
        else if (listType == COMMANDLIST_TYPE_COPY)
        { m_pDevice->GetCopyQueue(&pQueue); }

        if (pQueue != nullptr)
        {
            auto pWrapQueue = reinterpret_cast<Queue*>(pQueue);
            A3D_ASSERT(pWrapQueue != nullptr);
            queueFamilyIndex = pWrapQueue->GetFamilyIndex();
        }

        SafeRelease(pQueue);

        VkCommandPoolCreateInfo info = {};
        info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.pNext            = nullptr;
        info.queueFamilyIndex = queueFamilyIndex;
        info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        auto ret = vkCreateCommandPool( pNativeDevice, &info, nullptr, &m_CommandPool );
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    if ( listType == COMMANDLIST_TYPE_BUNDLE )
    {
        VkCommandBufferAllocateInfo info = {};
        info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.pNext              = nullptr;
        info.commandPool        = m_CommandPool;
        info.level              = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        info.commandBufferCount = 1;

        auto ret = vkAllocateCommandBuffers( pNativeDevice, &info, &m_CommandBuffer );
        if ( ret != VK_SUCCESS )
        { return false; }
    }
    else
    {

        VkCommandBufferAllocateInfo info = {};
        info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.pNext              = nullptr;
        info.commandPool        = m_CommandPool;
        info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandBufferCount = 1;

        auto ret = vkAllocateCommandBuffers( pNativeDevice, &info, &m_CommandBuffer );
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void CommandList::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    if (m_CommandBuffer != null_handle)
    {
        vkFreeCommandBuffers(pNativeDevice, m_CommandPool, 1, &m_CommandBuffer);
        m_CommandBuffer = null_handle;
    }

    if (m_CommandPool != null_handle)
    {
        vkDestroyCommandPool(pNativeDevice, m_CommandPool, nullptr);
        m_CommandPool = null_handle;
    }

    m_pFrameBuffer = nullptr;
    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void CommandList::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      参照カウントを減らします.
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
    VkCommandBufferInheritanceInfo inheritanceInfo = {};
    inheritanceInfo.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.pNext                = nullptr;
    inheritanceInfo.renderPass           = null_handle;
    inheritanceInfo.subpass              = 0;
    inheritanceInfo.framebuffer          = null_handle;
    inheritanceInfo.occlusionQueryEnable = VK_FALSE;
    inheritanceInfo.queryFlags           = 0;
    inheritanceInfo.pipelineStatistics   = 0;

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext            = nullptr;
    beginInfo.flags            = 0;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    auto result = vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);
    A3D_ASSERT( result == VK_SUCCESS );
    A3D_UNUSED( result );

    m_pFrameBuffer = nullptr;

    VkViewport dummyViewport = {};
    dummyViewport.width    = 1;
    dummyViewport.height   = 1;
    dummyViewport.minDepth = 0.0f;
    dummyViewport.maxDepth = 1.0f;
    vkCmdSetViewport( m_CommandBuffer, 0, 1, &dummyViewport );

    VkRect2D dummyScissor = {};
    vkCmdSetScissor( m_CommandBuffer, 0, 1, &dummyScissor );

    float blendConstant[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    vkCmdSetBlendConstants( m_CommandBuffer, blendConstant );

    vkCmdSetStencilReference( m_CommandBuffer, VK_STENCIL_FRONT_AND_BACK, 0 );
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

    // 同じフレームバッファであればコマンドは出さない.
    if (m_pFrameBuffer == pWrapFrameBuffer)
    { return; }

    pWrapFrameBuffer->Bind( this );
    m_pFrameBuffer = pWrapFrameBuffer;
}

//-------------------------------------------------------------------------------------------------
//      フレームバッファを解除します.
//-------------------------------------------------------------------------------------------------
void CommandList::EndFrameBuffer()
{
    // フレームバッファがバインド済みであればレンダーパスを終わらせる.
    if (m_pFrameBuffer != nullptr)
    { m_pFrameBuffer = nullptr; }

    vkCmdEndRenderPass(m_CommandBuffer);
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
    if (m_pFrameBuffer == nullptr)
    { return; }

    m_pFrameBuffer->Clear(this, clearColorCount, pClearColors, pClearDepthStencil);
}

//-------------------------------------------------------------------------------------------------
//      ブレンド定数を設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetBlendConstant(const float blendConstant[4])
{ vkCmdSetBlendConstants( m_CommandBuffer, blendConstant ); }

//-------------------------------------------------------------------------------------------------
//      ステンシル参照値を設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetStencilReference(uint32_t stencilRef)
{ vkCmdSetStencilReference( m_CommandBuffer, VK_STENCIL_FRONT_AND_BACK, stencilRef); }

//-------------------------------------------------------------------------------------------------
//      ビューポートを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetViewports(uint32_t count, Viewport* pViewports)
{
    if (count == 0 || pViewports == nullptr)
    { return; }

    VkViewport viewports[16];

    for(auto i=0u; i<count; ++i)
    {
        if (i >= 16)
        { break; }

        viewports[i].x        = pViewports[i].X;
        viewports[i].y        = pViewports[i].Y;
        viewports[i].width    = pViewports[i].Width;
        viewports[i].height   = pViewports[i].Height;
        viewports[i].minDepth = pViewports[i].MinDepth;
        viewports[i].maxDepth = pViewports[i].MaxDepth;
    }

    vkCmdSetViewport( m_CommandBuffer, 0, count, viewports );
}

//-------------------------------------------------------------------------------------------------
//      シザー矩形を設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetScissors(uint32_t count, Rect* pScissors)
{
    if (count == 0 || pScissors == nullptr)
    { return; }

    VkRect2D rects[16];

    for(auto i=0u; i<count; ++i)
    {
        if (i >= 16)
        { break; }

        rects[i].offset.x      = pScissors[i].Offset.X;
        rects[i].offset.y      = pScissors[i].Offset.Y;
        rects[i].extent.width  = pScissors[i].Extent.Width;
        rects[i].extent.height = pScissors[i].Extent.Height;
    }

    vkCmdSetScissor( m_CommandBuffer, 0, count, rects );
}

//-------------------------------------------------------------------------------------------------
//      パイプラインステートを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetPipelineState(IPipelineState* pPipelineState)
{
    if (pPipelineState == nullptr)
    { return; }

    auto pWrapPipelineState = static_cast<PipelineState*>(pPipelineState);
    A3D_ASSERT( pWrapPipelineState != nullptr );

    auto bindPoint = pWrapPipelineState->GetVulkanPipelineBindPoint();
    auto pNativePipelineState = pWrapPipelineState->GetVulkanPipeline();
    A3D_ASSERT( pNativePipelineState != null_handle );

    vkCmdBindPipeline( m_CommandBuffer, bindPoint, pNativePipelineState );
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタセットを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetDescriptorSet(IDescriptorSet* pDescriptorSet)
{
    if (pDescriptorSet == nullptr)
    { return; }

    auto pWrapDescriptorSet = static_cast<DescriptorSet*>(pDescriptorSet);
    A3D_ASSERT( pWrapDescriptorSet != nullptr );

    pWrapDescriptorSet->Issue( this );
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

    VkBuffer     buffers[32] = {};
    VkDeviceSize offsets[32] = {};

    for(auto i=0u; i<count; ++i)
    {
        if (i >= 32)
        { break; }

        auto pWrapResource = static_cast<Buffer*>(ppResources[i]);
        A3D_ASSERT( pWrapResource != nullptr );

        buffers[i] = pWrapResource->GetVulkanBuffer();
        offsets[i] = (pOffsets != nullptr) ? pOffsets[i] : 0;
    }

    vkCmdBindVertexBuffers( m_CommandBuffer, startSlot, count, buffers, offsets );
}

//-------------------------------------------------------------------------------------------------
//      インデックスバッファを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetIndexBuffer
(
    IBuffer*    pResource,
    uint64_t    offset
)
{
    if (pResource == nullptr)
    { return; }

    auto pWrapResource = static_cast<Buffer*>(pResource);
    A3D_ASSERT( pWrapResource != nullptr );

    auto type = (pWrapResource->GetDesc().Stride == sizeof(uint16_t))
                ? VK_INDEX_TYPE_UINT16 
                : VK_INDEX_TYPE_UINT32;

    vkCmdBindIndexBuffer( m_CommandBuffer, pWrapResource->GetVulkanBuffer(), offset, type );
}

//--------------------------------------------------------------------------------------------------
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
    A3D_ASSERT( pWrapResource != nullptr );

    auto pNativeImage = pWrapResource->GetVulkanImage();
    A3D_ASSERT( pNativeImage != null_handle );

    auto oldLayout = ToNativeImageLayout( prevState );
    auto newLayout = ToNativeImageLayout( nextState );

    VkImageSubresourceRange range = {};
    range.aspectMask     = pWrapResource->GetVulkanImageAspectFlags();
    range.baseArrayLayer = 0;
    range.baseMipLevel   = 0;
    range.layerCount     = pWrapResource->GetDesc().DepthOrArraySize;
    range.levelCount     = pWrapResource->GetDesc().MipLevels;

    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    if ( prevState == RESOURCE_STATE_PRESENT && nextState == RESOURCE_STATE_COLOR_WRITE )
    {
        VkImageMemoryBarrier barrier;
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext                           = nullptr;
        barrier.srcAccessMask                   = VK_ACCESS_MEMORY_READ_BIT;
        barrier.dstAccessMask                   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.oldLayout                       = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.newLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange                = range;
        barrier.image                           = pNativeImage;

        vkCmdPipelineBarrier(
            m_CommandBuffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier);
    }
    else if ( prevState == RESOURCE_STATE_COLOR_WRITE && nextState == RESOURCE_STATE_PRESENT )
    {
        VkImageMemoryBarrier barrier;
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext                           = nullptr;
        barrier.srcAccessMask                   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask                   = VK_ACCESS_MEMORY_READ_BIT;
        barrier.oldLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout                       = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange                = range;
        barrier.image                           = pNativeImage;

        vkCmdPipelineBarrier(
            m_CommandBuffer,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier);
    }
    else
    {
        SetImageLayout(
            m_CommandBuffer,
            pNativeImage,
            srcStageMask,
            dstStageMask,
            oldLayout,
            newLayout,
            range );
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
    if (pResource == nullptr)
    { return; }

    auto pWrapResource = static_cast<Buffer*>(pResource);
    A3D_ASSERT( pWrapResource != nullptr );

    auto pNativeBuffer = pWrapResource->GetVulkanBuffer();
    A3D_ASSERT( pNativeBuffer != null_handle );

    auto srcAccess = ToNativeAccessFlags( prevState );
    auto dstAccess = ToNativeAccessFlags( nextState );

    VkBufferMemoryBarrier barrier = {};
    barrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.pNext               = nullptr;
    barrier.srcAccessMask       = srcAccess;
    barrier.dstAccessMask       = dstAccess;
    barrier.srcQueueFamilyIndex = 0;
    barrier.dstQueueFamilyIndex = 0;
    barrier.buffer              = pNativeBuffer;
    barrier.offset              = 0;
    barrier.size                = pWrapResource->GetDesc().Size;

    vkCmdPipelineBarrier(
        m_CommandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        0,
        0, nullptr,
        1, &barrier,
        0, nullptr
    );
}

//-------------------------------------------------------------------------------------------------
//      インスタンスを描画します.
//-------------------------------------------------------------------------------------------------
void CommandList::DrawInstanced
(
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance
)
{
    if (vertexCount == 0 || instanceCount == 0)
    { return; }

    vkCmdDraw(
        m_CommandBuffer,
        vertexCount,
        instanceCount,
        firstVertex,
        firstInstance );
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
    if (indexCount == 0 || instanceCount == 0)
    { return; }

    vkCmdDrawIndexed(
        m_CommandBuffer,
        indexCount,
        instanceCount,
        firstIndex,
        vertexOffset,
        firstInstance );
}

//-------------------------------------------------------------------------------------------------
//      スレッドグループからコマンドリストを実行します.
//-------------------------------------------------------------------------------------------------
void CommandList::Dispatch(uint32_t x, uint32_t y, uint32_t z)
{
    if (x == 0 && y == 0 && z == 0)
    { return; }

    vkCmdDispatch( m_CommandBuffer, x, y, z );
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

    const auto& desc = pWrapCommandSet->GetDesc();

    auto pWrapArgumentBuffer = static_cast<Buffer*>(pArgumentBuffer);
    A3D_ASSERT(pWrapArgumentBuffer != nullptr);

    auto pNativeArgumentBuffer = pWrapArgumentBuffer->GetVulkanBuffer();
    A3D_ASSERT(pNativeArgumentBuffer != null_handle);

    uint32_t* pCounters = nullptr;
    if (pCounterBuffer != nullptr)
    { pCounters = static_cast<uint32_t*>(pCounterBuffer->Map()); }

    auto offset = argumentBufferOffset;
    for(auto i=0u; i<desc.ArgumentCount; ++i)
    {
        auto count = maxCommandCount;
        if (pCounters != nullptr)
        { count = (pCounters[i] < maxCommandCount) ? pCounters[i] : maxCommandCount; }

        switch( desc.pArguments[i] )
        {
        case INDIRECT_ARGUMENT_TYPE_DRAW:
            { vkCmdDrawIndirect( m_CommandBuffer, pNativeArgumentBuffer, offset, count, desc.ByteStride ); }
            break;

        case INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED:
            { vkCmdDrawIndexedIndirect( m_CommandBuffer, pNativeArgumentBuffer, offset, count, desc.ByteStride ); }
            break;

        case INDIRECT_ARGUMENT_TYPE_DISPATCH:
            { vkCmdDispatchIndirect( m_CommandBuffer, pNativeArgumentBuffer, offset ); }
            break;
        }

        offset += desc.ByteStride;
    }
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

    auto pNativeQueryPool = pWrapQueryPool->GetVulkanQueryPool();
    A3D_ASSERT(pNativeQueryPool != null_handle);

    auto nativeQueryType = pWrapQueryPool->GetVulkanQueryType();

    if (nativeQueryType == VK_QUERY_TYPE_TIMESTAMP)
    { vkCmdWriteTimestamp( m_CommandBuffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, pNativeQueryPool, index ); }
    else
    { vkCmdBeginQuery( m_CommandBuffer, pNativeQueryPool, index, 0 ); }
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

    auto pNativeQueryPool = pWrapQueryPool->GetVulkanQueryPool();
    A3D_ASSERT(pNativeQueryPool != null_handle);

    auto nativeQueryType = pWrapQueryPool->GetVulkanQueryType();

    if (nativeQueryType == VK_QUERY_TYPE_TIMESTAMP)
    { vkCmdWriteTimestamp( m_CommandBuffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, pNativeQueryPool, index ); }
    else
    { vkCmdEndQuery( m_CommandBuffer, pNativeQueryPool, index); }
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
    if (pQuery == nullptr || queryCount == 0)
    { return; }

    auto pWrapQueryPool = static_cast<QueryPool*>(pQuery);
    A3D_ASSERT(pWrapQueryPool != nullptr);

    auto pNativeQueryPool = pWrapQueryPool->GetVulkanQueryPool();
    A3D_ASSERT(pNativeQueryPool != null_handle);

    auto pWrapBuffer = static_cast<Buffer*>(pDstBuffer);
    A3D_ASSERT(pWrapBuffer != nullptr);

    auto pNativeBuffer = pWrapBuffer->GetVulkanBuffer();
    A3D_ASSERT(pNativeBuffer != null_handle);

    VkQueryResultFlags flags = 0; // TODO : 実装チェック.

    vkCmdCopyQueryPoolResults(
        m_CommandBuffer,
        pNativeQueryPool,
        startIndex,
        queryCount,
        pNativeBuffer,
        dstOffset,
        pDstBuffer->GetDesc().Size,
        flags );
}

//-------------------------------------------------------------------------------------------------
//      リソースをコピーします.
//-------------------------------------------------------------------------------------------------
void CommandList::CopyTexture( ITexture* pDst, ITexture* pSrc)
{
    if (pDst == nullptr || pSrc == nullptr)
    { return; }

    Offset3D offset = { 0, 0, 0 };
    Extent3D extent = {};
    const auto& desc = pDst->GetDesc();
    extent.Width    = desc.Width;
    extent.Height   = desc.Height;
    extent.Depth    = desc.DepthOrArraySize;
    CopyTextureRegion( pDst, 0, offset, pSrc, 0, offset, extent );
}

//-------------------------------------------------------------------------------------------------
//      リソースをコピーします.
//-------------------------------------------------------------------------------------------------
void CommandList::CopyBuffer(IBuffer* pDst, IBuffer* pSrc)
{
    if (pDst == nullptr || pSrc == nullptr)
    { return; }

    CopyBufferRegion(pDst, 0, pSrc, 0, pDst->GetDesc().Size);
}

//-------------------------------------------------------------------------------------------------
//      領域を指定してテクスチャをコピーします.
//-------------------------------------------------------------------------------------------------
void CommandList::CopyTextureRegion
(
    ITexture*       pDstResource,
    uint32_t        dstSubResource,
    Offset3D        dstOffset,
    ITexture*       pSrcResource,
    uint32_t        srcSubResource,
    Offset3D        srcOffset,
    Extent3D        srcExtent
)
{
    if (pDstResource == nullptr || pSrcResource == nullptr)
    { return; }

    auto pWrapSrc = static_cast<Texture*>(pSrcResource);
    auto pWrapDst = static_cast<Texture*>(pDstResource);
    A3D_ASSERT( pWrapSrc != nullptr );
    A3D_ASSERT( pWrapDst != nullptr );

    auto pNativeSrc = pWrapSrc->GetVulkanImage();
    auto pNativeDst = pWrapDst->GetVulkanImage();
    A3D_ASSERT( pNativeSrc != null_handle );
    A3D_ASSERT( pNativeDst != null_handle );

    auto srcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    auto dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    const auto& srcDesc = pWrapSrc->GetDesc();
    const auto& dstDesc = pWrapDst->GetDesc();

    VkImageCopy region = {};
    region.dstOffset.x = dstOffset.X;
    region.dstOffset.y = dstOffset.Y;
    region.dstOffset.z = dstOffset.Z;
    
    uint32_t placeSlice;
    region.dstSubresource.aspectMask     = pWrapDst->GetVulkanImageAspectFlags();
    region.dstSubresource.layerCount     = dstDesc.DepthOrArraySize;
    DecomposeSubresource(
        dstSubResource,
        dstDesc.MipLevels,
        dstDesc.DepthOrArraySize,
        region.dstSubresource.mipLevel,
        region.dstSubresource.baseArrayLayer,
        placeSlice);
    
    region.srcOffset.x = srcOffset.X;
    region.srcOffset.y = srcOffset.Y;
    region.srcOffset.z = srcOffset.Z;

    region.srcSubresource.aspectMask     = pWrapSrc->GetVulkanImageAspectFlags();
    region.srcSubresource.layerCount     = srcDesc.DepthOrArraySize;
    DecomposeSubresource(
        srcSubResource,
        srcDesc.MipLevels,
        srcDesc.DepthOrArraySize,
        region.srcSubresource.mipLevel,
        region.srcSubresource.baseArrayLayer,
        placeSlice);

    region.extent.width  = srcExtent.Width;
    region.extent.height = srcExtent.Height;
    region.extent.depth  = srcExtent.Depth;

    vkCmdCopyImage( m_CommandBuffer, pNativeSrc, srcLayout, pNativeDst, dstLayout, 1, &region );
}

//-------------------------------------------------------------------------------------------------
//      領域を指定してバッファをコピーします.
//-------------------------------------------------------------------------------------------------
void CommandList::CopyBufferRegion
(
    IBuffer*    pDstBuffer,
    uint64_t    dstOfset,
    IBuffer*    pSrcBuffer,
    uint64_t    srcOffset,
    uint64_t    byteCount
)
{
    if (pDstBuffer == nullptr || pSrcBuffer == nullptr)
    { return; }

    auto pWrapDst = static_cast<Buffer*>(pDstBuffer);
    auto pWrapSrc = static_cast<Buffer*>(pSrcBuffer);
    A3D_ASSERT( pWrapDst != nullptr );
    A3D_ASSERT( pWrapSrc != nullptr );

    VkBufferCopy region = {};
    region.dstOffset = dstOfset;
    region.srcOffset = srcOffset;
    region.size      = byteCount;

    vkCmdCopyBuffer( 
        m_CommandBuffer, 
        pWrapSrc->GetVulkanBuffer(),
        pWrapDst->GetVulkanBuffer(), 
        1, &region );
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
    A3D_ASSERT( pWrapDst != nullptr );
    A3D_ASSERT( pWrapSrc != nullptr );

    const auto& dstDesc = pWrapDst->GetDesc();

    auto nativeState = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask  = pWrapDst->GetVulkanImageAspectFlags();
    region.imageSubresource.layerCount  = dstDesc.DepthOrArraySize;
    region.imageOffset.x                = dstOffset.X;
    region.imageOffset.y                = dstOffset.Y;
    region.imageOffset.z                = dstOffset.Z;
    region.imageExtent.width            = dstDesc.Width;
    region.imageExtent.height           = dstDesc.Height;
    region.imageExtent.depth            = dstDesc.DepthOrArraySize;
    region.bufferOffset                 = srcOffset;

    uint32_t planeSlice;
    DecomposeSubresource(
        dstSubresource,
        dstDesc.MipLevels,
        dstDesc.DepthOrArraySize, 
        region.imageSubresource.mipLevel,
        region.imageSubresource.baseArrayLayer,
        planeSlice);

    vkCmdCopyBufferToImage(
        m_CommandBuffer,
        pWrapSrc->GetVulkanBuffer(),
        pWrapDst->GetVulkanImage(),
        nativeState,
        1, &region);
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

    const auto& srcDesc = pWrapSrc->GetDesc();

    VkBufferImageCopy region = {};
    region.bufferOffset                 = dstOffset;
    region.bufferRowLength              = static_cast<uint32_t>(pWrapDst->GetDesc().Size);
    region.bufferImageHeight            = 1;
    region.imageOffset.x                = srcOffset.X;
    region.imageOffset.y                = srcOffset.Y;
    region.imageOffset.z                = srcOffset.Z;
    region.imageExtent.width            = srcExtent.Width;
    region.imageExtent.height           = srcExtent.Height;
    region.imageExtent.depth            = srcExtent.Depth;
    region.imageSubresource.aspectMask  = pWrapSrc->GetVulkanImageAspectFlags();
    region.imageSubresource.layerCount  = srcDesc.DepthOrArraySize;

    uint32_t planeSlice;
    DecomposeSubresource(
        srcSubresource,
        srcDesc.MipLevels,
        srcDesc.DepthOrArraySize,
        region.imageSubresource.mipLevel,
        region.imageSubresource.baseArrayLayer,
        planeSlice);

    vkCmdCopyImageToBuffer(
        m_CommandBuffer,
        pWrapSrc->GetVulkanImage(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        pWrapDst->GetVulkanBuffer(),
        1, &region);
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

    auto pWrapSrc = static_cast<Texture*>(pSrcResource);
    auto pWrapDst = static_cast<Texture*>(pDstResource);
    A3D_ASSERT( pWrapSrc != nullptr );
    A3D_ASSERT( pWrapDst != nullptr );

    auto pNativeSrc = pWrapSrc->GetVulkanImage();
    auto pNativeDst = pWrapDst->GetVulkanImage();
    A3D_ASSERT( pNativeSrc != null_handle );
    A3D_ASSERT( pNativeDst != null_handle );

    auto srcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    auto dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    const auto &dstDesc = pWrapDst->GetDesc();
    const auto &srcDesc = pWrapSrc->GetDesc();

    VkImageResolve region = {};
    region.dstSubresource.aspectMask = pWrapDst->GetVulkanImageAspectFlags();
    region.dstSubresource.layerCount = dstDesc.DepthOrArraySize;

    uint32_t planeSlice;
    DecomposeSubresource(
        dstSubresource,
        dstDesc.MipLevels,
        dstDesc.DepthOrArraySize,
        region.dstSubresource.mipLevel,
        region.dstSubresource.baseArrayLayer,
        planeSlice);

    region.dstOffset.x = 0;
    region.dstOffset.y = 0;
    region.dstOffset.z = 0;

    region.srcSubresource.aspectMask     = pWrapSrc->GetVulkanImageAspectFlags();
    region.srcSubresource.layerCount     = srcDesc.DepthOrArraySize;
    DecomposeSubresource(
        srcSubresource,
        srcDesc.MipLevels,
        srcDesc.DepthOrArraySize,
        region.srcSubresource.mipLevel,
        region.srcSubresource.baseArrayLayer,
        planeSlice);

    region.srcOffset.x = 0;
    region.srcOffset.y = 0;
    region.srcOffset.z = 0;

    region.extent.width  = pWrapDst->GetDesc().Width;
    region.extent.height = pWrapDst->GetDesc().Height;
    region.extent.depth  = pWrapDst->GetDesc().DepthOrArraySize;

    vkCmdResolveImage( m_CommandBuffer, pNativeSrc, srcLayout, pNativeDst, dstLayout, 1, &region );
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

    auto pNativeBundle = pWrapCommandList->GetVulkanCommandBuffer();
    A3D_ASSERT(pNativeBundle != null_handle);

    vkCmdExecuteCommands(m_CommandBuffer, 1, &pNativeBundle);
}

//-------------------------------------------------------------------------------------------------
//      デバッグマーカーをプッシュします.
//-------------------------------------------------------------------------------------------------
void CommandList::PushMarker(const char* tag)
{
    if (!m_pDevice->IsSupportExtension(Device::EXT_DEBUG_MARKER))
    { return; }

    VkDebugMarkerMarkerInfoEXT info = {};
    info.sType          = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
    info.pNext          = nullptr;
    info.pMarkerName    = tag;
    info.color[0]       = 0.3f;
    info.color[1]       = 0.5f;
    info.color[2]       = 1.0f;
    info.color[3]       = 1.0f;

    vkCmdDebugMarkerBegin(m_CommandBuffer, &info);
}

//-------------------------------------------------------------------------------------------------
//      デバッグマーカーをポップします.
//-------------------------------------------------------------------------------------------------
void CommandList::PopMarker()
{
    if (!m_pDevice->IsSupportExtension(Device::EXT_DEBUG_MARKER))
    { return; }

    vkCmdDebugMarkerEnd(m_CommandBuffer);
}

//-------------------------------------------------------------------------------------------------
//      定数バッファを更新します.
//-------------------------------------------------------------------------------------------------
bool CommandList::UpdateConstantBuffer(IBuffer* pBuffer, size_t offset, size_t size, const void* pData)
{
    auto pWrapBuffer = static_cast<Buffer*>(pBuffer);
    if (pWrapBuffer == nullptr || size == 0 || pData == nullptr)
    { return false; }

    vkCmdUpdateBuffer(m_CommandBuffer, pWrapBuffer->GetVulkanBuffer(), offset, size, pData);
    return true;
}

//-------------------------------------------------------------------------------------------------
//      コマンドリストの記録を終了します.
//-------------------------------------------------------------------------------------------------
void CommandList::End()
{
    if (m_pFrameBuffer != nullptr)
    {
        vkCmdEndRenderPass( m_CommandBuffer );
        m_pFrameBuffer = nullptr;
    }

    vkEndCommandBuffer( m_CommandBuffer );
}

//-------------------------------------------------------------------------------------------------
//      コマンドを実行し，完了を待機します.
//-------------------------------------------------------------------------------------------------
void CommandList::Flush()
{
    IQueue* pQueue;
    m_pDevice->GetGraphicsQueue(&pQueue);

    auto pWrapQueue = static_cast<Queue*>(pQueue);
    A3D_ASSERT(pWrapQueue != nullptr);

    auto pNativeQueue = pWrapQueue->GetVulkanQueue();
    A3D_ASSERT(pNativeQueue != null_handle);

    VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo info = {};
    info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.pNext                  = nullptr;
    info.waitSemaphoreCount     = 0;
    info.pWaitSemaphores        = nullptr;
    info.pWaitDstStageMask      = &waitDstStageMask;
    info.commandBufferCount     = 1;
    info.pCommandBuffers        = &m_CommandBuffer;
    info.signalSemaphoreCount   = 0;
    info.pSignalSemaphores      = nullptr;

    vkQueueSubmit(pNativeQueue, 1, &info, null_handle);
    vkQueueWaitIdle(pNativeQueue);

    SafeRelease(pQueue);
}

//-------------------------------------------------------------------------------------------------
//      コマンドプールを取得します.
//-------------------------------------------------------------------------------------------------
VkCommandPool CommandList::GetVulkanCommandPool() const
{ return m_CommandPool; }

//-------------------------------------------------------------------------------------------------
//      コマンドバッファを取得します.
//-------------------------------------------------------------------------------------------------
VkCommandBuffer CommandList::GetVulkanCommandBuffer() const
{ return m_CommandBuffer; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool CommandList::Create
(
    IDevice*            pDevice,
    COMMANDLIST_TYPE    listType,
    ICommandList**      ppCommandList
)
{
    if (pDevice == nullptr || ppCommandList == nullptr)
    { return false; }

    auto instance = new CommandList;
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, listType))
    {
        SafeRelease(instance);
        return false;
    }

    *ppCommandList = instance;
    return true;
}

} // namespace a3d
