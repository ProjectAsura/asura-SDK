//-------------------------------------------------------------------------------------------------
// File : a3dConstantBufferView.cpp
// Desc : ConstantBufferView Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ConstantBufferView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
ConstantBufferView::ConstantBufferView()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pBuffer     (nullptr)
{ memset( &m_Desc, 0, sizeof(m_Desc) ); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
ConstantBufferView::~ConstantBufferView()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool ConstantBufferView::Init(IDevice* pDevice, IBuffer* pBuffer, const ConstantBufferViewDesc* pDesc)
{
    if (pDevice == nullptr || pBuffer == nullptr || pDesc == nullptr)
    {
        A3D_LOG("Error : Invlaid Argument.");
        return false;
    }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    auto pWrapBuffer = static_cast<Buffer*>(pBuffer);
    A3D_ASSERT(pWrapBuffer != nullptr);

    m_pBuffer = pWrapBuffer;
    m_pBuffer->AddRef();

    return true;
}

//----------------- --------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void ConstantBufferView::Term()
{
    SafeRelease(m_pBuffer);
    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void ConstantBufferView::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void ConstantBufferView::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t ConstantBufferView::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void ConstantBufferView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
ConstantBufferViewDesc ConstantBufferView::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      バッファを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Buffer* ConstantBufferView::GetD3D11Buffer() const
{
    if (m_pBuffer == nullptr)
    { return nullptr; }

    return m_pBuffer->GetD3D11Buffer();
}

//-------------------------------------------------------------------------------------------------
//      バッファを取得します.
//-------------------------------------------------------------------------------------------------
IBuffer* ConstantBufferView::GetResource() const
{ return m_pBuffer; }

//-------------------------------------------------------------------------------------------------
//      サブリソースを更新します.
//-------------------------------------------------------------------------------------------------
void ConstantBufferView::UpdateSubsource(ID3D11DeviceContext* pDeviceContext)
{
    auto ptr = reinterpret_cast<uint8_t*>(m_pBuffer->GetSubresourcePointer());
    if (ptr == nullptr)
    { return; }

    ptr += m_Desc.Offset;
    auto pD3D11Buffer = m_pBuffer->GetD3D11Buffer();
    pDeviceContext->UpdateSubresource(pD3D11Buffer, 0, nullptr, ptr, uint32_t(m_Desc.Range), 1);
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool ConstantBufferView::Create
(
    IDevice*                        pDevice,
    IBuffer*                        pBuffer,
    const ConstantBufferViewDesc*   pDesc,
    IConstantBufferView**           ppBufferView
)
{
    if (pDevice == nullptr || pBuffer == nullptr || pDesc == nullptr || ppBufferView == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new ConstantBufferView();
    if ( instance == nullptr )
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if ( !instance->Init(pDevice, pBuffer, pDesc) )
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppBufferView = instance;
    return true;
}

} // namespace a3d
