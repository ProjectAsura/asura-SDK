//-------------------------------------------------------------------------------------------------
// File : a3dFrameBuffer.cpp
// Desc : Frame Buffer Implmentation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameBuffer class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
FrameBuffer::FrameBuffer()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_FrameBuffer (null_handle)
, m_RenderPass  (null_handle)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
FrameBuffer::~FrameBuffer()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool FrameBuffer::Init(IDevice* pDevice, const FrameBufferDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    VkAttachmentDescription attachmentDesc[9] = {};
    VkAttachmentReference   attachmentRefs[9] = {};
    VkImageView             imageViews    [9] = {};
    VkAttachmentReference*  pDepthAttachmentRef = nullptr;

    uint32_t attachmentCount = pDesc->ColorCount;
    uint32_t width  = 0;
    uint32_t height = 0;
    uint32_t layers = 0;
    {
        for(auto i=0u; i<pDesc->ColorCount; ++i)
        {
            auto pWrapTexture = reinterpret_cast<TextureView*>(pDesc->pColorTargets[i]);
            A3D_ASSERT(pWrapTexture != nullptr);

            auto& desc = pWrapTexture->GetTextureDesc();
            attachmentDesc[i].format            = ToNativeFormat(desc.Format);
            attachmentDesc[i].samples           = ToNativeSampleCountFlags(desc.SampleCount);
            attachmentDesc[i].loadOp            = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDesc[i].storeOp           = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDesc[i].stencilLoadOp     = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDesc[i].stencilStoreOp    = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDesc[i].initialLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentDesc[i].finalLayout       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentDesc[i].flags             = 0;

            attachmentRefs[i].attachment    = i;
            attachmentRefs[i].layout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            imageViews[i] = pWrapTexture->GetVulkanImageView();

            if (width == 0 && height == 0 && layers == 0)
            {
                width  = desc.Width;
                height = desc.Height;
                layers = desc.DepthOrArraySize;
            }
        }

        if (pDesc->pDepthTarget != nullptr)
        {
            attachmentCount++;

            auto pWrapTexture = reinterpret_cast<TextureView*>(pDesc->pDepthTarget);
            A3D_ASSERT(pWrapTexture != nullptr);

            auto idx   = pDesc->ColorCount;
            auto& desc = pWrapTexture->GetTextureDesc();
            attachmentDesc[idx].format          = ToNativeFormat(desc.Format);
            attachmentDesc[idx].samples         = ToNativeSampleCountFlags(desc.SampleCount);
            attachmentDesc[idx].loadOp          = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDesc[idx].storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDesc[idx].stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDesc[idx].stencilStoreOp  = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDesc[idx].initialLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachmentDesc[idx].finalLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            attachmentRefs[idx].attachment  = idx;
            attachmentRefs[idx].layout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            imageViews[idx] = pWrapTexture->GetVulkanImageView();

            pDepthAttachmentRef = &attachmentRefs[idx];

            if (width == 0 && height == 0 && layers == 0)
            {
                width  = desc.Width;
                height = desc.Height;
                layers = desc.DepthOrArraySize;
            }
        }
    }

    // レンダーパスを生成します.
    {
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.flags                   = 0;
        subpass.inputAttachmentCount    = 0;
        subpass.colorAttachmentCount    = pDesc->ColorCount;
        subpass.pColorAttachments       = attachmentRefs;
        subpass.pResolveAttachments     = nullptr;
        subpass.pDepthStencilAttachment = pDepthAttachmentRef;
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments    = nullptr;

        VkRenderPassCreateInfo info = {};
        info.sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.pNext              = nullptr;
        info.flags              = 0;
        info.attachmentCount    = attachmentCount;
        info.pAttachments       = attachmentDesc;
        info.subpassCount       = 1;
        info.pSubpasses         = &subpass;

        auto ret = vkCreateRenderPass(pNativeDevice, &info, nullptr, &m_RenderPass);
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    // フレームバッファを生成します.
    {
        VkFramebufferCreateInfo info = {};
        info.sType              = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.pNext              = nullptr;
        info.flags              = 0;
        info.renderPass         = m_RenderPass;
        info.attachmentCount    = attachmentCount;
        info.pAttachments       = imageViews;
        info.width              = width;
        info.height             = height;
        info.layers             = layers;

        auto ret = vkCreateFramebuffer(pNativeDevice, &info, nullptr, &m_FrameBuffer);
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    // レンダーパス開始情報を設定します.
    {
        m_BeginInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        m_BeginInfo.pNext                    = nullptr;
        m_BeginInfo.renderPass               = m_RenderPass;
        m_BeginInfo.framebuffer              = m_FrameBuffer;
        m_BeginInfo.renderArea.offset.x      = 0;
        m_BeginInfo.renderArea.offset.y      = 0;
        m_BeginInfo.renderArea.extent.width  = width;
        m_BeginInfo.renderArea.extent.height = height;
        m_BeginInfo.clearValueCount          = 0;
        m_BeginInfo.pClearValues             = nullptr;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void FrameBuffer::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    if ( m_FrameBuffer != null_handle )
    {
        vkDestroyFramebuffer( pNativeDevice, m_FrameBuffer, nullptr );
        m_FrameBuffer = null_handle;
    }

    if ( m_RenderPass != null_handle )
    {
        vkDestroyRenderPass( pNativeDevice, m_RenderPass, nullptr );
        m_RenderPass = null_handle;
    }

    SafeRelease( m_pDevice );
    memset(&m_Desc, 0, sizeof(m_Desc));
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void FrameBuffer::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void FrameBuffer::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t FrameBuffer::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void FrameBuffer::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      フレームバッファを設定するコマンドを発行します.
//-------------------------------------------------------------------------------------------------
void FrameBuffer::Bind(ICommandList* pCommandList)
{
    auto pWrapCommandList = reinterpret_cast<CommandList*>(pCommandList);
    A3D_ASSERT( pWrapCommandList != nullptr );

    auto pNativeCommandBuffer = pWrapCommandList->GetVulkanCommandBuffer();
    A3D_ASSERT( pNativeCommandBuffer != null_handle );

    vkCmdBeginRenderPass( pNativeCommandBuffer, &m_BeginInfo, VK_SUBPASS_CONTENTS_INLINE );
}

//-------------------------------------------------------------------------------------------------
//      フレームバッファをクリアするコマンドを発行します.
//-------------------------------------------------------------------------------------------------
void FrameBuffer::Clear
(
    ICommandList*                   pCommandList,
    uint32_t                        clearColorCount,
    const ClearColorValue*          pClearColors,
    const ClearDepthStencilValue*   pClearDepthStencil
)
{
    auto count = clearColorCount;
    auto pWrapCommandList = reinterpret_cast<CommandList*>(pCommandList);
    A3D_ASSERT( pWrapCommandList != nullptr );

    auto pNativeCommandBuffer = pWrapCommandList->GetVulkanCommandBuffer();
    A3D_ASSERT( pNativeCommandBuffer != null_handle );

    VkClearAttachment clearAttachment[9] = {};
    VkClearRect       clearRect[9] = {};

    if ( m_Desc.ColorCount > 0 && pClearColors != nullptr)
    {
        for(auto i=0u; i<clearColorCount; ++i)
        {
            auto pWrapResources = reinterpret_cast<TextureView*>(m_Desc.pColorTargets[i]);
            clearAttachment[i].clearValue.color.float32[0] = pClearColors[i].Float[0];
            clearAttachment[i].clearValue.color.float32[1] = pClearColors[i].Float[1];
            clearAttachment[i].clearValue.color.float32[2] = pClearColors[i].Float[2];
            clearAttachment[i].clearValue.color.float32[3] = pClearColors[i].Float[3];

            clearAttachment[i].aspectMask      = pWrapResources->GetVulkanImageAspectFlags();
            clearAttachment[i].colorAttachment = i;

            auto& desc     = pWrapResources->GetTextureDesc();
            auto& viewDesc = pWrapResources->GetDesc();
            clearRect[i].baseArrayLayer     = viewDesc.FirstArraySlice;
            clearRect[i].layerCount         = viewDesc.ArraySize;
            clearRect[i].rect.offset.x      = 0;
            clearRect[i].rect.offset.y      = 0;
            clearRect[i].rect.extent.width  = desc.Width;
            clearRect[i].rect.extent.height = desc.Height;
        }
    }

    if (m_Desc.pDepthTarget != nullptr && pClearDepthStencil != nullptr)
    {
        auto pWrapResources = reinterpret_cast<TextureView*>(m_Desc.pDepthTarget);

        VkImageAspectFlags mask = 0;
        if (pClearDepthStencil->EnableClearDepth)
        { mask |= VK_IMAGE_ASPECT_DEPTH_BIT; }
        if (pClearDepthStencil->EnableClearStencil)
        { mask |= VK_IMAGE_ASPECT_STENCIL_BIT; }

        if (mask != 0)
        {
            auto idx = count;
            count++;
            clearAttachment[idx].clearValue.depthStencil.depth   = pClearDepthStencil->Depth;
            clearAttachment[idx].clearValue.depthStencil.stencil = pClearDepthStencil->Stencil;

            clearAttachment[idx].aspectMask      = mask;
            clearAttachment[idx].colorAttachment = m_Desc.ColorCount;

            auto& desc     = pWrapResources->GetTextureDesc();
            auto& viewDesc = pWrapResources->GetDesc();
            clearRect[idx].baseArrayLayer       = viewDesc.FirstArraySlice;
            clearRect[idx].layerCount           = viewDesc.ArraySize;
            clearRect[idx].rect.offset.x        = 0;
            clearRect[idx].rect.offset.y        = 0;
            clearRect[idx].rect.extent.width    = desc.Width;
            clearRect[idx].rect.extent.height   = desc.Height;
        }
    }

    vkCmdClearAttachments(pNativeCommandBuffer, count, clearAttachment, count, clearRect);
}

//-------------------------------------------------------------------------------------------------
//      横幅を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t FrameBuffer::GetWidth() const
{ return m_BeginInfo.renderArea.extent.width; }

//-------------------------------------------------------------------------------------------------
//      縦幅を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t FrameBuffer::GetHeight() const
{ return m_BeginInfo.renderArea.extent.height; }

//-------------------------------------------------------------------------------------------------
//      カラーターゲット数を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t FrameBuffer::GetColorTargetCount() const
{ return m_Desc.ColorCount; }

//-------------------------------------------------------------------------------------------------
//      レンダーパスを取得します.
//-------------------------------------------------------------------------------------------------
VkRenderPass FrameBuffer::GetRenderPass() const
{ return m_RenderPass; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool FrameBuffer::Create(IDevice* pDevice, const FrameBufferDesc* pDesc, IFrameBuffer** ppFrameBuffer)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    auto instance = new FrameBuffer;
    if ( instance == nullptr )
    { return false; }

    if (!instance->Init(pDevice, pDesc))
    {
        SafeRelease(instance);
        return false;
    }

    *ppFrameBuffer = instance;
    return true;
}

} // namespace a3d
