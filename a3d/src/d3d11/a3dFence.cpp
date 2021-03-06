﻿//-------------------------------------------------------------------------------------------------
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
    { return false; }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pD3D11Device = m_pDevice->GetD3D11Device();
    A3D_ASSERT(pD3D11Device != nullptr);

    {
        D3D11_QUERY_DESC desc = {};
        desc.Query = D3D11_QUERY_EVENT;

        auto hr = pD3D11Device->CreateQuery(&desc, &m_pQuery);
        if ( FAILED(hr) )
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Fence::Term()
{
    SafeRelease(m_pQuery);
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

    return pDeviceContext->GetData(m_pQuery, nullptr, 0, 0) != S_FALSE;
}

//-------------------------------------------------------------------------------------------------
//      完了を待機します.
//-------------------------------------------------------------------------------------------------
bool Fence::Wait(uint32_t timeoutMsec)
{
    auto pDeviceContext = m_pDevice->GetD3D11DeviceContext();
    A3D_ASSERT(pDeviceContext != nullptr);

    auto time = std::chrono::system_clock::now();
    while(pDeviceContext->GetData(m_pQuery, nullptr, 0, 0) == S_FALSE)
    {
        auto cur = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(cur - time).count();
        if (elapsed >= timeoutMsec)
        { return false; }

        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      クエリを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Query* Fence::GetD3D11Query() const
{ return m_pQuery; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Fence::Create(IDevice* pDevice, IFence** ppFence)
{
    if (pDevice == nullptr || ppFence == nullptr)
    { return false; }

    auto instance = new Fence;
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice))
    {
        SafeRelease(instance);
        return false;
    }

    *ppFence = instance;
    return true;
}

} // namespace a3d
