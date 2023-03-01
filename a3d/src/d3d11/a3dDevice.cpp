//-------------------------------------------------------------------------------------------------
// File : a3dDevice.cpp
// Desc : Device Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <ShlObj.h>
#include <strsafe.h>

namespace {

inline int Max(int a, int b)
{ return (a > b) ? a : b; }

inline int Min(int a, int b)
{ return (a < b) ? a : b; }

inline int ComputeIntersectionArea(int ax1, int ay1, int ax2, int ay2, int bx1, int by1, int bx2, int by2)
{ return Max(0, Min(ax2, bx2) - Max(ax1, bx1)) * Max(0, Min(ay2, by2) - Max(ay1, by1)); }

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
, m_pGraphicsQueue  (nullptr)
, m_pComputeQueue   (nullptr)
, m_pCopyQueue      (nullptr)
, m_pFactory        (nullptr)
, m_pAdapter        (nullptr)
, m_pOutput         (nullptr)
, m_pDevice         (nullptr)
, m_pDeviceContext  (nullptr)
#if defined(A3D_FOR_WINDOWS10)
, m_pFactory5       (nullptr)
, m_pAdapter3       (nullptr)
, m_pOutput4        (nullptr)
#endif
, m_UniqueId        (1)
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
    if (pDesc == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    if (pDesc->EnableCapture)
    { LoadPixGpuCpatureDll(); }

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    uint32_t createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    if (pDesc->EnableDebug)
    { createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG; }

    // BGRAサポートを有効化.
    createDeviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    // ファクトリーを生成.
    {
        auto hr = CreateDXGIFactory( IID_PPV_ARGS(&m_pFactory) );
        if ( FAILED(hr) )
        {
            A3D_LOG("Error : CreateDXGIFactory() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // デフォルトアダプターを取得.
    {
        auto hr = m_pFactory->EnumAdapters( 0, &m_pAdapter );
        if ( FAILED(hr) )
        {
            A3D_LOG("Error : IDXGIFactory::EnumAdapters() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // デフォルトディスプレイを取得.
    {
        auto hr = m_pAdapter->EnumOutputs(0, &m_pOutput);
        if ( FAILED(hr) )
        {
            A3D_LOG("Error : IDXGIAdapter::EnumOutputs() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    #if defined(A3D_FOR_WINDOWS10)
    {
        HRESULT hr = S_OK;
        hr = m_pFactory->QueryInterface( IID_PPV_ARGS(&m_pFactory5) );
        if ( FAILED(hr) )
        { SafeRelease(m_pFactory5); }

        hr = m_pAdapter->QueryInterface( IID_PPV_ARGS(&m_pAdapter3) );
        if ( FAILED(hr) )
        { SafeRelease(m_pAdapter3); }

        hr = m_pOutput->QueryInterface( IID_PPV_ARGS(&m_pOutput4) );
        if ( FAILED(hr) )
        { SafeRelease(m_pOutput4); }
    }
    #endif

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

        ID3D11Device*           pDevice  = nullptr;
        ID3D11DeviceContext*    pContext = nullptr;

        auto hr = D3D11CreateDevice(
            nullptr, 
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            featureLevels,
            _countof(featureLevels),
            D3D11_SDK_VERSION,
            &pDevice,
            &m_FeatureLevel,
            &pContext );
        if ( FAILED(hr) )
        {
            A3D_LOG("Error : D3D11CreateDevice() Failed. errcode = 0x%x", hr);
            return false;
        }

        hr = pDevice->QueryInterface(IID_PPV_ARGS(&m_pDevice));
        if (FAILED(hr))
        {
            pContext->Release();
            pDevice ->Release();
            A3D_LOG("Error : ID3D11Device::QueryInterface() Failed. errcode = 0x%x", hr);
            return false;
        }

#ifdef A3D_FOR_WINDOWS10
        ID3D11DeviceContext3* pContext3 = nullptr;
        m_pDevice->GetImmediateContext3(&pContext3);
        hr = pContext3->QueryInterface(IID_PPV_ARGS(&m_pDeviceContext));

        pContext3->Release();
        if (FAILED(hr))
        {
            pContext->Release();
            pDevice->Release();
            A3D_LOG("Error : ID3D11DeviceContext3::QueryInterface() Failed. errcode = 0x%x", hr);
            return false;
        }
#else
        m_pDevice->GetImmediateContext2(&m_pDeviceContext);
#endif

        pContext->Release();
        pDevice ->Release();
    }

    if (pDesc->EnableDebug)
    {
        ID3D11InfoQueue* pInfoQueue = nullptr;
        auto hr = m_pDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
        if (SUCCEEDED(hr))
        {
            pInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            pInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
        }
        SafeRelease(pInfoQueue);
    }

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
        m_Info.ConstantBufferAlignment          = 16;
        m_Info.MaxTargetWidth                   = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
        m_Info.MaxTargetHeight                  = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
        m_Info.MaxTargetArraySize               = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
        m_Info.MaxColorSampleCount              = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;
        m_Info.MaxDepthSampleCount              = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;
        m_Info.MaxStencilSampleCount            = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;
        m_Info.SupportAsycCompute               = false;
        m_Info.SupportRayTracing                = false;
        m_Info.SupportMeshShader                = false;
    }

    {
        D3D11_QUERY_DESC desc = {};
        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;

        ID3D11Query* pQuery = nullptr;
        auto hr = m_pDevice->CreateQuery(&desc, &pQuery);
        if (FAILED(hr))
        {
            SafeRelease(pQuery);
            A3D_LOG("Error : ID3D11Query::CreateQuery() Failed. errcode = 0x%x", hr);
            return false;
        }

        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT data = {};
        m_pDeviceContext->Begin(pQuery);
        m_pDeviceContext->End(pQuery);
        while(m_pDeviceContext->GetData(pQuery, &data, sizeof(data), 0) != S_OK);

        m_TimeStampFrequency = data.Frequency;

        SafeRelease(pQuery);
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

#if defined(A3D_FOR_WINDOWS10)
    SafeRelease(m_pOutput4);
    SafeRelease(m_pAdapter3);
    SafeRelease(m_pFactory5);
#endif// defined(A3D_FOR_WINDOW10)

    SafeRelease(m_pOutput);
    SafeRelease(m_pAdapter);
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
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void Device::SetName(const char* name)
{
    m_Name = name;
    m_pDevice->SetPrivateData(WKPDID_D3DDebugObjectName, m_Name.size(), m_Name.c_str());
}

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* Device::GetName() const
{ return m_Name.c_str(); }

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
    IRenderTargetView**     ppRenderTargetView
)
{ return RenderTargetView::Create(this, pTexture, pDesc, ppRenderTargetView); }

//-------------------------------------------------------------------------------------------------
//      深度ステンシルビューを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateDepthStencilView
(
    ITexture*               pTexture,
    const TargetViewDesc*   pDesc,
    IDepthStencilView**     ppDepthStencilView
)
{ return DepthStencilView::Create(this, pTexture, pDesc, ppDepthStencilView); }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateShaderResourceView
(
    IResource*                      pResource,
    const ShaderResourceViewDesc*   pDesc,
    IShaderResourceView**           ppShaderResourceView
)
{ return ShaderResourceView::Create(this, pResource, pDesc, ppShaderResourceView); }

//-------------------------------------------------------------------------------------------------
//      バッファビューを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateConstantBufferView
(
    IBuffer*                        pBuffer,
    const ConstantBufferViewDesc*   pDesc,
    IConstantBufferView**           ppBufferView
)
{ return ConstantBufferView::Create(this, pBuffer, pDesc, ppBufferView); }

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
{
    // D3D11ではメッシュシェーダパイプラインがサポートされていないため常に失敗扱いにする.
    A3D_UNUSED(pDesc);
    A3D_UNUSED(ppPipelineState);
    return false;
}

//-------------------------------------------------------------------------------------------------
//      レイトレーシングパイプラインを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateRayTracingPipeline(const RayTracingPipelineStateDesc* pDesc, IPipelineState** ppPipelineState)
{
    // D3D11ではレイトレーシングパイプラインがサポートされていないため常に失敗扱いにする.
    A3D_UNUSED(pDesc);
    A3D_UNUSED(ppPipelineState);
    return false;
}

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
    IAccelerationStructure**            ppAS
)
{
    // D3D11ではレイトレーシングパイプラインがサポートされていないため常に失敗扱いにする.
    A3D_UNUSED(pDesc);
    A3D_UNUSED(ppAS);
    return false;
}

//-------------------------------------------------------------------------------------------------
//      アイドル状態になるまで待機します.
//-------------------------------------------------------------------------------------------------
void Device::WaitIdle()
{
    m_pDeviceContext->Flush();
    m_pDeviceContext->ClearState();
}

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11DeviceA3D* Device::GetD3D11Device() const
{ return m_pDevice; }

//-------------------------------------------------------------------------------------------------
//      デバイスコンテキストを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11DeviceContextA3D* Device::GetD3D11DeviceContext() const
{ return m_pDeviceContext; }

//-------------------------------------------------------------------------------------------------
//      DXGIファクトリーを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIFactory* Device::GetDXGIFactory() const
{ return m_pFactory; }

//-------------------------------------------------------------------------------------------------
//      デフォルトアダプターを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIAdapter* Device::GetDXGIAdapter() const
{ return m_pAdapter; }

//-------------------------------------------------------------------------------------------------
//      デフォルトディスプレイを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIOutput* Device::GetDXGIOutput() const
{ return m_pOutput; }

#if defined(A3D_FOR_WINDOWS10)

//-------------------------------------------------------------------------------------------------
//      DXGIファクトリーを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIFactory5* Device::GetDXGIFactory5() const
{ return m_pFactory5; }

//-------------------------------------------------------------------------------------------------
//      デフォルトアダプターを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIAdapter3* Device::GetDXGIAdapter3() const
{ return m_pAdapter3; }

//-------------------------------------------------------------------------------------------------
//      デフォルトディスプレイを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIOutput4* Device::GetDXGIOutput4() const
{ return m_pOutput4; }

//-------------------------------------------------------------------------------------------------
//      HDRディスプレイをサポートしているかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool Device::CheckDisplayHDRSupport(RECT region)
{
    if (m_pFactory5->IsCurrent() == false)
    { CreateDXGIFactory2(0, IID_PPV_ARGS(&m_pFactory5)); }

    IDXGIAdapter1* pAdapter = nullptr;
    auto hr = m_pFactory5->EnumAdapters1(0, &pAdapter);
    if (FAILED(hr))
    {
        A3D_LOG("Error : IDXGIFactory::EnumAdapters1() Failed. errcode = 0x%x", hr);
        return false;
    }

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
            A3D_LOG("Error : IDXGIOutput::GetDesc() Failed. errcode = 0x%x", hr);
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
        A3D_LOG("Error : IDXGIOutput::QueryInterface() Failed. errcode = 0x%x", hr);
        return false;
    }

    DXGI_OUTPUT_DESC1 desc1 = {};
    hr = pOutput6->GetDesc1(&desc1);
    if (FAILED(hr))
    {
        SafeRelease(pBestOutput);
        SafeRelease(pCurrentOutput);
        A3D_LOG("Error : IDXGIOutput6::GetDesc1() Failed. errcode = 0x%x", hr);
    }

    return desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_STUDIO_G2084_NONE_P2020;
}

#endif// defined(A3D_FOR_WINDOWS10)

//-------------------------------------------------------------------------------------------------
//      ユニークIDを割り当てます.
//-------------------------------------------------------------------------------------------------
uint64_t Device::AllocUniqueId() 
{ return m_UniqueId++; }

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
