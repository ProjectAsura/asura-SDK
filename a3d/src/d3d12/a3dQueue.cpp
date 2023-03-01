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
, m_pQueue          (nullptr)
, m_MaxSubmitCount  (0)
, m_SubmitIndex     (0)
, m_pSubmitList     (nullptr)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Queue::~Queue()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool Queue::Init(IDevice* pDevice, COMMANDLIST_TYPE type, uint32_t maxSubmitCount)
{
    if (pDevice == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    // NOTE : Deviceから呼ばれるので，参照カウントを増やしてまうと
    // Device が解放されなくなるので AddRef() しないこと!!
    m_pDevice = static_cast<Device*>(pDevice);

    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        switch(type)
        {
        case COMMANDLIST_TYPE_DIRECT:
            desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            break;

        case COMMANDLIST_TYPE_COMPUTE:
            desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            break;

        case COMMANDLIST_TYPE_COPY:
            desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
            break;

        case COMMANDLIST_TYPE_BUNDLE:
            desc.Type = D3D12_COMMAND_LIST_TYPE_BUNDLE;
            break;
        }

        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.NodeMask = 0;
        desc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;;

        auto hr = pNativeDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pQueue));
        if ( FAILED(hr) )
        {
            A3D_LOG("Error : ID3D12Device::CreateCommandQueue() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    m_MaxSubmitCount = maxSubmitCount;

    {
        m_pSubmitList = new ID3D12CommandList* [maxSubmitCount];
        if (m_pSubmitList == nullptr)
        {
            A3D_LOG("Error : Out Of Memory.");
            return false;
        }

        m_SubmitIndex = 0;
    }

    {
        auto hr = pNativeDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence) );
        if ( FAILED(hr) )
        {
            A3D_LOG("Error : ID3D12Device::CreateFence() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_Event = CreateEventEx( nullptr, FALSE, FALSE, EVENT_ALL_ACCESS );
        if ( m_Event == nullptr )
        {
            A3D_LOG("Error : CreateEventEx() Failed.");
            return false;
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Queue::Term()
{
    // 完了を待機.
    if (m_pQueue != nullptr && m_pFence != nullptr && m_Event != nullptr)
    { WaitIdle(); }

    SafeRelease(m_pQueue);

    if (m_pSubmitList != nullptr)
    {
        delete[] m_pSubmitList;
        m_pSubmitList = nullptr;
    }

    if (m_Event != nullptr)
    {
        CloseHandle(m_Event);
        m_Event = nullptr;
    }

    SafeRelease(m_pFence);
    SafeRelease(m_pDevice);

    m_SubmitIndex    = 0;
    m_MaxSubmitCount = 0;
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
    m_pQueue->SetPrivateData(WKPDID_D3DDebugObjectName, m_Name.size(), m_Name.c_str());
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
    { m_pDevice->AddRef();}
}

//-------------------------------------------------------------------------------------------------
//      キューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12CommandQueue* Queue::GetD3D12Queue() const
{ return m_pQueue; }

//-------------------------------------------------------------------------------------------------
//      コマンドリストを登録します.
//-------------------------------------------------------------------------------------------------
bool IQueue::Submit( ICommandList* pCommandList )
{
    auto pThis = static_cast<Queue*>(this);
    A3D_ASSERT(pThis != nullptr);

    LockGuard locker(&pThis->m_Lock);

    if (pThis->m_SubmitIndex + 1 >= pThis->m_MaxSubmitCount)
    { return false; }

    auto pWrapList = static_cast<CommandList*>(pCommandList);
    A3D_ASSERT( pWrapList != nullptr );

    auto pNativeList = pWrapList->GetD3D12GraphicsCommandList();

    pThis->m_pSubmitList[pThis->m_SubmitIndex] = static_cast<ID3D12CommandList*>(pNativeList);
    pThis->m_SubmitIndex++;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      登録したコマンドリストを実行します.
//-------------------------------------------------------------------------------------------------
void IQueue::Execute( IFence* pFence )
{
    auto pThis = static_cast<Queue*>(this);
    A3D_ASSERT(pThis != nullptr);

    pThis->m_pQueue->ExecuteCommandLists(pThis->m_SubmitIndex, pThis->m_pSubmitList );

    if (pFence != nullptr)
    {
        auto pWrapFence = static_cast<Fence*>(pFence);
        A3D_ASSERT( pWrapFence != nullptr );

        auto pNativeFence = pWrapFence->GetD3D12Fence();
        auto fenceValue = pWrapFence->GetFenceValue();
        pThis->m_pQueue->Signal( pNativeFence, fenceValue );

        pWrapFence->AdvanceValue();
    }

    pThis->m_SubmitIndex = 0;
}

//-------------------------------------------------------------------------------------------------
//      コマンドの実行が完了するまで待機します.
//-------------------------------------------------------------------------------------------------
void IQueue::WaitIdle()
{
    auto pThis = static_cast<Queue*>(this);
    A3D_ASSERT(pThis != nullptr);

    pThis->m_pFence->Signal( 0 );
    pThis->m_pFence->SetEventOnCompletion( 1, pThis->m_Event );
    pThis->m_pQueue->Signal( pThis->m_pFence, 1 );
    WaitForSingleObject( pThis->m_Event, INFINITE );
}

//-------------------------------------------------------------------------------------------------
//      画面に表示を行います.
//-------------------------------------------------------------------------------------------------
void IQueue::Present( ISwapChain* pSwapChain )
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
    IDevice*            pDevice,
    COMMANDLIST_TYPE    type,
    uint32_t            maxSubmitCount,
    IQueue**            ppQueue
)
{
    auto instance = new Queue();
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if (!instance->Init(pDevice, type, maxSubmitCount))
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppQueue = instance;
    return true;
}

} // namespace a3d
