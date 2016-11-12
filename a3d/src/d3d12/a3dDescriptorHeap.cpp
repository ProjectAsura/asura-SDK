//-------------------------------------------------------------------------------------------------
// File : a3dDescriptorHeap.cpp
// Desc : Descritptor Heap Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// DescritptorHeap class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorHeap::DescriptorHeap()
: m_pBuffer         (nullptr)
, m_pActive         (nullptr)
, m_pFree           (nullptr)
, m_pHeap           (nullptr)
, m_IncrementSize   (0)
, m_AllocatedCount  (0)
, m_HandleCount     (0)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorHeap::~DescriptorHeap()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool DescriptorHeap::Init(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    D3D12_DESCRIPTOR_HEAP_DESC desc = *pDesc;
    desc.NumDescriptors += 2;   // 管理用に2つ余計に必要.

    auto hr = pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pHeap));
    if ( FAILED(hr) )
    { return false; }

    m_IncrementSize = pDevice->GetDescriptorHandleIncrementSize(desc.Type);

    auto allocCount = desc.NumDescriptors;
    auto blockSize  = sizeof(Descriptor);

    m_pBuffer = static_cast<uint8_t*>( a3d_alloc( blockSize * allocCount, alignof(Descriptor) ) );
    if ( m_pBuffer == nullptr )
    { return false; }

    m_HandleCount = pDesc->NumDescriptors;
    memset( m_pBuffer, 0, blockSize * allocCount );

    auto hasHandleGPU = (pDesc->Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

    // 全ハンドルにアドレス割り当てとヒープポインタをの設定を行う.
    for(auto i=2u; i<allocCount; ++i)
    {
        // ハンドルにメモリを割り当てる.
        auto handle = AssignHandle(i);

        // CPUハンドルをディスクリプタを割り当て.
        {
            auto handleCPU = m_pHeap->GetCPUDescriptorHandleForHeapStart();
            handleCPU.ptr += m_IncrementSize * i;
            handle->m_HandleCPU = handleCPU;
        }

        // GPUハンドルをディスクリプタを割り当て
        if (hasHandleGPU)
        {
            auto handleGPU = m_pHeap->GetGPUDescriptorHandleForHeapStart();
            handleGPU.ptr += m_IncrementSize * i;
            handle->m_HandleGPU = handleGPU;
        }
    }

    // リストの初期設定
    {
        m_pActive = GetHandle(0);
        m_pActive->m_pPrev = m_pActive->m_pNext = m_pActive;

        m_pFree = GetHandle(1);

        for( auto i=1u; i<m_HandleCount + 1; ++i)
        {
            auto handle = GetHandle(i);
            handle->m_pPrev = nullptr;
            handle->m_pNext = GetHandle(i+1);
        }

        GetHandle(m_HandleCount + 1)->m_pPrev = m_pFree;
    }

    // カウンタリセット.
    m_AllocatedCount = 0;


    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorHeap::Term()
{
    if (m_pBuffer == nullptr)
    { return; }

    // ロックできるまでループ.
    for(;;)
    {
        if ( m_Mutex.try_lock() )
        { break; }

        // 1フレーム分寝かす.
        Sleep( 16 );
    }

    a3d_free(m_pBuffer);
    m_pBuffer = nullptr;

    m_HandleCount  = 0;
    m_AllocatedCount = 0;

    m_pActive = nullptr;
    m_pFree   = nullptr;

    m_Mutex.unlock();

    SafeRelease(m_pHeap);
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタを生成します.
//-------------------------------------------------------------------------------------------------
Descriptor* DescriptorHeap::CreateDescriptor()
{ return Alloc(); }

//-------------------------------------------------------------------------------------------------
//      ディスクリプタヒープを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12DescriptorHeap* DescriptorHeap::GetD3D12DescriptorHeap() const
{ return m_pHeap; }

//-------------------------------------------------------------------------------------------------
//      ハンドルを初期化します.
//-------------------------------------------------------------------------------------------------
Descriptor* DescriptorHeap::AssignHandle(uint32_t index) const
{
    if (index > m_HandleCount + 2)
    { return nullptr; }

    auto ptr = m_pBuffer;
    ptr += sizeof(Descriptor) * index;

    return new (ptr) Descriptor();
}

//-------------------------------------------------------------------------------------------------
//      ハンドルを取得します.
//-------------------------------------------------------------------------------------------------
Descriptor* DescriptorHeap::GetHandle(uint32_t index) const
{
    if (index > m_HandleCount + 2)
    { return nullptr; }

    auto ptr = m_pBuffer;
    ptr += sizeof(Descriptor) * index;
    return reinterpret_cast<Descriptor*>(ptr);
}

//-------------------------------------------------------------------------------------------------
//      ハンドルを割り当てます.
//-------------------------------------------------------------------------------------------------
Descriptor* DescriptorHeap::Alloc()
{
    std::lock_guard<std::mutex> gurad(m_Mutex);

    if ( m_pFree->m_pNext == m_pFree || m_AllocatedCount + 1 > m_HandleCount )
    { return nullptr; }

    auto handle = m_pFree->m_pNext;
    m_pFree->m_pNext = handle->m_pNext;

    handle->m_pPrev = m_pActive->m_pPrev;
    handle->m_pNext = m_pActive;
    handle->m_pPrev->m_pNext = handle->m_pNext->m_pPrev = handle;
    handle->m_pHeap = this;
    handle->AddRef();

    A3D_ASSERT(handle->GetHandleCPU().ptr != 0);

    m_AllocatedCount++;

    return handle;
}

//-------------------------------------------------------------------------------------------------
//      ハンドルを解放します.
//-------------------------------------------------------------------------------------------------
void DescriptorHeap::Free(Descriptor* handle)
{
    if ( handle == nullptr )
    { return; }

    std::lock_guard<std::mutex> gurad(m_Mutex);

    handle->m_pHeap = nullptr;
    handle->m_pNext = m_pFree->m_pNext;
    handle->m_pPrev->m_pNext = handle->m_pNext;
    A3D_ASSERT( handle->GetCount() == 0 );

    m_AllocatedCount--;
}

//-------------------------------------------------------------------------------------------------
//      割り当て済みハンドル数を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorHeap::GetAllocatedHandleCount() const
{ return m_AllocatedCount; }

//-------------------------------------------------------------------------------------------------
//      割り当て可能なハンドル数を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorHeap::GetAvailableHandleCount() const
{ return m_HandleCount - m_AllocatedCount; }

//-------------------------------------------------------------------------------------------------
//      ハンドル数を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorHeap::GetHandleCount() const
{ return m_HandleCount; }
 

} // namespace a3d
