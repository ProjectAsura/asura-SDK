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
: m_RefCount            (1)
, m_pDevice             (nullptr)
, m_Queue               (null_handle)
, m_SubmitIndex         (0)
, m_pSubmitList         (nullptr)
, m_FamilyIndex         (0)
, m_MaxSubmitCount      (0)
, m_CurrentBufferIndex  (0)
, m_PreviousBufferIndex (0)
{
    for(auto i=0u; i<MaxBufferCount; ++i)
    {
        m_WaitSemaphore[i]   = null_handle;
        m_SignalSemaphore[i] = null_handle;
        m_Fence[i]           = null_handle;
    }
}

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
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    // NOTE : Device 無いから呼ばれるため，
    // AddRef() してしまうと Device が解放できなくなるため，AddRef() してはいけない !!
    m_pDevice = static_cast<Device*>(pDevice);

    auto pNativeDevice = m_pDevice->GetVkDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    {
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;

        for(auto i=0u; i<MaxBufferCount; ++i)
        {
            auto ret = vkCreateSemaphore( pNativeDevice, &info, nullptr, &m_SignalSemaphore[i]);
            if ( ret != VK_SUCCESS )
            {
                A3D_LOG("Error : vkCreateSemaphore() Failed. VkResult = %s", ToString(ret));
                return false;
            }

            ret = vkCreateSemaphore( pNativeDevice, &info, nullptr, &m_WaitSemaphore[i] );
            if ( ret != VK_SUCCESS )
            {
                A3D_LOG("Error : vkCreateSemaphore() Failed. VkResult = %s", ToString(ret));
                return false;
            }
        }
    }

    {
        VkFenceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;

        for(auto i=0; i<MaxBufferCount; ++i)
        {
            auto ret = vkCreateFence( pNativeDevice, &info, nullptr, &m_Fence[i]);
            if ( ret != VK_SUCCESS )
            {
                A3D_LOG("Error : vkCreateFence() Failed. VkResult = %s", ToString(ret));
                return false;
            }

            ret = vkResetFences( pNativeDevice, 1, &m_Fence[i] );
            if ( ret != VK_SUCCESS )
            {
                info.flags = 0;
            }
        }
    }

    vkGetDeviceQueue(pNativeDevice, familyIndex, queueIndex, &m_Queue);

    m_MaxSubmitCount = maxSubmitCount;
    m_FamilyIndex    = familyIndex;

    m_pSubmitList = new VkCommandBuffer[maxSubmitCount];
    if (m_pSubmitList == nullptr)
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    for(auto i=0u; i<maxSubmitCount; ++i)
    { m_pSubmitList[i] = null_handle; }

    m_SubmitIndex = 0;
    m_CurrentBufferIndex  = 0;
    m_PreviousBufferIndex = 0;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Queue::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    auto pNativeDevice = m_pDevice->GetVkDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    // 完了を待機する.
    if (m_Queue != null_handle)
    { vkQueueWaitIdle( m_Queue ); }

    for(auto i=0u; i<MaxBufferCount; ++i)
    {
        if (m_SignalSemaphore[i] != null_handle)
        {
            vkDestroySemaphore(pNativeDevice, m_SignalSemaphore[i], nullptr);
            m_SignalSemaphore[i] = null_handle;
        }

        if (m_WaitSemaphore[i] != null_handle)
        {
            vkDestroySemaphore(pNativeDevice, m_WaitSemaphore[i], nullptr);
            m_WaitSemaphore[i] = null_handle;
        }

        if (m_Fence[i] != null_handle)
        {
            vkDestroyFence(pNativeDevice, m_Fence[i], nullptr);
            m_Fence[i] = null_handle;
        }
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
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void Queue::SetName(const char* name)
{
    m_Name = name;
    if (vkDebugMarkerSetObjectName != nullptr)
    {
        auto pWrapDevice = static_cast<Device*>(m_pDevice);
        A3D_ASSERT(pWrapDevice != nullptr);

        VkDebugMarkerObjectNameInfoEXT info = {};
        info.sType          = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
        info.objectType     = VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT;
        info.object         = uint64_t(m_Queue);
        info.pObjectName    = name;

        vkDebugMarkerSetObjectName(pWrapDevice->GetVkDevice(), &info);
    }
}

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* Queue::GetName() const
{ return m_Name.c_str(); }

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
//      コマンドキューを取得します.
//-------------------------------------------------------------------------------------------------
VkQueue Queue::GetVkQueue() const
{ return m_Queue; }

//-------------------------------------------------------------------------------------------------
//      シグナルセマフォを取得します.
//-------------------------------------------------------------------------------------------------
VkSemaphore Queue::GetVkSignalSemaphore(uint32_t index) const
{
    A3D_ASSERT(0 <= index && index < MaxBufferCount);
    return m_SignalSemaphore[index];
}

//-------------------------------------------------------------------------------------------------
//      ウェイトセマフォを取得します.
//-------------------------------------------------------------------------------------------------
VkSemaphore Queue::GetVkWaitSemaphore(uint32_t index) const
{
    A3D_ASSERT(0 <= index && index < MaxBufferCount);
    return m_WaitSemaphore[index];
}

//-------------------------------------------------------------------------------------------------
//      フェンスを取得します.
//-------------------------------------------------------------------------------------------------
VkFence Queue::GetVkFence(uint32_t index) const
{
    A3D_ASSERT(0 <= index && index < MaxBufferCount);
    return m_Fence[index];
}

//-------------------------------------------------------------------------------------------------
//      ファミリーインデックスを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Queue::GetFamilyIndex() const
{ return m_FamilyIndex; }

//-------------------------------------------------------------------------------------------------
//      現在のバッファ番号を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Queue::GetCurrentBufferIndex() const
{ return m_CurrentBufferIndex; }

//-------------------------------------------------------------------------------------------------
//      以前のバッファ番号を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Queue::GetPreviousBufferIndex() const
{ return m_PreviousBufferIndex; }

//-------------------------------------------------------------------------------------------------
//      同期オブジェクトをリセットします.
//-------------------------------------------------------------------------------------------------
bool Queue::ResetSyncObject()
{
    // この関数は SwapChain::ResizerBuffer() した後に，
    // セマフォがシグナルでもウェイトでもどちらでもない状態になることがあるので，
    // バッファ番号をいったんリセットし，セマフォも正しい状態に戻すため再作成を行います.

    auto pNativeDevice = m_pDevice->GetVkDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    // 一旦破棄.
    for(auto i=0u; i<MaxBufferCount; ++i)
    {
        if (m_SignalSemaphore[i] != null_handle)
        {
            vkDestroySemaphore(pNativeDevice, m_SignalSemaphore[i], nullptr);
            m_SignalSemaphore[i] = null_handle;
        }

        if (m_WaitSemaphore[i] != null_handle)
        {
            vkDestroySemaphore(pNativeDevice, m_WaitSemaphore[i], nullptr);
            m_WaitSemaphore[i] = null_handle;
        }

        if (m_Fence[i] != null_handle)
        {
            vkDestroyFence(pNativeDevice, m_Fence[i], nullptr);
            m_Fence[i] = null_handle;
        }
    }

    // セマフォ再作成.
    {
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;

        for(auto i=0u; i<MaxBufferCount; ++i)
        {
            auto ret = vkCreateSemaphore( pNativeDevice, &info, nullptr, &m_SignalSemaphore[i]);
            if ( ret != VK_SUCCESS )
            {
                A3D_LOG("Error : vkCreateSemaphore() Failed. VkResult = %s", ToString(ret));
                return false;
            }

            ret = vkCreateSemaphore( pNativeDevice, &info, nullptr, &m_WaitSemaphore[i] );
            if ( ret != VK_SUCCESS )
            {
                A3D_LOG("Error : vkCreateSemaphore() Failed. VkResult = %s", ToString(ret));
                return false;
            }
        }
    }

    // フェンス再作成.
    {
        VkFenceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;

        for(auto i=0; i<MaxBufferCount; ++i)
        {
            auto ret = vkCreateFence( pNativeDevice, &info, nullptr, &m_Fence[i]);
            if ( ret != VK_SUCCESS )
            {
                A3D_LOG("Error : vkCreateFence() Failed. VkResult = %s", ToString(ret));
                return false;
            }

            ret = vkResetFences( pNativeDevice, 1, &m_Fence[i] );
            if ( ret != VK_SUCCESS )
            {
                info.flags = 0;
            }
        }
    }

    // バッファ番号リセット.
    m_PreviousBufferIndex = 0;
    m_CurrentBufferIndex  = 0;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      コマンドリストを登録します.
//-------------------------------------------------------------------------------------------------
bool IQueue::Submit(ICommandList* pCommandList)
{
    auto pThis = static_cast<Queue*>(this);
    A3D_ASSERT(pThis != nullptr);

    LockGuard locker(&pThis->m_Lock);

    if (pThis->m_SubmitIndex + 1 >= pThis->m_MaxSubmitCount)
    { return false; }

    auto pWrapList = static_cast<CommandList*>(pCommandList);
    A3D_ASSERT( pWrapList != nullptr );

    auto pNativeCommandBuffer = pWrapList->GetVkCommandBuffer();

    pThis->m_pSubmitList[pThis->m_SubmitIndex] = pNativeCommandBuffer;
    pThis->m_SubmitIndex++;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      登録したコマンドリストを実行します.
//-------------------------------------------------------------------------------------------------
void IQueue::Execute(IFence* pFence)
{
    auto pThis = static_cast<Queue*>(this);
    A3D_ASSERT(pThis != nullptr);

    VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    VkFence nativeFence = VK_NULL_HANDLE;
    VkSubmitInfo info = {};

    if ( pFence != nullptr )
    {
        info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.pNext                  = nullptr;
        info.pCommandBuffers        = pThis->m_pSubmitList;
        info.commandBufferCount     = pThis->m_SubmitIndex;
        info.waitSemaphoreCount     = 1;
        info.pWaitSemaphores        = &pThis->m_WaitSemaphore[pThis->m_CurrentBufferIndex];
        info.pWaitDstStageMask      = &stageMask;
        info.signalSemaphoreCount   = 0;
        info.pSignalSemaphores      = nullptr;

        auto pWrapFence = reinterpret_cast<Fence*>(pFence);
        A3D_ASSERT(pWrapFence != nullptr);

        nativeFence = pWrapFence->GetVkFence();
    }
    else
    {
        info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.pNext                  = nullptr;
        info.pCommandBuffers        = pThis->m_pSubmitList;
        info.commandBufferCount     = pThis->m_SubmitIndex;
        info.waitSemaphoreCount     = 0;
        info.pWaitSemaphores        = nullptr;
        info.pWaitDstStageMask      = &stageMask;
        info.signalSemaphoreCount   = 0;
        info.pSignalSemaphores      = nullptr;
    }
 
    auto ret = vkQueueSubmit( pThis->m_Queue, 1, &info, nativeFence );
    A3D_ASSERT( ret == VK_SUCCESS );
    A3D_UNUSED( ret );

    // 実行したら戻す.
    pThis->m_SubmitIndex = 0;

    // バッファリング.
    pThis->m_PreviousBufferIndex = pThis->m_CurrentBufferIndex;
    pThis->m_CurrentBufferIndex  = (pThis->m_CurrentBufferIndex + 1) % pThis->MaxBufferCount;
}

//-------------------------------------------------------------------------------------------------
//      コマンドの実行が完了するまで待機します.
//-------------------------------------------------------------------------------------------------
void IQueue::WaitIdle()
{
    auto pThis = static_cast<Queue*>(this);
    A3D_ASSERT(pThis != nullptr);

    auto ret = vkQueueWaitIdle( pThis->m_Queue );
    A3D_ASSERT( ret == VK_SUCCESS );
    A3D_UNUSED( ret );

    pThis->m_PreviousBufferIndex = pThis->m_CurrentBufferIndex;
    pThis->m_CurrentBufferIndex  = 0;
}

//-------------------------------------------------------------------------------------------------
//      画面に表示を行います.
//-------------------------------------------------------------------------------------------------
void IQueue::Present(ISwapChain* pSwapChain)
{
    auto pWrapSwapChain = reinterpret_cast<SwapChain*>(pSwapChain);
    if (pWrapSwapChain == nullptr)
    { return; }

    pWrapSwapChain->Present();
}

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
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new Queue;
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if (!instance->Init(pDevice, familyIndex, queueIndex, maxSubmitCount))
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppQueue = instance;
    return true;
}

} // namespace a3d
