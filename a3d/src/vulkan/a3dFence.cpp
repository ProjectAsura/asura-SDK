//-------------------------------------------------------------------------------------------------
// File : a3dFence.cpp
// Desc : Fence Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Fence class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Fence::Fence()
: m_RefCount(1)
, m_pDevice (nullptr)
, m_Fence   (null_handle)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Fence::~Fence()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool Fence::Init(IDevice* pDevice)
{
    if (pDevice == nullptr)
    { return false; }

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != VK_NULL_HANDLE);

    VkFenceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;

    auto ret = vkCreateFence(pNativeDevice, &info, nullptr, &m_Fence);
    if ( ret != VK_SUCCESS )
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Fence::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != VK_NULL_HANDLE);

    vkDestroyFence( pNativeDevice, m_Fence, nullptr );
    m_Fence = VK_NULL_HANDLE;

    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void Fence::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void Fence::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを返却します.
//-------------------------------------------------------------------------------------------------
uint32_t Fence::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void Fence::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      シグナル状態かどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool Fence::IsSignaled() const
{
    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != VK_NULL_HANDLE);

    auto ret = vkGetFenceStatus(pNativeDevice, m_Fence);
    if (ret == VK_SUCCESS)
    { vkResetFences(pNativeDevice, 1, &m_Fence);  }

    return ret == VK_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
//      完了を待機します.
//-------------------------------------------------------------------------------------------------
bool Fence::Wait(uint32_t timeoutMsec)
{
    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != VK_NULL_HANDLE);

    const uint32_t MilliSecToNanoSec = 1000 * 1000;

    auto ret = vkWaitForFences(pNativeDevice, 1, &m_Fence, VK_TRUE, timeoutMsec * MilliSecToNanoSec);
    vkResetFences(pNativeDevice, 1, &m_Fence);
    return ( ret == VK_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//      フェンスを取得します.
//-------------------------------------------------------------------------------------------------
VkFence Fence::GetVulkanFence() const
{ return m_Fence; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Fence::Create(IDevice* pDevice, IFence** ppFence)
{
    if (pDevice == nullptr || ppFence == nullptr)
    { return false; }

    auto instance = new (std::nothrow) Fence();
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice))
    {
        SafeRelease(instance);
        return false;
    }

    *ppFence = instance;
    return true;
}

} // namespace a3d
