//-------------------------------------------------------------------------------------------------
// File : a3dDevice.cpp
// Desc : Device Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstdio>
#include <cstdarg>
#include <allocator/a3dBaseAllocator.h>


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
    return a3d_alloc(size, alignment);
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
    return a3d_realloc(pUserData, size, alignment);
}

//-------------------------------------------------------------------------------------------------
//      メモリ解放処理.
//-------------------------------------------------------------------------------------------------
VKAPI_ATTR
void VKAPI_CALL Free(void* pUserData, void* pMemory)
{
    A3D_UNUSED(pUserData);
    a3d_free(pMemory);
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

#define GET_INSTANCE_PROC(instance, name) reinterpret_cast<PFN_##name>(vkGetInstanceProcAddr(instance, #name))
#define GET_DEVICE_PROC(device, name)     reinterpret_cast<PFN_##name>(vkGetDeviceProcAddr(device, #name))

#if 1
void CheckInstanceLayer
(
    size_t                      requestCount,
    const char**                requestName,
    a3d::dynamic_array<char*>&  result
)
{
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);

    if (count == 0)
    { return; }

    a3d::dynamic_array<VkLayerProperties> props;
    props.resize(count);

    vkEnumerateInstanceLayerProperties(&count, props.data());

    for(size_t i=0; i<props.size(); ++i)
    {
        bool hit = false;
        for(size_t j=0; j<requestCount; ++j)
        {
            if (strcmp(props[i].layerName, requestName[j]) == 0)
            {
                hit = true;
                break;
            }
        }

        if (!hit)
        { continue; }

        auto layerName = new char [VK_MAX_EXTENSION_NAME_SIZE];
        memset(layerName, 0, sizeof(char) * VK_MAX_EXTENSION_NAME_SIZE);
        memcpy(layerName, props[i].layerName, sizeof(char) * VK_MAX_EXTENSION_NAME_SIZE);
        result.push_back(layerName);
    }

    props.clear();
}
#endif

//-------------------------------------------------------------------------------------------------
//      インスタンス拡張機能をチェックします.
//-------------------------------------------------------------------------------------------------
void CheckInstanceExtension
(
    const char*     layer,
    size_t          requestCount,
    const char**    requestNames,
    a3d::dynamic_array<char*>& result
)
{
    uint32_t count;
    vkEnumerateInstanceExtensionProperties(layer, &count, nullptr);

    a3d::dynamic_array<VkExtensionProperties> temp;
    temp.resize(count);

    vkEnumerateInstanceExtensionProperties(layer, &count, temp.data());

    result.reserve(count);
    for(size_t i=0; i<temp.size(); ++i)
    {
        bool hit = false;
        for(size_t j=0; j<requestCount; ++j)
        {
            if (strcmp(temp[i].extensionName, requestNames[j]) == 0)
            {
                hit = true;
                break;
            }
        }

        if (!hit)
        { continue; }

        auto extname = new char[VK_MAX_EXTENSION_NAME_SIZE];
        memset(extname, 0, sizeof(char) * VK_MAX_EXTENSION_NAME_SIZE);
        memcpy(extname, temp[i].extensionName, sizeof(char) * VK_MAX_EXTENSION_NAME_SIZE);
        result.push_back(extname);
    }

    result.shrink_to_fit();
    temp.clear();
}

