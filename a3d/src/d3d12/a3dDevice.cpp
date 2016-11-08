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
, m_pFactory(nullptr)
, m_pDevice (nullptr)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Device::~Device()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool Device::Init(const DeviceDesc* pDesc)
{
    if (pDesc == nullptr)
    { return false; }

    // デバッグレイヤーを有効化.
    if (pDesc->EnableDebug)
    {
        ID3D12Debug* pDebug;
        auto hr = D3D12GetDebugInterface( IID_PPV_ARGS(&pDebug) );
        if ( SUCCEEDED(hr) )
        { pDebug->EnableDebugLayer(); }

        SafeRelease(pDebug);
    }

    auto hr = CreateDXGIFactory1( IID_PPV_ARGS(&m_pFactory) );
    if ( FAILED(hr) )
    { return false; }

    hr = D3D12CreateDevice( nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice) );
    if ( FAILED(hr) )
    { return false; }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = pDesc->MaxShaderResourceCount;
        desc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice, &desc ) )
        { return false; }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = pDesc->MaxSamplerCount;
        desc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice, &desc ) )
        { return false; }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = pDesc->MaxColorTargetCount;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice, &desc ) )
        { return false; }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = pDesc->MaxDepthTargetCount;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice, &desc ) )
        { return false; }
    }

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    if (!Queue::Create(this, COMMANDLIST_TYPE_DIRECT, pDesc->MaxGraphicsQueueSubmitCount, &m_pGraphicsQueue))
    { return false; }

    if (!Queue::Create(this, COMMANDLIST_TYPE_COMPUTE, pDesc->MaxComputeQueueSubmitCount, &m_pComputeQueue))
    { return false; }

    if (!Queue::Create(this, COMMANDLIST_TYPE_COPY, pDesc->MaxCopyQueueSubmitCount, &m_pCopyQueue))
    { return false; }

    // デバイス情報の設定.
    {
        m_Info.ConstantBufferMemoryAlignment    = 256;
        m_Info.MaxTargetWidth                   = D3D12_REQ_RENDER_TO_BUFFER_WINDOW_WIDTH;
        m_Info.MaxTargetHeight                  = D3D12_REQ_RENDER_TO_BUFFER_WINDOW_WIDTH;
        m_Info.MaxTargetArraySize               = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
        m_Info.MaxColorSampleCount              = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT;
        m_Info.MaxDepthSampleCount              = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT;
        m_Info.MaxStencilSampleCount            = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Device::Term()
{
    for(auto i=0; i<4; ++i)
    { m_DescriptorHeap[i].Term(); }

    SafeRelease(m_pGraphicsQueue);
    SafeRelease(m_pComputeQueue);
    SafeRelease(m_pCopyQueue);

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
bool Device::CreateCommandList(COMMANDLIST_TYPE type, void* pOption, ICommandList** ppCommandList)
{ return CommandList::Create(this, type, pOption, ppCommandList); }

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
//      テクスチャを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateTexture(const TextureDesc* pDesc, ITexture** ppResource)
{ return Texture::Create(this, pDesc, ppResource); }

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
bool Device::CreateDescriptorSetLayout(const DescriptorSetLayoutDesc* pDesc, IDescriptorSetLayout** ppDescriptorSetLayout)
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
//      DXGIファクトリを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIFactory1* Device::GetDXGIFactory() const
{ return m_pFactory; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Device* Device::GetD3D12Device() const
{ return m_pDevice; }

//-------------------------------------------------------------------------------------------------
//      ディスクリプタヒープを取得します.
//-------------------------------------------------------------------------------------------------
DescriptorHeap* Device::GetDescriptorHeap(uint32_t index)
{
    A3D_ASSERT(index < 4);
    return &m_DescriptorHeap[index];
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Device::Create(const DeviceDesc* pDesc, IDevice** ppDevice)
{
    if (pDesc == nullptr || ppDevice == nullptr)
    { return false; }

    auto instance = new(std::nothrow) Device;
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDesc))
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
bool A3D_APIENTRY CreateDevice(const DeviceDesc* pDesc, IDevice** ppDevice)
{ return Device::Create(pDesc, ppDevice); }

} // namespace a3d
