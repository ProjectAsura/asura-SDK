﻿//-------------------------------------------------------------------------------------------------
// File : a3dDescriptorSet.cpp
// Desc : DescriptorSet Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorSet class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorSet::DescriptorSet()
: m_RefCount            (1)
, m_pDevice             (nullptr)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorSet::~DescriptorSet()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorSet::GetCount() const 
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool DescriptorSet::Init
(
    IDevice*                    pDevice,
    DescriptorSetLayoutDesc*    pDesc
)
{
    if (pDevice == nullptr || pDesc == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    m_pLayoutDesc = pDesc;
    m_pDescriptors = new void* [pDesc->EntryCount];
    if (m_pDescriptors == nullptr)
    { return false; }

    for(auto i=0u; i<pDesc->EntryCount; ++i)
    { m_pDescriptors[i] = nullptr; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::Term()
{
    if (m_pDescriptors != nullptr)
    {
        delete [] m_pDescriptors;
        m_pDescriptors = nullptr;
    }

    if (m_pLayoutDesc != nullptr)
    { m_pLayoutDesc = nullptr; }

    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      定数バッファを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetView(uint32_t index, IConstantBufferView* const pResource)
{
    A3D_ASSERT(index < m_pLayoutDesc->EntryCount);
    m_pDescriptors[index] = pResource;
}

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetView(uint32_t index, IShaderResourceView* const pResource)
{
    A3D_ASSERT(index < m_pLayoutDesc->EntryCount);
    m_pDescriptors[index] = pResource;
}

//-------------------------------------------------------------------------------------------------
//      アンオーダードアクセスを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetView(uint32_t index, IUnorderedAccessView* const pResource)
{
    A3D_ASSERT(index < m_pLayoutDesc->EntryCount);
    m_pDescriptors[index] = pResource;
}

//-------------------------------------------------------------------------------------------------
//      サンプラーを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetSampler(uint32_t index, ISampler* const pSampler)
{
    A3D_ASSERT(index < m_pLayoutDesc->EntryCount);
    m_pDescriptors[index] = pSampler;
}

//-------------------------------------------------------------------------------------------------
//      コマンドを生成します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::MakeCommand(ImCmdSetDescriptorSet* pCmd)
{
    pCmd->Id    = CMD_SET_DESCRIPTORSET;
    pCmd->pDesc = m_pLayoutDesc;
    for(auto i=0u; i<m_pLayoutDesc->EntryCount; ++i)
    { pCmd->pDescriptor[i] = m_pDescriptors[i]; }
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool DescriptorSet::Create
(
    IDevice*                    pDevice,
    DescriptorSetLayoutDesc*    pDesc,
    IDescriptorSet**            ppDescriptorSet
)
{
    if (pDevice         == nullptr 
    || pDesc            == nullptr 
    || ppDescriptorSet  == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new DescriptorSet;
    if ( instance == nullptr )
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if ( !instance->Init(pDevice, pDesc ) )
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppDescriptorSet = instance;
    return true;
}

} // namespace a3d
