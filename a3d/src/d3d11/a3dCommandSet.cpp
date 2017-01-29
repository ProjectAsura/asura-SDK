﻿//-------------------------------------------------------------------------------------------------
// File : a3dCommandSet.cpp
// Desc : CommandSet Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// CommandSet class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
CommandSet::CommandSet()
: m_RefCount            (1)
, m_pDevice             (nullptr)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
CommandSet::~CommandSet()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool CommandSet::Init(IDevice* pDevice, const CommandSetDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void CommandSet::Term()
{ SafeRelease(m_pDevice); }

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void CommandSet::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void CommandSet::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t CommandSet::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void CommandSet::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
CommandSetDesc CommandSet::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool CommandSet::Create(IDevice* pDevice, const CommandSetDesc* pDesc, ICommandSet** ppCommandSet)
{
    if (pDevice == nullptr || pDesc == nullptr || ppCommandSet == nullptr)
    { return false; }

    auto instance = new CommandSet;
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, pDesc))
    {
        SafeRelease(instance);
        return false;
    }

    *ppCommandSet = instance;
    return true;
}

} // namespace a3d
