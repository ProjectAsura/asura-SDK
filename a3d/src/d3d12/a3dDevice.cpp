//-------------------------------------------------------------------------------------------------
// File : a3dDevice.cpp
// Desc : Device Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace {

inline int Max(int a, int b)
{ return (a > b) ? a : b; }

inline int Min(int a, int b)
{ return (a < b) ? a : b; }

inline int ComputeIntersectionArea(int ax1, int ay1, int ax2, int ay2, int bx1, int by1, int bx2, int by2)
{ return Max(0, Min(ax2, bx2) - Max(ax1, bx1)) * Max(0, Min(ay2, by2) - Max(ay1, by1)); }

} // namespace 

namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Device class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Device::Device()
: m_RefCount        (1)
, m_pFactory        (nullptr)
, m_pAdapter        (nullptr)
, m_pOutput         (nullptr)
, m_pDevice         (nullptr)
, m_pGraphicsQueue  (nullptr)
, m_pComputeQueue   (nullptr)
, m_pCopyQueue      (nullptr)
, m_TearingSupport  (false)
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

    uint32_t flags = 0;
    if (pDesc->EnableDebug)
    { flags |= DXGI_CREATE_FACTORY_DEBUG; }

    // DXGIファクトリを生成.
    auto hr = CreateDXGIFactory2( flags, IID_PPV_ARGS(&m_pFactory) );
    if ( FAILED(hr) )
    { return false; }

    BOOL allowTearing;
    hr = m_pFactory->CheckFeatureSupport( DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing) );
    if ( SUCCEEDED(hr) )
    { m_TearingSupport = (allowTearing == TRUE); }

    // デフォルトアダプターを取得.
    IDXGIAdapter1* pAdapter = nullptr;
    hr = m_pFactory->EnumAdapters1( 0, &pAdapter );
    if ( FAILED(hr) )
    {
        SafeRelease(pAdapter);
        hr = m_pFactory->EnumWarpAdapter( IID_PPV_ARGS(&pAdapter) );
        if ( FAILED(hr) )
        {
            SafeRelease(pAdapter);
            return false;
        }
    }

    // IDXGIAdapter4に変換
    hr = pAdapter->QueryInterface( IID_PPV_ARGS(&m_pAdapter));
    SafeRelease(pAdapter);
    if ( FAILED(hr) )
    { return false; }

    // デフォルトディスプレイを取得.
    IDXGIOutput* pOutput;
    hr = m_pAdapter->EnumOutputs(0, &pOutput);
    if ( FAILED(hr) )
    { return false; }

    // IDXGOutput6に変換.
    hr = pOutput->QueryInterface( IID_PPV_ARGS(&m_pOutput) );
    SafeRelease(pOutput);
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

    if (!Queue::Create(
        this,
        COMMANDLIST_TYPE_DIRECT,
        pDesc->MaxGraphicsQueueSubmitCount,
        reinterpret_cast<IQueue**>(&m_pGraphicsQueue)))
    { return false; }

    if (!Queue::Create(
        this,
        COMMANDLIST_TYPE_COMPUTE,
        pDesc->MaxComputeQueueSubmitCount,
        reinterpret_cast<IQueue**>(&m_pComputeQueue)))
    { return false; }

    if (!Queue::Create(
        this,
        COMMANDLIST_TYPE_COPY,
        pDesc->MaxCopyQueueSubmitCount,
        reinterpret_cast<IQueue**>(&m_pCopyQueue)))
    { return false; }

    // デバイス情報の設定.
    {
        m_Info.ConstantBufferMemoryAlignment    = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
        m_Info.MaxTargetWidth                   = D3D12_REQ_RENDER_TO_BUFFER_WINDOW_WIDTH;
        m_Info.MaxTargetHeight                  = D3D12_REQ_RENDER_TO_BUFFER_WINDOW_WIDTH;
        m_Info.MaxTargetArraySize               = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
        m_Info.MaxColorSampleCount              = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT;
        m_Info.MaxDepthSampleCount              = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT;
        m_Info.MaxStencilSampleCount            = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT;
    }

    hr = m_pGraphicsQueue->GetD3D12Queue()->GetTimestampFrequency(&m_TimeStampFrequency);
    if (FAILED(hr))
    { return false; }

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

    SafeRelease(m_pOutput);
    SafeRelease(m_pAdapter);
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
//      GPUタイムスタンプの更新頻度を取得します.
//-------------------------------------------------------------------------------------------------
uint64_t Device::GetTimeStampFrequency() const
{ return m_TimeStampFrequency; }

