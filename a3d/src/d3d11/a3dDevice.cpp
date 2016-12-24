//-------------------------------------------------------------------------------------------------
// File : a3dDevice.cpp
// Desc : Device Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Device class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Device::Device()
: m_RefCount(1)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Device::~Device()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool Device::Init(const DeviceDesc* pDesc, const void* pOption)
{
    A3D_UNUSED(pOption);

    if (pDesc == nullptr)
    { return false; }

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    uint32_t createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    if (pDesc->EnableDebug)
    { createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG; }

    // BGRAサポートを有効化.
    createDeviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    {
        uint32_t flags = 0;
        if (pDesc->EnableDebug)
        { flags |= DXGI_CREATE_FACTORY_DEBUG; }
 
        auto hr = CreateDXGIFactory2( flags, IID_PPV_ARGS(&m_pFactory) );
        if ( FAILED(hr) )
        { return false; }
    }

    // デフォルトアダプターを取得.
    IDXGIAdapter1* pAdapter = nullptr;
    {
        auto hr = m_pFactory->EnumAdapters1( 0, &pAdapter );
        if ( FAILED(hr) )
        {
            SafeRelease(pAdapter);
            return false;
        }
    }

    // IDXGIAdapter2に変換
    {
        auto hr = pAdapter->QueryInterface( IID_PPV_ARGS(&m_pAdapter));
        SafeRelease(pAdapter);
        if ( FAILED(hr) )
        { return false; }
    }

    // デフォルトディスプレイを取得.
    IDXGIOutput* pOutput;
    {
        auto hr = m_pAdapter->EnumOutputs(0, &pOutput);
        if ( FAILED(hr) )
        { return false; }
    }

    // IDXGOutput3に変換.
    {
        auto hr = pOutput->QueryInterface( IID_PPV_ARGS(&m_pOutput) );
        SafeRelease(pOutput);
        if ( FAILED(hr) )
        { return false; }
    }

    {
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

        auto hr = D3D11CreateDevice(
            nullptr, 
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            featureLevels,
            7,
            D3D11_SDK_VERSION,
            &m_pDevice,
            &m_FeatureLevel,
            &m_pDeviceContext );
        if ( FAILED(hr) )
        { return false; }
    }

    if (!Queue::Create(this, COMMANDLIST_TYPE_DIRECT, pDesc->MaxGraphicsQueueSubmitCount, &m_pGraphicsQueue))
    { return false; }

    if (!Queue::Create(this, COMMANDLIST_TYPE_COMPUTE, pDesc->MaxComputeQueueSubmitCount, &m_pComputeQueue))
    { return false; }

    if (!Queue::Create(this, COMMANDLIST_TYPE_COPY, pDesc->MaxCopyQueueSubmitCount, &m_pCopyQueue))
    { return false; }

    // デバイス情報の設定.
    {
        m_Info.ConstantBufferMemoryAlignment    = 16;
        m_Info.MaxTargetWidth                   = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
        m_Info.MaxTargetHeight                  = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
        m_Info.MaxTargetArraySize               = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
        m_Info.MaxColorSampleCount              = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;
        m_Info.MaxDepthSampleCount              = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;
        m_Info.MaxStencilSampleCount            = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Device::Term()
{
    if (m_pDeviceContext != nullptr)
    {
        m_pDeviceContext->Flush();
        m_pDeviceContext->ClearState();
    }

    SafeRelease(m_pGraphicsQueue);
    SafeRelease(m_pComputeQueue);
    SafeRelease(m_pCopyQueue);
    SafeRelease(m_pDeviceContext);
    SafeRelease(m_pDevice);
    SafeRelease(m_pFactory);
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void Device::AddRef() 
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void Device::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Device::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
DeviceDesc Device::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      デバイス情報を取得します.
//-------------------------------------------------------------------------------------------------
DeviceInfo Device::GetInfo() const
{ return m_Info; }

//-------------------------------------------------------------------------------------------------
//      グラフィックスキューを取得します.
//-------------------------------------------------------------------------------------------------
void Device::GetGraphicsQueue(IQueue** ppQueue)
{
    *ppQueue = m_pGraphicsQueue;
    if (m_pGraphicsQueue != nullptr)
    { m_pGraphicsQueue->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      コンピュートキューを取得します.
//-------------------------------------------------------------------------------------------------
void Device::GetComputeQueue(IQueue** ppQueue)
{
    *ppQueue = m_pComputeQueue;
    if (m_pComputeQueue != nullptr)
    { m_pComputeQueue->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      コピーキューを取得します.
//-------------------------------------------------------------------------------------------------
void Device::GetCopyQueue(IQueue** ppQueue)
{
    *ppQueue = m_pCopyQueue;
    if (m_pCopyQueue != nullptr)
    { m_pCopyQueue->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      コマンドリストを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateCommandList(COMMANDLIST_TYPE type, const void* pOption, ICommandList** ppCommandList)
{ return CommandList::Create(this, type, 4096, ppCommandList); }

//-------------------------------------------------------------------------------------------------
//      スワップチェインを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateSwapChain(const SwapChainDesc* pDesc, ISwapChain** ppSwapChain)
{ return SwapChain::Create(this, m_pGraphicsQueue, pDesc, ppSwapChain); }

//-------------------------------------------------------------------------------------------------
//      バッファを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateBuffer(const BufferDesc* pDesc, IBuffer** ppResource)
{ return Buffer::Create(this, pDesc, ppResource); }

//-------------------------------------------------------------------------------------------------
//      バッファビューを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateBufferView
(
    IBuffer*                pBuffer,
    const BufferViewDesc*   pDesc,
    IBufferView**           ppBufferView
)
{ return BufferView::Create(this, pBuffer, pDesc, ppBufferView); }

//-------------------------------------------------------------------------------------------------
//      テクスチャを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateTexture(const TextureDesc* pDesc, ITexture** ppResource)
{ return Texture::Create(this, pDesc, ppResource); }

//-------------------------------------------------------------------------------------------------
//      テクスチャビューを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateTextureView
(
    ITexture*               pTexture,
    const TextureViewDesc*  pDesc,
    ITextureView**          ppTextureView
)
{ return TextureView::Create(this, pTexture, pDesc, ppTextureView); }

//-------------------------------------------------------------------------------------------------
//      サンプラーを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateSampler(const SamplerDesc* pDesc, ISampler** ppSampler)
{ return Sampler::Create(this, pDesc, ppSampler); }

//-------------------------------------------------------------------------------------------------
//      グラフィックスパイプラインを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateGraphicsPipeline(const GraphicsPipelineStateDesc* pDesc, IPipelineState** ppPipelineState)
{ return PipelineState::CreateAsGraphics(this, pDesc, ppPipelineState); }

//-------------------------------------------------------------------------------------------------
//      コンピュートパイプラインを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateComputePipeline(const ComputePipelineStateDesc* pDesc, IPipelineState** ppPipelineState)
{ return PipelineState::CreateAsCompute(this, pDesc, ppPipelineState); }

//-------------------------------------------------------------------------------------------------
//      ディスクリプタセットレイアウトを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateDescriptorSetLayout
(
    const DescriptorSetLayoutDesc*  pDesc,
    IDescriptorSetLayout**          ppDescriptorSetLayout
)
{ return DescriptorSetLayout::Create(this, pDesc, ppDescriptorSetLayout); }

//-------------------------------------------------------------------------------------------------
//      フレームバッファを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateFrameBuffer(const FrameBufferDesc* pDesc, IFrameBuffer** ppFrameBuffer)
{ return FrameBuffer::Create(this, pDesc, ppFrameBuffer); }

//-------------------------------------------------------------------------------------------------
//      クエリプールを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateQueryPool(const QueryPoolDesc* pDesc, IQueryPool** ppQueryPool)
{ return QueryPool::Create(this, pDesc, ppQueryPool); }

//-------------------------------------------------------------------------------------------------
//      コマンドセットを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateCommandSet(const CommandSetDesc* pDesc, ICommandSet** ppCommandSet)
{ return CommandSet::Create(this, pDesc, ppCommandSet); }

//-------------------------------------------------------------------------------------------------
//      フェンスを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateFence(IFence** ppFence)
{ return Fence::Create(this, ppFence); }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Device* Device::GetD3D11Device() const
{ return m_pDevice; }

//-------------------------------------------------------------------------------------------------
//      デバイスコンテキストを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11DeviceContext* Device::GetD3D11DeviceContext() const
{ return m_pDeviceContext; }

//-------------------------------------------------------------------------------------------------
//      DXGIファクトリーを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIFactory3* Device::GetDXGIFactory() const
{ return m_pFactory; }

//-------------------------------------------------------------------------------------------------
//      デフォルトアダプターを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIAdapter2* Device::GetDXGIAdapter() const
{ return m_pAdapter; }

//-------------------------------------------------------------------------------------------------
//      デフォルトディスプレイを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIOutput3* Device::GetDXGIOutput() const
{ return m_pOutput; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Device::Create(const DeviceDesc* pDesc, const void* pOption, IDevice** ppDevice)
{
    if (pDesc == nullptr || ppDevice == nullptr)
    { return false; }

    auto instance = new Device;
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDesc, pOption))
    {
        SafeRelease(instance);
        return false;
    }

    *ppDevice = instance;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      デバイスを生成します.
//-------------------------------------------------------------------------------------------------
bool A3D_APIENTRY CreateDevice(const DeviceDesc* pDesc, const void* pOption, IDevice** ppDevice)
{
    if (pDesc == nullptr || ppDevice == nullptr)
    { return false; }

    return Device::Create(pDesc, pOption, ppDevice);
}

} // namespace a3d
