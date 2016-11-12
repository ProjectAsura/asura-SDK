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
    { return false; }

    // NOTE : Deviceから呼ばれるので，参照カウントを増やしてまうと
    // Device が解放されなくなるので AddRef() しないこと!!
    m_pDevice = pDevice;

    auto pWrapDevice = reinterpret_cast<Device*>(pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetD3D12Device();
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
        { return false; }
    }

    m_MaxSubmitCount = maxSubmitCount;

    {
        m_pSubmitList = new ID3D12CommandList* [maxSubmitCount];
        if (m_pSubmitList == nullptr)
        { return false; }

        m_SubmitIndex = 0;
    }

    {
        auto hr = pNativeDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence) );
        if ( FAILED(hr) )
        { return false; }

        m_Event = CreateEventEx( nullptr, FALSE, FALSE, EVENT_ALL_ACCESS );
        if ( m_Event == nullptr )
        { return false; }
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
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void Queue::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef();}
}

//-------------------------------------------------------------------------------------------------
//      コマンドリストを登録します.
//-------------------------------------------------------------------------------------------------
bool Queue::Submit( ICommandList* pCommandList )
{
    std::lock_guard<std::mutex> locker(m_Mutex);

    if (m_SubmitIndex + 1 >= m_MaxSubmitCount)
    { return false; }

    auto pWrapList = reinterpret_cast<CommandList*>(pCommandList);
    A3D_ASSERT( pWrapList != nullptr );

    auto pNativeList = pWrapList->GetD3D12GraphicsCommandList();

    m_pSubmitList[m_SubmitIndex] = reinterpret_cast<ID3D12CommandList*>(pNativeList);
    m_SubmitIndex++;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      登録したコマンドリストを実行します.
//-------------------------------------------------------------------------------------------------
void Queue::Execute( IFence* pFence )
{
    m_pQueue->ExecuteCommandLists( m_SubmitIndex, m_pSubmitList );

    if (pFence != nullptr)
    {
        auto pWrapFence = reinterpret_cast<Fence*>(pFence);
        A3D_ASSERT( pWrapFence != nullptr );

        auto pNativeFence = pWrapFence->GetD3D12Fence();
        auto fenceValue = pWrapFence->GetFenceValue();
        m_pQueue->Signal( pNativeFence, fenceValue );

        pWrapFence->AdvanceValue();
    }

    m_SubmitIndex = 0;
}

//-------------------------------------------------------------------------------------------------
//      コマンドの実行が完了するまで待機します.
//-------------------------------------------------------------------------------------------------
void Queue::WaitIdle()
{
    m_pFence->Signal( 0 );
    m_pFence->SetEventOnCompletion( 1, m_Event );
    m_pQueue->Signal( m_pFence, 1 );
    WaitForSingleObject( m_Event, INFINITE );
}

//-------------------------------------------------------------------------------------------------
//      キューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12CommandQueue* Queue::GetD3D12Queue() const
{ return m_pQueue; }

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
    { return false; }

    if (!instance->Init(pDevice, type, maxSubmitCount))
    {
        SafeRelease(instance);
        return false;
    }

    *ppQueue = instance;
    return true;
}

} // namespace a3d
