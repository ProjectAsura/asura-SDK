//-------------------------------------------------------------------------------------------------
// File : a3dDevice.cpp
// Desc : Device Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <ShlObj.h>
#include <strsafe.h>

namespace {

//-------------------------------------------------------------------------------------------------
//      交差領域を計算します.
//-------------------------------------------------------------------------------------------------
inline int ComputeIntersectionArea(int ax1, int ay1, int ax2, int ay2, int bx1, int by1, int bx2, int by2)
{ return a3d::Max(0, a3d::Min(ax2, bx2) - a3d::Max(ax1, bx1)) * a3d::Max(0, a3d::Min(ay2, by2) - a3d::Max(ay1, by1)); }

//-------------------------------------------------------------------------------------------------
//      メモリ確保のラッパー関数です.
//-------------------------------------------------------------------------------------------------
void* CustomAlloc(size_t size, size_t alignment, void* pUser)
{ 
    A3D_UNUSED(pUser);
    return a3d_alloc(size, alignment); 
}

//-------------------------------------------------------------------------------------------------
//      メモリ解放のラッパー関数です.
//-------------------------------------------------------------------------------------------------
void CustomFree(void* ptr, void* pUser)
{
    A3D_UNUSED(pUser);
    return a3d_free(ptr);
}

#if A3D_IS_WIN
//-------------------------------------------------------------------------------------------------
//      PIXキャプチャー用のDLLをロードします.
//-------------------------------------------------------------------------------------------------
void LoadPixGpuCpatureDll()
{
    LPWSTR programFilesPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

    wchar_t pixSearchPath[MAX_PATH] = {};
    StringCchCopy(pixSearchPath, MAX_PATH, programFilesPath);
    StringCchCat(pixSearchPath, MAX_PATH, L"\\Microsoft PIX\\*");

    WIN32_FIND_DATA findData;
    bool foundPixInstallation = false;
    wchar_t newestVersionFound[MAX_PATH] = {};

    HANDLE hFind = FindFirstFile(pixSearchPath, &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do 
        {
            if (((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) &&
                 (findData.cFileName[0] != '.'))
            {
                if (!foundPixInstallation || wcscmp(newestVersionFound, findData.cFileName) <= 0)
                {
                    foundPixInstallation = true;
                    StringCchCopy(newestVersionFound, _countof(newestVersionFound), findData.cFileName);
                }
            }
        } 
        while (FindNextFile(hFind, &findData) != 0);
    }

    FindClose(hFind);

    if (!foundPixInstallation)
    {
        return;
    }

    wchar_t dllPath[MAX_PATH] = {};
    StringCchCopy(dllPath, wcslen(pixSearchPath), pixSearchPath);
    StringCchCat(dllPath, MAX_PATH, &newestVersionFound[0]);
    StringCchCat(dllPath, MAX_PATH, L"\\WinPixGpuCapturer.dll");

    if (GetModuleHandleW(L"WinPixGpuCapturer.dll") == 0)
    {
        LoadLibraryW(dllPath);
    }
}
#endif

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
{
    memset(&m_Info, 0, sizeof(m_Info));
}

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
    // プラットフォーム依存の初期化処理.
    if (!InitNative(pDesc))
    { return false; }

    // アロケータ生成.
    {
        D3D12MA::ALLOCATION_CALLBACKS allocationCallbacks = {};
        allocationCallbacks.pAllocate   = &CustomAlloc;
        allocationCallbacks.pFree       = &CustomFree;
 
        D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
        allocatorDesc.pDevice               = m_pDevice;
        allocatorDesc.pAdapter              = m_pAdapter;
        allocatorDesc.pAllocationCallbacks  = &allocationCallbacks;

        auto hr = D3D12MA::CreateAllocator(&allocatorDesc, &m_pAllocator);
        if ( FAILED(hr) )
        {
            A3D_LOG("Error : D3D12MA::CreateAllocator() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = pDesc->MaxShaderResourceCount;
        desc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice, &desc ) )
        {
            A3D_LOG("Error : DescriptorHeap::Init() Failed.");
            return false;
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = pDesc->MaxSamplerCount;
        desc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice, &desc ) )
        {
            A3D_LOG("Error : DescriptorHeap::Init() Failed.");
            return false;
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = pDesc->MaxColorTargetCount;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice, &desc ) )
        {
            A3D_LOG("Error : DescriptorHeap::Init() Failed.");
            return false; 
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = pDesc->MaxDepthTargetCount;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice, &desc ) )
        {
            A3D_LOG("Error : DescriptorHeap::Init() Failed.");
            return false;
        }
    }

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    if (!Queue::Create(
        this,
        COMMANDLIST_TYPE_DIRECT,
        pDesc->MaxGraphicsQueueSubmitCount,
        reinterpret_cast<IQueue**>(&m_pGraphicsQueue)))
    {
        A3D_LOG("Error : Queue::Create() Failed.");
        return false;
    }

    if (!Queue::Create(
        this,
        COMMANDLIST_TYPE_COMPUTE,
        pDesc->MaxComputeQueueSubmitCount,
        reinterpret_cast<IQueue**>(&m_pComputeQueue)))
    {
        A3D_LOG("Error : Queue::Create() Failed.");
        return false;
    }

    if (!Queue::Create(
        this,
        COMMANDLIST_TYPE_COPY,
        pDesc->MaxCopyQueueSubmitCount,
        reinterpret_cast<IQueue**>(&m_pCopyQueue)))
    {
        A3D_LOG("Error : Queue::Create() Failed.");
        return false;
    }

    // デバイス情報の設定.
    {
        m_Info.ConstantBufferAlignment          = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
        m_Info.MaxTargetWidth                   = D3D12_REQ_RENDER_TO_BUFFER_WINDOW_WIDTH;
        m_Info.MaxTargetHeight                  = D3D12_REQ_RENDER_TO_BUFFER_WINDOW_WIDTH;
        m_Info.MaxTargetArraySize               = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
        m_Info.MaxColorSampleCount              = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT;
        m_Info.MaxDepthSampleCount              = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT;
        m_Info.MaxStencilSampleCount            = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT;
        m_Info.ShaderIdentifierSize             = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        m_Info.RayTracingShaderRecordAlignment  = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
        m_Info.RayTracingShaderTableAlignment   = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
    }

    // タイムスタンプ周波数取得.
    {
        auto hr = m_pGraphicsQueue->GetD3D12Queue()->GetTimestampFrequency(&m_TimeStampFrequency);
        if (FAILED(hr))
        {
            A3D_LOG("Error : ID3D12CommandQueue::GetTimestampFrequency() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    return true;
}

#if A3D_IS_WIN
//-------------------------------------------------------------------------------------------------
//      プラットフォーム依存の初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool Device::InitNative(const DeviceDesc* pDesc)
{
    if (pDesc == nullptr)
    {
        A3D_LOG("Error : Invalid Argument");
        return false;
    }

    if (pDesc->EnableCapture)
    { LoadPixGpuCpatureDll(); }

    // デバッグレイヤーを有効化.
    if (pDesc->EnableDebug)
    {
        ID3D12Debug* pDebug;
        auto hr = D3D12GetDebugInterface( IID_PPV_ARGS(&pDebug) );
        if ( SUCCEEDED(hr) )
        { pDebug->EnableDebugLayer(); }

        ID3D12DeviceRemovedExtendedDataSettings1* pDredSettings = nullptr;
        hr = D3D12GetDebugInterface(IID_PPV_ARGS(&pDredSettings));
        if (SUCCEEDED(hr))
        {
            pDredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            pDredSettings->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            pDredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            //pDredSettings->SetWatsonDumpEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
        }

        SafeRelease(pDredSettings);
        SafeRelease(pDebug);
    }

    uint32_t flags = 0;
    if (pDesc->EnableDebug)
    { flags |= DXGI_CREATE_FACTORY_DEBUG; }

    // DXGIファクトリを生成.
    auto hr = CreateDXGIFactory2( flags, IID_PPV_ARGS(&m_pFactory) );
    if ( FAILED(hr) )
    {
        A3D_LOG("Error : CreateDXGIFactory2() Failed. errcode = 0x%x", hr);
        return false;
    }

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
            A3D_LOG("Error : IDXGIFactory::EnumWarpAdapter() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // IDXGIAdapter4に変換
    hr = pAdapter->QueryInterface( IID_PPV_ARGS(&m_pAdapter));
    SafeRelease(pAdapter);
    if ( FAILED(hr) )
    {
        A3D_LOG("Error : IDXGIAdapter4::QueryInterface() Failed. errcode = 0x%x", hr);
        return false;
    }

    // デフォルトディスプレイを取得.
    IDXGIOutput* pOutput;
    hr = m_pAdapter->EnumOutputs(0, &pOutput);
    if ( FAILED(hr) )
    {
        A3D_LOG("Error : IDXGIAdapter4::EnumOutputs() Failed. errcode = 0x%x", hr);
        return false;
    }

    // IDXGOutput6に変換.
    hr = pOutput->QueryInterface( IID_PPV_ARGS(&m_pOutput) );
    SafeRelease(pOutput);
    if ( FAILED(hr) )
    {
        A3D_LOG("Error : IDXGIOutput6::QueryInterface() Failed. errcode = 0x%x", hr);
        return false;
    }

    // デバイス生成.
    hr = D3D12CreateDevice( nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pDevice) );
    if ( FAILED(hr) )
    {
        A3D_LOG("Error : D3D12CreateDevice() Failed. errcode = 0x%x", hr);
        return false;
    }

    if (pDesc->EnableDebug)
    {
        ID3D12InfoQueue* pInfoQueue = nullptr;
        hr = m_pDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
        if (SUCCEEDED(hr))
        {
            // レンダーターゲットと深度ステンシルビューのクリア値が異なる警告は抑制する.
            D3D12_MESSAGE_ID muteIds[] = {
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
            };

            D3D12_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs  = _countof(muteIds);
            filter.DenyList.pIDList = muteIds;

            pInfoQueue->AddStorageFilterEntries(&filter);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        }
        SafeRelease(pInfoQueue);
    }

    // 非同期コンピュートのサポート.
    {
        m_Info.SupportAsycCompute = true;
    }

    // ハードウェアレイトレーシングがサポートされているかどうかチェック.
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 options = {};
        hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options));
        if (SUCCEEDED(hr))
        { m_Info.SupportRayTracing = (options.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED); }
    }

    // メッシュシェーダがサポートされているかどうかチェック.
    {
        // シェーダモデルをチェック.
        {
            D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_5 };
            hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel));
            if (SUCCEEDED(hr) && (shaderModel.HighestShaderModel >= D3D_SHADER_MODEL_6_5))
            {
                // レイトレ機能をチェック.
                D3D12_FEATURE_DATA_D3D12_OPTIONS7 options = {};
                hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options, sizeof(options));
                if (SUCCEEDED(hr))
                { m_Info.SupportMeshShader = (options.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED); }
            }
        }
    }

    // 正常終了.
    return true;
}
#endif

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

    SafeRelease(m_pAllocator);

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
bool Device::CreateCommandList(const CommandListDesc* pDesc, ICommandList** ppCommandList)
{ return CommandList::Create(this, pDesc, ppCommandList); }

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
//      レンダーターゲットビューを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateRenderTargetView
(
    ITexture*               pTexture,
    const TargetViewDesc*   pDesc,
    IRenderTargetView**     ppTargetView
)
{ return RenderTargetView::Create(this, pTexture, pDesc, ppTargetView); }

