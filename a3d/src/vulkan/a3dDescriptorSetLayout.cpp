//-------------------------------------------------------------------------------------------------
// File : a3dDescriptorSetLayout.cpp
// Desc : Descriptor Set Layout Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      シェーダマスクをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
VkShaderStageFlags ToNativeShaderFlags(uint32_t mask)
{
    VkShaderStageFlags result = 0;

    if ( mask == a3d::SHADER_STAGE_VS )
    { result |= VK_SHADER_STAGE_VERTEX_BIT; }
    else if ( mask == a3d::SHADER_STAGE_DS )
    { result |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; }
    else if ( mask == a3d::SHADER_STAGE_HS )
    { result |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; }
    //else if ( mask == a3d::SHADER_STAGE_GS )
    //{ result |= VK_SHADER_STAGE_GEOMETRY_BIT; }
    else if ( mask == a3d::SHADER_STAGE_PS )
    { result |= VK_SHADER_STAGE_FRAGMENT_BIT; }
    else if ( mask == a3d::SHADER_STAGE_CS )
    { result |= VK_SHADER_STAGE_COMPUTE_BIT; }
    else if ( mask == a3d::SHADER_STAGE_RAYGEN )
    { result |= VK_SHADER_STAGE_RAYGEN_BIT_KHR; }
    else if ( mask == a3d::SHADER_STAGE_MISS )
    { result |= VK_SHADER_STAGE_MISS_BIT_KHR; }
    else if ( mask == a3d::SHADER_STAGE_INTERSECTION )
    { result |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR; }
    else if ( mask == a3d::SHADER_STAGE_CLOSEST_HIT )
    { result |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR; }
    else if ( mask == a3d::SHADER_STAGE_ANY_HIT )
    { result |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR; }
    else if ( mask == a3d::SHADER_STAGE_CALLABLE )
    { result |= VK_SHADER_STAGE_CALLABLE_BIT_KHR; }

    return result;
}

} // namespace /* anonymous */

namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorSetLayout class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorSetLayout::DescriptorSetLayout()
: m_RefCount            (1)
, m_pDevice             (nullptr)
, m_DescriptorSetLayout (null_handle)
, m_PipelineLayout      (null_handle)
, m_DescriptorPool      (null_handle)
, m_ImageCount          (0)
, m_BufferCount         (0)
, m_SamplerCount        (0)
, m_DescriptorSet       (null_handle)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorSetLayout::~DescriptorSetLayout()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool DescriptorSetLayout::Init(IDevice* pDevice, const DescriptorSetLayoutDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pNativeDevice = m_pDevice->GetVkDevice();
    A3D_ASSERT( pNativeDevice != null_handle );

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    bool isGraphics = true;
    for(auto i=0u; i<pDesc->EntryCount; ++i)
    {
        if (pDesc->Entries[i].ShaderStage == SHADER_STAGE_CS)
        { isGraphics = false; }

        if (pDesc->Entries[i].ShaderStage == SHADER_STAGE_VS)
        {
            if (!isGraphics)
            {
                A3D_LOG("Error : Invalid Argument.");
                return false;
            }
        }

        if ((pDesc->Entries[i].ShaderStage == SHADER_STAGE_AS)
         || (pDesc->Entries[i].ShaderStage == SHADER_STAGE_MS))
        {
            if (!isGraphics)
            {
                A3D_LOG("Error : Invalid Argument.");
                return false;
            }
        }
    }

    m_BindPoint = (isGraphics)
                   ? VK_PIPELINE_BIND_POINT_GRAPHICS 
                   : VK_PIPELINE_BIND_POINT_COMPUTE;

    {
        auto bufferCount  = 0;
        auto samplerCount = 0;
        auto imageCount   = 0;

        auto bindings = new VkDescriptorSetLayoutBinding [pDesc->EntryCount];
        for(auto i=0u; i<pDesc->EntryCount; ++i)
        {
            bindings[i].binding             = pDesc->Entries[i].BindLocation;
            bindings[i].descriptorType      = ToNativeDescriptorType(pDesc->Entries[i].Type);
            bindings[i].stageFlags          = ToNativeShaderFlags(pDesc->Entries[i].ShaderStage);
            bindings[i].descriptorCount     = 1;
            bindings[i].pImmutableSamplers  = nullptr;

            if (bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER )
            { bufferCount++; }
            else if (bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
                bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            { imageCount++; }
            else if (bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER)
            { samplerCount++; }
        }

        m_BufferCount  = bufferCount;
        m_ImageCount   = imageCount;
        m_SamplerCount = samplerCount;

        VkDescriptorSetLayoutCreateFlags flags = 0;
        #if defined(VK_KHR_push_descriptor)
        if (m_pDevice->IsSupportExtension(Device::EXT_KHR_PUSH_DESCRIPTOR))
        { flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR; }
        #endif

        VkDescriptorSetLayoutCreateInfo info = {};
        info.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.pNext          = nullptr;
        info.flags          = flags;
        info.bindingCount   = pDesc->EntryCount;
        info.pBindings      = bindings;

        auto ret = vkCreateDescriptorSetLayout( pNativeDevice, &info, nullptr, &m_DescriptorSetLayout );
        if ( ret != VK_SUCCESS )
        {
            A3D_LOG("Error : vkCreateDescriptorSetLayout() Failed. VkResult = %s", ToString(ret));
            return false;
        }
    }

    {
        VkPipelineLayoutCreateInfo info = {};
        info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.pNext                  = nullptr;
        info.flags                  = 0;
        info.setLayoutCount         = 1;
        info.pSetLayouts            = &m_DescriptorSetLayout;
        info.pushConstantRangeCount = 0;
        info.pPushConstantRanges    = nullptr;

        auto ret = vkCreatePipelineLayout( pNativeDevice, &info, nullptr, &m_PipelineLayout );
        if ( ret != VK_SUCCESS )
        {
            A3D_LOG("Error : vkCreatePipelineLayout() Failed. VkResult = %s", ToString(ret));
            return false;
        }
    }

    if (!m_pDevice->CreateVkDescriptorPool(1, &m_DescriptorPool))
    {
        A3D_LOG("Error : Device::CreateVulknDescriptorPool() Failed.");
        return false;
    }

    // ディスクリプタセットを生成します.
    if (!m_pDevice->IsSupportExtension(Device::EXT_KHR_PUSH_DESCRIPTOR))
    {
        VkDescriptorSetAllocateInfo info = {};
        info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        info.pNext              = nullptr;
        info.descriptorPool     = m_DescriptorPool;
        info.descriptorSetCount = 1;
        info.pSetLayouts        = &m_DescriptorSetLayout;

        auto ret = vkAllocateDescriptorSets( pNativeDevice, &info, &m_DescriptorSet );
        if (ret != VK_SUCCESS)
        {
            A3D_LOG("Error : vkAllocateDescriptorSets() Failed. VkResult = %s", ToString(ret));
            return false;
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSetLayout::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    auto pNativeDevice = m_pDevice->GetVkDevice();
    A3D_ASSERT( pNativeDevice != null_handle );

    if ( m_DescriptorSet != null_handle )
    {
        vkFreeDescriptorSets(pNativeDevice, m_DescriptorPool, 1, &m_DescriptorSet);
        m_DescriptorSet = null_handle;
    }

    if ( m_DescriptorSetLayout != null_handle )
    {
        vkDestroyDescriptorSetLayout(pNativeDevice, m_DescriptorSetLayout, nullptr);
        m_DescriptorSetLayout = null_handle;
    }

    if ( m_PipelineLayout != null_handle )
    {
        vkDestroyPipelineLayout(pNativeDevice, m_PipelineLayout, nullptr);
        m_PipelineLayout = null_handle;
    }

    if ( m_DescriptorPool != null_handle )
    {
        vkDestroyDescriptorPool(pNativeDevice, m_DescriptorPool, nullptr);
        m_DescriptorPool = null_handle;
    }

    m_BufferCount  = 0;
    m_ImageCount   = 0;
    m_SamplerCount = 0;
    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void DescriptorSetLayout::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSetLayout::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorSetLayout::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSetLayout::SetName(const char* name)
{
    m_Name = name;
    if (vkDebugMarkerSetObjectName != nullptr)
    {
        auto pWrapDevice = static_cast<Device*>(m_pDevice);
        A3D_ASSERT(pWrapDevice != nullptr);

        VkDebugMarkerObjectNameInfoEXT info = {};
        info.sType          = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
        info.objectType     = VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT;
        info.object         = uint64_t(m_DescriptorSetLayout);
        info.pObjectName    = name;

        vkDebugMarkerSetObjectName(pWrapDevice->GetVkDevice(), &info);
    }
}

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* DescriptorSetLayout::GetName() const
{ return m_Name.c_str(); }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void DescriptorSetLayout::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
DescriptorSetLayoutDesc DescriptorSetLayout::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      パイプラインレイアウトを取得します.
//-------------------------------------------------------------------------------------------------
VkPipelineLayout DescriptorSetLayout::GetVkPipelineLayout() const
{ return m_PipelineLayout; }

//-------------------------------------------------------------------------------------------------
//      パイプラインバインドポイントを取得します.
//-------------------------------------------------------------------------------------------------
VkPipelineBindPoint DescriptorSetLayout::GetVkPipelineBindPoint() const
{ return m_BindPoint; }

//-------------------------------------------------------------------------------------------------
//      ディスクリプタプールを取得します.
//-------------------------------------------------------------------------------------------------
VkDescriptorPool DescriptorSetLayout::GetVkDescriptorPool() const
{ return m_DescriptorPool; }

//-------------------------------------------------------------------------------------------------
//      ディスクリプタセットレイアウトを取得します.
//-------------------------------------------------------------------------------------------------
VkDescriptorSetLayout DescriptorSetLayout::GetVkDescriptorSetLayout() const
{ return m_DescriptorSetLayout; }

//-------------------------------------------------------------------------------------------------
//      バッファ数を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorSetLayout::GetBufferCount() const
{ return m_BufferCount; }

//-------------------------------------------------------------------------------------------------
//      イメージ数を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorSetLayout::GetImageCount() const
{ return m_ImageCount; }

//-------------------------------------------------------------------------------------------------
//      サンプラー数を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorSetLayout::GetSamplerCount() const
{ return m_SamplerCount; }

//-------------------------------------------------------------------------------------------------
//      ディスクリプタ―セットを取得します.
//-------------------------------------------------------------------------------------------------
VkDescriptorSet DescriptorSetLayout::GetVkDescriptorSet() const
{ return m_DescriptorSet; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool DescriptorSetLayout::Create
(
    IDevice*                        pDevice,
    const DescriptorSetLayoutDesc*  pDesc,
    IDescriptorSetLayout**          ppLayout
)
{
    if (pDevice == nullptr || pDesc == nullptr || ppLayout == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new DescriptorSetLayout;
    if ( instance == nullptr )
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if ( !instance->Init(pDevice, pDesc) )
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppLayout = instance;
    return true;
}

} // namespace a3d
