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
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    auto pD3D11Device = m_pDevice->GetD3D11Device();
    A3D_ASSERT(pD3D11Device != nullptr);

    m_pQuery = new ID3D11Query* [pDesc->Count];
    if (m_pQuery == nullptr)
    { return false; }

    {
        D3D11_QUERY_DESC desc = {};

        switch(pDesc->Type)
        {
        case QUERY_TYPE_OCCLUSION:
            { desc.Query = D3D11_QUERY_OCCLUSION; }
            break;

        case QUERY_TYPE_TIMESTAMP:
            { desc.Query = D3D11_QUERY_TIMESTAMP; }
            break;

        case QUERY_TYPE_PIPELINE_STATISTICS:
            { desc.Query = D3D11_QUERY_PIPELINE_STATISTICS; }
            break;
        }

        for(auto i=0u; i<m_Desc.Count; ++i)
        {
            ID3D11Query* pQuery;
            auto hr = pD3D11Device->CreateQuery(&desc, &pQuery);
            if ( FAILED(hr) )
            {
                A3D_LOG("Error : ID3D11Device::CreateQuery() Failed. errcode = 0x%x", hr);
                return false;
            }

            m_pQuery[i] = pQuery;
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void QueryPool::Term()
{
    if (m_pQuery != nullptr)
    {
        for(auto i=0u; i<m_Desc.Count; ++i)
        { SafeRelease(m_pQuery[i]); }

        delete [] m_pQuery;
        m_pQuery = nullptr;
    };

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
//      クエリを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Query* QueryPool::GetD3D11Query(uint32_t index) const
{
    if(index >= m_Desc.Count)
    { return nullptr; }

    return m_pQuery[index];
}

//-------------------------------------------------------------------------------------------------
//      クエリプールを生成します.
//-------------------------------------------------------------------------------------------------
bool QueryPool::Create(IDevice* pDevice, const QueryPoolDesc* pDesc, IQueryPool** ppQueryPool)
{
    if (pDevice == nullptr || pDesc == nullptr || ppQueryPool == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new QueryPool;
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if (!instance->Init(pDevice, pDesc))
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppQueryPool = instance;
    return true;
}

} // namespace a3d
