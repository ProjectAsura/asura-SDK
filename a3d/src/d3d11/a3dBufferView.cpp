//-------------------------------------------------------------------------------------------------
// File : a3dBufferView.cpp
// Desc : BufferView Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// BufferView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
BufferView::BufferView()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pBuffer     (nullptr)
{ memset( &m_Desc, 0, sizeof(m_Desc) ); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
BufferView::~BufferView()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool BufferView::Init(IDevice* pDevice, IBuffer* pBuffer, const BufferViewDesc* pDesc)
{
    if (pDevice == nullptr || pBuffer == nullptr || pDesc == nullptr)
    { return false; }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    auto pWrapBuffer = static_cast<Buffer*>(pBuffer);
    A3D_ASSERT(pWrapBuffer != nullptr);

    m_pBuffer = pWrapBuffer;
    m_pBuffer->AddRef();

    auto pD3D11Buffer = pWrapBuffer->GetD3D11Buffer();
    A3D_ASSERT(pD3D11Buffer != nullptr);

    auto pD3D11Device = m_pDevice->GetD3D11Device();
    A3D_ASSERT(pD3D11Device != nullptr);

    auto& desc = m_pBuffer->GetDesc();
    auto usage  = desc.Usage;
    auto stride = static_cast<uint32_t>((desc.Stride == 0) ? 1 : desc.Stride);

    if (usage & RESOURCE_USAGE_SHADER_RESOURCE)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension        = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.ElementOffset = static_cast<uint32_t>(pDesc->Offset);
        srvDesc.Buffer.ElementWidth  = static_cast<uint32_t>(desc.Size);
        srvDesc.Buffer.FirstElement  = 0;
        srvDesc.Buffer.NumElements   = static_cast<uint32_t>(desc.Size / stride);

        auto hr = pD3D11Device->CreateShaderResourceView(pD3D11Buffer, &srvDesc, &m_pSRV);
        if ( FAILED(hr) )
        { return false; }
    }
    if (usage & RESOURCE_USAGE_UNORDERD_ACCESS)
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements  = static_cast<uint32_t>(desc.Size / stride);

        auto hr = pD3D11Device->CreateUnorderedAccessView(pD3D11Buffer, &uavDesc, &m_pUAV);
        if ( FAILED(hr) )
        { return false; }
    }

    return true;
}

//----------------- --------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void BufferView::Term()
{
    SafeRelease(m_pSRV);
    SafeRelease(m_pUAV);
    SafeRelease(m_pBuffer);
    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void BufferView::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void BufferView::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t BufferView::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void BufferView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
BufferViewDesc BufferView::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      バッファを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Buffer* BufferView::GetD3D11Buffer() const
{
    if (m_pBuffer == nullptr)
    { return nullptr; }

    return m_pBuffer->GetD3D11Buffer();
}

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11ShaderResourceView* BufferView::GetD3D11ShaderResourceView() const
{ return m_pSRV; }

//-------------------------------------------------------------------------------------------------
//      アンオーダードアクセスビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11UnorderedAccessView* BufferView::GetD3D11UnorderedAccessView() const
{ return m_pUAV; }

//-------------------------------------------------------------------------------------------------
//      バッファを取得します.
//-------------------------------------------------------------------------------------------------
IBuffer* BufferView::GetResource() const
{ return m_pBuffer; }

//-------------------------------------------------------------------------------------------------
//      サブリソースを更新します.
//-------------------------------------------------------------------------------------------------
void BufferView::UpdateSubsource(ID3D11DeviceContext* pDeviceContext)
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
bool BufferView::Create
(
    IDevice*                pDevice,
    IBuffer*                pBuffer,
    const BufferViewDesc*   pDesc,
    IBufferView**           ppBufferView
)
{
    if (pDevice == nullptr || pBuffer == nullptr || pDesc == nullptr || ppBufferView == nullptr)
    { return false; }

    auto instance = new BufferView;
    if ( instance == nullptr )
    { return false; }

    if ( !instance->Init(pDevice, pBuffer, pDesc) )
    {
        SafeRelease(instance);
        return false;
    }

    *ppBufferView = instance;
    return true;
}

} // namespace a3d
