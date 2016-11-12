﻿//-------------------------------------------------------------------------------------------------
// File : a3dDevice.cpp
// Desc : Device Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstdio>


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
// Global Variables.
//-------------------------------------------------------------------------------------------------
VkDebugReportCallbackEXT            vkDebugReportCallback           = null_handle;
PFN_vkCreateDebugReportCallbackEXT  vkCreateDebugReportCallback     = nullptr;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback    = nullptr;
PFN_vkDebugReportMessageEXT         vkDebugReportMessage            = nullptr;
size_t g_AllocationSize[VK_SYSTEM_ALLOCATION_SCOPE_RANGE_SIZE];


//-------------------------------------------------------------------------------------------------
//      ログ出力を行います.
//-------------------------------------------------------------------------------------------------
void OutputLog(const char* format, ...)
{
    #if A3D_IS_WIN
        char temp[2048];
        va_list va;
        va_start(va, format);
        vsprintf_s( temp, format, va );
        va_end(va);

        OutputDebugStringA(temp);
        printf_s( "%s", temp );
    #else
        va_list va;
        va_start(va, format);
        vprintf(format, va);
        va_end(va);
    #endif
}

//-------------------------------------------------------------------------------------------------
//      メモリ確保処理.
//-------------------------------------------------------------------------------------------------
VKAPI_ATTR
void* VKAPI_CALL Alloc
(
    void*                   pUserData,
    size_t                  size,
    size_t                  alignment,
    VkSystemAllocationScope scope
)
{
    A3D_UNUSED(pUserData);
    g_AllocationSize[scope] += size;
    return a3d::a3d_alloc(size, alignment);
}

//-------------------------------------------------------------------------------------------------
//      メモリ再確保処理.
//-------------------------------------------------------------------------------------------------
VKAPI_ATTR 
void* VKAPI_CALL Realloc
(
    void*                   pUserData,
    void*                   pOriginal,
    size_t                  size,
    size_t                  alignment,
    VkSystemAllocationScope scope
)
{
    A3D_UNUSED(pUserData);
    A3D_UNUSED(scope);
    return a3d::a3d_realloc(pUserData, size, alignment);
}

//-------------------------------------------------------------------------------------------------
//      メモリ解放処理.
//-------------------------------------------------------------------------------------------------
VKAPI_ATTR
void VKAPI_CALL Free(void* pUserData, void* pMemory)
{
    A3D_UNUSED(pUserData);
    a3d::a3d_free(pMemory);
}

//-------------------------------------------------------------------------------------------------
//      デバッグログを出力します.
//-------------------------------------------------------------------------------------------------
VKAPI_ATTR
VkBool32 VKAPI_CALL DebugReport
(
    VkFlags                     msgFlags,
    VkDebugReportObjectTypeEXT  objType,
    uint64_t                    srcObject,
    size_t                      location,
    int32_t                     msgCode,
    const char*                 pLayerPrefix,
    const char*                 pMsg,
    void*                       pUserData
)
{
    A3D_UNUSED(objType);
    A3D_UNUSED(srcObject);
    A3D_UNUSED(location);
    A3D_UNUSED(pUserData);

    if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        OutputLog( "Error [%s] Code %d : ", pLayerPrefix, msgCode );
    }
    else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        OutputLog( "Warning [%s] Code %d : ", pLayerPrefix, msgCode );
    }
    else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
    {
        OutputLog( "Information [%s] Code %d : ", pLayerPrefix, msgCode );
    }
    else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
    {
        OutputLog( "Debug [%s] Code %d : ", pLayerPrefix, msgCode );
    }
    else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
    {
        OutputLog( "Performance Warning [%s] Code %d : ", pLayerPrefix, msgCode );
    }

    OutputLog( pMsg );
    OutputLog( "\n" );

    return VK_TRUE;
}

//-------------------------------------------------------------------------------------------------
//      インスタンスプロシージャアドレスを取得します.
//-------------------------------------------------------------------------------------------------
template<typename T>
inline T GetProc(VkInstance instance, const char* name)
{ return reinterpret_cast<T>(vkGetInstanceProcAddr(instance, name)); }

