//-------------------------------------------------------------------------------------------------
// File : a3dQueryPool.cpp
// Desc : QueryPool Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// QueryPool class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
QueryPool::QueryPool()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pQueryHeap  (nullptr)
, m_QueryType   (D3D12_QUERY_TYPE_TIMESTAMP)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
QueryPool::~QueryPool()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool QueryPool::Init(IDevice* pDevice, const QueryPoolDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    auto pWrapDevice = reinterpret_cast<Device*>(pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    {
        D3D12_QUERY_HEAP_DESC desc = {};
        desc.Count = pDesc->Count;

        switch(pDesc->Type)
        {
        case QUERY_TYPE_OCCLUSION:
            {
                desc.Type   = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
                m_QueryType = D3D12_QUERY_TYPE_OCCLUSION;
            }
            break;

        case QUERY_TYPE_TIMESTAMP:
            {
                desc.Type   = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
                m_QueryType = D3D12_QUERY_TYPE_TIMESTAMP;
            }
            break;

        case QUERY_TYPE_PIPELINE_STATISTICS:
            {
                desc.Type   = D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
                m_QueryType = D3D12_QUERY_TYPE_PIPELINE_STATISTICS;
            }
            break;
        }

        auto hr = pNativeDevice->CreateQueryHeap(&desc, IID_PPV_ARGS(&m_pQueryHeap));
        if (FAILED(hr))
        { return false; }
    }

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void QueryPool::Term()
{
    SafeRelease(m_pQueryHeap);
    SafeRelease(m_pDevice);

    memset(&m_Desc, 0, sizeof(m_Desc));
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void QueryPool::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void QueryPool::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t QueryPool::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void QueryPool::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
QueryPoolDesc QueryPool::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      クエリヒープを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12QueryHeap* QueryPool::GetD3D12QueryHeap() const
{ return m_pQueryHeap; }

//-------------------------------------------------------------------------------------------------
//      クエリタイプを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_QUERY_TYPE QueryPool::GetD3D12QueryType() const
{ return m_QueryType; }

//-------------------------------------------------------------------------------------------------
//      クエリプールを生成します.
//-------------------------------------------------------------------------------------------------
bool QueryPool::Create(IDevice* pDevice, const QueryPoolDesc* pDesc, IQueryPool** ppQueryPool)
{
    if (pDevice == nullptr || pDesc == nullptr || ppQueryPool == nullptr)
    { return false; }

    auto instance = new QueryPool;
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, pDesc))
    {
        SafeRelease(instance);
        return false;
    }

    *ppQueryPool = instance;
    return true;
}

} // namespace a3d
