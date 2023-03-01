//-------------------------------------------------------------------------------------------------
// File : a3dFence.cpp
// Desc : Fence Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Fence class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Fence::Fence()
: m_RefCount        (1)
, m_pDevice         (nullptr)
, m_pFence          (nullptr)
, m_CurrentValue    (0)
, m_PreviousValue   (0)
, m_Event           (nullptr)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Fence::~Fence()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool Fence::Init(IDevice* pDevice)
{
    if (pDevice == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT( pNativeDevice != nullptr );

    m_PreviousValue = 0;
    m_CurrentValue  = 0;

    auto hr = pNativeDevice->CreateFence( m_CurrentValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence) );
    if ( FAILED(hr) )
    {
        A3D_LOG("Error : ID3D12Device::CreateFence() Failed. errcode = 0x%x", hr);
        return false;
    }

    m_Event = CreateEventEx( nullptr, FALSE, FALSE, EVENT_ALL_ACCESS );
    if ( m_Event == nullptr )
    {
        A3D_LOG("Error : CreateEventExt() Failed.");
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Fence::Term()
{
    SafeRelease(m_pFence);

    if (m_Event != nullptr)
    {
        CloseHandle(m_Event);
        m_Event = nullptr;
    }

    SafeRelease(m_pDevice);

    m_CurrentValue  = 0;
    m_PreviousValue = 0;
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void Fence::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void Fence::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Fence::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void Fence::SetName(const char* name)
{
    m_Name = name;
    m_pFence->SetPrivateData(WKPDID_D3DDebugObjectName, m_Name.size(), m_Name.c_str());
}

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* Fence::GetName() const
{ return m_Name.c_str(); }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void Fence::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      フェンスを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Fence* Fence::GetD3D12Fence() const
{ return m_pFence; }

//-------------------------------------------------------------------------------------------------
//      フェンスカウンターを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Fence::GetFenceValue() const
{ return m_CurrentValue; }

//-------------------------------------------------------------------------------------------------
//      イベントを取得します.
//-------------------------------------------------------------------------------------------------
HANDLE Fence::GetEvent() const
{ return m_Event; }

//-------------------------------------------------------------------------------------------------
//      フェンスカウンターを進めます.
//-------------------------------------------------------------------------------------------------
void Fence::AdvanceValue()
{
    m_PreviousValue = m_CurrentValue;
    m_CurrentValue++;
}

//-------------------------------------------------------------------------------------------------
//      シグナル状態かどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool IFence::IsSignaled() const
{
    auto pThis = static_cast<const Fence*>(this);
    A3D_ASSERT(pThis != nullptr);

    return pThis->m_pFence->GetCompletedValue() >= pThis->m_PreviousValue;
}

//-------------------------------------------------------------------------------------------------
//      完了を待機します.
//-------------------------------------------------------------------------------------------------
bool IFence::Wait(uint32_t timeoutMsec)
{
    auto pThis = static_cast<Fence*>(this);
    A3D_ASSERT(pThis != nullptr);

    if ( pThis->m_pFence->GetCompletedValue() < pThis->m_PreviousValue )
    {
        auto hr = pThis->m_pFence->SetEventOnCompletion( pThis->m_PreviousValue, pThis->m_Event );
        if (FAILED(hr))
        { return false; }

        if (WAIT_OBJECT_0 != WaitForSingleObjectEx( pThis->m_Event, timeoutMsec, FALSE ))
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Fence::Create(IDevice* pDevice, IFence** ppFence)
{
    if (pDevice == nullptr || ppFence == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new Fence;
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if (!instance->Init(pDevice))
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppFence = instance;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      ネイティブフェンスを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Fence* A3D_APIENTRY GetD3D12Fence(IFence* pFence)
{
    auto pWrapFence = static_cast<Fence*>(pFence);
    if (pWrapFence == nullptr)
    { return nullptr; }

    return pWrapFence->GetD3D12Fence();
}

} // namespace a3d
