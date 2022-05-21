//-------------------------------------------------------------------------------------------------
// File : a3dAccelerationStructure.cpp
// Desc : Acceleration Structure.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace {

//-------------------------------------------------------------------------------------------------
//      VkIndexTypeに変換します.
//-------------------------------------------------------------------------------------------------
VkIndexType ToNativeIndexType(a3d::RESOURCE_FORMAT format)
{
    if (format == a3d::RESOURCE_FORMAT_R16_UINT)
    { return VK_INDEX_TYPE_UINT16; }
    else if (format == a3d::RESOURCE_FORMAT_R32_UINT)
    { return VK_INDEX_TYPE_UINT32; }

    return VK_INDEX_TYPE_NONE_KHR;
}

//-------------------------------------------------------------------------------------------------
//      VkBuildAcclerationStructureFlagsKHRに変換します.
//-------------------------------------------------------------------------------------------------
VkBuildAccelerationStructureFlagsKHR ToNativeFlags(a3d::ACCELERATION_STRUCTURE_BUILD_FLAGS flags)
{
    VkBuildAccelerationStructureFlagsKHR result = 0;

    if (flags & a3d::ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE)
    { result |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR; }
    if (flags & a3d::ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION)
    { result |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR; }
    if (flags & a3d::ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE)
    { result |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR; }
    if (flags & a3d::ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD)
    { result |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR; }
    if (flags & a3d::ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY)
    { result |= VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR; }

    return result;
}