//-------------------------------------------------------------------------------------------------
//      深度ステンシルビューを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateDepthStencilView
(
    ITexture*               pTexture,
    const TargetViewDesc*   pDesc,
    IDepthStencilView**     ppTargetView
)
{ return DepthStencilView::Create(this, pTexture, pDesc, ppTargetView); }

//-------------------------------------------------------------------------------------------------
//      定数バッファビューを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateConstantBufferView
(
    IBuffer*                        pBuffer,
    const ConstantBufferViewDesc*   pDesc,
    IConstantBufferView**           ppBufferView
)
{ return ConstantBufferView::Create(this, pBuffer, pDesc, ppBufferView); }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateShaderResourceView
(
    IResource*                      pResource,
    const ShaderResourceViewDesc*   pDesc,
    IShaderResourceView**           ppTextureView
)
{ return ShaderResourceView::Create(this, pResource, pDesc, ppTextureView); }

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
//      メッシュレットパイプラインを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateMeshletPipeline(const MeshletPipelineStateDesc* pDesc, IPipelineState** ppPipelineState)
{ return PipelineState::CreateAsMesh(this, pDesc, ppPipelineState); }

//-------------------------------------------------------------------------------------------------
//      レイトレーシングパイプラインステートを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateRayTracingPipeline(const RayTracingPipelineStateDesc* pDesc, IPipelineState** ppPipelineState)
{ return RayTracingPipelineState::Create(this, pDesc, ppPipelineState); }

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
//      加速機構を生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateAccelerationStructure
(
    const AccelerationStructureDesc*    pDesc,
    IAccelerationStructure**            ppAccelerationStructure
)
{ return AccelerationStructure::Create(this, pDesc, ppAccelerationStructure); }

//-------------------------------------------------------------------------------------------------
//      アイドル状態になるまで待機します.
//-------------------------------------------------------------------------------------------------
void Device::WaitIdle()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      DXGIファクトリを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIFactoryA3D* Device::GetDXGIFactory() const
{ return m_pFactory; }

//-------------------------------------------------------------------------------------------------
//      デフォルトアダプターを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIAdapterA3D* Device::GetDXGIAdapter() const
{ return m_pAdapter; }

//-------------------------------------------------------------------------------------------------
//      デフォルトディスプレイを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIOutputA3D* Device::GetDXGIOutput() const
{ return m_pOutput; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12DeviceA3D* Device::GetD3D12Device() const
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
    IDXGIOutput* pCurrentOutput     = nullptr;
    IDXGIOutput* pBestOutput        = nullptr;
    int          bestIntersectArea  = -1;

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
//      アロケータを取得します.
//-------------------------------------------------------------------------------------------------
D3D12MA::Allocator* Device::GetAllocator() const
{ return m_pAllocator; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Device::Create(const DeviceDesc* pDesc, IDevice** ppDevice)
{
    if (pDesc == nullptr || ppDevice == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new Device;
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if (!instance->Init(pDesc))
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
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
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

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
