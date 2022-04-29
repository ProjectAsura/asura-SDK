//-------------------------------------------------------------------------------------------------
// File : a3dFence.cpp
// Desc : Fence Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <chrono>
#include <thread>


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

    auto pD3D11Device = m_pDevice->GetD3D11Device();
    A3D_ASSERT(pD3D11Device != nullptr);

#ifdef A3D_FOR_WINDOWS10
    m_PreviousValue = 0;
    m_CurrentValue  = 0;

    auto hr = pD3D11Device->CreateFence(m_CurrentValue, D3D11_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
    if (FAILED(hr))
    {
        A3D_LOG("Error : ID3D11Device::CreateFence() Failed. errcode = 0x%x", hr);
        return false;
    }

    m_Event = CreateEventEx( nullptr, FALSE, FALSE, EVENT_ALL_ACCESS );
    if ( m_Event == nullptr )
    {
        A3D_LOG("Error : CreateEventExt() Failed.");
        return false;
    }
#else
    {
        D3D11_QUERY_DESC desc = {};
        desc.Query = D3D11_QUERY_EVENT;

        auto hr = pD3D11Device->CreateQuery(&desc, &m_pQuery);
        if ( FAILED(hr) )
        {
            A3D_LOG("Error : ID3D11Device::CreateQuery() Failed. errcode = 0x%x", hr);
            return false;
        }
    }
#endif

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Fence::Term()
{
#ifdef A3D_FOR_WINDOWS10
    SafeRelease(m_pFence);
    if (m_Event != nullptr)
    {
        CloseHandle(m_Event);
        m_Event = nullptr;
    }

    m_CurrentValue  = 0;
    m_PreviousValue = 0;
#else
    SafeRelease(m_pQuery);
#endif
    SafeRelease(m_pDevice);
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
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void Fence::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      シグナル状態かどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool Fence::IsSignaled() const
{
    auto pDeviceContext = m_pDevice->GetD3D11DeviceContext();
    A3D_ASSERT(pDeviceContext != nullptr);

#ifdef A3D_FOR_WINDOWS10
    return m_pFence->GetCompletedValue() >= m_PreviousValue;
#else
    return pDeviceContext->GetData(m_pQuery, nullptr, 0, 0) != S_FALSE;
#endif
}

//-------------------------------------------------------------------------------------------------
//      完了を待機します.
//-------------------------------------------------------------------------------------------------
bool Fence::Wait(uint32_t timeoutMsec)
{
    auto pDeviceContext = m_pDevice->GetD3D11DeviceContext();
    A3D_ASSERT(pDeviceContext != nullptr);

#ifdef A3D_FOR_WINDOWS10
    if ( m_pFence->GetCompletedValue() < m_PreviousValue )
    {
        auto hr = m_pFence->SetEventOnCompletion( m_PreviousValue, m_Event );
        if (FAILED(hr))
        { return false; }

        if (WAIT_OBJECT_0 != WaitForSingleObjectEx( m_Event, timeoutMsec, FALSE ))
        { return false; }
    }
#else
    auto time = std::chrono::system_clock::now();
    while(pDeviceContext->GetData(m_pQuery, nullptr, 0, 0) == S_FALSE)
    {
        auto cur = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(cur - time).count();
        if (elapsed >= timeoutMsec)
        { return false; }

        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
#endif

    return true;
}

#ifdef A3D_FOR_WINDOWS10
//-------------------------------------------------------------------------------------------------
//      フェンスを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Fence* Fence::GetD3D11Fence() const
{ return m_pFence; }

//-------------------------------------------------------------------------------------------------
//      イベントを取得します.
//-------------------------------------------------------------------------------------------------
HANDLE Fence::GetEvent() const 
{ return m_Event; }

//-------------------------------------------------------------------------------------------------
//      フェンス値を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Fence::GetFenceValue() const
{ return m_CurrentValue; }

//-------------------------------------------------------------------------------------------------
//      フェンスカウンターを薦めます.
//-------------------------------------------------------------------------------------------------
void Fence::AdvanceCount()
{
    m_PreviousValue = m_CurrentValue;
    m_CurrentValue++;
}
#else
//-------------------------------------------------------------------------------------------------
//      クエリを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Query* Fence::GetD3D11Query() const
{ return m_pQuery; }
#endif

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

} // namespace a3d
