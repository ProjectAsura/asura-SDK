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
, m_BindRenderPass  (false)
, m_DirtyDescriptor (false)
, m_pDescriptorSet  (nullptr)
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
        {
            A3D_LOG("Error : vkCreateCommandPool() Failed. VkResult = %s", ToString(ret));
            return false;
        }
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
        {
            A3D_LOG("Error : vkAllocateCommandBuffers() Failed. VkResult = %s", ToString(ret));
            return false;
        }
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
        {
            A3D_LOG("Error : VkAllocateCommandBuffers() Failed. VkResult = %s", ToString(ret));
            return false;
        }
    }

    for(auto i=0u; i<64; ++i)
    {
        m_WriteDescriptorSet[i].sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        m_WriteDescriptorSet[i].pNext               = nullptr;
        m_WriteDescriptorSet[i].dstSet              = null_handle;
        m_WriteDescriptorSet[i].dstBinding          = 0;
        m_WriteDescriptorSet[i].dstArrayElement     = 0;
        m_WriteDescriptorSet[i].descriptorCount     = 0;
        m_WriteDescriptorSet[i].descriptorType      = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        m_WriteDescriptorSet[i].pImageInfo          = nullptr;
        m_WriteDescriptorSet[i].pBufferInfo         = nullptr;
        m_WriteDescriptorSet[i].pTexelBufferView    = nullptr;
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

    m_pDescriptorSet    = nullptr;
    m_DirtyDescriptor   = false;
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
    A3D_ASSERT(!m_BindRenderPass);
    A3D_ASSERT(vkCmdBeginRendering != nullptr);
    if (renderTargetViewCount >= 8)
    { renderTargetViewCount = 8; }

    uint32_t width  = 0;
    uint32_t height = 0;

    VkRenderingAttachmentInfoKHR colorAttachments[8] = {};
    for(auto i=0u; i<renderTargetViewCount; ++i)
    {
        auto pWrapRTV = static_cast<RenderTargetView*>(pRenderTargetViews[i]);
        A3D_ASSERT(pWrapRTV != nullptr);

        colorAttachments[i].sType               = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        colorAttachments[i].pNext               = nullptr;
        colorAttachments[i].imageView           = pWrapRTV->GetVulkanImageView();
        colorAttachments[i].imageLayout         = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachments[i].resolveMode         = VK_RESOLVE_MODE_NONE;
        colorAttachments[i].resolveImageView    = null_handle;
        colorAttachments[i].resolveImageLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachments[i].loadOp              = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachments[i].storeOp             = VK_ATTACHMENT_STORE_OP_STORE;

        if (i == 0)
        {
            auto texDesc = pWrapRTV->GetTextureDesc();
            width  = texDesc.Width;
            height = texDesc.Height;
        }
    }

    VkRenderingAttachmentInfoKHR depthAttachment = {};
    if (pDepthStencilView != nullptr)
    {
        auto pWrapDSV = static_cast<DepthStencilView*>(pDepthStencilView);
        A3D_ASSERT(pWrapDSV != nullptr);

        auto desc = pWrapDSV->GetTextureDesc();

        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        if (desc.Format == RESOURCE_FORMAT_D24_UNORM_S8_UINT)
        { imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; }

        depthAttachment.sType               = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        depthAttachment.pNext               = nullptr;
        depthAttachment.imageView           = pWrapDSV->GetVulkanImageView();
        depthAttachment.imageLayout         = imageLayout;
        depthAttachment.resolveMode         = VK_RESOLVE_MODE_NONE;
        depthAttachment.resolveImageView    = null_handle;
        depthAttachment.resolveImageLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.loadOp              = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.storeOp             = VK_ATTACHMENT_STORE_OP_STORE;

        width   = desc.Width;
        height  = desc.Height;
    }

    VkRect2D area = {};
    area.offset.x       = 0;
    area.offset.y       = 0;
    area.extent.width   = width;
    area.extent.height  = height;

    VkRenderingInfoKHR info = {};
    info.sType                  = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    info.pNext                  = nullptr;
    info.flags                  = 0;
    info.renderArea             = area;
    info.layerCount             = 1;
    info.viewMask               = 0;
    info.colorAttachmentCount   = renderTargetViewCount;
    info.pColorAttachments      = colorAttachments;
    info.pDepthAttachment       = (pDepthStencilView != nullptr) ? &depthAttachment : nullptr;

    vkCmdBeginRendering(m_CommandBuffer, &info);
    m_BindRenderPass = true;

    VkClearAttachment clearAttachments[9] = {};
    VkClearRect       clearRects[9] = {};

    auto count = 0u;
    if (pClearColors != nullptr && clearColorCount > 0)
    {
        for(auto i=0u; i<clearColorCount; ++i)
        {
            clearAttachments[i].aspectMask                  = VK_IMAGE_ASPECT_COLOR_BIT;
            clearAttachments[i].clearValue.color.float32[0] = pClearColors[i].R;
            clearAttachments[i].clearValue.color.float32[1] = pClearColors[i].G;
            clearAttachments[i].clearValue.color.float32[2] = pClearColors[i].B;
            clearAttachments[i].clearValue.color.float32[3] = pClearColors[i].A;
            clearAttachments[i].colorAttachment             = pClearColors[i].SlotIndex;

            auto viewDesc = pRenderTargetViews[pClearColors[i].SlotIndex]->GetDesc();
            clearRects[i].baseArrayLayer = viewDesc.FirstArraySlice;
            clearRects[i].layerCount     = viewDesc.ArraySize;
            clearRects[i].rect           = area;
        }

        count += clearColorCount;
    }

    if (pClearDepthStencil != nullptr)
    {
        VkImageAspectFlags aspect = VK_IMAGE_ASPECT_NONE_KHR;
        if (pClearDepthStencil->EnableClearDepth)
        { aspect |= VK_IMAGE_ASPECT_DEPTH_BIT; }
        if (pClearDepthStencil->EnableClearStencil)
        { aspect |= VK_IMAGE_ASPECT_STENCIL_BIT; }

        clearAttachments[count].aspectMask                      = aspect;
        clearAttachments[count].clearValue.depthStencil.depth   = pClearDepthStencil->Depth;
        clearAttachments[count].clearValue.depthStencil.stencil = uint32_t(pClearDepthStencil->Stencil);

        auto viewDesc = pDepthStencilView->GetDesc();
        clearRects[count].baseArrayLayer = viewDesc.FirstArraySlice;
        clearRects[count].layerCount     = viewDesc.ArraySize;
        clearRects[count].rect           = area;

        count++;
    }

    if (count > 0)
    { vkCmdClearAttachments(m_CommandBuffer, count, clearAttachments, count, clearRects); }
}

