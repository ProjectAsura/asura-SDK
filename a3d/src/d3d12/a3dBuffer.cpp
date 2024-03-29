﻿//-------------------------------------------------------------------------------------------------
// File : a3dBuffer.cpp
// Desc : Buffer Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Buffer::Buffer()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pResource   (nullptr)
{ /* DO_NOTIHNG */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Buffer::~Buffer()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理です.
//-------------------------------------------------------------------------------------------------
bool Buffer::Init(IDevice* pDevice, const BufferDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width              = pDesc->Size;
    desc.Height             = 1;
    desc.DepthOrArraySize   = 1;
    desc.Format             = DXGI_FORMAT_UNKNOWN;
    desc.MipLevels          = 1;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags              = ToNativeResourceFlags(pDesc->Usage);

    D3D12_RESOURCE_STATES   state = ToNativeState(pDesc->InitState);

    D3D12MA::ALLOCATION_DESC allocDesc = {};
    allocDesc.HeapType = ToNativeHeapType(pDesc->HeapType);

    auto hr = m_pDevice->GetAllocator()->CreateResource(
        &allocDesc, &desc, state, nullptr, &m_pAllocation, IID_PPV_ARGS(&m_pResource));
    if ( FAILED(hr) )
    { return false; }

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void Buffer::Term()
{
    SafeRelease(m_pAllocation);
    SafeRelease(m_pResource);
    SafeRelease(m_pDevice);

    memset(&m_Desc, 0, sizeof(m_Desc));
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void Buffer::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解法処理を行います.
//-------------------------------------------------------------------------------------------------
void Buffer::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Buffer::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void Buffer::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
BufferDesc Buffer::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      メモリマッピングします.
//-------------------------------------------------------------------------------------------------
void* Buffer::Map()
{ 
    void* ptr = nullptr;

    if (m_Desc.HeapType == HEAP_TYPE_READBACK)
    {
        D3D12_RANGE range = {};
        range.Begin = 0;
        range.End   = SIZE_T(m_Desc.Size);
    
        auto hr = m_pResource->Map(0, &range, &ptr);
        if ( FAILED(hr) )
        { return nullptr; }
    }
    else
    {
        auto hr = m_pResource->Map(0, nullptr, &ptr);
        if (FAILED(hr))
        { return nullptr; }
    }

    return ptr;
}

//-------------------------------------------------------------------------------------------------
//      メモリマッピングを解除します.
//-------------------------------------------------------------------------------------------------
void Buffer::Unmap()
{
    if (m_Desc.HeapType == HEAP_TYPE_READBACK)
    {
        m_pResource->Unmap(0, nullptr);
    }
    else
    {
        D3D12_RANGE range = {};
        range.Begin = 0;
        range.End   = SIZE_T(m_Desc.Size);

        m_pResource->Unmap(0, &range);
    }
}

//-------------------------------------------------------------------------------------------------
//      リソースタイプを取得します.
//-------------------------------------------------------------------------------------------------
RESOURCE_KIND Buffer::GetKind() const
{ return RESOURCE_KIND_BUFFER; }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* Buffer::GetD3D12Resource() const
{ return m_pResource; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Buffer::Create(IDevice* pDevice, const BufferDesc* pDesc, IBuffer** ppResource)
{
    if (pDevice == nullptr || pDesc == nullptr || ppResource == nullptr)
    { return false; }

    auto instance = new Buffer;
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, pDesc))
    {
        SafeRelease(instance);
        return false;
    }

    *ppResource = instance;
    return true;
}

} // namespace a3d