//-------------------------------------------------------------------------------------------------
//      コマンドリストを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateCommandList(COMMANDLIST_TYPE type, ICommandList** ppCommandList)
{ return CommandList::Create(this, type, ppCommandList); }

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
//      アンオーダードアクセスビューを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateUnorderedAccessView
(
    IResource*                      pResource,
    const UnorderedAccessViewDesc*  pDesc,
    IUnorderedAccessView**          ppStorageView
)
{ return UnorderedAccessView::Create(this, pResource, pDesc, ppStorageView); }

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
//      アイドル状態になるまで待機します.
//-------------------------------------------------------------------------------------------------
void Device::WaitIdle()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      DXGIファクトリを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIFactory5* Device::GetDXGIFactory() const
{ return m_pFactory; }

//-------------------------------------------------------------------------------------------------
//      デフォルトアダプターを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIAdapter4* Device::GetDXGIAdapter() const
{ return m_pAdapter; }

//-------------------------------------------------------------------------------------------------
//      デフォルトディスプレイを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIOutput6* Device::GetDXGIOutput() const
{ return m_pOutput; }

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
//      ティアリングサポートしているかどうか.
//-------------------------------------------------------------------------------------------------
bool Device::IsTearingSupport() const
{ return m_TearingSupport; }

//-------------------------------------------------------------------------------------------------
//      HDRディスプレイをサポートしているかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool Device::CheckDisplayHDRSupport(RECT region)
{
    if (m_pFactory->IsCurrent() == false)
    { CreateDXGIFactory2(0, IID_PPV_ARGS(&m_pFactory)); }

    IDXGIAdapter1* pAdapter = nullptr;
    auto hr = m_pFactory->EnumAdapters1(0, &pAdapter);
    if (FAILED(hr))
    { return false; }

    UINT i=0;
    IDXGIOutput* pCurrentOutput = nullptr;
    IDXGIOutput* pBestOutput    = nullptr;
    int          bestIntersectArea = -1;

    auto ax1 = region.left;
    auto ay1 = region.top;
    auto ax2 = region.right;
    auto ay2 = region.bottom;

    while(pAdapter->EnumOutputs(i, &pCurrentOutput) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_OUTPUT_DESC desc;
        hr = pCurrentOutput->GetDesc(&desc);
        if (FAILED(hr))
        {
            SafeRelease(pCurrentOutput);
            SafeRelease(pBestOutput);
            return false;
        }

        auto bx1 = desc.DesktopCoordinates.left;
        auto by1 = desc.DesktopCoordinates.top;
        auto bx2 = desc.DesktopCoordinates.right;
        auto by2 = desc.DesktopCoordinates.bottom;

        auto intersectArea = ComputeIntersectionArea(ax1, ay1, ax2, ay2, bx1, by1, bx2, by2);
        if (intersectArea > bestIntersectArea)
        {
            SafeRelease(pBestOutput);
            pBestOutput       = pCurrentOutput;
            bestIntersectArea = intersectArea;
        }

        i++;
    }

    IDXGIOutput6* pOutput6 = nullptr;
    hr = pBestOutput->QueryInterface(IID_PPV_ARGS(&pOutput6));
    if (FAILED(hr))
    {
        SafeRelease(pBestOutput);
        SafeRelease(pCurrentOutput);
        return false;
    }

    DXGI_OUTPUT_DESC1 desc1 = {};
    hr = pOutput6->GetDesc1(&desc1);
    if (FAILED(hr))
    {
        SafeRelease(pBestOutput);
        SafeRelease(pCurrentOutput);
    }

    return desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_STUDIO_G2084_NONE_P2020;
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Device::Create(const DeviceDesc* pDesc, IDevice** ppDevice)
{
    if (pDesc == nullptr || ppDevice == nullptr)
    { return false; }

    auto instance = new Device;
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
{
    if (pDesc == nullptr || ppDevice == nullptr)
    { return false; }

    return Device::Create(pDesc, ppDevice);
}

//-------------------------------------------------------------------------------------------------
//      グラフィックスシステムを初期化します.
//-------------------------------------------------------------------------------------------------
bool A3D_APIENTRY InitSystem(const SystemDesc* pDesc)
{ return InitSystemAllocator(pDesc->pSystemAllocator); }

//-------------------------------------------------------------------------------------------------
//      グラフィクスシステムが初期化済みかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool A3D_APIENTRY IsInitSystem()
{ return IsInitSystemAllocator(); }

//-------------------------------------------------------------------------------------------------
//      グラフィックスシステムの終了処理を行います.
//-------------------------------------------------------------------------------------------------
void A3D_APIENTRY TermSystem()
{ TermSystemAllocator(); }

} // namespace a3d
