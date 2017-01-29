//-------------------------------------------------------------------------------------------------
// File : a3dFrameBuffer.cpp
// Desc : Frame Buffer Implmentation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameBuffer class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
FrameBuffer::FrameBuffer()
: m_RefCount(1)
, m_pDevice (nullptr)
, m_pDSV    (nullptr)
{
    for(auto i=0; i<8; ++i)
    { m_pRTVs[i] = nullptr; }

    memset(&m_Desc, 0, sizeof(m_Desc));
}

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
FrameBuffer::~FrameBuffer()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool FrameBuffer::Init(IDevice* pDevice, const FrameBufferDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    for(auto i=0u; i<pDesc->ColorCount; ++i)
    {
        auto pWrapTextureView = static_cast<TextureView*>(pDesc->pColorTargets[i]);
        A3D_ASSERT(pWrapTextureView != nullptr);

        m_pRTVs[i] = pWrapTextureView->GetD3D11RenderTargetView();
        A3D_ASSERT(m_pRTVs[i] != nullptr);

        m_pRTVs[i]->AddRef();
    }

    if (pDesc->pDepthTarget != nullptr)
    {
        auto pWrapTextureView = static_cast<TextureView*>(pDesc->pDepthTarget);
        A3D_ASSERT(pWrapTextureView != nullptr);

        m_pDSV = pWrapTextureView->GetD3D11DepthStencilView();
        A3D_ASSERT(m_pDSV != nullptr);

        m_pDSV->AddRef();
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void FrameBuffer::Term()
{
    for(auto i=0; i<8; ++i)
    { SafeRelease(m_pRTVs[i]); }

    SafeRelease(m_pDSV);
    SafeRelease(m_pDevice);

    memset(&m_Desc, 0, sizeof(m_Desc));
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void FrameBuffer::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void FrameBuffer::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t FrameBuffer::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void FrameBuffer::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
FrameBufferDesc FrameBuffer::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      カラーターゲット数を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t FrameBuffer::GetColorCount() const
{ return m_Desc.ColorCount; }

//-------------------------------------------------------------------------------------------------
//      カラーターゲットを持つかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool FrameBuffer::HasColorTarget() const
{ return m_Desc.ColorCount > 0; }

//-------------------------------------------------------------------------------------------------
//      深度ターゲットをもつかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool FrameBuffer::HasDepthTarget() const
{ return m_pDSV != nullptr; }

//-------------------------------------------------------------------------------------------------
//      フレームバッファを関連付けます.
//-------------------------------------------------------------------------------------------------
void FrameBuffer::Bind(ID3D11DeviceContext* pDeviceContext)
{ pDeviceContext->OMSetRenderTargets(m_Desc.ColorCount, m_pRTVs, m_pDSV); }

//-------------------------------------------------------------------------------------------------
//      フレームバッファをクリアします.
//-------------------------------------------------------------------------------------------------
void FrameBuffer::Clear
(
    ID3D11DeviceContext*            pDeviceContext,
    uint32_t                        clearColorCount,
    const ClearColorValue*          pClearColor,
    const ClearDepthStencilValue*   pClearDepthStencil)
{
    if (pClearColor != nullptr)
    {
        for(auto i=0u; i<clearColorCount; ++i)
        { pDeviceContext->ClearRenderTargetView(m_pRTVs[i], pClearColor[i].Float); }
    }

    if (pClearDepthStencil != nullptr)
    {
        uint32_t flags = 0;
        if (pClearDepthStencil->EnableClearDepth)
        { flags |= D3D11_CLEAR_DEPTH; }
        if (pClearDepthStencil->EnableClearStencil)
        { flags |= D3D11_CLEAR_STENCIL; }

        pDeviceContext->ClearDepthStencilView(
            m_pDSV,
            flags,
            pClearDepthStencil->Depth,
            pClearDepthStencil->Stencil);
    }
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool FrameBuffer::Create
(
    IDevice*                pDevice,
    const FrameBufferDesc*  pDesc,
    IFrameBuffer**          ppFrameBuffer
)
{
    if (pDevice == nullptr || pDesc == nullptr || ppFrameBuffer == nullptr)
    { return false; }

    auto instance = new FrameBuffer;
    if ( instance == nullptr )
    { return false; }

    if ( !instance->Init(pDevice, pDesc) )
    {
        SafeRelease(instance);
        return false;
    }

    *ppFrameBuffer = instance;
    return true;
}

} // namespace a3d
