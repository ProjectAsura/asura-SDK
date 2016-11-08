//-------------------------------------------------------------------------------------------------
// File : a3dFrameBuffer.cpp
// Desc : Frame Buffer Implmentation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeRenderTargetView( a3d::ITexture* pResource, D3D12_RENDER_TARGET_VIEW_DESC& result )
{
    auto desc = pResource->GetDesc();

    result.Format = a3d::ToNativeFormat( desc.Format );

    switch(desc.Dimension)
    {
    case a3d::RESOURCE_DIMENSION_BUFFER:
        {
            result.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
            result.Buffer.FirstElement = 0;
            result.Buffer.NumElements  = desc.Width;
        }
        break;

    case a3d::RESOURCE_DIMENSION_TEXTURE1D:
        {
            if (desc.DepthOrArraySize > 1)
            {
                result.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                result.Texture1DArray.ArraySize         = desc.DepthOrArraySize;
                result.Texture1DArray.FirstArraySlice   = 0;
                result.Texture1DArray.MipSlice          = 0;
            }
            else
            {
                result.Texture1D.MipSlice = 0;
            }
        }
        break;

    case a3d::RESOURCE_DIMENSION_TEXTURE2D:
    case a3d::RESOURCE_DIMENSION_CUBEMAP:
        {
            if (desc.DepthOrArraySize > 1)
            {
                if (desc.SampleCount > 1)
                {
                    result.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    result.Texture2DMSArray.ArraySize       = desc.DepthOrArraySize;
                    result.Texture2DMSArray.FirstArraySlice = 0;
                }
                else
                {
                    result.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    result.Texture2DArray.ArraySize         = desc.DepthOrArraySize;
                    result.Texture2DArray.FirstArraySlice   = 0;
                    result.Texture2DArray.MipSlice          = 0;
                    result.Texture2DArray.PlaneSlice        = 0;
                }
            }
            else
            {
                if (desc.SampleCount > 1)
                {
                    result.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    result.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                    result.Texture2D.MipSlice   = 0;
                    result.Texture2D.PlaneSlice = 0;
                }
            }
        }
        break;

    case a3d::RESOURCE_DIMENSION_TEXTURE3D:
        {
            result.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
            result.Texture3D.FirstWSlice = 0;
            result.Texture3D.MipSlice    = 0;
            result.Texture3D.WSize       = desc.DepthOrArraySize;
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeDepthStencilView( a3d::ITexture* pResource, D3D12_DEPTH_STENCIL_VIEW_DESC& result )
{
    auto desc = pResource->GetDesc();

    result.Format = a3d::ToNativeFormat( desc.Format );
    result.Flags  = D3D12_DSV_FLAG_NONE;

    switch( desc.Dimension )
    {
    case a3d::RESOURCE_DIMENSION_BUFFER:
        { A3D_ASSERT(false); }
        break;

    case a3d::RESOURCE_DIMENSION_TEXTURE1D:
        {
            if ( desc.DepthOrArraySize > 1 )
            {
                result.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                result.Texture1DArray.ArraySize         = desc.DepthOrArraySize;
                result.Texture1DArray.FirstArraySlice   = 0;
                result.Texture1DArray.MipSlice          = 0;
            }
            else
            {
                result.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                result.Texture1D.MipSlice = 0;
            }
        }
        break;

    case a3d::RESOURCE_DIMENSION_TEXTURE2D:
    case a3d::RESOURCE_DIMENSION_CUBEMAP:
        {
            if ( desc.DepthOrArraySize > 1 )
            {
                if ( desc.SampleCount > 1 )
                {
                    result.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
                    result.Texture2DMSArray.ArraySize       = desc.DepthOrArraySize;
                    result.Texture2DMSArray.FirstArraySlice = 0;
                }
                else
                {
                    result.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                    result.Texture2DArray.ArraySize         = desc.DepthOrArraySize;
                    result.Texture2DArray.FirstArraySlice   = 0;
                    result.Texture2DArray.MipSlice          = 0;
                }
            }
            else
            {
                if ( desc.SampleCount > 1 )
                {
                    result.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    result.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                    result.Texture2D.MipSlice = 0;
                }
            }
        }
        break;

    case a3d::RESOURCE_DIMENSION_TEXTURE3D:
        { A3D_ASSERT(false); }
        break;
    }
}

} // namespace /* anonymous */


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
, m_pDSV                (nullptr)
, m_DSVFormat           (DXGI_FORMAT_UNKNOWN)
{
    for(auto i=0; i<8; ++i)
    {
        m_pRTV[i]       = nullptr;
        m_RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
        m_RTVHandles[i].ptr = 0;
    }

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
        m_pRTV[i] = pWrapDevice->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)->CreateDescriptor();

        D3D12_RENDER_TARGET_VIEW_DESC view = {};
        ToNativeRenderTargetView( pDesc->pColorTargets[i], view );

        auto pWrapResource = reinterpret_cast<Texture*>(pDesc->pColorTargets[i]);
        A3D_ASSERT(pWrapResource != nullptr);

        auto pNativeResource = pWrapResource->GetD3D12Resource();
        A3D_ASSERT(pNativeResource != nullptr);

        m_RTVFormats[i] = ToNativeFormat( pWrapResource->GetDesc().Format );
        m_RTVHandles[i] = m_pRTV[i]->GetHandleCPU();

        pNativeDevice->CreateRenderTargetView( pNativeResource, &view, m_RTVHandles[i] );
    }

    if (pDesc->pDepthTarget != nullptr)
    {
        m_pDSV = pWrapDevice->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)->CreateDescriptor();

        D3D12_DEPTH_STENCIL_VIEW_DESC view = {};
        ToNativeDepthStencilView( pDesc->pDepthTarget, view );

        auto pWrapResource = reinterpret_cast<Texture*>(pDesc->pDepthTarget);
        A3D_ASSERT(pWrapResource != nullptr);

        auto pNativeResource = pWrapResource->GetD3D12Resource();
        A3D_ASSERT(pNativeResource != nullptr);

        m_DSVFormat = ToNativeFormat( pWrapResource->GetDesc().Format );

        pNativeDevice->CreateDepthStencilView( pNativeResource, &view, m_pDSV->GetHandleCPU() );
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
        SafeRelease(m_pRTV[i]);
        m_RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
        m_RTVHandles[i].ptr = 0;
    }

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
            HasDepthTarget() ? &m_pDSV->GetHandleCPU() : nullptr );
    }
    else
    {
        pNativeCommandList->OMSetRenderTargets(
            0,
            nullptr,
            FALSE,
            HasDepthTarget() ? &m_pDSV->GetHandleCPU() : nullptr );
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
               m_pDSV->GetHandleCPU(),
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
{ return m_pDSV != nullptr; }

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

    auto instance = new (std::nothrow) FrameBuffer;
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
