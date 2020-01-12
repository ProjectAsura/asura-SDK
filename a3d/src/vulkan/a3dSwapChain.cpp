//-------------------------------------------------------------------------------------------------
// File : a3dSwapChain.cpp
// Desc : SwapChain Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "a3dVulkanFunc.h"


namespace {


} // namespace

namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// SwapChain class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
SwapChain::SwapChain()
: m_RefCount            (1)
, m_pDevice             (nullptr)
, m_pQueue              (nullptr)
, m_Surface             (null_handle)
, m_SwapChain           (null_handle)
, m_pBuffers            (nullptr)
, m_pImages             (nullptr)
, m_pImageViews         (nullptr)
, m_IsFullScreen        (false)
, m_SurfaceFormatCount  (0)
, m_pSurfaceFormats     (nullptr)
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

    SafeRelease(m_pQueue);
    SafeRelease(m_pDevice);

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    m_pDevice->GetGraphicsQueue(reinterpret_cast<IQueue**>(&m_pQueue));

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    #if A3D_IS_WIN
    {
        m_hInstance = static_cast<HINSTANCE>(pDesc->InstanceHandle);
        m_hWnd      = static_cast<HWND>(pDesc->WindowHandle);

        GetWindowRect(m_hWnd, &m_Rect);
        m_WindowStyle = GetWindowLong(m_hWnd, GWL_STYLE);

        if (pDesc->EnableFullScreen)
        {
            m_Desc.Extent.Width  = GetSystemMetrics(SM_CXSCREEN);
            m_Desc.Extent.Height = GetSystemMetrics(SM_CYSCREEN);
            SetFullScreenMode(pDesc->EnableFullScreen);
        }
    }
    #elif A3D_IS_LINUX
    { /* TODO : Implement. */ }
    #elif A3D_IS_ANDROID
    { /* DO_NOTHING */ }
    #elif A3D_IS_NX
    { /* DO_NMOTHING */ }
    #endif

    if (!InitSurface(&m_Surface))
    { return false; }

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    auto pNativePhysicalDevice = m_pDevice->GetVulkanPhysicalDevice(0);
    A3D_ASSERT(pNativePhysicalDevice != null_handle);

    // フォーマットをチェック
    m_ImageFormat   = VK_FORMAT_UNDEFINED;
    {
        auto familyIndex = m_pQueue->GetFamilyIndex();
        VkBool32 support = VK_FALSE;

        auto ret = vkGetPhysicalDeviceSurfaceSupportKHR(pNativePhysicalDevice, familyIndex, m_Surface, &support);
        if ( ret != VK_SUCCESS || support == VK_FALSE )
        { return false; }

        ret = vkGetPhysicalDeviceSurfaceFormatsKHR(pNativePhysicalDevice, m_Surface, &m_SurfaceFormatCount, nullptr);
        if ( ret != VK_SUCCESS )
        { return false; }

        m_pSurfaceFormats = new (std::nothrow) VkSurfaceFormatKHR [m_SurfaceFormatCount];
        if (m_pSurfaceFormats == nullptr)
        { return false; }

        ret = vkGetPhysicalDeviceSurfaceFormatsKHR(pNativePhysicalDevice, m_Surface, &m_SurfaceFormatCount, m_pSurfaceFormats);
        if ( ret != VK_SUCCESS )
        {
            delete [] m_pSurfaceFormats;
            m_SurfaceFormatCount = 0;
            return false;
        }

        bool isFind = false;

        auto nativeFormat     = ToNativeFormat(pDesc->Format);
        auto nativeColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        for(auto i=0u; i<m_SurfaceFormatCount; ++i)
        {
            if (nativeFormat     == m_pSurfaceFormats[i].format &&
                nativeColorSpace == m_pSurfaceFormats[i].colorSpace)
            {
                m_ImageFormat   = m_pSurfaceFormats[i].format;
                m_ColorSpace    = m_pSurfaceFormats[i].colorSpace;
                isFind          = true;
                break;
            }
        }

        if (!isFind)
        { return false; }
    }

    // バッファ数をチェック
    VkSurfaceCapabilitiesKHR capabilities;
    m_PreTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    {
        // 呼ばないとバリデーションレイヤーからエラー出力されるので，呼び出し必須!!
        auto ret = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            pNativePhysicalDevice, m_Surface, &capabilities);
        if ( ret != VK_SUCCESS )
        { return false; }

        if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        { m_PreTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; }
        else
        { m_PreTransform = capabilities.currentTransform; }

        if (capabilities.maxImageCount < m_Desc.BufferCount)
        { return false; }

        if (capabilities.minImageCount > m_Desc.BufferCount)
        { return false; }

        // 横幅を最大値に制限する.
        if (capabilities.maxImageExtent.width < m_Desc.Extent.Width)
        { m_Desc.Extent.Width = capabilities.maxImageExtent.width; }

        // 縦幅を最大値に制限する.
        if (capabilities.maxImageExtent.height < m_Desc.Extent.Height)
        { m_Desc.Extent.Height = capabilities.maxImageExtent.height; }
    }

    // 表示モードを選択.
    m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
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
                    m_PresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
                    break;
                }
            }
            else if (pDesc->SyncInterval == -1)
            {
                if (pPresentModes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
                {
                    m_PresentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
                    break;
                }
            }
            else
            {
                if (pPresentModes[i] == VK_PRESENT_MODE_FIFO_KHR)
                {
                    m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
                    break;
                }
            }
        }

        delete [] pPresentModes;
    }

    m_CompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[] = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };

    for(auto i=0u; i<sizeof(compositeAlphaFlags) / sizeof(compositeAlphaFlags[0]); ++i)
    {
        if (capabilities.supportedCompositeAlpha & compositeAlphaFlags[i])
        {
            m_CompositeAlpha = compositeAlphaFlags[i];
            break;
        }
    }

    // スワップチェインを生成
    {
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.surface                  = m_Surface;
        createInfo.minImageCount            = m_Desc.BufferCount;
        createInfo.imageFormat              = m_ImageFormat;
        createInfo.imageColorSpace          = m_ColorSpace;
        createInfo.imageExtent              = { m_Desc.Extent.Width, m_Desc.Extent.Height };
        createInfo.imageArrayLayers         = 1;
        createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
        createInfo.preTransform             = m_PreTransform;
        createInfo.compositeAlpha           = m_CompositeAlpha;
        createInfo.presentMode              = m_PresentMode;
        createInfo.clipped                  = VK_TRUE;
        createInfo.oldSwapchain             = null_handle;

        auto ret = vkCreateSwapchain(pNativeDevice, &createInfo, nullptr, &m_SwapChain);
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    // イメージを取得.
    {
        uint32_t chainCount;
        auto ret = vkGetSwapchainImages(pNativeDevice, m_SwapChain, &chainCount, nullptr);
        if ( ret != VK_SUCCESS )
        { return false; }

        if ( chainCount != pDesc->BufferCount )
        { return false; }

        m_pImages = new VkImage [chainCount];
        if ( m_pImages == nullptr )
        { return false; }

        for(auto i=0u; i<chainCount; ++i)
        {  m_pImages[i] = null_handle; }

        m_pImageViews = new VkImageView [chainCount];
        if ( m_pImageViews == nullptr )
        { return false; }

        for(auto i=0u; i<chainCount; ++i)
        {  m_pImageViews[i] = null_handle; }

        ret = vkGetSwapchainImages(pNativeDevice, m_SwapChain, &chainCount, m_pImages);
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    // イメージビューを生成.
    {
        VkImageSubresourceRange range = {};
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.layerCount = 1;
        range.levelCount = m_Desc.MipLevels;

        for(auto i=0u; i<m_Desc.BufferCount; ++i)
        {
            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext            = nullptr;
            viewInfo.flags            = 0;
            viewInfo.image            = m_pImages[i];
            viewInfo.viewType         = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format           = m_ImageFormat;
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
        m_pBuffers = new Texture* [m_Desc.BufferCount];
        if ( m_pBuffers == nullptr )
        { return false; }

        for(auto i=0u; i<m_Desc.BufferCount; ++i)
        {
            if (!Texture::Create(
                pDevice,
                &m_Desc,
                m_pImages[i],
                m_pImageViews[i],
                reinterpret_cast<ITexture**>(&m_pBuffers[i])))
            { return false; }
        }
    }

    // イメージレイアウトを変更.
    {
        ICommandList* pCmdList;
        if (!m_pDevice->CreateCommandList(COMMANDLIST_TYPE_DIRECT, &pCmdList))
        { return false; }

        auto pWrapCmdList = static_cast<CommandList*>(pCmdList);
        pWrapCmdList->Begin();

        auto cmdBuffer = pWrapCmdList->GetVulkanCommandBuffer();

        for(auto i=0u; i<m_Desc.BufferCount; ++i)
        { 
            VkImageMemoryBarrier barrier = {};
            barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.pNext               = nullptr;
            barrier.srcAccessMask       = 0;
            barrier.dstAccessMask       = VK_ACCESS_MEMORY_READ_BIT;
            barrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            barrier.srcQueueFamilyIndex = 0;
            barrier.dstQueueFamilyIndex = 0;
            barrier.image               = m_pBuffers[i]->GetVulkanImage();

            barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.baseMipLevel   = 0;
            barrier.subresourceRange.layerCount     = m_pBuffers[i]->GetDesc().DepthOrArraySize;
            barrier.subresourceRange.levelCount     = m_pBuffers[i]->GetDesc().MipLevels;

            vkCmdPipelineBarrier(
                cmdBuffer,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                0, 0, nullptr, 0, nullptr, 1, &barrier );
        }

        pWrapCmdList->End();
        pWrapCmdList->Flush();

        SafeRelease(pCmdList);
    }

    // バックバッファ取得.
    {
        auto index      = m_pQueue->GetCurrentBufferIndex();
        auto semaphore  = m_pQueue->GetVulkanWaitSemaphore(index);
        auto fence      = m_pQueue->GetVulkanFence(index);

        auto ret = vkAcquireNextImage(
            pNativeDevice,
            m_SwapChain,
            UINT64_MAX,
            semaphore,
            fence,
            &m_CurrentBufferIndex);
        if ( ret != VK_SUCCESS )
        { return false; }

        // 待機.
        vkWaitForFences(pNativeDevice, 1, &fence, VK_FALSE, UINT64_MAX);
        vkResetFences(pNativeDevice, 1, &fence);
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

    auto pNativeInstance = m_pDevice->GetVulkanInstance();
    A3D_ASSERT(pNativeInstance != null_handle);

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    // キューの完了を待機.
    if (m_pQueue != nullptr)
    {
        auto pNativeQueue = m_pQueue->GetVulkanQueue();
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
        vkDestroySwapchain( pNativeDevice, m_SwapChain, nullptr );
        m_SwapChain = null_handle;
    }

    if ( m_Surface != null_handle )
    {
        vkDestroySurfaceKHR( pNativeInstance, m_Surface, nullptr );
        m_Surface = null_handle;
    }

    SafeDeleteArray(m_pSurfaceFormats);
    m_SurfaceFormatCount = 0;

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

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    auto index      = m_pQueue->GetCurrentBufferIndex();
    auto semaphore  = m_pQueue->GetVulkanWaitSemaphore(index);
    uint64_t Infinite = 0xFFFFFFFF;

    auto ret = vkQueuePresentKHR(m_pQueue->GetVulkanQueue(), &info);
    A3D_ASSERT(ret == VK_SUCCESS);

    auto fence = m_pQueue->GetVulkanFence(index);
    A3D_ASSERT(fence != null_handle);

    ret = vkAcquireNextImageKHR(
        pNativeDevice,
        m_SwapChain,
        Infinite,
        semaphore,
        fence,
        &m_CurrentBufferIndex);
    A3D_ASSERT( ret == VK_SUCCESS );

    vkWaitForFences(pNativeDevice, 1, &fence, VK_FALSE, Infinite);
    vkResetFences(pNativeDevice, 1, &fence);
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
//      バッファをリサイズします.
//-------------------------------------------------------------------------------------------------
bool SwapChain::ResizeBuffers(uint32_t width, uint32_t height)
{
    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    m_pQueue->WaitIdle();

    auto pNativePhysicalDevice = m_pDevice->GetVulkanPhysicalDevice(0);
    A3D_ASSERT(pNativePhysicalDevice != null_handle);

    m_Desc.Extent.Width  = width;
    m_Desc.Extent.Height = height;

    // バッファ数をチェック
    VkSurfaceCapabilitiesKHR capabilities;
    m_PreTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    {
        // 再作成時も呼ばないとバリデーションレイヤーからエラー出力されるので，呼び出し必須!!
        auto ret = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            pNativePhysicalDevice, m_Surface, &capabilities);
        if ( ret != VK_SUCCESS )
        { return false; }

        if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        { m_PreTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; }
        else
        { m_PreTransform = capabilities.currentTransform; }

        if (capabilities.maxImageCount < m_Desc.BufferCount)
        { return false; }

        if (capabilities.minImageCount > m_Desc.BufferCount)
        { return false; }

        if (capabilities.maxImageExtent.width < m_Desc.Extent.Width)
        { m_Desc.Extent.Width = capabilities.maxImageExtent.width; }

        if (capabilities.maxImageExtent.height < m_Desc.Extent.Height)
        { m_Desc.Extent.Height = capabilities.maxImageExtent.height; }
    }

    // スワップチェインを生成
    VkSwapchainKHR swapChain = null_handle;
    {
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.surface                  = m_Surface;
        createInfo.minImageCount            = m_Desc.BufferCount;
        createInfo.imageFormat              = m_ImageFormat;
        createInfo.imageColorSpace          = m_ColorSpace;
        createInfo.imageExtent              = { capabilities.currentExtent.width, capabilities.currentExtent.height };
        createInfo.imageArrayLayers         = 1;
        createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
        createInfo.preTransform             = m_PreTransform;
        createInfo.compositeAlpha           = m_CompositeAlpha;
        createInfo.presentMode              = m_PresentMode;
        createInfo.clipped                  = VK_TRUE;
        createInfo.oldSwapchain             = m_SwapChain;

        auto ret = vkCreateSwapchainKHR(pNativeDevice, &createInfo, nullptr, &swapChain);
        if ( ret != VK_SUCCESS )
        { return false; }
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

    if (m_SwapChain != null_handle)
    {
        vkDestroySwapchainKHR(pNativeDevice, m_SwapChain, nullptr);
    }

    // 新しく生成したものに差し替え.
    m_SwapChain = swapChain;

    // イメージを取得.
    {
        uint32_t chainCount;
        auto ret = vkGetSwapchainImagesKHR(pNativeDevice, m_SwapChain, &chainCount, nullptr);
        if ( ret != VK_SUCCESS )
        { return false; }

        if ( chainCount != m_Desc.BufferCount )
        { return false; }

        m_pImages = new VkImage [chainCount];
        if ( m_pImages == nullptr )
        { return false; }

        for(auto i=0u; i<chainCount; ++i)
        {  m_pImages[i] = null_handle; }

        m_pImageViews = new VkImageView [chainCount];
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
        range.levelCount = m_Desc.MipLevels;

        for(auto i=0u; i<m_Desc.BufferCount; ++i)
        {
            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext            = nullptr;
            viewInfo.flags            = 0;
            viewInfo.image            = m_pImages[i];
            viewInfo.viewType         = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format           = m_ImageFormat;
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
        m_pBuffers = new Texture* [m_Desc.BufferCount];
        if ( m_pBuffers == nullptr )
        { return false; }

        for(auto i=0u; i<m_Desc.BufferCount; ++i)
        {
            if (!Texture::Create(
                m_pDevice,
                &m_Desc,
                m_pImages[i],
                m_pImageViews[i],
                reinterpret_cast<ITexture**>(&m_pBuffers[i])))
            { return false; }
        }
    }

    // イメージレイアウトを変更.
    {
        ICommandList* pCmdList;
        if (!m_pDevice->CreateCommandList(COMMANDLIST_TYPE_DIRECT, &pCmdList))
        { return false; }

        auto pWrapCmdList = static_cast<CommandList*>(pCmdList);
        pWrapCmdList->Begin();

        auto cmdBuffer = pWrapCmdList->GetVulkanCommandBuffer();

        for(auto i=0u; i<m_Desc.BufferCount; ++i)
        { 
            VkImageMemoryBarrier barrier = {};
            barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.pNext               = nullptr;
            barrier.srcAccessMask       = 0;
            barrier.dstAccessMask       = VK_ACCESS_MEMORY_READ_BIT;
            barrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            barrier.srcQueueFamilyIndex = 0;
            barrier.dstQueueFamilyIndex = 0;
            barrier.image               = m_pBuffers[i]->GetVulkanImage();

            barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.baseMipLevel   = 0;
            barrier.subresourceRange.layerCount     = m_pBuffers[i]->GetDesc().DepthOrArraySize;
            barrier.subresourceRange.levelCount     = m_pBuffers[i]->GetDesc().MipLevels;

            vkCmdPipelineBarrier(
                cmdBuffer,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                0, 0, nullptr, 0, nullptr, 1, &barrier );
        }

        pWrapCmdList->End();
        pWrapCmdList->Flush();
        SafeRelease(pCmdList);
    }

    // バッファ番号をリセット.
    m_CurrentBufferIndex = 0;

    // 同期オブジェクトをリセット
    m_pQueue->ResetSyncObject();

    // バックバッファ取得.
    {
        auto index      = m_pQueue->GetCurrentBufferIndex();
        auto semaphore  = m_pQueue->GetVulkanWaitSemaphore(index);
        auto fence      = m_pQueue->GetVulkanFence(index);
        A3D_ASSERT(semaphore != null_handle);
        A3D_ASSERT(fence != null_handle);

        uint64_t Infinite = 0xFFFFFFFF;

        auto ret = vkAcquireNextImageKHR(
            pNativeDevice,
            m_SwapChain,
            Infinite,
            semaphore, 
            fence,
            &m_CurrentBufferIndex);
        if ( ret != VK_SUCCESS )
        { return false; }

        // 取得できるまで待機.
        vkWaitForFences(pNativeDevice, 1, &fence, VK_FALSE, Infinite);
        vkResetFences(pNativeDevice, 1, &fence);
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      メタデータを設定します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::SetMetaData(META_DATA_TYPE type, void* pData)
{
    #if defined(VK_EXT_hdr_metadata)
        if (!m_pDevice->IsSupportExtension(Device::EXT_HDR_METADATA))
        { return false; }

        if (type == META_DATA_HDR10 && pData != nullptr)
        {
            auto pWrap = static_cast<MetaDataHDR10*>(pData);
            A3D_ASSERT(pWrap != nullptr);

            VkHdrMetadataEXT meta = {};
            meta.displayPrimaryRed.x        = pWrap->PrimaryR[0];
            meta.displayPrimaryRed.y        = pWrap->PrimaryR[1];
            meta.displayPrimaryGreen.x      = pWrap->PrimaryG[0];
            meta.displayPrimaryGreen.y      = pWrap->PrimaryG[1];
            meta.displayPrimaryBlue.x       = pWrap->PrimaryB[0];
            meta.displayPrimaryBlue.y       = pWrap->PrimaryB[1];
            meta.maxLuminance               = pWrap->MaxMasteringLuminance;
            meta.minLuminance               = pWrap->MinMasteringLuminance;
            meta.maxContentLightLevel       = pWrap->MaxContentLightLevel;
            meta.maxFrameAverageLightLevel  = pWrap->MaxFrameAverageLightLevel;

            auto pDevice = m_pDevice->GetVulkanDevice();
            vkSetHdrMetadata( pDevice, 1, &m_SwapChain, &meta );

            return true;
        }

        return false;
    #else
        /* NOT SUPPORT */
        A3D_UNUSED(type);
        A3D_UNUSED(pData);
        return false;
    #endif
}

//-------------------------------------------------------------------------------------------------
//      色空間がサポートされているかチェックします.
//-------------------------------------------------------------------------------------------------
bool SwapChain::CheckColorSpaceSupport(COLOR_SPACE_TYPE type)
{
    if (m_pSurfaceFormats == nullptr || m_SurfaceFormatCount == 0)
    { return false; }

    bool isFind = false;

    auto nativeFormat     = ToNativeFormat(m_Desc.Format);
    auto nativeColorSpace = ToNativeColorSpace(type);

    for(auto i=0u; i<m_SurfaceFormatCount; ++i)
    {
        if (nativeFormat     == m_pSurfaceFormats[i].format &&
            nativeColorSpace == m_pSurfaceFormats[i].colorSpace)
        {
            isFind = true;
            break;
        }
    }

    return isFind;
}

//-------------------------------------------------------------------------------------------------
//      色空間を設定します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::SetColorSpace(COLOR_SPACE_TYPE type)
{
    // Vulkanは対応するメソッドがないため非サポートです.
    return false;
}

//-------------------------------------------------------------------------------------------------
//      フルスクリーンモードかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool SwapChain::IsFullScreenMode() const
{ return m_IsFullScreen; }

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
bool SwapChain::InitSurface(VkSurfaceKHR* pSurface)
{
    auto pNativeInstance = m_pDevice->GetVulkanInstance();
    A3D_ASSERT(pNativeInstance != null_handle);

    VkWin32SurfaceCreateInfoKHR info = {};
    info.sType      = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    info.pNext      = nullptr;
    info.flags      = 0;
    info.hinstance  = m_hInstance;
    info.hwnd       = m_hWnd;

    auto ret = vkCreateWin32SurfaceKHR(pNativeInstance, &info, nullptr, pSurface);
    if (ret != VK_SUCCESS)
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      Windows向けにフルスクリーンモードを設定します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::SetFullScreenMode(bool enable)
{
    if (enable)
    {
        // ウィンドウサイズを保存しておく.
        GetWindowRect(m_hWnd, &m_Rect);

        // 余計なものを外す.
        auto style =  m_WindowStyle & 
                        ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME);

        // ウィンドウのスタイルを変更.
        SetWindowLong(m_hWnd, GWL_STYLE, style);

        DEVMODE devMode = {};
        devMode.dmSize = sizeof(DEVMODE);
        EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

        SetWindowPos(
            m_hWnd,
            HWND_TOPMOST,
            devMode.dmPosition.x,
            devMode.dmPosition.y,
            devMode.dmPosition.x + devMode.dmPelsWidth,
            devMode.dmPosition.y + devMode.dmPelsHeight,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);

        // 最大化.
        ShowWindow(m_hWnd, SW_MAXIMIZE);
    }
    else
    {
        // ウィンドウスタイルを元に戻す.
        SetWindowLong(m_hWnd, GWL_STYLE, m_WindowStyle);

        SetWindowPos(
            m_hWnd,
            HWND_NOTOPMOST,
            m_Rect.left,
            m_Rect.top,
            m_Rect.right - m_Rect.left,
            m_Rect.bottom - m_Rect.top,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);

        ShowWindow(m_hWnd, SW_NORMAL);
    }

    m_IsFullScreen = enable;

    return true;
}


#elif A3D_IS_LINUX
//-------------------------------------------------------------------------------------------------
//      Linux向けにサーフェイスを作成します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::InitSurface(VkSurfaceKHR* pSurface)
{
    VkXcbSurfaceCreateInfoKHR info = {};
    info.sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    info.pNext      = nullptr;
    info.flags      = 0;
    info.connection = static_cast<xcb_connection_t*>(m_Desc.InstanceHandle);
    info.windows    = static_cast<xcb_window_t>(m_Desc.WindowHandle);

    auto pNativeInstance = m_pDevice->GetVulkanInstance();
    A3D_ASSERT(pNativeInstance != null_handle);

    auto ret = vkCreateXcbSurfaceKHR(pNativeInstance, &info, nullptr, pSurface);
    if (ret != VK_SUCCESS)
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      Linux向けにフルスクリーンモードを設定します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::SetFullScreenMode(bool enable)
{
    xcb_intern_atom_cookie_t wm_state_ck    = getCookieForAtom("_NET_WM_STATE");
    xcb_intern_atom_cookie_t wm_state_fs_ck = getCookieForAtom("_NET_WM_STATE_FULLSCREEN");

    static const uint32_t _NET_WM_STATE_REMOVE = 0;    // remove/unset property
    static const uint32_t _NET_WM_STATE_ADD    = 1;    // add/set property

    xcb_client_message_event_t ev;
    ev.response_type    = XCB_CLIENT_MESSAGE;
    ev.type             = getReplyAtomFromCookie(wm_state_ck);
    ev.format           = 32;
    ev.window           = window;
    ev.data.data32[0]   = (enable) ? _NEW_WM_STATE_ADD : _NET_STATE_REMOVE;
    ev.data.data32[1]   = getReplyAtomFromCookie(wm_state_fs_ck);
    ev.data.data32[2]   = XCB_ATOM_NONE;
    ev.data.data32[3]   = 0;
    ev.data.data32[4]   = 0;

    xcb_send_event(
        connection,
        1,
        window,
        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
        reinterpret_cast<const char*>(&ev));

    m_IsFullScreen = enable;

    return false;
}

#elif A3D_IS_ANDROID
//-------------------------------------------------------------------------------------------------
//      Android向けにサーフェイスを作成します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::InitSurface(VkSurfaceKHR* pSurface)
{
    VkAndroidSurfaceCreateInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    info.pNext = nulllptr;
    info.flags = 0;
    info.window = static_cast<ANativeWindow*>(m_Desc.WindowHandle);

    auto pNativeInstance = m_pDevice->GetVulkanInstance();
    A3D_ASSERT(pNativeInstance != null_handle);

    auto ret = vkCreateAndroidSurfaceKHR(pNativeInstance, &info, nullptr, pSurface);
    if (ret != VK_SUCCESS)
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      Android向けにフルスクリーンモードを設定します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::SetFullScreenMode(bool enable)
{
    /* DO_NOTHING */
    m_IsFullScreen = enable;
    return true;
}

#elif A3D_IS_NX
//-------------------------------------------------------------------------------------------------
//      NX 向けにサーフェイスを作成します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::InitSurface(VkSurfaceKHR* pSurface)
{
    VkViSurfaceCreateInfoNN info = {};
    info.sType  = VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN;
    info.pNext  = nullptr;
    info.flags  = 0;
    info.window = m_Desc.WindowHandle;

    auto pNativeInstance = m_pDevice->GetVulkanInstance();
    A3D_ASSERT(pNativeInstance != null_handle);

    auto ret = vkCreateViSurfaceNN(pNativeInstance, &info, nullptr, pSurface);
    if (ret != VK_SUCCESS)
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      NX 向けにフルスクリーンモードを設定します.
//-------------------------------------------------------------------------------------------------
bool SwapChain::SetFullScreenMode(bool enable)
{
    /* DO_NOTHING */
    m_IsFullScreen = enable;
    return true;
}

#endif

} // namespace a3d