//-------------------------------------------------------------------------------------------------
//      デバイス拡張機能をチェックします.
//-------------------------------------------------------------------------------------------------
void CheckDeviceExtension
(
    const char*      layer,
    VkPhysicalDevice physicalDevice,
    a3d::dynamic_array<char*>& result
)
{
    uint32_t count;
    vkEnumerateDeviceExtensionProperties(physicalDevice, layer, &count, nullptr);

    VkExtensionProperties* temp;
    temp = static_cast<VkExtensionProperties*>(a3d_alloc(count * sizeof(VkExtensionProperties), 4));
    vkEnumerateDeviceExtensionProperties(physicalDevice, layer, &count, temp);

    const char* requestExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
        VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME,
        VK_NVX_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME,
        VK_AMD_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
        VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
        VK_EXT_HDR_METADATA_EXTENSION_NAME,
        VK_NV_MESH_SHADER_EXTENSION_NAME,
        //VK_NV_RAY_TRACING_EXTENSION_NAME,
    };

    result.reserve(count);
    for(size_t i=0; i<count; ++i)
    {
        auto hit = false;
        for(size_t j=0; j<sizeof(requestExtensions) / sizeof(requestExtensions[0]); ++j)
        {
            if (strcmp(temp[i].extensionName, requestExtensions[j]) == 0)
            {
                hit = true;
                break;
            }
        }

        if (!hit)
        { continue; }

        auto extname = new char[VK_MAX_EXTENSION_NAME_SIZE];
        memset(extname, 0, sizeof(char) * VK_MAX_EXTENSION_NAME_SIZE);
        memcpy(extname, temp[i].extensionName, sizeof(char) * VK_MAX_EXTENSION_NAME_SIZE);
        result.push_back(extname);
    }

    result.shrink_to_fit();
    a3d_free(temp);
}

} // namespace /* anonymous */

//-------------------------------------------------------------------------------------------------
//  Vulkan Device Extension Functions.
//-------------------------------------------------------------------------------------------------
#if defined(VK_KHR_swapchain)
PFN_vkCreateSwapchainKHR                     vkCreateSwapchain                       = nullptr;
PFN_vkDestroySwapchainKHR                    vkDestroySwapchain                      = nullptr;
PFN_vkGetSwapchainImagesKHR                  vkGetSwapchainImages                    = nullptr;
PFN_vkAcquireNextImageKHR                    vkAcquireNextImage                      = nullptr;
PFN_vkQueuePresentKHR                        vkQueuePresent                          = nullptr;
PFN_vkGetDeviceGroupPresentCapabilitiesKHR   vkGetDeviceGroupPresentCapabilities     = nullptr;
PFN_vkGetDeviceGroupSurfacePresentModesKHR   vkGetDeviceGroupSurfacePresentModes     = nullptr;
PFN_vkGetPhysicalDevicePresentRectanglesKHR  vkGetPhysicalDevicePresentRectangles    = nullptr;
PFN_vkAcquireNextImage2KHR                   vkAcquireNextImage2                     = nullptr;
#endif

#if defined(VK_EXT_debug_marker)
PFN_vkDebugMarkerSetObjectTagEXT     vkDebugMarkerSetObjectTag  = nullptr;
PFN_vkDebugMarkerSetObjectNameEXT    vkDebugMarkerSetObjectName = nullptr;
PFN_vkCmdDebugMarkerBeginEXT         vkCmdDebugMarkerBegin      = nullptr;
PFN_vkCmdDebugMarkerEndEXT           vkCmdDebugMarkerEnd        = nullptr;
PFN_vkCmdDebugMarkerInsertEXT        vkCmdDebugMarkerInsert     = nullptr;
#endif

#if defined(VK_KHR_push_descriptor)
PFN_vkCmdPushDescriptorSetKHR        vkCmdPushDescriptorSet     = nullptr;
#endif

