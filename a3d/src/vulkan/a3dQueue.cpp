//-------------------------------------------------------------------------------------------------
// File : a3dQueue.cpp
// Desc : Command Queue.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Queue class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Queue::Queue()
: m_RefCount        (1)
, m_pDevice         (nullptr)
, m_Queue           (null_handle)
, m_SubmitIndex     (0)
, m_pSubmitList     (nullptr)
, m_FamilyIndex     (0)
, m_MaxSubmitCount  (0)
, m_SignalSemaphore (null_handle)
, m_WaitSemaphore   (null_handle)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Queue::~Queue()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool Queue::Init
(
    IDevice*    pDevice,
    uint32_t    familyIndex,
    uint32_t    queueIndex,
    uint32_t    maxSubmitCount
)
{
    if (pDevice == nullptr || maxSubmitCount == 0)
    { return false; }

    // NOTE : Device 無いから呼ばれるため，
    // AddRef() してしまうと Device が解放できなくなるため，AddRef() してはいけない !!
    m_pDevice = pDevice;

    auto pWrapDevice = reinterpret_cast<Device*>(pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    {
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;

        auto ret = vkCreateSemaphore( pNativeDevice, &info, nullptr, &m_SignalSemaphore);
        if ( ret != VK_SUCCESS )
        { return false; }

        ret = vkCreateSemaphore( pNativeDevice, &info, nullptr, &m_WaitSemaphore );
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    vkGetDeviceQueue(pNativeDevice, familyIndex, queueIndex, &m_Queue);

    m_MaxSubmitCount = maxSubmitCount;
    m_FamilyIndex    = familyIndex;

    m_pSubmitList = new (std::nothrow) VkCommandBuffer[maxSubmitCount];
    if (m_pSubmitList == nullptr)
    { return false; }

    for(auto i=0u; i<maxSubmitCount; ++i)
    { m_pSubmitList[i] = null_handle; }

    m_SubmitIndex = 0;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Queue::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    // 完了を待機する.
    if (m_Queue != null_handle)
    { vkQueueWaitIdle( m_Queue ); }

    if (m_SignalSemaphore != null_handle)
    {
        vkDestroySemaphore(pNativeDevice, m_SignalSemaphore, nullptr);
        m_SignalSemaphore = null_handle;
    }

    if (m_WaitSemaphore != null_handle)
    {
        vkDestroySemaphore(pNativeDevice, m_WaitSemaphore, nullptr);
        m_WaitSemaphore = null_handle;
    }

    if (m_pSubmitList != nullptr)
    {
        delete [] m_pSubmitList;
        m_pSubmitList = nullptr;
    }

    m_SubmitIndex    = 0;
    m_MaxSubmitCount = 0;
    m_FamilyIndex    = 0;
    m_Queue          = null_handle;
    SafeRelease( m_pDevice );
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void Queue::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void Queue::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Queue::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void Queue::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      コマンドリストを登録します.
//-------------------------------------------------------------------------------------------------
bool Queue::Submit(ICommandList* pCommandList)
{
    std::lock_guard<std::mutex> locker(m_Mutex);

    if (m_SubmitIndex + 1 >= m_MaxSubmitCount)
    { return false; }

    auto pWrapList = reinterpret_cast<CommandList*>(pCommandList);
    A3D_ASSERT( pWrapList != nullptr );

    auto pNativeCommandBuffer = pWrapList->GetVulkanCommandBuffer();

    m_pSubmitList[m_SubmitIndex] = pNativeCommandBuffer;
    m_SubmitIndex++;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      登録したコマンドリストを実行します.
//-------------------------------------------------------------------------------------------------
void Queue::Execute(IFence* pFence)
{
    VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    VkFence nativeFence = VK_NULL_HANDLE;
    VkSubmitInfo info = {};

    if ( pFence != nullptr )
    {
        info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.pNext                  = nullptr;
        info.pCommandBuffers        = m_pSubmitList;
        info.commandBufferCount     = m_SubmitIndex;
        info.waitSemaphoreCount     = 1;
        info.pWaitSemaphores        = &m_WaitSemaphore;
        info.pWaitDstStageMask      = &stageMask;
        info.signalSemaphoreCount   = 0;
        info.pSignalSemaphores      = nullptr;

        auto pWrapFence = reinterpret_cast<Fence*>(pFence);
        A3D_ASSERT(pWrapFence != nullptr);

        nativeFence = pWrapFence->GetVulkanFence();
    }
    else
    {
        info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.pNext                  = nullptr;
        info.pCommandBuffers        = m_pSubmitList;
        info.commandBufferCount     = m_SubmitIndex;
        info.waitSemaphoreCount     = 0;
        info.pWaitSemaphores        = nullptr;
        info.pWaitDstStageMask      = &stageMask;
        info.signalSemaphoreCount   = 0;
        info.pSignalSemaphores      = nullptr;
    }
 
    auto ret = vkQueueSubmit( m_Queue, 1, &info, nativeFence );
    A3D_ASSERT( ret == VK_SUCCESS );
    A3D_UNUSED( ret );

    // 実行したら戻す.
    m_SubmitIndex = 0;
}

//-------------------------------------------------------------------------------------------------
//      コマンドの実行が完了するまで待機します.
//-------------------------------------------------------------------------------------------------
void Queue::WaitIdle()
{
    auto ret = vkQueueWaitIdle( m_Queue );
    A3D_ASSERT( ret == VK_SUCCESS );
    A3D_UNUSED( ret );
}

//-------------------------------------------------------------------------------------------------
//      コマンドキューを取得します.
//-------------------------------------------------------------------------------------------------
VkQueue Queue::GetVulkanQueue() const
{ return m_Queue; }

//-------------------------------------------------------------------------------------------------
//      シグナルセマフォを取得します.
//-------------------------------------------------------------------------------------------------
VkSemaphore Queue::GetVulkanSignalSemaphore() const
{ return m_SignalSemaphore; }

//-------------------------------------------------------------------------------------------------
//      ウェイトセマフォを取得します.
//-------------------------------------------------------------------------------------------------
VkSemaphore Queue::GetVulkanWaitSemaphore() const
{ return m_WaitSemaphore; }

//-------------------------------------------------------------------------------------------------
//      ファミリーインデックスを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Queue::GetFamilyIndex() const
{ return m_FamilyIndex; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Queue::Create
(
    IDevice*    pDevice,
    uint32_t    familyIndex,
    uint32_t    queueIndex,
    uint32_t    maxSubmitCount,
    IQueue**    ppQueue
)
{
    if (pDevice == nullptr || maxSubmitCount == 0)
    { return false; }

    auto instance = new(std::nothrow) Queue;
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, familyIndex, queueIndex, maxSubmitCount))
    {
        SafeRelease(instance);
        return false;
    }

    *ppQueue = instance;
    return true;
}

} // namespace a3d
