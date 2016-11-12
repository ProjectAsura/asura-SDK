//-------------------------------------------------------------------------------------------------
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
, m_pCommandSignature   (nullptr)
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

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    auto pWarpDevice = reinterpret_cast<Device*>(pDevice);
    A3D_ASSERT(pWarpDevice != nullptr);

    auto pNativeDevice = pWarpDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    {
        auto pArguments = new D3D12_INDIRECT_ARGUMENT_DESC[pDesc->ArgumentCount];
        if (pArguments == nullptr)
        { return false; }

        D3D12_INDIRECT_ARGUMENT_TYPE table[] = {
            D3D12_INDIRECT_ARGUMENT_TYPE_DRAW,
            D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED,
            D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH
        };

        for(auto i=0u; i<pDesc->ArgumentCount; ++i)
        {
            memset(&pArguments[i], 0, sizeof(D3D12_INDIRECT_ARGUMENT_DESC));
            pArguments[i].Type = table[pDesc->pArguments[i]];
        }

        D3D12_COMMAND_SIGNATURE_DESC desc = {};
        desc.ByteStride         = pDesc->ByteStride;
        desc.NumArgumentDescs   = pDesc->ArgumentCount;
        desc.pArgumentDescs     = pArguments;

        auto hr = pNativeDevice->CreateCommandSignature(&desc, nullptr, IID_PPV_ARGS(&m_pCommandSignature));

        delete[] pArguments;

        if ( FAILED(hr) )
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void CommandSet::Term()
{
    SafeRelease(m_pCommandSignature);
    SafeRelease(m_pDevice);
}

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
//      コマンドシグニチャを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12CommandSignature* CommandSet::GetD3D12CommandSignature() const
{ return m_pCommandSignature; }

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
