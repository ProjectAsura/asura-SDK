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
: m_pHeap   (nullptr)
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
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    if (pDesc->NumDescriptors == 0)
    {
        A3D_LOG("Error : Invalid Argument.");
        return true;
    }

    auto hr = pDevice->CreateDescriptorHeap(pDesc, IID_PPV_ARGS(&m_pHeap));
    if ( FAILED(hr) )
    {
        A3D_LOG("Error : ID3D12Device::CreateDescritoprHeap() Failed. errcode = 0x%x", hr);
        return false;
    }

    // インクリメントサイズを取得.
    m_IncrementSize = pDevice->GetDescriptorHandleIncrementSize(pDesc->Type);

    // 総ハンドル数設定.
    m_TotalHandleCount = pDesc->NumDescriptors;

    // ディスクリプタ生成.
    m_pDescriptors = new Descriptor[m_TotalHandleCount];

    auto hasHandleGPU = (pDesc->Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

    for(auto i=0u; i<m_TotalHandleCount; ++i)
    {
        m_pDescriptors[i].m_Index = i;
        m_pDescriptors[i].m_HandleCPU = m_pHeap->GetCPUDescriptorHandleForHeapStart();
        m_pDescriptors[i].m_HandleCPU.ptr += (SIZE_T)m_IncrementSize * i;

        if (hasHandleGPU)
        {
            m_pDescriptors[i].m_HandleGPU = m_pHeap->GetGPUDescriptorHandleForHeapStart();
            m_pDescriptors[i].m_HandleGPU.ptr += (UINT64)m_IncrementSize * i;
        }

        m_FreeList.PushBack(&m_pDescriptors[i]);
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorHeap::Term()
{
    LockGuard locker(&m_SpinLock);
    m_TotalHandleCount = 0;
    m_FreeList.Clear();
    if (m_pDescriptors)
    {
        delete[] m_pDescriptors;
        m_pDescriptors = nullptr;
    }
    SafeRelease(m_pHeap);
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタを生成します.
//-------------------------------------------------------------------------------------------------
Descriptor* DescriptorHeap::CreateDescriptor()
{
    LockGuard locker(&m_SpinLock);
    return m_FreeList.PopFront();
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタを破棄します.
//-------------------------------------------------------------------------------------------------
void DescriptorHeap::DisposeDescriptor(Descriptor* pValue)
{
    LockGuard locker(&m_SpinLock);
    return m_FreeList.PushBack(pValue);
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタヒープを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12DescriptorHeap* DescriptorHeap::GetD3D12DescriptorHeap() const
{ return m_pHeap; }

//-------------------------------------------------------------------------------------------------
//      割り当て済みハンドル数を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorHeap::GetAllocatedHandleCount() const
{ return uint32_t(m_TotalHandleCount - m_FreeList.GetCount()); }

//-------------------------------------------------------------------------------------------------
//      割り当て可能なハンドル数を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorHeap::GetAvailableHandleCount() const
{ return uint32_t(m_FreeList.GetCount()); }

//-------------------------------------------------------------------------------------------------
//      ハンドル数を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorHeap::GetHandleCount() const
{ return m_TotalHandleCount; }
 

} // namespace a3d
