//-------------------------------------------------------------------------------------------------
// File : a3dDescriptorSetLayout.cpp
// Desc : Descriptor Set Layout Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorSetLayout class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorSetLayout::DescriptorSetLayout()
: m_RefCount            (1)
, m_pDevice             (nullptr)
{ memset( &m_Desc, 0, sizeof(m_Desc) ); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorSetLayout::~DescriptorSetLayout()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool DescriptorSetLayout::Init(IDevice* pDevice, const DescriptorSetLayoutDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSetLayout::Term()
{
    SafeRelease(m_pDevice);

    memset( &m_Desc, 0, sizeof(m_Desc) );
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void DescriptorSetLayout::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSetLayout::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorSetLayout::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void DescriptorSetLayout::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタセットを割り当てます.
//-------------------------------------------------------------------------------------------------
bool DescriptorSetLayout::CreateDescriptorSet(IDescriptorSet** ppDesctiproSet)
{
    if (ppDesctiproSet == nullptr)
    { return false; }

    if (!DescriptorSet::Create(m_pDevice, &m_Desc, ppDesctiproSet))
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool DescriptorSetLayout::Create
(
    IDevice*                        pDevice,
    const DescriptorSetLayoutDesc*  pDesc,
    IDescriptorSetLayout**          ppLayout
)
{
    if (pDevice == nullptr || pDesc == nullptr || ppLayout == nullptr)
    { return false; }

    auto instance = new DescriptorSetLayout;
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, pDesc))
    {
        instance->Release();
        return false;
    }

    *ppLayout = instance;
    return true;
}

} // namespace a3d
