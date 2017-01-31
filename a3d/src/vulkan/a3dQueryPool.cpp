//-------------------------------------------------------------------------------------------------
// File : a3dQueryPool.cpp
// Desc : QueryPool Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// QueryPool class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
QueryPool::QueryPool()
: m_RefCount(1)
, m_pDevice (nullptr)
, m_Pool    (null_handle)
, m_Type    (VK_QUERY_TYPE_TIMESTAMP)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
QueryPool::~QueryPool()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool QueryPool::Init(IDevice* pDevice, const QueryPoolDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != VK_NULL_HANDLE);

    VkQueryType table[] = {
        VK_QUERY_TYPE_OCCLUSION,
        VK_QUERY_TYPE_TIMESTAMP,
        VK_QUERY_TYPE_PIPELINE_STATISTICS
    };

    m_Type = table[pDesc->Type];

    VkQueryPoolCreateInfo info = {};
    info.sType      = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    info.pNext      = nullptr;
    info.flags      = 0;
    info.queryType  = m_Type;
    info.queryCount = pDesc->Count;

    if (m_Type == VK_QUERY_TYPE_PIPELINE_STATISTICS)
    {
        info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT;
        info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT;
        info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT;
        info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT;
        info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT;
        info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT;
        info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT;
        info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;
        info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT;
        info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT;
        info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
    }

    auto ret = vkCreateQueryPool( pNativeDevice, &info, nullptr, &m_Pool );
    if ( ret != VK_SUCCESS )
    { return false; }

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void QueryPool::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != VK_NULL_HANDLE);

    vkDestroyQueryPool(pNativeDevice, m_Pool, nullptr);
    m_Pool = VK_NULL_HANDLE;

    SafeRelease(m_pDevice);
    memset( &m_Desc, 0, sizeof(m_Desc) );
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void QueryPool::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void QueryPool::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t QueryPool::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void QueryPool::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
QueryPoolDesc QueryPool::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      クエリプールを取得します.
//-------------------------------------------------------------------------------------------------
VkQueryPool QueryPool::GetVulkanQueryPool() const
{ return m_Pool; }

//-------------------------------------------------------------------------------------------------
//      クエリタイプを取得します.
//-------------------------------------------------------------------------------------------------
VkQueryType QueryPool::GetVulkanQueryType() const
{ return m_Type; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool QueryPool::Create(IDevice* pDevice, const QueryPoolDesc* pDesc, IQueryPool** ppQueryPool)
{
    if (pDevice == nullptr || pDesc == nullptr || ppQueryPool == nullptr)
    { return false; }

    auto instance = new QueryPool;
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, pDesc))
    {
        SafeRelease(instance);
        return false;
    }

    *ppQueryPool = instance;
    return true;
}

} // namespace a3d
