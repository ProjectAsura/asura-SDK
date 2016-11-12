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
: m_RefCount            (1)
, m_pDevice             (nullptr)
, m_DSVFormat           (DXGI_FORMAT_UNKNOWN)
, m_HasDepth            (false)
{
    for(auto i=0; i<8; ++i)
    {
        m_RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
        m_RTVHandles[i].ptr = 0;
    }

    m_DSVHandle.ptr = 0;

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

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    auto pWrapDevice = reinterpret_cast<Device*>(pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    for(auto i=0u; i<pDesc->ColorCount; ++i)
    {
        auto pWrapResource = reinterpret_cast<TextureView*>(pDesc->pColorTargets[i]);
        A3D_ASSERT(pWrapResource != nullptr);

        m_RTVFormats[i] = ToNativeFormat( pWrapResource->GetDesc().Format );
        m_RTVHandles[i] = pWrapResource->GetTargetDescriptor()->GetHandleCPU();
    }

    if (pDesc->pDepthTarget != nullptr)
    {
        auto pWrapResource = reinterpret_cast<TextureView*>(pDesc->pDepthTarget);
        A3D_ASSERT(pWrapResource != nullptr);

        m_DSVFormat = ToNativeFormat( pWrapResource->GetDesc().Format );
        m_DSVHandle  = pWrapResource->GetTargetDescriptor()->GetHandleCPU();

        m_HasDepth = true;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void FrameBuffer::Term()
{
    for(auto i=0; i<8; ++i)
    { 
        m_RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
        m_RTVHandles[i].ptr = 0;
    }

    m_DSVFormat     = DXGI_FORMAT_UNKNOWN;
    m_DSVHandle.ptr = 0;
    m_HasDepth      = false;

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
//      フレームバッファを設定するコマンドを発行します.
//-------------------------------------------------------------------------------------------------
void FrameBuffer::Bind(ICommandList* pCommandList)
{
    A3D_ASSERT(pCommandList != nullptr);

    auto pWrapCommandList = reinterpret_cast<CommandList*>(pCommandList);
    A3D_ASSERT(pWrapCommandList != nullptr);

    auto pNativeCommandList = pWrapCommandList->GetD3D12GraphicsCommandList();
    A3D_ASSERT(pNativeCommandList != nullptr);

    if ( HasColorTarget() )
    {
        pNativeCommandList->OMSetRenderTargets(
            m_Desc.ColorCount,
            m_RTVHandles,
            FALSE,
            m_HasDepth ? &m_DSVHandle : nullptr );
    }
    else if ( m_HasDepth )
    {
        pNativeCommandList->OMSetRenderTargets(
            0,
            nullptr,
            FALSE,
            m_HasDepth ? &m_DSVHandle : nullptr );
    }
}

//-------------------------------------------------------------------------------------------------
//      フレームバッファをクリアするコマンドを発行します.
//-------------------------------------------------------------------------------------------------
void FrameBuffer::Clear
(
    ICommandList*                   pCommandList,
    uint32_t                        clearColorCount,
    const ClearColorValue*          pClearColors, 
    const ClearDepthStencilValue*   pClearDepthStencil
)
{
    auto pWrapCommandList = reinterpret_cast<CommandList*>(pCommandList);
    A3D_ASSERT(pWrapCommandList != nullptr);

    auto pNativeCommandList = pWrapCommandList->GetD3D12GraphicsCommandList();
    A3D_ASSERT(pNativeCommandList != nullptr);

    if (HasColorTarget())
    {
        for(auto i=0u; i<clearColorCount; ++i)
        {
            pNativeCommandList->ClearRenderTargetView(
                m_RTVHandles[i],
                pClearColors[i].Float,
                0,
                nullptr);
        }
    }

    if (pClearDepthStencil != nullptr && HasDepthTarget())
    {
        auto flags = D3D12_CLEAR_FLAGS(0);
        if (pClearDepthStencil->EnableClearDepth)
        { flags |= D3D12_CLEAR_FLAG_DEPTH; }
        if (pClearDepthStencil->EnableClearStencil)
        { flags |= D3D12_CLEAR_FLAG_STENCIL; }

        if (flags != 0)
        {
            pNativeCommandList->ClearDepthStencilView(
                m_DSVHandle,
                flags,
                pClearDepthStencil->Depth,
                pClearDepthStencil->Stencil,
                0,
                nullptr);
        }
    }
}

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
{ return m_HasDepth; }

//-------------------------------------------------------------------------------------------------
//      レンダーターゲットビューのフォーマットの配列を取得します.
//-------------------------------------------------------------------------------------------------
const DXGI_FORMAT* FrameBuffer::GetD3D12RenderTargetViewFormats() const
{ return m_RTVFormats; }

//-------------------------------------------------------------------------------------------------
//      深度ステンシルビューのフォーマットを取得します.
//-------------------------------------------------------------------------------------------------
DXGI_FORMAT FrameBuffer::GetD3D12DepthStencilViewFormat() const
{ return m_DSVFormat; }

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
