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
, m_Pool    ()
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

    if (pDesc->NumDescriptors == 0)
    { return true; }

    auto hr = pDevice->CreateDescriptorHeap(pDesc, IID_PPV_ARGS(&m_pHeap));
    if ( FAILED(hr) )
    { return false; }

    // インクリメントサイズを取得.
    m_IncrementSize = pDevice->GetDescriptorHandleIncrementSize(pDesc->Type);

    if (!m_Pool.Init(pDesc->NumDescriptors))
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorHeap::Term()
{
    m_Pool.Term();
    SafeRelease(m_pHeap);
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタを生成します.
//-------------------------------------------------------------------------------------------------
Descriptor* DescriptorHeap::CreateDescriptor()
{
    auto desc = m_pHeap->GetDesc();
    auto hasHandleGPU = (desc.Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

    auto initializer = [&](uint32_t index, Descriptor* value)
    {
        // ヒープを設定.
        value->m_pHeap = this;

        // CPUハンドルをディスクリプタを割り当て.
        {
            auto handleCPU = m_pHeap->GetCPUDescriptorHandleForHeapStart();
            handleCPU.ptr += m_IncrementSize * index;
            value->m_HandleCPU = handleCPU;
        }

        // GPUハンドルをディスクリプタを割り当て
        if (hasHandleGPU)
        {
            auto handleGPU = m_pHeap->GetGPUDescriptorHandleForHeapStart();
            handleGPU.ptr += m_IncrementSize * index;
            value->m_HandleGPU = handleGPU;
        }
    };

    return m_Pool.Alloc(initializer);
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタを破棄します.
//-------------------------------------------------------------------------------------------------
void DescriptorHeap::DisposeDescriptor(Descriptor* pValue)
{ m_Pool.Free(pValue); }

//-------------------------------------------------------------------------------------------------
//      ディスクリプタヒープを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12DescriptorHeap* DescriptorHeap::GetD3D12DescriptorHeap() const
{ return m_pHeap; }

//-------------------------------------------------------------------------------------------------
//      割り当て済みハンドル数を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorHeap::GetAllocatedHandleCount() const
{ return m_Pool.GetUsedCount(); }

//-------------------------------------------------------------------------------------------------
//      割り当て可能なハンドル数を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorHeap::GetAvailableHandleCount() const
{ return m_Pool.GetAvailableCount(); }

//-------------------------------------------------------------------------------------------------
//      ハンドル数を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorHeap::GetHandleCount() const
{ return m_Pool.GetSize(); }
 

} // namespace a3d
