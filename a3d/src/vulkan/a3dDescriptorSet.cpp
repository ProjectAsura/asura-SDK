//-------------------------------------------------------------------------------------------------
// File : a3dDescriptorSet.cpp
// Desc : DescriptorSet Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "a3dVulkanFunc.h"


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorSet class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorSet::DescriptorSet()
: m_RefCount        (1)
, m_pDevice         (nullptr)
, m_pLayout         (nullptr)
, m_DescriptorSet   (null_handle)
, m_pWrites         (nullptr)
, m_pInfos          (nullptr)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorSet::~DescriptorSet()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool DescriptorSet::Init
(
    IDevice*                pDevice,
    DescriptorSetLayout*    pLayout
)
{
    if (pDevice == nullptr || pLayout == nullptr)
    {
        A3D_LOG("Error : Invalid Argument");
        return false;
    }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    m_pLayout = pLayout;
    m_pLayout->AddRef();

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    // ディスクリプタセットを生成します.
    if (!m_pDevice->IsSupportExtension(Device::EXT_KHR_PUSH_DESCRIPTOR))
    {
        auto pNativeDescriptorPool   = pLayout->GetVulkanDescriptorPool();
        auto pNativeDescriptorLayout = pLayout->GetVulkanDescriptorSetLayout();

        VkDescriptorSetAllocateInfo info = {};
        info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        info.pNext              = nullptr;
        info.descriptorPool     = pNativeDescriptorPool;
        info.descriptorSetCount = 1;
        info.pSetLayouts        = &pNativeDescriptorLayout;

        auto ret = vkAllocateDescriptorSets( pNativeDevice, &info, &m_DescriptorSet );
        if (ret != VK_SUCCESS)
        {
            A3D_LOG("Error : vkAllocateDescriptorSets() Failed. VkResult = %s", ToString(ret));
            return false;
        }
    }

    if (pLayout->GetDesc().EntryCount > 0)
    {
        const auto& desc = pLayout->GetDesc();
        auto count       = desc.EntryCount;

        m_pWrites = new VkWriteDescriptorSet [count];
        if (m_pWrites == nullptr)
        {
            A3D_LOG("Error : Out Of Memory.");
            return false;
        }

        memset( m_pWrites, 0, sizeof(VkWriteDescriptorSet) * count );

        m_pInfos = new DescriptorInfo [count];
        if (m_pInfos == nullptr)
        {
            A3D_LOG("Error : Out Of Memory.");
            return false;
        }

        memset( m_pInfos, 0, sizeof(DescriptorInfo) * count );

        auto bufferIndex = 0;
        auto imageIndex  = 0;
        for(auto i=0u; i<count; ++i)
        {
            m_pWrites[i].sType              = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            m_pWrites[i].pNext              = nullptr;
            m_pWrites[i].dstSet             = m_DescriptorSet;
            m_pWrites[i].dstBinding         = desc.Entries[i].BindLocation;
            m_pWrites[i].dstArrayElement    = 0;
            m_pWrites[i].descriptorCount    = 1;
            m_pWrites[i].descriptorType     = ToNativeDescriptorType(desc.Entries[i].Type);
            m_pWrites[i].pImageInfo         = nullptr;
            m_pWrites[i].pBufferInfo        = nullptr;
            m_pWrites[i].pTexelBufferView   = nullptr;

            if (desc.Entries[i].Type == DESCRIPTOR_TYPE_CBV ||
                desc.Entries[i].Type == DESCRIPTOR_TYPE_UAV)
            {
                m_pWrites[i].pBufferInfo = &m_pInfos[bufferIndex].Buffer;
                bufferIndex++;
            }
            else if (desc.Entries[i].Type == DESCRIPTOR_TYPE_SRV ||
                     desc.Entries[i].Type == DESCRIPTOR_TYPE_SMP)
            {
                m_pWrites[i].pImageInfo = &m_pInfos[imageIndex].Image;
                imageIndex++;
            }
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    auto pNativeDescriptorPool = m_pLayout->GetVulkanDescriptorPool();
    A3D_ASSERT(pNativeDescriptorPool != null_handle);

    if (m_pWrites != nullptr)
    {
        delete[] m_pWrites;
        m_pWrites = nullptr;
    }

    if (m_pInfos != nullptr)
    {
        delete[] m_pInfos;
        m_pInfos = nullptr;
    }

    if (m_DescriptorSet != null_handle)
    {
        vkFreeDescriptorSets(pNativeDevice, pNativeDescriptorPool, 1, &m_DescriptorSet);
        m_DescriptorSet = null_handle;
    }

    SafeRelease(m_pLayout);
    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを返却します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorSet::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      バッファを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetView(uint32_t index, IConstantBufferView* const pResource)
{
    A3D_ASSERT(index < m_pLayout->GetDesc().EntryCount );

    auto pWrapCBV = static_cast<ConstantBufferView*>(pResource);
    A3D_ASSERT(pWrapCBV != nullptr);

    const auto& desc = pWrapCBV->GetDesc();

    m_pInfos[index].Buffer.buffer = pWrapCBV->GetVulkanBuffer();
    m_pInfos[index].Buffer.offset = desc.Offset;
    m_pInfos[index].Buffer.range  = desc.Range;
    m_pInfos[index].StorageBuffer = false;
}

//-------------------------------------------------------------------------------------------------
//      テクスチャを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetView(uint32_t index, IShaderResourceView* const pResource)
{
    A3D_ASSERT(index < m_pLayout->GetDesc().EntryCount );

    auto pWrapSRV = static_cast<ShaderResourceView*>(pResource);
    A3D_ASSERT(pWrapSRV != nullptr);

    auto desc = pWrapSRV->GetDesc();

    auto kind = pWrapSRV->GetResource()->GetKind();

    if (kind == RESOURCE_KIND_BUFFER)
    {
        m_pInfos[index].Buffer.buffer   = pWrapSRV->GetVulkanBuffer();
        m_pInfos[index].Buffer.offset   = desc.FirstElement;
        m_pInfos[index].Buffer.range    = desc.ElementCount;
        m_pInfos[index].StorageBuffer   = false;
    }
    else if (kind == RESOURCE_KIND_TEXTURE)
    {
        m_pInfos[index].Image.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        m_pInfos[index].Image.imageView   = pWrapSRV->GetVulkanImageView();
        m_pInfos[index].StorageBuffer     = false;
    }
}

//-------------------------------------------------------------------------------------------------
//      ストレージを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetView(uint32_t index, IUnorderedAccessView* const pResource)
{
    A3D_ASSERT(index < m_pLayout->GetDesc().EntryCount );

    auto pWrapView = static_cast<UnorderedAccessView*>(pResource);
    A3D_ASSERT(pWrapView != nullptr);

    auto desc = pWrapView->GetDesc();

    auto kind = pWrapView->GetResource()->GetKind();
    if (kind == RESOURCE_KIND_BUFFER)
    {
        m_pInfos[index].Buffer.buffer = pWrapView->GetVulkanBuffer();
        m_pInfos[index].Buffer.offset = desc.FirstElement;
        m_pInfos[index].Buffer.range  = desc.ElementCount;
        m_pInfos[index].StorageBuffer = true;
    }
    else if (kind == RESOURCE_KIND_TEXTURE)
    {
        m_pInfos[index].Image.imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        m_pInfos[index].Image.imageView     = pWrapView->GetVulkanImageView();
        m_pInfos[index].StorageBuffer       = false;
    }
}

//-------------------------------------------------------------------------------------------------
//      サンプラーを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetSampler(uint32_t index, ISampler* pSampler)
{
    A3D_ASSERT(index < m_pLayout->GetDesc().EntryCount );

    auto pWrapSampler = static_cast<Sampler*>(pSampler);
    A3D_ASSERT(pWrapSampler != nullptr);

    m_pInfos[index].Image.sampler = pWrapSampler->GetVulkanSampler();
}

//-------------------------------------------------------------------------------------------------
//      更新処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::Update()
{
    if (m_pDevice->IsSupportExtension(Device::EXT_KHR_PUSH_DESCRIPTOR))
    { return; }

    const auto& desc = m_pLayout->GetDesc();
    auto count = desc.EntryCount;

    for(auto i=0u; i<count; ++i)
    {
        if (desc.Entries[i].Type == DESCRIPTOR_TYPE_CBV)
        {
            m_pWrites[i].pBufferInfo = &m_pInfos[i].Buffer;
            m_pWrites[i].pImageInfo  = nullptr;
        }
        else if (desc.Entries[i].Type == DESCRIPTOR_TYPE_SRV ||
                 desc.Entries[i].Type == DESCRIPTOR_TYPE_SMP)
        {
            m_pWrites[i].pImageInfo  = &m_pInfos[i].Image;
            m_pWrites[i].pBufferInfo = nullptr;
        }
        else if (desc.Entries[i].Type == DESCRIPTOR_TYPE_UAV)
        {
            if (m_pInfos[i].StorageBuffer)
            {
                m_pWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                m_pWrites[i].pBufferInfo    = &m_pInfos[i].Buffer;
                m_pWrites[i].pImageInfo     = nullptr;
            }
            else
            {
                m_pWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                m_pWrites[i].pBufferInfo    = nullptr;
                m_pWrites[i].pImageInfo     = &m_pInfos[i].Image;
            }
        }
    }
    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    vkUpdateDescriptorSets(pNativeDevice, count, m_pWrites, 0, nullptr);
}

//-------------------------------------------------------------------------------------------------
//      描画コマンドを生成します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::Issue(ICommandList* pCommandList)
{
#if defined(VK_KHR_PUSH_DESCRIPTOR_SPEC_VERSION)
    if (m_pDevice->IsSupportExtension(Device::EXT_KHR_PUSH_DESCRIPTOR))
    {
        auto desc  = m_pLayout->GetDesc();
        auto count = desc.EntryCount;

        for(auto i=0u; i<count; ++i)
        {
            if (desc.Entries[i].Type == DESCRIPTOR_TYPE_CBV)
            {
                m_pWrites[i].pBufferInfo = &m_pInfos[i].Buffer;
                m_pWrites[i].pImageInfo  = nullptr;
            }
            else if (desc.Entries[i].Type == DESCRIPTOR_TYPE_SRV ||
                     desc.Entries[i].Type == DESCRIPTOR_TYPE_SMP)
            {
                m_pWrites[i].pImageInfo  = &m_pInfos[i].Image;
                m_pWrites[i].pBufferInfo = nullptr;
            }
            else if (desc.Entries[i].Type == DESCRIPTOR_TYPE_UAV)
            {
                if (m_pInfos[i].StorageBuffer)
                {
                    m_pWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    m_pWrites[i].pBufferInfo    = &m_pInfos[i].Buffer;
                    m_pWrites[i].pImageInfo     = nullptr;
                }
                else
                {
                    m_pWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    m_pWrites[i].pImageInfo     = &m_pInfos[i].Image;
                    m_pWrites[i].pBufferInfo    = nullptr;
                }
            }
        }

        auto pWrapCommandList = static_cast<CommandList*>(pCommandList);
        A3D_ASSERT(pWrapCommandList != nullptr);

        auto pNativeCommandBuffer = pWrapCommandList->GetVulkanCommandBuffer();
        A3D_ASSERT(pNativeCommandBuffer != null_handle);

        vkCmdPushDescriptorSet(
            pNativeCommandBuffer,
            m_pLayout->GetVulkanPipelineBindPoint(),
            m_pLayout->GetVulkanPipelineLayout(),
            0,                                      // 0番目を更新.
            count,
            m_pWrites);
    }
    else
    {
        auto pWrapCommandList = static_cast<CommandList*>(pCommandList);
        A3D_ASSERT(pWrapCommandList != nullptr);

        auto pNativeCommandBuffer = pWrapCommandList->GetVulkanCommandBuffer();
        A3D_ASSERT(pNativeCommandBuffer != null_handle);

        vkCmdBindDescriptorSets(
            pNativeCommandBuffer,
            m_pLayout->GetVulkanPipelineBindPoint(),
            m_pLayout->GetVulkanPipelineLayout(),
            0,
            1,
            &m_DescriptorSet,
            0,
            nullptr);
    }
#else
    auto pWrapCommandList = static_cast<CommandList*>(pCommandList);
    A3D_ASSERT(pWrapCommandList != nullptr);

    auto pNativeCommandBuffer = pWrapCommandList->GetVulkanCommandBuffer();
    A3D_ASSERT(pNativeCommandBuffer != null_handle);

    vkCmdBindDescriptorSets(
        pNativeCommandBuffer,
        m_pLayout->GetVulkanPipelineBindPoint(),
        m_pLayout->GetVulkanPipelineLayout(),
        0,
        1,
        &m_DescriptorSet,
        0,
        nullptr);
#endif
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool DescriptorSet::Create
(
    IDevice*                pDevice,
    DescriptorSetLayout*    pLayout,
    IDescriptorSet**        ppDescriptorSet
)
{
    if (pDevice         == nullptr 
     || pLayout         == nullptr 
     || ppDescriptorSet == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new DescriptorSet;
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if (!instance->Init(pDevice, pLayout))
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppDescriptorSet = instance;
    return true;
}

} // namespace a3d
