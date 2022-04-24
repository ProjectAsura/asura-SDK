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
//      ディスクリプタセットレイアウトを取得します.
//-------------------------------------------------------------------------------------------------
DescriptorSetLayout* DescriptorSet::GetLayout() const
{ return m_pLayout; }

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