//-------------------------------------------------------------------------------------------------
//      デバイスプロシージャアドレスを取得します.
//-------------------------------------------------------------------------------------------------
template<typename T>
inline T GetProc(VkDevice device, const char* name)
{ return reinterpret_cast<T>(vkGetDeviceProcAddr(device, name)); }

} // namespace /* anonymous */


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Device class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Device::Device()
: m_RefCount            (1)
, m_Instance            (null_handle)
, m_Device              (null_handle)
, m_PhysicalDeviceCount (0)
, m_pPhysicalDeviceInfos(nullptr)
, m_pGraphicsQueue      (nullptr)
, m_pComputeQueue       (nullptr)
, m_pCopyQueue          (nullptr)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Device::~Device()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理です.
//-------------------------------------------------------------------------------------------------
bool Device::Init(const DeviceDesc* pDesc)
{
    if (pDesc == nullptr)
    { return false; }

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    const char* layerExtension[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
    #if A3D_IS_WIN
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    #elif A3D_IS_LINUX
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,
    #elif A3D_IS_ANDROID
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
    #endif
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    };

    const char* layer[] = {
        "VK_LAYER_LUNARG_standard_validation",
    };

    uint32_t layerExtensionCount = 2;
    uint32_t layerCount = 0;

    if (pDesc->EnableDebug)
    {
        layerCount          = 1;
        layerExtensionCount = 3;
    }

    // インスタンスの生成.
    {
        VkApplicationInfo appInfo = {};
        appInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext               = nullptr;
        appInfo.pApplicationName    = "a3d";
        appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName         = "a3d";
        appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion          = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instanceInfo = {};
        instanceInfo.sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pNext                      = nullptr;
        instanceInfo.flags                      = 0;
        instanceInfo.pApplicationInfo           = &appInfo;
        instanceInfo.enabledLayerCount          = layerCount;
        instanceInfo.ppEnabledLayerNames        = (pDesc->EnableDebug) ? layer : nullptr;
        instanceInfo.enabledExtensionCount      = layerExtensionCount;
        instanceInfo.ppEnabledExtensionNames    = layerExtension;

        m_Allocator.pfnAllocation           = Alloc;
        m_Allocator.pfnFree                 = Free;
        m_Allocator.pfnReallocation         = Realloc;
        m_Allocator.pfnInternalAllocation   = nullptr;
        m_Allocator.pfnInternalFree         = nullptr;

        auto ret = vkCreateInstance(&instanceInfo, &m_Allocator, &m_Instance);
        if ( ret != VK_SUCCESS )
        { return false; }
    }
        
    if (pDesc->EnableDebug)
    {
        vkCreateDebugReportCallback  = GetProc<PFN_vkCreateDebugReportCallbackEXT>(
                                        m_Instance, "vkCreateDebugReportCallbackEXT");
        vkDestroyDebugReportCallback = GetProc<PFN_vkDestroyDebugReportCallbackEXT>(
                                        m_Instance, "vkDestroyDebugReportCallbackEXT");
        vkDebugReportMessage         = GetProc<PFN_vkDebugReportMessageEXT>(
                                        m_Instance, "vkDebugReportMessageEXT");

        if (vkCreateDebugReportCallback  != nullptr &&
            vkDestroyDebugReportCallback != nullptr &&
            vkDebugReportMessage         != nullptr)
        {
            VkFlags flags = VK_DEBUG_REPORT_ERROR_BIT_EXT;
            flags |= VK_DEBUG_REPORT_WARNING_BIT_EXT;
            //flags |= VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            //flags |= VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
            //flags |= VK_DEBUG_REPORT_DEBUG_BIT_EXT;

            VkDebugReportCallbackCreateInfoEXT info = {};
            info.sType          = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
            info.pNext          = nullptr;
            info.pfnCallback    = DebugReport;
            info.pUserData      = nullptr;
            info.flags          = flags;

            // vkCreateDebugReportCallback()で何故か2回メモリ確保がされてリークするので，カウンターを無効化.
            a3d_disable_counter();

            auto ret = vkCreateDebugReportCallback( 
                m_Instance,
                &info,
                nullptr,
                &vkDebugReportCallback );

            // 無効にしたカウンターを元に戻す.
            a3d_enable_counter();

            if ( ret != VK_SUCCESS )
            { return false; }
        }
    }

    // 物理デバイスの取得.
    {
        uint32_t count = 0;
        auto ret = vkEnumeratePhysicalDevices( m_Instance, &count, nullptr );
        if ( ret != VK_SUCCESS || count < 1 )
        { return false; }

        m_PhysicalDeviceCount = count;

        m_pPhysicalDeviceInfos = new PhysicalDeviceInfo [count];
        if (m_pPhysicalDeviceInfos == nullptr)
        { return false; }

        VkPhysicalDevice* gpus = new VkPhysicalDevice[count];
        if (gpus == nullptr)
        { return false; }

        ret = vkEnumeratePhysicalDevices( m_Instance, &count, gpus );
        if ( ret != VK_SUCCESS )
        {
            delete [] gpus;
            return false;
        }

        for(auto i=0u; i<count; ++i)
        {
            m_pPhysicalDeviceInfos[i].Device = gpus[i];
            vkGetPhysicalDeviceMemoryProperties( gpus[i], &m_pPhysicalDeviceInfos[i].MemoryProperty );
            vkGetPhysicalDeviceProperties( gpus[i], &m_pPhysicalDeviceInfos[i].DeviceProperty );
        }

        delete [] gpus;
    }

    auto physicalDevice = m_pPhysicalDeviceInfos[0].Device;

    // デバイスとキューの生成.
    {
        uint32_t propCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &propCount, nullptr );

        VkQueueFamilyProperties* pProps = new VkQueueFamilyProperties[propCount];
        if (pProps == nullptr)
        { return false; }

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &propCount, pProps);

        auto graphicsIndex = UINT32_MAX;
        auto computeIndex  = UINT32_MAX;
        auto transferIndex = UINT32_MAX;

        VkDeviceQueueCreateInfo* pQueueInfos = new VkDeviceQueueCreateInfo[propCount];
        if (pQueueInfos == nullptr)
        {
            delete [] pProps;
            return false;
        }

        auto graphicsQueueIndex = UINT32_MAX;
        auto computeQueueIndex  = UINT32_MAX;
        auto transferQueueindex = UINT32_MAX;

        auto queueIndex = 0u;
        auto totalQueueCount = 0;
        for(auto i=0u; i<propCount; ++i)
        {
            pQueueInfos[i].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            pQueueInfos[i].pNext            = nullptr;
            pQueueInfos[i].flags            = 0;
            pQueueInfos[i].queueCount       = pProps[i].queueCount;
            pQueueInfos[i].queueFamilyIndex = i;

            totalQueueCount += pProps[i].queueCount;
;
            if (pProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                if (graphicsIndex == UINT32_MAX)
                {
                    graphicsIndex = i;
                    graphicsQueueIndex = queueIndex;
                    queueIndex++;
                }
            }

            if (pProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                if (computeIndex == UINT32_MAX)
                {
                    computeIndex = i;
                    computeQueueIndex = queueIndex;
                    queueIndex++;
                }
            }

            if (pProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                if (transferIndex == UINT32_MAX)
                {
                    transferIndex = i;
                    transferQueueindex = queueIndex;
                    queueIndex++;
                }
            }
        }

        auto pPriorities = new float [totalQueueCount];
        if (pPriorities == nullptr)
        {
            delete [] pProps;
            delete [] pQueueInfos;
            return false;
        }

        memset( pPriorities, 0, sizeof(float) * totalQueueCount);

        auto offset = 0u;
        for(auto i=0u; i<propCount; ++i)
        {
            pQueueInfos[i].pQueuePriorities = &pPriorities[offset];
            offset += pProps[i].queueCount;
        }

        const char* deviceExtensions[]   = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        uint32_t    deviceExtensionCount = 1;

        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.pNext                    = nullptr;
        deviceInfo.queueCreateInfoCount     = propCount;
        deviceInfo.pQueueCreateInfos        = pQueueInfos;
        deviceInfo.enabledLayerCount        = layerCount;
        deviceInfo.ppEnabledLayerNames      = layer;
        deviceInfo.enabledExtensionCount    = deviceExtensionCount;
        deviceInfo.ppEnabledExtensionNames  = deviceExtensions;
        deviceInfo.pEnabledFeatures         = nullptr;

        auto ret = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &m_Device);

        delete[] pProps;
        delete[] pQueueInfos;
        delete[] pPriorities;

        if (ret != VK_SUCCESS )
        { return false; }

        if (!Queue::Create(this, graphicsIndex, graphicsQueueIndex, pDesc->MaxGraphicsQueueSubmitCount, &m_pGraphicsQueue))
        { return false; }

        if (!Queue::Create(this, computeIndex, computeQueueIndex, pDesc->MaxComputeQueueSubmitCount, &m_pComputeQueue))
        { return false; }

        if (!Queue::Create(this, transferIndex, transferQueueindex, pDesc->MaxCopyQueueSubmitCount, &m_pCopyQueue))
        { return false; }
    }

    // デバイス情報の設定.
    {
        auto& limits = m_pPhysicalDeviceInfos[0].DeviceProperty.limits;
        m_Info.ConstantBufferMemoryAlignment    = static_cast<uint32_t>(limits.minUniformBufferOffsetAlignment);
        m_Info.MaxTargetWidth                   = limits.maxFramebufferWidth;
        m_Info.MaxTargetHeight                  = limits.maxFramebufferHeight;
        m_Info.MaxTargetArraySize               = limits.maxFramebufferLayers;
        m_Info.MaxColorSampleCount              = static_cast<uint32_t>(limits.framebufferColorSampleCounts);
        m_Info.MaxDepthSampleCount              = static_cast<uint32_t>(limits.framebufferDepthSampleCounts);
        m_Info.MaxStencilSampleCount            = static_cast<uint32_t>(limits.framebufferStencilSampleCounts);
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void Device::Term()
{
    SafeRelease(m_pGraphicsQueue);
    SafeRelease(m_pComputeQueue);
    SafeRelease(m_pCopyQueue);

    if (m_Device != null_handle)
    {
        vkDeviceWaitIdle(m_Device);
        vkDestroyDevice(m_Device, nullptr);
        m_Device = null_handle;
    }

    if (m_pPhysicalDeviceInfos != nullptr)
    {
        delete [] m_pPhysicalDeviceInfos;
        m_pPhysicalDeviceInfos = nullptr;
    }

    if (m_Desc.EnableDebug)
    {
        if (vkDebugReportCallback != null_handle)
        {
            // vkDestroyDebugReportCallback()では，
            // 2回確保されたメモリーが1つしか解放されずリークするのでカウンターを無効化.
            a3d_disable_counter();

            vkDestroyDebugReportCallback(
                m_Instance,
                vkDebugReportCallback,
                nullptr);
            vkDebugReportCallback = null_handle;

            // 無効にしたカウンターを戻す.
            a3d_enable_counter();
        }

        if (vkCreateDebugReportCallback  != nullptr &&
            vkDestroyDebugReportCallback != nullptr &&
            vkDebugReportMessage         != nullptr)
        {
            vkCreateDebugReportCallback     = nullptr;
            vkDestroyDebugReportCallback    = nullptr;
            vkDebugReportMessage            = nullptr;
        }
    }

    if (m_Instance != null_handle)
    {
        vkDestroyInstance(m_Instance, &m_Allocator);
        m_Instance = null_handle;
    }

    #if 0
    //if (m_Desc.EnableDebug)
    //{
    //    OutputLog("----------------------------------------------------\n");
    //    for(auto i=0; i<VK_SYSTEM_ALLOCATION_SCOPE_RANGE_SIZE; ++i)
    //    { OutputLog("Info : scope = %d, Total Allocated Size = %zu\n", i, g_AllocationSize[i]); }
    //    OutputLog("----------------------------------------------------\n");
    //}
    #endif
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void Device::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void Device::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Device::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
DeviceDesc Device::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      デバイス情報を取得します.
//-------------------------------------------------------------------------------------------------
DeviceInfo Device::GetInfo() const
{ return m_Info; }

//-------------------------------------------------------------------------------------------------
//      グラフィックスキューを取得します.
//-------------------------------------------------------------------------------------------------
void Device::GetGraphicsQueue(IQueue** ppQueue)
{
    *ppQueue = m_pGraphicsQueue;
    if (m_pGraphicsQueue != nullptr)
    { m_pGraphicsQueue->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      コンピュートキューを取得します.
//-------------------------------------------------------------------------------------------------
void Device::GetComputeQueue(IQueue** ppQueue)
{
    *ppQueue = m_pComputeQueue;
    if (m_pComputeQueue != nullptr)
    { m_pComputeQueue->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      コピーキューを取得します.
//-------------------------------------------------------------------------------------------------
void Device::GetCopyQueue(IQueue** ppQueue)
{
    *ppQueue = m_pComputeQueue;
    if (m_pCopyQueue != nullptr)
    { m_pCopyQueue->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      コマンドリストを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateCommandList(COMMANDLIST_TYPE type, void* pOption, ICommandList** ppCommandList)
{ return CommandList::Create(this, type, pOption, ppCommandList); }

//-------------------------------------------------------------------------------------------------
//      スワップチェインを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateSwapChain(const SwapChainDesc* pDesc, ISwapChain** ppSwapChain)
{ return SwapChain::Create(this, pDesc, ppSwapChain); }

//-------------------------------------------------------------------------------------------------
//      バッファを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateBuffer(const BufferDesc* pDesc, IBuffer** ppResource)
{ return Buffer::Create(this, pDesc, ppResource); }

//-------------------------------------------------------------------------------------------------
//      バッファビューを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateBufferView
(
    IBuffer*                pBuffer,
    const BufferViewDesc*   pDesc,
    IBufferView**           ppBufferView
)
{ return BufferView::Create(this, pBuffer, pDesc, ppBufferView); }

//-------------------------------------------------------------------------------------------------
//      テクスチャを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateTexture(const TextureDesc* pDesc, ITexture** ppResource)
{ return Texture::Create(this, pDesc, ppResource); }

//-------------------------------------------------------------------------------------------------
//      テクスチャビューを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateTextureView
(
    ITexture*               pTexture,
    const TextureViewDesc*  pDesc,
    ITextureView**          ppTextureView
)
{ return TextureView::Create(this, pTexture, pDesc, ppTextureView); }

//-------------------------------------------------------------------------------------------------
//      サンプラーを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateSampler(const SamplerDesc* pDesc, ISampler** ppSampler)
{ return Sampler::Create(this, pDesc, ppSampler); }

//-------------------------------------------------------------------------------------------------
//      グラフィックスパイプラインステートを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateGraphicsPipeline(const GraphicsPipelineStateDesc* pDesc, IPipelineState** ppPipelineState)
{ return PipelineState::CreateAsGraphics(this, pDesc, ppPipelineState); }

//-------------------------------------------------------------------------------------------------
//      コンピュートパイプラインステートを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateComputePipeline(const ComputePipelineStateDesc* pDesc, IPipelineState** ppPipelineState)
{ return PipelineState::CreateAsCompute(this, pDesc, ppPipelineState); }

//-------------------------------------------------------------------------------------------------
//      ディスクリプタセットレイアウトを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateDescriptorSetLayout(const DescriptorSetLayoutDesc* pDesc, IDescriptorSetLayout** ppLayout)
{ return DescriptorSetLayout::Create(this, pDesc, ppLayout); }

//-------------------------------------------------------------------------------------------------
//      フレームバッファを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateFrameBuffer(const FrameBufferDesc* pDesc, IFrameBuffer** ppFrameBuffer)
{ return FrameBuffer::Create(this, pDesc, ppFrameBuffer); }

//-------------------------------------------------------------------------------------------------
//      クエリプールを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateQueryPool(const QueryPoolDesc* pDesc, IQueryPool** ppQueryPool)
{ return QueryPool::Create(this, pDesc, ppQueryPool); }

//-------------------------------------------------------------------------------------------------
//      コマンドセットを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateCommandSet(const CommandSetDesc* pDesc, ICommandSet** ppCommandSet)
{ return CommandSet::Create(this, pDesc, ppCommandSet); }

//-------------------------------------------------------------------------------------------------
//      フェンスを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateFence(IFence** ppFence)
{ return Fence::Create(this, ppFence); }

//-------------------------------------------------------------------------------------------------
//      インスタンスを取得します.
//-------------------------------------------------------------------------------------------------
VkInstance Device::GetVulkanInstance() const
{ return m_Instance; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
VkDevice Device::GetVulkanDevice() const
{ return m_Device; }

//-------------------------------------------------------------------------------------------------
//      物理デバイス数を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Device::GetVulkanPhysicalDeviceCount() const
{ return m_PhysicalDeviceCount; }

//-------------------------------------------------------------------------------------------------
//      物理デバイスを取得します.
//-------------------------------------------------------------------------------------------------
VkPhysicalDevice Device::GetVulkanPhysicalDevice(uint32_t index) const
{
    A3D_ASSERT(index < m_PhysicalDeviceCount);
    return m_pPhysicalDeviceInfos[index].Device;
}

//-------------------------------------------------------------------------------------------------
//      物理デバイスメモリプロパティを取得します.
//-------------------------------------------------------------------------------------------------
VkPhysicalDeviceMemoryProperties Device::GetVulkanPhysicalDeviceMemoryProperties(uint32_t index) const
{
    A3D_ASSERT(index < m_PhysicalDeviceCount);
    return m_pPhysicalDeviceInfos[index].MemoryProperty;
}

//-------------------------------------------------------------------------------------------------
//      物理デバイスプロパティを取得します.
//-------------------------------------------------------------------------------------------------
VkPhysicalDeviceProperties Device::GetVulkanPhysicalDeviceProperties(uint32_t index) const
{
    A3D_ASSERT(index < m_PhysicalDeviceCount);
    return m_pPhysicalDeviceInfos[index].DeviceProperty;
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタプールを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateVulkanDescriptorPool(uint32_t maxSet, VkDescriptorPool* pPool)
{
    VkDescriptorPoolSize poolSize[10] = {};
    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize[0].descriptorCount = m_Desc.MaxShaderResourceCount;

    poolSize[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSize[1].descriptorCount = m_Desc.MaxShaderResourceCount;

    poolSize[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    poolSize[2].descriptorCount = m_Desc.MaxShaderResourceCount;

    poolSize[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize[3].descriptorCount = m_Desc.MaxShaderResourceCount;

    poolSize[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    poolSize[4].descriptorCount = m_Desc.MaxShaderResourceCount;

    poolSize[5].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    poolSize[5].descriptorCount = m_Desc.MaxShaderResourceCount;

    poolSize[6].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSize[6].descriptorCount = m_Desc.MaxShaderResourceCount;

    poolSize[7].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSize[7].descriptorCount = m_Desc.MaxShaderResourceCount;

    poolSize[8].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSize[8].descriptorCount = m_Desc.MaxSamplerCount;

    poolSize[9].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize[9].descriptorCount = m_Desc.MaxSamplerCount;

    VkDescriptorPoolCreateInfo info = {};
    info.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.pNext          = nullptr;
    info.flags          = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    info.maxSets        = maxSet;
    info.poolSizeCount  = 10;
    info.pPoolSizes     = poolSize;

    auto ret = vkCreateDescriptorPool(m_Device, &info, nullptr, pPool);
    if (ret != VK_SUCCESS)
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Device::Create(const DeviceDesc* pDesc, IDevice** ppDevice)
{
    if (pDesc == nullptr || ppDevice == nullptr)
    { return false; }

    auto instance = new Device;
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDesc))
    {
        SafeRelease(instance);
        return false;
    }

    *ppDevice = instance;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      デバイスを生成します.
//-------------------------------------------------------------------------------------------------
bool A3D_APIENTRY CreateDevice(const DeviceDesc* pDesc, IDevice** ppDevice)
{
    if (pDesc == nullptr || ppDevice == nullptr)
    { return false; }

    for(auto i=0; i<VK_SYSTEM_ALLOCATION_SCOPE_RANGE_SIZE; ++i)
    { g_AllocationSize[i] = 0; }

    return Device::Create(pDesc, ppDevice);
}

} // namespace a3d
