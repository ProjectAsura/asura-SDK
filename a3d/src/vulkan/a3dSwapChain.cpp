//-------------------------------------------------------------------------------------------------
// File : a3dSwapChain.cpp
// Desc : SwapChain Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// SwapChain class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
SwapChain::SwapChain()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pQueue      (nullptr)
, m_Surface     (null_handle)
, m_SwapChain   (null_handle)
, m_pBuffers    (nullptr)
, m_pImages     (nullptr)
, m_pImageViews (nullptr)
{ memset( &m_Desc, 0, sizeof(m_Desc) ); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
SwapChain::~SwapChain()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool SwapChain::Init(IDevice* pDevice, const SwapChainDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    m_pDevice->GetGraphicsQueue(&m_pQueue);

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    if (!InitSurface())
    { return false; }

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    auto pNativePhysicalDevice = pWrapDevice->GetVulkanPhysicalDevice(0);
    A3D_ASSERT(pNativePhysicalDevice != null_handle);

    auto pWrapQueue = reinterpret_cast<Queue*>(m_pQueue);
    A3D_ASSERT(pWrapQueue != null_handle);

    // フォーマットをチェック
    auto imageFormat     = VK_FORMAT_UNDEFINED;
    auto imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    {
        auto familyIndex = pWrapQueue->GetFamilyIndex();
        VkBool32 support = VK_FALSE;

        auto ret = vkGetPhysicalDeviceSurfaceSupportKHR(pNativePhysicalDevice, familyIndex, m_Surface, &support);
        if ( ret != VK_SUCCESS || support == VK_FALSE )
        { return false; }

        uint32_t count = 0;
        ret = vkGetPhysicalDeviceSurfaceFormatsKHR(pNativePhysicalDevice, m_Surface, &count, nullptr);
        if ( ret != VK_SUCCESS )
        { return false; }

        auto pFormats = new (std::nothrow) VkSurfaceFormatKHR [count];
        if (pFormats == nullptr)
        { return false; }

        ret = vkGetPhysicalDeviceSurfaceFormatsKHR(pNativePhysicalDevice, m_Surface, &count, pFormats);
        if ( ret != VK_SUCCESS )
        {
            delete [] pFormats;
            return false;
        }

        bool isFind = false;

        auto nativeFormat = ToNativeFormat(pDesc->Format);
        for(auto i=0u; i<count; ++i)
        {
            if (nativeFormat == pFormats[i].format)
            {
                imageFormat     = pFormats[i].format;
                imageColorSpace = pFormats[i].colorSpace;
                isFind          = true;
                break;
            }
        }

        delete [] pFormats;

        if (!isFind)
        { return false; }
    }

    // バッファ数をチェック
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    {
        auto ret = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            pNativePhysicalDevice, m_Surface, &capabilities);
        if ( ret != VK_SUCCESS )
        { return false; }

        if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        { preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; }
        else
        { preTransform = capabilities.currentTransform; }

        if (capabilities.maxImageCount < pDesc->BufferCount)
        { return false; }
    }

    // 表示モードを選択.
    auto presentMode = VK_PRESENT_MODE_FIFO_KHR;
    {
        uint32_t presentModeCount;
        auto ret = vkGetPhysicalDeviceSurfacePresentModesKHR(
            pNativePhysicalDevice, m_Surface, &presentModeCount, nullptr);
        if (ret != VK_SUCCESS)
        { return false; }

        auto pPresentModes = new (std::nothrow) VkPresentModeKHR [presentModeCount];
        if (pPresentModes == nullptr)
        { return false; }

        ret = vkGetPhysicalDeviceSurfacePresentModesKHR(
            pNativePhysicalDevice, m_Surface, &presentModeCount, pPresentModes);
        if (ret != VK_SUCCESS)
        {
            delete [] pPresentModes;
            return false;
        }

        for(auto i=0u; i<presentModeCount; ++i)
        {
            if (pDesc->SyncInterval == 0)
            {
                if (pPresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
                {
                    presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
                    break;
                }
            }
            else if (pDesc->SyncInterval == -1)
            {
                if (pPresentModes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
                {
                    presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
                    break;
                }
            }
            else
            {
                if (pPresentModes[i] == VK_PRESENT_MODE_FIFO_KHR)
                {
                    presentMode = VK_PRESENT_MODE_FIFO_KHR;
                    break;
                }
            }
        }

        delete [] pPresentModes;
    }

    // スワップチェインを生成
    {
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.surface                  = m_Surface;
        createInfo.minImageCount            = pDesc->BufferCount;
        createInfo.imageFormat              = imageFormat;
        createInfo.imageColorSpace          = imageColorSpace;
        createInfo.imageExtent              = { pDesc->Extent.Width, pDesc->Extent.Height };
        createInfo.imageArrayLayers         = 1;
        createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
        createInfo.preTransform             = preTransform;
        createInfo.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode              = presentMode;
        createInfo.clipped                  = VK_TRUE;
        createInfo.oldSwapchain             = null_handle;

        auto ret = vkCreateSwapchainKHR(pNativeDevice, &createInfo, nullptr, &m_SwapChain);
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    // イメージを取得.
    {
        uint32_t chainCount;
        auto ret = vkGetSwapchainImagesKHR(pNativeDevice, m_SwapChain, &chainCount, nullptr);
        if ( ret != VK_SUCCESS )
        { return false; }

        if ( chainCount != pDesc->BufferCount )
        { return false; }

        m_pImages = new(std::nothrow) VkImage [chainCount];
        if ( m_pImages == nullptr )
        { return false; }

        for(auto i=0u; i<chainCount; ++i)
        {  m_pImages[i] = null_handle; }

        m_pImageViews = new(std::nothrow) VkImageView [chainCount];
        if ( m_pImageViews == nullptr )
        { return false; }

        for(auto i=0u; i<chainCount; ++i)
        {  m_pImageViews[i] = null_handle; }

        ret = vkGetSwapchainImagesKHR(pNativeDevice, m_SwapChain, &chainCount, m_pImages);
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    // イメージビューを生成.
    {
        VkImageSubresourceRange range = {};
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.layerCount = 1;
        range.levelCount = pDesc->MipLevels;

        for(auto i=0u; i<m_Desc.BufferCount; ++i)
        {
            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext            = nullptr;
            viewInfo.flags            = 0;
            viewInfo.image            = m_pImages[i];
            viewInfo.viewType         = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format           = imageFormat;
            viewInfo.components.r     = VK_COMPONENT_SWIZZLE_R;
            viewInfo.components.g     = VK_COMPONENT_SWIZZLE_G;
            viewInfo.components.b     = VK_COMPONENT_SWIZZLE_B;
            viewInfo.components.a     = VK_COMPONENT_SWIZZLE_A;
            viewInfo.subresourceRange = range;

            auto ret = vkCreateImageView(pNativeDevice, &viewInfo, nullptr, &m_pImageViews[i]);
            if ( ret != VK_SUCCESS )
            { return false; }
        }
    }

    // リソース初期化
    {
        m_pBuffers = new (std::nothrow) ITexture* [m_Desc.BufferCount];
        if ( m_pBuffers == nullptr )
        { return false; }

        for(auto i=0u; i<m_Desc.BufferCount; ++i)
        {
            if (!Texture::Create(pDevice, pDesc, m_pImages[i], m_pImageViews[i], &m_pBuffers[i]))
            { return false; }
        }
    }

    // イメージレイアウトを変更.
    {
        ICommandList* pCmdList;
        if (!m_pDevice->CreateCommandList(COMMANDLIST_TYPE_DIRECT, nullptr, &pCmdList))
        { return false; }

        auto pWrapCmdList = reinterpret_cast<CommandList*>(pCmdList);
        pWrapCmdList->Begin();

        auto cmdBuffer = pWrapCmdList->GetVulkanCommandBuffer();

        for(auto i=0u; i<m_Desc.BufferCount; ++i)
        { 
            auto pWrapTexture = reinterpret_cast<Texture*>(m_pBuffers[i]);
            A3D_ASSERT(pWrapTexture != nullptr);

            VkImageMemoryBarrier barrier = {};
            barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.pNext               = nullptr;
            barrier.srcAccessMask       = 0;
            barrier.dstAccessMask       = VK_ACCESS_MEMORY_READ_BIT;
            barrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            barrier.srcQueueFamilyIndex = 0;
            barrier.dstQueueFamilyIndex = 0;
            barrier.image               = pWrapTexture->GetVulkanImage();

            barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.baseMipLevel   = 0;
            barrier.subresourceRange.layerCount     = pWrapTexture->GetDesc().DepthOrArraySize;
            barrier.subresourceRange.levelCount     = pWrapTexture->GetDesc().MipLevels;

            vkCmdPipelineBarrier(
                cmdBuffer,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                0, 0, nullptr, 0, nullptr, 1, &barrier );

            pWrapTexture->SetState( RESOURCE_STATE_PRESENT );
        }

        pWrapCmdList->End();
        pWrapCmdList->Flush();

        SafeRelease(pCmdList);
    }

    // バックバッファ取得.
    {
        auto ret = vkAcquireNextImageKHR(
            pNativeDevice,
            m_SwapChain,
            UINT64_MAX,
            pWrapQueue->GetVulkanWaitSemaphore(), 
            null_handle,
            &m_CurrentBufferIndex);
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void SwapChain::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeInstance = pWrapDevice->GetVulkanInstance();
    A3D_ASSERT(pNativeInstance != null_handle);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    // キューの完了を待機.
    if (m_pQueue != nullptr)
    {
        auto pWrapQueue = reinterpret_cast<Queue*>(m_pQueue);
        A3D_ASSERT(pWrapQueue != nullptr);

        auto pNativeQueue = pWrapQueue->GetVulkanQueue();
        A3D_ASSERT(pNativeQueue != null_handle);

        vkQueueWaitIdle(pNativeQueue);
    }

    if (m_pImageViews != nullptr)
    {
        for(auto i=0u; i<m_Desc.BufferCount; ++i)
        {
            if (m_pImageViews[i] != null_handle)
            {
                vkDestroyImageView(pNativeDevice, m_pImageViews[i], nullptr);
                m_pImageViews[i] = null_handle;
            }
        }

        delete [] m_pImageViews;
        m_pImageViews = nullptr;
    }

    if (m_pImages != nullptr)
    {
        // リソース側で解放するので, 存在しない場合のみ解放.
        if (m_pBuffers == nullptr)
        {
            for(auto i=0u; i<m_Desc.BufferCount; ++i)
            {
                if (m_pImages[i] != null_handle)
                {
                    vkDestroyImage(pNativeDevice, m_pImages[i], nullptr);
                    m_pImages[i] = null_handle;
                }
            }
        }

        delete [] m_pImages;
        m_pImages = nullptr;
    }

    if (m_pBuffers != nullptr)
    {
        for(auto i=0u; i<m_Desc.BufferCount; ++i)
        { SafeRelease(m_pBuffers[i]); }

        delete[] m_pBuffers;
        m_pBuffers = nullptr;
    }

    if ( m_SwapChain != null_handle )
    {
        vkDestroySwapchainKHR( pNativeDevice, m_SwapChain, nullptr );
        m_SwapChain = null_handle;
    }

    if ( m_Surface != null_handle )
    {
        vkDestroySurfaceKHR( pNativeInstance, m_Surface, nullptr );
        m_Surface = null_handle;
    }

    SafeRelease( m_pQueue );
    SafeRelease( m_pDevice );
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void SwapChain::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void SwapChain::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t SwapChain::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void SwapChain::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
SwapChainDesc SwapChain::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      画面に表示します.
//-------------------------------------------------------------------------------------------------
void SwapChain::Present()
{
    VkPresentInfoKHR info = {};
    info.sType          = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.pNext          = nullptr;
    info.swapchainCount = 1;
    info.pSwapchains    = &m_SwapChain;
    info.pImageIndices  = &m_CurrentBufferIndex;

    auto pWrapQueue = reinterpret_cast<Queue*>(m_pQueue);
    A3D_ASSERT(pWrapQueue != nullptr);

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    auto semaphore = pWrapQueue->GetVulkanWaitSemaphore();
    uint64_t Infinite = 0xFFFFFFFF;

    auto ret = vkQueuePresentKHR(pWrapQueue->GetVulkanQueue(), &info);
    A3D_ASSERT(ret == VK_SUCCESS);

    ret = vkAcquireNextImageKHR(pNativeDevice, m_SwapChain, Infinite, semaphore, null_handle, &m_CurrentBufferIndex);
    A3D_ASSERT( ret == VK_SUCCESS);
}

//-------------------------------------------------------------------------------------------------
//      現在のバッファ番号を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t SwapChain::GetCurrentBufferIndex()
{ return m_CurrentBufferIndex; }

//-------------------------------------------------------------------------------------------------
//      バッファを取得します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::GetBuffer(uint32_t index, ITexture** ppResource)
{
    if (index >= m_Desc.BufferCount)
    { return false; }

    *ppResource = m_pBuffers[index];
    if (m_pBuffers[index] != nullptr)
    {
        m_pBuffers[index]->AddRef();
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      メタデータを設定します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::SetMetaData(META_DATA_TYPE type, void* pData)
{
    /* NOT SUPPORT */
    A3D_UNUSED(type);
    A3D_UNUSED(pData);
    return false;
}

//-------------------------------------------------------------------------------------------------
//      色空間がサポートされているかチェックします.
//-------------------------------------------------------------------------------------------------
bool SwapChain::CheckColorSpaceSupport(COLOR_SPACE_TYPE type, uint32_t* pFlags)
{
    /* NOT SUPPROT */
    A3D_UNUSED(type);
    A3D_UNUSED(pFlags);
    return false;
}

//-------------------------------------------------------------------------------------------------
//      フルスクリーンモードかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool SwapChain::IsFullScreenMode() const
{
    /* TODO : Implement */
    // VulkanではOpenGL同様APIがサポートされないらしいので，OS依存になる 
    return false;
}

//-------------------------------------------------------------------------------------------------
//      フルスクリーンモードを設定します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::SetFullScreenMode(bool enable)
{
    A3D_UNUSED(enable);
    /* TODO : Implement */
    // VulkanではOpenGL同様APIがサポートされないらしいので，OS依存になる 
    return false;
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool SwapChain::Create
(
    IDevice*                pDevice,
    const SwapChainDesc*    pDesc,
    ISwapChain**            ppSwapChain
)
{
    if (pDevice == nullptr || pDesc == nullptr || ppSwapChain == nullptr)
    { return false; }

    auto instance = new SwapChain();
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, pDesc))
    {
        SafeRelease(instance);
        return false;
    }

    *ppSwapChain = instance;
    return true;
}

#if A3D_IS_WIN
//-------------------------------------------------------------------------------------------------
//      Windows向けにサーフェイスを作成します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::InitSurface()
{
    VkWin32SurfaceCreateInfoKHR info = {};
    info.sType      = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    info.pNext      = nullptr;
    info.flags      = 0;
    info.hinstance  = static_cast<HINSTANCE>(m_Desc.InstanceHandle);
    info.hwnd       = static_cast<HWND>(m_Desc.WindowHandle);

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeInstance = pWrapDevice->GetVulkanInstance();
    A3D_ASSERT(pNativeInstance != null_handle);

    auto ret = vkCreateWin32SurfaceKHR(pNativeInstance, &info, nullptr, &m_Surface);
    if (ret != VK_SUCCESS)
    { return false; }

    return true;
}

#elif A3D_IS_LINUX
//-------------------------------------------------------------------------------------------------
//      Linux向けにサーフェイスを作成します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::InitSurface()
{
    VkXcbSurfaceCreateInfoKHR info = {};
    info.sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    info.pNext      = nullptr;
    info.flags      = 0;
    info.connection = static_cast<xcb_connection_t*>(m_Desc.InstanceHandle);
    info.windows    = static_cast<xcb_window_t>(m_Desc.WindowHandle);

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeInstance = pWrapDevice->GetVulkanInstance();
    A3D_ASSERT(pNativeInstance != null_handle);

    auto ret = vkCreateXcbSurfaceKHR(pNativeInstance, &info, nullptr, &m_Surface);
    if (ret != VK_SUCCESS)
    { return false; }

    return true;
}

#elif A3D_IS_ANDROID
//-------------------------------------------------------------------------------------------------
//      Android向けにサーフェイスを作成します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::InitSurface()
{
    VkAndroidSurfaceCreateInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    info.pNext = nulllptr;
    info.flags = 0;
    info.window = static_cast<ANativeWindow*>(m_Desc.WindowHandle);

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeInstance = pWrapDevice->GetVulkanInstance();
    A3D_ASSERT(pNativeInstance != null_handle);

    auto ret = vkCreateAndroidSurfaceKHR(pNativeInstance, &info, nullptr, &m_Surface);
    if (ret != VK_SUCCESS)
    { return false; }

    return true;
}

#endif

} // namespace a3d