//-------------------------------------------------------------------------------------------------
//      VkBuildAccelerationStructureModeKHRに変換します.
//-------------------------------------------------------------------------------------------------
VkBuildAccelerationStructureModeKHR ToNativeMode(a3d::ACCELERATION_STRUCTURE_BUILD_FLAGS flags)
{
    if (flags & a3d::ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE)
    { return VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR; }

    return VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
}

} // namespace


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// AccelerationStructure
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
AccelerationStructure::AccelerationStructure()
: m_RefCount        (1)
, m_pDevice         (nullptr)
, m_AS              (null_handle)
, m_AllowUpdate     (false)
, m_GeometryCount   (0)
, m_pGeometries     (nullptr)
, m_pBuildRangeInfos(nullptr)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
AccelerationStructure::~AccelerationStructure()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool AccelerationStructure::Init(IDevice* pDevice, const AccelerationStructureDesc* pDesc)
{
    auto pWrapDevice = static_cast<Device*>(pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);
    m_pDevice = pWrapDevice;
    m_pDevice->AddRef();

    auto pNativeDevice = m_pDevice->GetVkDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    if (pDesc->Type == ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
    {
        if (!InitAsBlas(pDesc))
        {
            A3D_LOG("Error : InitAsBlas() Failed.");
            return false;
        }
    }
    else if (pDesc->Type == ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL)
    {
        if (!InitAsTlas(pDesc))
        {
            A3D_LOG("Error : InitAsTlas() Failed.");
            return false;
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void AccelerationStructure::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    auto pNativeDevice = m_pDevice->GetVkDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    if (m_Structure.Buffer != null_handle)
    {
        vmaDestroyBuffer(m_pDevice->GetAllocator(), m_Structure.Buffer, m_Structure.Allocation);
        m_Structure.Buffer      = null_handle;
        m_Structure.Allocation  = null_handle;
    }

    if (m_Scratch.Buffer != null_handle)
    {
        vmaDestroyBuffer(m_pDevice->GetAllocator(), m_Scratch.Buffer, m_Scratch.Allocation);
        m_Scratch.Buffer        = null_handle;
        m_Scratch.Allocation    = null_handle;
    }

    if (m_AS != null_handle)
    {
        vkDestroyAccelerationStructure(pNativeDevice, m_AS, nullptr);
        m_AS = null_handle;
    }

    SafeDeleteArray(m_pGeometries);
    SafeDeleteArray(m_pBuildRangeInfos);

    SafeRelease(m_pDevice);

    memset(&m_BuildGeometryInfo, 0, sizeof(m_BuildGeometryInfo));
    m_GeometryCount = 0;
    m_AllowUpdate   = false;
}

//-------------------------------------------------------------------------------------------------
//      下位レベル高速化機構として初期化します.
//-------------------------------------------------------------------------------------------------
bool AccelerationStructure::InitAsBlas(const AccelerationStructureDesc* pDesc)
{
    m_GeometryCount = pDesc->Count;

    auto primitiveCounts = new uint32_t[m_GeometryCount];
    m_pGeometries        = new VkAccelerationStructureGeometryKHR [m_GeometryCount];
    m_pBuildRangeInfos   = new VkAccelerationStructureBuildRangeInfoKHR [m_GeometryCount];
    A3D_ASSERT(primitiveCounts    != nullptr);
    A3D_ASSERT(m_pGeometries      != nullptr);
    A3D_ASSERT(m_pBuildRangeInfos != nullptr);

    for(auto i=0u; i<m_GeometryCount; ++i)
    {
        auto& desc = pDesc->pDescs[i];

        VkAccelerationStructureGeometryKHR geometry = {};
        geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        geometry.pNext = nullptr;
        geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

        if (desc.Type == a3d::GEOMETRY_TYPE_TRIANGLES)
        {
            VkDeviceOrHostAddressConstKHR addressVB = {};
            VkDeviceOrHostAddressConstKHR addressIB = {};
            VkDeviceOrHostAddressConstKHR addressTB = {};

            addressVB.deviceAddress = desc.Triangles.VertexAddress;
            addressIB.deviceAddress = desc.Triangles.IndexAddress;
            addressTB.deviceAddress = desc.Triangles.TransformAddress;

            geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
            geometry.geometry.triangles.sType           = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
            geometry.geometry.triangles.pNext           = nullptr;
            geometry.geometry.triangles.vertexFormat    = ToNativeFormat(desc.Triangles.VertexFormat);
            geometry.geometry.triangles.vertexData      = addressVB;
            geometry.geometry.triangles.maxVertex       = desc.Triangles.VertexCount;
            geometry.geometry.triangles.indexType       = (desc.Triangles.IndexAddress != 0) ? ToNativeIndexType(desc.Triangles.IndexFormat) : VK_INDEX_TYPE_NONE_KHR;
            geometry.geometry.triangles.indexData       = addressIB;
            geometry.geometry.triangles.transformData   = addressTB;

            primitiveCounts[i] = desc.Triangles.IndexCount / 3;

        }
        else if (desc.Type == a3d::GEOMETRY_TYPE_AABBS)
        {
            VkDeviceOrHostAddressConstKHR address = {};
            address.deviceAddress = desc.AABBs.StartAddress;

            geometry.geometryType           = VK_GEOMETRY_TYPE_AABBS_KHR;
            geometry.geometry.aabbs.sType   = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
            geometry.geometry.aabbs.pNext   = nullptr;
            geometry.geometry.aabbs.data    = address;
            geometry.geometry.aabbs.stride  = desc.AABBs.Stride;

            primitiveCounts[i] = desc.AABBs.BoxCount;
        }

        m_pGeometries[i] = geometry;
    }

    // ビルドジオメトリ情報.
    memset(&m_BuildGeometryInfo, 0, sizeof(m_BuildGeometryInfo));
    m_BuildGeometryInfo.sType         = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    m_BuildGeometryInfo.pNext         = nullptr;
    m_BuildGeometryInfo.flags         = ToNativeFlags(pDesc->Flags);
    m_BuildGeometryInfo.mode          = ToNativeMode(pDesc->Flags);
    m_BuildGeometryInfo.geometryCount = m_GeometryCount;
    m_BuildGeometryInfo.pGeometries   = m_pGeometries;

    // ビルドサイズを取得.
    VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo = {};
    buildSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    buildSizeInfo.pNext = nullptr;
    vkGetAccelerationStructureBuildSizes(
        m_pDevice->GetVkDevice(),
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &m_BuildGeometryInfo,
        primitiveCounts,
        &buildSizeInfo);

    for(auto i=0u; i<m_GeometryCount; ++i)
    {
        m_pBuildRangeInfos[i].primitiveCount  = primitiveCounts[i];
        m_pBuildRangeInfos[i].primitiveOffset = 0;
        m_pBuildRangeInfos[i].firstVertex     = 0;
        m_pBuildRangeInfos[i].transformOffset = 0;
    }

    SafeDeleteArray(primitiveCounts);

    // 高速化機構用バッファを作成.
    {
        VkBufferCreateInfo createInfo = {};
        createInfo.sType    = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext    = nullptr;
        createInfo.size     = buildSizeInfo.accelerationStructureSize;
        createInfo.usage    = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        auto ret = vmaCreateBuffer(
            m_pDevice->GetAllocator(),
            &createInfo,
            &allocInfo,
            &m_Structure.Buffer,
            &m_Structure.Allocation,
            nullptr);
        if (ret != VK_SUCCESS)
        {
            A3D_LOG("Error : vmaCreateBuffer() Failed. VkResult = %s", ToString(ret));
            return false;
        }
    }

    // 高速化機構生成.
    {
        VkAccelerationStructureCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.buffer = m_Structure.Buffer;
        createInfo.size = buildSizeInfo.accelerationStructureSize;
        createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        auto ret = vkCreateAccelerationStructure(
            m_pDevice->GetVkDevice(),
            &createInfo,
            nullptr, 
            &m_AS);
        if (ret != VK_SUCCESS)
        {
            A3D_LOG("Error : vkCreateAccelerationStructureKHR() Failed. VkResult = %s", ToString(ret));
            return false;
        }
    }

    // スクラッチバッファ生成.
    if (!CreateScratchBuffer(buildSizeInfo.buildScratchSize))
    {
        A3D_LOG("Error : CreateScratchBuffer() Failed.");
        return false;
    }

    // スクラッチバッファのアドレスを設定.
    {
        VkBufferDeviceAddressInfo info = {};
        info.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        info.pNext  = nullptr;
        info.buffer = m_Scratch.Buffer;

        auto address = vkGetBufferDeviceAddress(m_pDevice->GetVkDevice(), &info);

        m_BuildGeometryInfo.dstAccelerationStructure  = m_AS;
        m_BuildGeometryInfo.scratchData.deviceAddress = address;

        if (pDesc->Flags & a3d::ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE)
        { m_AllowUpdate = true; }
    }

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      上位レベル高速化機構として初期化します.
//-------------------------------------------------------------------------------------------------
bool AccelerationStructure::InitAsTlas(const AccelerationStructureDesc* pDesc)
{
    VkDeviceOrHostAddressConstKHR addressIB = {};
    addressIB.deviceAddress = pDesc->InstanceDescs;

    m_GeometryCount     = 1;
    m_pBuildRangeInfos  = new VkAccelerationStructureBuildRangeInfoKHR [m_GeometryCount];
    m_pGeometries       = new VkAccelerationStructureGeometryKHR [m_GeometryCount];

    memset(&m_pGeometries[0], 0, sizeof(m_pGeometries[0]));
    m_pGeometries->sType                              = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    m_pGeometries->pNext                              = nullptr;
    m_pGeometries->geometryType                       = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    m_pGeometries->geometry.instances.sType           = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    m_pGeometries->geometry.instances.arrayOfPointers = VK_FALSE;
    m_pGeometries->geometry.instances.data            = addressIB;

    // ビルドジオメトリ情報.
    memset(&m_BuildGeometryInfo, 0, sizeof(m_BuildGeometryInfo));
    m_BuildGeometryInfo.sType         = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    m_BuildGeometryInfo.pNext         = nullptr;
    m_BuildGeometryInfo.flags         = ToNativeFlags(pDesc->Flags);
    m_BuildGeometryInfo.mode          = ToNativeMode(pDesc->Flags);
    m_BuildGeometryInfo.geometryCount = 1;
    m_BuildGeometryInfo.pGeometries   = m_pGeometries;

    // ビルド範囲情報.
   {
        m_pBuildRangeInfos[0].primitiveCount    = 1;
        m_pBuildRangeInfos[0].primitiveOffset   = 0;
        m_pBuildRangeInfos[0].firstVertex       = 0;
        m_pBuildRangeInfos[0].transformOffset   = 0;
    }

    // ビルドサイズを取得.
    VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo = {};
    buildSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    buildSizeInfo.pNext = nullptr;
    vkGetAccelerationStructureBuildSizes(
        m_pDevice->GetVkDevice(),
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &m_BuildGeometryInfo,
        &m_GeometryCount,
        &buildSizeInfo);

    // 高速化機構用バッファを生成.
    {
        VkBufferCreateInfo createInfo = {};
        createInfo.sType    = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext    = nullptr;
        createInfo.size     = buildSizeInfo.accelerationStructureSize;
        createInfo.usage    = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        auto ret = vmaCreateBuffer(
            m_pDevice->GetAllocator(),
            &createInfo,
            &allocInfo,
            &m_Structure.Buffer,
            &m_Structure.Allocation,
            nullptr);

        if (ret != VK_SUCCESS)
        {
            A3D_LOG("Error : vmaCreateBuffer() Failed. VkResult = %s", ToString(ret));
            return false;
        }
    }

    // 高速化機構生成.
    {
        VkAccelerationStructureCreateInfoKHR createInfo = {};
        createInfo.sType    = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        createInfo.pNext    = nullptr;
        createInfo.size     = buildSizeInfo.accelerationStructureSize;
        createInfo.type     = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

        auto ret = vkCreateAccelerationStructure(
            m_pDevice->GetVkDevice(),
            &createInfo,
            nullptr,
            &m_AS);
        if (ret != VK_SUCCESS)
        {
            A3D_LOG("Error : vkCreateAccelerationStructureKHR() Failed. VkResult = %s", ToString(ret));
            return false;
        }
    }

    // スクラッチバッファ生成.
    if (!CreateScratchBuffer(buildSizeInfo.buildScratchSize))
    {
        A3D_LOG("Error : CreateScratchBuffer() Failed.");
        return false;
    }

    // スクラッチバッファのアドレスを設定.
    {
        VkBufferDeviceAddressInfo info = {};
        info.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        info.pNext  = nullptr;
        info.buffer = m_Scratch.Buffer;

        auto address = vkGetBufferDeviceAddress(m_pDevice->GetVkDevice(), &info);

        m_BuildGeometryInfo.dstAccelerationStructure  = m_AS;
        m_BuildGeometryInfo.scratchData.deviceAddress = address;

        if (pDesc->Flags & a3d::ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE)
        { m_AllowUpdate = true; }
    }

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void AccelerationStructure::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void AccelerationStructure::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t AccelerationStructure::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void AccelerationStructure::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      デバイスアドレスを取得します.
//-------------------------------------------------------------------------------------------------
uint64_t AccelerationStructure::GetDeviceAddress() const
{
    if (m_Structure.Buffer == null_handle)
    { return 0; }

    VkBufferDeviceAddressInfoKHR addressInfo = {};
    addressInfo.sType   = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
    addressInfo.buffer  = m_Structure.Buffer;
    return vkGetBufferDeviceAddress(m_pDevice->GetVkDevice(), &addressInfo);
}

//-------------------------------------------------------------------------------------------------
//      高速化機構を取得します.
//-------------------------------------------------------------------------------------------------
VkAccelerationStructureKHR AccelerationStructure::GetVkAccelerationStructure() const
{ return m_AS; }

//-------------------------------------------------------------------------------------------------
//      描画コマンドを発行します.
//-------------------------------------------------------------------------------------------------
void AccelerationStructure::Issue(ICommandList* pCommandList)
{
    auto pWrapCmdList = static_cast<CommandList*>(pCommandList);
    A3D_ASSERT(pWrapCmdList != nullptr);

    auto commandBuffer = pWrapCmdList->GetVkCommandBuffer();
    A3D_ASSERT(commandBuffer != null_handle);

    vkCmdBuildAccelerationStructures(
        commandBuffer,
        m_GeometryCount,
        &m_BuildGeometryInfo,
        &m_pBuildRangeInfos);

    //if (m_AllowUpdate)
    //{ m_BuildGeometryInfo.srcAccelerationStructure = m_AS; }
}

//-------------------------------------------------------------------------------------------------
//      スクラッチバッファを生成します.
//-------------------------------------------------------------------------------------------------
bool AccelerationStructure::CreateScratchBuffer(size_t size)
{
    VkBufferCreateInfo createInfo = {};
    createInfo.sType    = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext    = nullptr;
    createInfo.size     = size;
    createInfo.usage    = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    auto ret = vmaCreateBuffer(
        m_pDevice->GetAllocator(),
        &createInfo,
        &allocInfo,
        &m_Scratch.Buffer,
        &m_Scratch.Allocation,
        nullptr);

    if (ret != VK_SUCCESS)
    {
        A3D_LOG("Error : vmaCreateBuffer() Failed. errcode = 0x%x", ret);
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool AccelerationStructure::Create
(
    IDevice*                            pDevice,
    const AccelerationStructureDesc*    pDesc,
    IAccelerationStructure**            ppAS
)
{
    if (pDevice == nullptr || pDesc == nullptr || ppAS == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new AccelerationStructure();
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out of Memory.");
        return false;
    }

    if (!instance->Init(pDevice, pDesc))
    {
        A3D_LOG("Error : Init() Failed.");
        delete instance;
        return false;
    }

    *ppAS = instance;
    return true;
}

} // namespace a3d