//-------------------------------------------------------------------------------------------------
//      フレームバッファを解除します.
//-------------------------------------------------------------------------------------------------
void CommandList::EndFrameBuffer()
{
    A3D_ASSERT(m_BindRenderPass);
    A3D_ASSERT(vkCmdEndRendering != nullptr);
    vkCmdEndRendering(m_CommandBuffer);
    m_BindRenderPass = false;
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

    if (count >= 16)
    { count = 16; }

    VkViewport viewports[16] = {};

    for(auto i=0u; i<count; ++i)
    {
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

    if (count >= 16)
    { count = 16; }

    VkRect2D rects[16] = {};

    for(auto i=0u; i<count; ++i)
    {
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
    {
        m_pDescriptorSet = nullptr;
        return;
    }

    auto pWrapDescriptorSet = static_cast<DescriptorSet*>(pDescriptorSet);
    A3D_ASSERT( pWrapDescriptorSet != nullptr );

    m_pDescriptorSet  = pWrapDescriptorSet;
    m_DirtyDescriptor = true;
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
    VkBuffer     buffers[32] = {};
    VkDeviceSize offsets[32] = {};

    if (count == 0 || ppResources == nullptr)
    {
        vkCmdBindVertexBuffers( m_CommandBuffer, 0, 32, buffers, offsets );
        return;
    }

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
    {
        vkCmdBindIndexBuffer( m_CommandBuffer, null_handle, 0, VK_INDEX_TYPE_NONE_KHR );
        return;
    }

    auto pWrapResource = static_cast<Buffer*>(pResource);
    A3D_ASSERT( pWrapResource != nullptr );

    auto type = (pWrapResource->GetDesc().Stride == sizeof(uint16_t))
                ? VK_INDEX_TYPE_UINT16 
                : VK_INDEX_TYPE_UINT32;

    vkCmdBindIndexBuffer( m_CommandBuffer, pWrapResource->GetVulkanBuffer(), offset, type );
}

//-------------------------------------------------------------------------------------------------
//      定数バッファビュー設定をします.
//-------------------------------------------------------------------------------------------------
void CommandList::SetView(uint32_t index, IConstantBufferView* const pResource)
{
    A3D_ASSERT(index < 64);

    auto pWrapCBV = static_cast<ConstantBufferView*>(pResource);
    A3D_ASSERT(pWrapCBV != nullptr);

    const auto& desc = pWrapCBV->GetDesc();

    m_DescriptorInfo[index].Buffer.buffer = pWrapCBV->GetVulkanBuffer();
    m_DescriptorInfo[index].Buffer.offset = desc.Offset;
    m_DescriptorInfo[index].Buffer.range  = desc.Range;
    m_DescriptorInfo[index].StorageBuffer = false;
    m_DirtyDescriptor = true;
}

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetView(uint32_t index, IShaderResourceView* const pResource)
{
    A3D_ASSERT(index < 64 );

    auto pWrapSRV = static_cast<ShaderResourceView*>(pResource);
    A3D_ASSERT(pWrapSRV != nullptr);

    auto desc = pWrapSRV->GetDesc();

    auto kind = pWrapSRV->GetResource()->GetKind();

    if (kind == RESOURCE_KIND_BUFFER)

    {
        m_DescriptorInfo[index].Buffer.buffer   = pWrapSRV->GetVulkanBuffer();
        m_DescriptorInfo[index].Buffer.offset   = desc.FirstElement;
        m_DescriptorInfo[index].Buffer.range    = desc.ElementCount;
        m_DescriptorInfo[index].StorageBuffer   = false;
    }
    else if (kind == RESOURCE_KIND_TEXTURE)
    {
        m_DescriptorInfo[index].Image.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        m_DescriptorInfo[index].Image.imageView   = pWrapSRV->GetVulkanImageView();
        m_DescriptorInfo[index].StorageBuffer     = false;
    }
    m_DirtyDescriptor = true;
}

//-------------------------------------------------------------------------------------------------
//      アンオーダードアクセスビューを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetView(uint32_t index, IUnorderedAccessView* const pResource)
{
    A3D_ASSERT(index < 64 );

    auto pWrapView = static_cast<UnorderedAccessView*>(pResource);
    A3D_ASSERT(pWrapView != nullptr);

    auto desc = pWrapView->GetDesc();

    auto kind = pWrapView->GetResource()->GetKind();
    if (kind == RESOURCE_KIND_BUFFER)
    {
        m_DescriptorInfo[index].Buffer.buffer = pWrapView->GetVulkanBuffer();
        m_DescriptorInfo[index].Buffer.offset = desc.FirstElement;
        m_DescriptorInfo[index].Buffer.range  = desc.ElementCount;
        m_DescriptorInfo[index].StorageBuffer = true;
    }
    else if (kind == RESOURCE_KIND_TEXTURE)
    {
        m_DescriptorInfo[index].Image.imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        m_DescriptorInfo[index].Image.imageView     = pWrapView->GetVulkanImageView();
        m_DescriptorInfo[index].StorageBuffer       = false;
    }
    m_DirtyDescriptor = true;
}

//-------------------------------------------------------------------------------------------------
//      サンプラーを設定します.
//-------------------------------------------------------------------------------------------------
void CommandList::SetSampler(uint32_t index, ISampler* pSampler)
{
    A3D_ASSERT(index < 64 );

    auto pWrapSampler = static_cast<Sampler*>(pSampler);
    A3D_ASSERT(pWrapSampler != nullptr);

    m_DescriptorInfo[index].Image.sampler = pWrapSampler->GetVulkanSampler();
    m_DirtyDescriptor = true;
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
    if (pResource == nullptr || prevState == nextState)
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
    if (pResource == nullptr || prevState == nextState)
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

    UpdateDescriptor();

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

    UpdateDescriptor();

    vkCmdDrawIndexed(
        m_CommandBuffer,
        indexCount,
        instanceCount,
        firstIndex,
        vertexOffset,
        firstInstance );
}

//-------------------------------------------------------------------------------------------------
//      コンピュートシェーダを起動します.
//-------------------------------------------------------------------------------------------------
void CommandList::DispatchCompute(uint32_t x, uint32_t y, uint32_t z)
{
    if (x == 0 && y == 0 && z == 0)
    { return; }

    UpdateDescriptor();

    vkCmdDispatch( m_CommandBuffer, x, y, z );
}

//-------------------------------------------------------------------------------------------------
//      メッシュシェーダを起動します.
//-------------------------------------------------------------------------------------------------
void CommandList::DispatchMesh(uint32_t x, uint32_t y, uint32_t z)
{
    if (x == 0 && y == 0 && z == 0)
    { return; }

    // NVIDIA拡張ではVulkanでは y=1, z=1 に固定とのことなので，
    // 条件を満たしていなければ実行できない扱いにする.
    if (y != 1 || z != 1)
    {
        A3D_ASSERT(y == 1);
        A3D_ASSERT(z == 1);

        // Release版を考慮して処理を抜ける.
        return;
    }

    if (vkCmdDrawMeshTasks == nullptr)
    { return; }

    UpdateDescriptor();

    vkCmdDrawMeshTasks( m_CommandBuffer, x, 0 );
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

    UpdateDescriptor();

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

        case INDIRECT_ARGUMENT_TYPE_DISPATCH_COMPUTE:
            { vkCmdDispatchIndirect( m_CommandBuffer, pNativeArgumentBuffer, offset ); }
            break;

        case INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH:
            { vkCmdDrawMeshTasksIndirect( m_CommandBuffer, pNativeArgumentBuffer, offset, count, desc.ByteStride ); }
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
//      クエリをリセットします.
//-------------------------------------------------------------------------------------------------
void CommandList::ResetQuery(IQueryPool* pQuery)
{
    if (pQuery == nullptr)
    { return; }

    auto pWrapQueryPool = static_cast<QueryPool*>(pQuery);
    A3D_ASSERT(pWrapQueryPool != nullptr);

    auto pNativeQueryPool = pWrapQueryPool->GetVulkanQueryPool();
    A3D_ASSERT(pNativeQueryPool != null_handle);

    vkCmdResetQueryPool( m_CommandBuffer, pNativeQueryPool, 0, pQuery->GetDesc().Count );
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
//      ディスクリプタを更新します.
//-------------------------------------------------------------------------------------------------
void CommandList::UpdateDescriptor()
{
    if (!m_DirtyDescriptor || m_pDescriptorSet == nullptr)
    { return; }

    auto layout = m_pDescriptorSet->GetLayout();
    auto desc   = layout->GetDesc();
    auto count  = desc.EntryCount;

    for(auto i=0u; i<count; ++i)
    {
        m_WriteDescriptorSet[i].dstSet          = m_pDescriptorSet->GetVkDescriptorSet();
        m_WriteDescriptorSet[i].dstBinding      = i;
        m_WriteDescriptorSet[i].descriptorCount = 1;

        if (desc.Entries[i].Type == DESCRIPTOR_TYPE_CBV)
        {
            m_WriteDescriptorSet[i].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            m_WriteDescriptorSet[i].pBufferInfo     = &m_DescriptorInfo[i].Buffer;
            m_WriteDescriptorSet[i].pImageInfo      = nullptr;
        }
        else if (desc.Entries[i].Type == DESCRIPTOR_TYPE_SRV)
        {
            m_WriteDescriptorSet[i].descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            m_WriteDescriptorSet[i].pImageInfo      = &m_DescriptorInfo[i].Image;
            m_WriteDescriptorSet[i].pBufferInfo     = nullptr;
        }
        else if (desc.Entries[i].Type == DESCRIPTOR_TYPE_UAV)
        {
            if (m_DescriptorInfo[i].StorageBuffer)
            {
                m_WriteDescriptorSet[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                m_WriteDescriptorSet[i].pBufferInfo    = &m_DescriptorInfo[i].Buffer;
                m_WriteDescriptorSet[i].pImageInfo     = nullptr;
            }
            else
            {
                m_WriteDescriptorSet[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                m_WriteDescriptorSet[i].pImageInfo     = &m_DescriptorInfo[i].Image;
                m_WriteDescriptorSet[i].pBufferInfo    = nullptr;
            }
        }
        else if (desc.Entries[i].Type == DESCRIPTOR_TYPE_SMP)
        {
            m_WriteDescriptorSet[i].descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER;
            m_WriteDescriptorSet[i].pImageInfo      = &m_DescriptorInfo[i].Image;
            m_WriteDescriptorSet[i].pBufferInfo     = nullptr;
        }
    }

    vkCmdPushDescriptorSet(
        m_CommandBuffer,
        layout->GetVulkanPipelineBindPoint(),
        layout->GetVulkanPipelineLayout(),
        0,                                      // 0番目を更新.
        count,
        m_WriteDescriptorSet);

    m_DirtyDescriptor = false;
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

    auto instance = new CommandList;
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, pDesc->Type))
    {
        SafeRelease(instance);
        return false;
    }

    *ppCommandList = instance;
    return true;
}

} // namespace a3d