#if defined(VK_EXT_hdr_metadata)
PFN_vkSetHdrMetadataEXT              vkSetHdrMetadata           = nullptr;
#endif


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

    const char* instanceExtension[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
    #if A3D_IS_WIN
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    #elif A3D_IS_LINUX
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,
    #elif A3D_IS_ANDROID
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
    #elif A3D_IS_NX
        VK_NN_VI_SURFACE_EXTENSION_NAME,
    #elif A3D_IS_IOS
        VK_MVK_IOS_SURFACE_EXTENSION_NAME,
    #elif A3D_IS_MAC
        VK_MVK_MACOS_SURFACE_EXTENSION_NAME
    #endif
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
        VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
        VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    };

    const char* layerNames[] = {
        "VK_LAYER_KHRONOS_validation",
    };

    uint32_t instanceExtensionCount = 7;
    uint32_t layerCount = 0;

    if (pDesc->EnableDebug)
    {
    #if !A3D_IS_NX
        layerCount++;
    #endif
    }

    if (!pDesc->EnableDebug)
    { instanceExtensionCount--; }

    #if 0
    //a3d::dynamic_array<char*> layers;
    //CheckInstanceLayer(
    //    layerCount,
    //    layerNames,
    //    layers);
    #endif

    // インスタンスの生成.
    {
        a3d::dynamic_array<char*> extensions;
        CheckInstanceExtension(
            nullptr,
            instanceExtensionCount,
            instanceExtension,
            extensions
        );

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
        instanceInfo.ppEnabledLayerNames        = (layerCount == 0) ? nullptr : layerNames;
        instanceInfo.enabledExtensionCount      = static_cast<uint32_t>(extensions.size());
        instanceInfo.ppEnabledExtensionNames    = extensions.data();

    #if 0
        //m_Allocator.pfnAllocation           = Alloc;
        //m_Allocator.pfnFree                 = Free;
        //m_Allocator.pfnReallocation         = Realloc;
        //m_Allocator.pfnInternalAllocation   = nullptr;
        //m_Allocator.pfnInternalFree         = nullptr;
        //m_Allocator.pUserData               = nullptr;
    #endif

        auto ret = vkCreateInstance(&instanceInfo, nullptr, &m_Instance);

        for(size_t i=0; i<extensions.size(); ++i)
        {
            delete [] extensions[i];
            extensions[i] = nullptr;
        }
        extensions.clear();

        if ( ret != VK_SUCCESS )
        { return false; }
    }

    #if defined(VK_KHR_swapchain)
    {
        vkGetDeviceGroupPresentCapabilities  = GET_INSTANCE_PROC(m_Instance, vkGetDeviceGroupPresentCapabilitiesKHR);
        vkGetDeviceGroupSurfacePresentModes  = GET_INSTANCE_PROC(m_Instance, vkGetDeviceGroupSurfacePresentModesKHR);
        vkGetPhysicalDevicePresentRectangles = GET_INSTANCE_PROC(m_Instance, vkGetPhysicalDevicePresentRectanglesKHR);
        
        vkCreateSwapchain       = GET_INSTANCE_PROC(m_Instance, vkCreateSwapchainKHR);
        vkDestroySwapchain      = GET_INSTANCE_PROC(m_Instance, vkDestroySwapchainKHR);
        vkGetSwapchainImages    = GET_INSTANCE_PROC(m_Instance, vkGetSwapchainImagesKHR);
        vkAcquireNextImage      = GET_INSTANCE_PROC(m_Instance, vkAcquireNextImageKHR);
        vkQueuePresent          = GET_INSTANCE_PROC(m_Instance, vkQueuePresentKHR);
        vkAcquireNextImage2     = GET_INSTANCE_PROC(m_Instance, vkAcquireNextImage2KHR);
    }
    #endif

    if (pDesc->EnableDebug)
    {
        vkCreateDebugReportCallback  = GET_INSTANCE_PROC(m_Instance, vkCreateDebugReportCallbackEXT);
        vkDestroyDebugReportCallback = GET_INSTANCE_PROC(m_Instance, vkDestroyDebugReportCallbackEXT);
        vkDebugReportMessage         = GET_INSTANCE_PROC(m_Instance, vkDebugReportMessageEXT);

        if (vkCreateDebugReportCallback  != nullptr &&
            vkDestroyDebugReportCallback != nullptr &&
            vkDebugReportMessage         != nullptr)
        {
            VkFlags flags = VK_DEBUG_REPORT_ERROR_BIT_EXT;
            flags |= VK_DEBUG_REPORT_WARNING_BIT_EXT;
            flags |= VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            //flags |= VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
            //flags |= VK_DEBUG_REPORT_DEBUG_BIT_EXT;

            VkDebugReportCallbackCreateInfoEXT info = {};
            info.sType          = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
            info.pNext          = nullptr;
            info.pfnCallback    = DebugReport;
            info.pUserData      = nullptr;
            info.flags          = flags;

            // vkCreateDebugReportCallback()で何故か2回メモリ確保がされてリークするので，カウンターを無効化.
            a3d_enable_counter(false);

            auto ret = vkCreateDebugReportCallback( 
                m_Instance,
                &info,
                nullptr,
                &vkDebugReportCallback );

            // 無効にしたカウンターを元に戻す.
            a3d_enable_counter(true);

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

            if (pProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                if (graphicsIndex == UINT32_MAX)
                {
                    graphicsIndex = i;
                    graphicsQueueIndex = queueIndex;
                    queueIndex++;
                }
            }

            // コンピュート専用キューを見つける.
            if ( (pProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT) 
             && ((pProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != VK_QUEUE_GRAPHICS_BIT) )
            {
                if (computeIndex == UINT32_MAX)
                {
                    computeIndex = i;
                    computeQueueIndex = queueIndex;
                    queueIndex++;
                }
            }

            // 転送専用キューを見つける.
            if ( (pProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
             && ((pProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != VK_QUEUE_GRAPHICS_BIT) )
            {
                if (transferIndex == UINT32_MAX)
                {
                    transferIndex = i;
                    transferQueueindex = queueIndex;
                    queueIndex++;
                }
            }
        }

        // 1つも見つからなければ仕方ないので共用のものを探す.
        if (computeIndex == UINT32_MAX)
        {
            for(auto i=0u; i<propCount; ++i)
            {
                if ( pProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT) 
                {
                    if (computeIndex == UINT32_MAX)
                    {
                        computeIndex = i;
                        computeQueueIndex = queueIndex;
                        queueIndex++;
                    }
                }
            }
        }

        // 1つも見つからなければ仕方ないので共用のものを探す.
        if (transferIndex == UINT32_MAX)
        {
            for(auto i=0u; i<propCount; ++i)
            {
          
                if ( pProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
                {
                    if (transferIndex == UINT32_MAX)
                    {
                        transferIndex = i;
                        transferQueueindex = queueIndex;
                        queueIndex++;
                    }
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

        a3d::dynamic_array<char*> deviceExtensions;
        {
            CheckDeviceExtension(
                nullptr,
                physicalDevice,
                deviceExtensions);

            // 初期化のためフラグを下しておく.
            for(auto i=0; i<EXT_COUNT; ++i)
            { m_IsSupportExt[i] = false; }

            for(size_t i=0; i<deviceExtensions.size(); ++i)
            {
                if (strcmp(deviceExtensions[i], VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME) == 0)
                { m_IsSupportExt[EXT_KHR_PUSH_DESCRIPTOR] = true; }

                if (strcmp(deviceExtensions[i], VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME) == 0)
                { m_IsSupportExt[EXT_KHR_DESCRIPTOR_UPDATE_TEMPLATE] = true; }

                if (strcmp(deviceExtensions[i], VK_NVX_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME) == 0)
                { m_IsSupportExt[EXT_NVX_DEVICE_GENERATE_COMMAND] = true; }

                if (strcmp(deviceExtensions[i], VK_AMD_DRAW_INDIRECT_COUNT_EXTENSION_NAME) == 0)
                { m_IsSupportExt[EXT_AMD_DRAW_INDIRECT_COUNT] = true; }

                if (strcmp(deviceExtensions[i], VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0)
                { m_IsSupportExt[EXT_DEBUG_MARKER] = true; }

                if (strcmp(deviceExtensions[i], VK_EXT_HDR_METADATA_EXTENSION_NAME) == 0)
                { m_IsSupportExt[EXT_HDR_METADATA] = true; }

                if (strcmp(deviceExtensions[i], VK_NV_RAY_TRACING_EXTENSION_NAME) == 0)
                { m_IsSupportExt[EXT_NV_RAY_TRACING] = true; }

                if (strcmp(deviceExtensions[i], VK_NV_MESH_SHADER_EXTENSION_NAME) == 0)
                { m_IsSupportExt[EXT_NV_MESH_SHADER] = true; }
            }
        }

        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.pNext                    = nullptr;
        deviceInfo.queueCreateInfoCount     = propCount;
        deviceInfo.pQueueCreateInfos        = pQueueInfos;
        deviceInfo.enabledLayerCount        = layerCount;
        deviceInfo.ppEnabledLayerNames      = (layerCount == 0) ? nullptr : layerNames;
        deviceInfo.enabledExtensionCount    = uint32_t(deviceExtensions.size());
        deviceInfo.ppEnabledExtensionNames  = deviceExtensions.data();
        deviceInfo.pEnabledFeatures         = nullptr;

        auto ret = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &m_Device);

        for(size_t i=0; i<deviceExtensions.size(); ++i)
        {
            delete [] deviceExtensions[i];
            deviceExtensions[i] = nullptr;
        }
        deviceExtensions.clear();

        delete[] pProps;
        delete[] pQueueInfos;
        delete[] pPriorities;

        if (ret != VK_SUCCESS )
        { return false; }


        #if defined(VK_EXT_debug_marker)
        {
            if (m_IsSupportExt[EXT_DEBUG_MARKER])
            {
                vkDebugMarkerSetObjectTag   = GET_DEVICE_PROC(m_Device, vkDebugMarkerSetObjectTagEXT);
                vkDebugMarkerSetObjectName  = GET_DEVICE_PROC(m_Device, vkDebugMarkerSetObjectNameEXT);
                vkCmdDebugMarkerBegin       = GET_DEVICE_PROC(m_Device, vkCmdDebugMarkerBeginEXT);
                vkCmdDebugMarkerEnd         = GET_DEVICE_PROC(m_Device, vkCmdDebugMarkerEndEXT);
                vkCmdDebugMarkerInsert      = GET_DEVICE_PROC(m_Device, vkCmdDebugMarkerInsertEXT);
            }
        }
        #endif

        #if defined(VK_KHR_push_descriptor)
        {
            if (m_IsSupportExt[EXT_KHR_PUSH_DESCRIPTOR])
            {
                vkCmdPushDescriptorSet = GET_DEVICE_PROC(m_Device, vkCmdPushDescriptorSetKHR);
            }
        }
        #endif

        #if defined(VK_EXT_hdr_metadata)
        {
            if (m_IsSupportExt[EXT_HDR_METADATA])
            {
                vkSetHdrMetadata = GET_DEVICE_PROC(m_Device, vkSetHdrMetadataEXT);
            }
        }
        #endif

        if (!Queue::Create(
            this, 
            graphicsIndex,
            graphicsQueueIndex,
            pDesc->MaxGraphicsQueueSubmitCount,
            reinterpret_cast<IQueue**>(&m_pGraphicsQueue)))
        { return false; }

        if (!Queue::Create(
            this,
            computeIndex,
            computeQueueIndex,
            pDesc->MaxComputeQueueSubmitCount,
            reinterpret_cast<IQueue**>(&m_pComputeQueue)))
        { return false; }

        if (!Queue::Create(
            this,
            transferIndex,
            transferQueueindex,
            pDesc->MaxCopyQueueSubmitCount,
            reinterpret_cast<IQueue**>(&m_pCopyQueue)))
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

        if (limits.timestampComputeAndGraphics)
        {
            auto nanoToSec = 1000 * 1000 * 1000;
            m_TimeStampFrequency = static_cast<uint64_t>(limits.timestampPeriod * nanoToSec);
        }
        else
        { m_TimeStampFrequency = 1; }
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
            a3d_enable_counter(false);

            vkDestroyDebugReportCallback(
                m_Instance,
                vkDebugReportCallback,
                nullptr);
            vkDebugReportCallback = null_handle;

            // 無効にしたカウンターを戻す.
            a3d_enable_counter(true);
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
        vkDestroyInstance(m_Instance, nullptr);
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
//      GPUタイムスタンプの更新頻度を取得します.
//-------------------------------------------------------------------------------------------------
uint64_t Device::GetTimeStampFrequency() const 
{ return m_TimeStampFrequency; }

//-------------------------------------------------------------------------------------------------
//      コマンドリストを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateCommandList(COMMANDLIST_TYPE type, ICommandList** ppCommandList)
{ return CommandList::Create(this, type, ppCommandList); }

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
//      アンオーダードアクセスビューを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateUnorderedAccessView
(
    IResource*                      pResource,
    const UnorderedAccessViewDesc*  pDesc,
    IUnorderedAccessView**          ppStorageView
)
{ return UnorderedAccessView::Create(this, pResource, pDesc, ppStorageView); }

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
//      アイドル状態になるまで待機します.
//-------------------------------------------------------------------------------------------------
void Device::WaitIdle()
{ vkDeviceWaitIdle(m_Device); }

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
    poolSize[0].descriptorCount = Max(1u, m_Desc.MaxShaderResourceCount);

    poolSize[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSize[1].descriptorCount = Max(1u, m_Desc.MaxShaderResourceCount);

    poolSize[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    poolSize[2].descriptorCount = Max(1u, m_Desc.MaxShaderResourceCount);

    poolSize[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize[3].descriptorCount = Max(1u, m_Desc.MaxShaderResourceCount);

    poolSize[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    poolSize[4].descriptorCount = Max(1u, m_Desc.MaxShaderResourceCount);

    poolSize[5].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    poolSize[5].descriptorCount = Max(1u, m_Desc.MaxShaderResourceCount);

    poolSize[6].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSize[6].descriptorCount = Max(1u, m_Desc.MaxShaderResourceCount);

    poolSize[7].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSize[7].descriptorCount = Max(1u, m_Desc.MaxShaderResourceCount);

    poolSize[8].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSize[8].descriptorCount = Max(1u, m_Desc.MaxSamplerCount);

    poolSize[9].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize[9].descriptorCount = Max(1u, m_Desc.MaxSamplerCount);

    VkDescriptorPoolCreateInfo info = {};
    info.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.pNext          = nullptr;
    info.flags          = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    info.maxSets        = Max(1u, maxSet);
    info.poolSizeCount  = 10;
    info.pPoolSizes     = poolSize;

    auto ret = vkCreateDescriptorPool(m_Device, &info, nullptr, pPool);
    if (ret != VK_SUCCESS)
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      拡張機能がサポートされているかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool Device::IsSupportExtension(EXTENSION value) const
{ return m_IsSupportExt[value]; }

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

//-------------------------------------------------------------------------------------------------
//      グラフィックスシステムを初期化します.
//-------------------------------------------------------------------------------------------------
bool A3D_APIENTRY InitSystem(const SystemDesc* pDesc)
{ return InitSystemAllocator(pDesc->pSystemAllocator); }

//-------------------------------------------------------------------------------------------------
//      グラフィクスシステムが初期化済みかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool A3D_APIENTRY IsInitSystem()
{ return IsInitSystemAllocator(); }

//-------------------------------------------------------------------------------------------------
//      グラフィックスシステムの終了処理を行います.
//-------------------------------------------------------------------------------------------------
void A3D_APIENTRY TermSystem()
{ TermSystemAllocator(); }

} // namespace a3d
