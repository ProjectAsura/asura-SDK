//-------------------------------------------------------------------------------------------------
// File : a3dRayTracingPipelineState.cpp
// Desc : Ray Tracing Pipeline State.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace {

//-------------------------------------------------------------------------------------------------
//      ワイド文字列に変換します.
//-------------------------------------------------------------------------------------------------
wchar_t* ToStringW(const char* input)
{
    auto srcSize = strlen(input);
    auto dstSize = MultiByteToWideChar(CP_ACP, 0, input, int(srcSize + 1), nullptr, 0 );
    auto buffer = static_cast<wchar_t*>(a3d_alloc(sizeof(wchar_t) * dstSize, alignof(wchar_t)));
    A3D_ASSERT(buffer != nullptr);

    MultiByteToWideChar(CP_ACP, 0, input, int(srcSize + 1), buffer, dstSize);
    return buffer;
}

//-------------------------------------------------------------------------------------------------
//      ヒットグループタイプを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_HIT_GROUP_TYPE ToNativeHitGroupType(a3d::RAYTRACING_SHADER_GROUP_TYPE type)
{
    if (type == a3d::RAYTRACING_SHADER_GROUP_TYPE_TRIANGLE_HIT)
    { return D3D12_HIT_GROUP_TYPE_TRIANGLES; }
    else if (type == a3d::RAYTRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT)
    { return D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE; }

    return D3D12_HIT_GROUP_TYPE_TRIANGLES;
}

} // namespace


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// RayTracingPipelineState class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
RayTracingPipelineState::RayTracingPipelineState()
: m_RefCount        (1)
, m_pDevice         (nullptr)
, m_pGlobalLayout   (nullptr)
, m_pLocalLayout    (nullptr)
, m_pStateObject    (nullptr)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
RayTracingPipelineState::~RayTracingPipelineState()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理です.
//-------------------------------------------------------------------------------------------------
bool RayTracingPipelineState::Init(IDevice* pDevice, const RayTracingPipelineStateDesc* pDesc)
{
    if (pDesc->StageCount       == 0 
     || pDesc->GroupCount       == 0 
     || pDesc->pGlobalLayout    == nullptr
     || pDesc->pStages          == nullptr
     || pDesc->pGroups          == nullptr)
    {
        A3D_LOG("Error : Invalid Argument");
        return false;
    }

    auto pWrapDevice = static_cast<Device*>(pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    if (!pWrapDevice->GetInfo().SupportRayTracing)
    {
        A3D_LOG("Error : RayTracing feature is not supported by hardware.");
        return false;
    }

    m_pDevice = pWrapDevice;
    m_pDevice->AddRef();

    {
        auto subObjectCount = pDesc->GroupCount + 5;
        auto subObjects     = new D3D12_STATE_SUBOBJECT[subObjectCount];
        auto subObjectIndex = 0;

        auto pWrapGlobalLayout = static_cast<DescriptorSetLayout*>(pDesc->pGlobalLayout);
        A3D_ASSERT(pWrapGlobalLayout != nullptr);

        m_pGlobalLayout = pWrapGlobalLayout;
        m_pGlobalLayout->AddRef();

        // グローバルルートシグニチャの設定.
        D3D12_GLOBAL_ROOT_SIGNATURE globalRootSig = {};
        globalRootSig.pGlobalRootSignature = pWrapGlobalLayout->GetD3D12RootSignature();

        subObjects[subObjectIndex].pDesc = &globalRootSig;
        subObjects[subObjectIndex].Type  = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
        subObjectIndex++;

        // ローカルルートシグニチャの設定.
        D3D12_LOCAL_ROOT_SIGNATURE localRootSig = {};
        if (pDesc->pLocalLayout != nullptr)
        {
            auto pWrapLocalLayout = static_cast<DescriptorSetLayout*>(pDesc->pLocalLayout);
            A3D_ASSERT(pWrapLocalLayout != nullptr);

            m_pLocalLayout = pWrapLocalLayout;
            m_pLocalLayout->AddRef();

            localRootSig.pLocalRootSignature = pWrapLocalLayout->GetD3D12RootSignature();

            subObjects[subObjectIndex].pDesc = &localRootSig;
            subObjects[subObjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
            subObjectIndex++;
        }

        auto exports = new D3D12_EXPORT_DESC[pDesc->StageCount];
        A3D_ASSERT(exports != nullptr);

        auto names = new wchar_t*[pDesc->StageCount];
        A3D_ASSERT(names != nullptr);

        for(auto i=0u; i<pDesc->StageCount; ++i)
        {
            names[i] = ToStringW(pDesc->pStages[i].EntryPoint);

            exports[i].Name             = names[i];
            exports[i].ExportToRename   = nullptr;
            exports[i].Flags            = D3D12_EXPORT_FLAG_NONE;
        }

        // DXILライブラリの設定.
        D3D12_DXIL_LIBRARY_DESC dxilLib = {};
        dxilLib.NumExports = pDesc->StageCount;
        dxilLib.pExports   = exports;

        subObjects[subObjectIndex].pDesc = &dxilLib;
        subObjects[subObjectIndex].Type  = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
        subObjectIndex++;

        // ヒットグループの設定.
        auto groups = new D3D12_HIT_GROUP_DESC[pDesc->GroupCount];
        A3D_ASSERT(groups != nullptr);

        for(auto i=0u; i<pDesc->GroupCount; ++i)
        {
            auto& g = pDesc->pGroups[i];

            groups[i].Type                      = ToNativeHitGroupType(g.Type);
            groups[i].HitGroupExport            = (g.HitGroupShader     == SHADER_UNUSED) ? nullptr : names[g.HitGroupShader];
            groups[i].AnyHitShaderImport        = (g.AnyHitShader       == SHADER_UNUSED) ? nullptr : names[g.AnyHitShader];
            groups[i].ClosestHitShaderImport    = (g.ClosestHitShader   == SHADER_UNUSED) ? nullptr : names[g.ClosestHitShader];
            groups[i].IntersectionShaderImport  = (g.IntersectionShader == SHADER_UNUSED) ? nullptr : names[g.IntersectionShader];

            subObjects[subObjectIndex].pDesc = &groups[i];
            subObjects[subObjectIndex].Type  = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
            subObjectIndex++;
        }

        // シェーダ設定.
        D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
        shaderConfig.MaxPayloadSizeInBytes   = pDesc->MaxPayloadSize;
        shaderConfig.MaxAttributeSizeInBytes = pDesc->MaxAttributeSize;

        subObjects[subObjectIndex].pDesc = &shaderConfig;
        subObjects[subObjectIndex].Type  = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
        subObjectIndex++;

        // パイプライン設定.
        D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
        pipelineConfig.MaxTraceRecursionDepth = pDesc->MaxTraceRecursionDepth;
        
        subObjects[subObjectIndex].pDesc = &pipelineConfig;
        subObjects[subObjectIndex].Type  = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
        subObjectIndex++;

        // ステート設定.
        D3D12_STATE_OBJECT_DESC desc = {};
        desc.Type           = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
        desc.NumSubobjects  = subObjectCount;
        desc.pSubobjects    = subObjects;

        auto hr = pWrapDevice->GetD3D12Device()->CreateStateObject(
            &desc, IID_PPV_ARGS(&m_pStateObject));

        for(auto i=0u; i<pDesc->GroupCount; ++i)
        {
            a3d_free(names[i]);
            exports[i].Name = nullptr;
        }

        if (exports != nullptr)
        {
            delete[] exports;
            exports = nullptr;
        }

        if (names != nullptr)
        {
            delete[] names;
            names = nullptr;
        }

        if (subObjects != nullptr)
        {
            delete[] subObjects;
            subObjects = nullptr;
        }

        if (FAILED(hr))
        {
            A3D_LOG("Error : ID3D12Device::CreateStateObject() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void RayTracingPipelineState::Term()
{
    SafeRelease(m_pGlobalLayout);
    SafeRelease(m_pLocalLayout);
    SafeRelease(m_pStateObject);
    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void RayTracingPipelineState::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void RayTracingPipelineState::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t RayTracingPipelineState::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void RayTracingPipelineState::SetName(const char* name)
{
    m_Name = name;
    m_pStateObject->SetPrivateData(WKPDID_D3DDebugObjectName, m_Name.size(), m_Name.c_str());
}

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* RayTracingPipelineState::GetName() const
{ return m_Name.c_str(); }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void RayTracingPipelineState::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      パイプラインステートタイプを取得します.
//-------------------------------------------------------------------------------------------------
PIPELINE_STATE_TYPE RayTracingPipelineState::GetType() const
{ return PIPELINE_STATE_TYPE_RAYTRACING; }

//-------------------------------------------------------------------------------------------------
//      描画コマンドを発行します.
//-------------------------------------------------------------------------------------------------
void RayTracingPipelineState::Issue(ICommandList* pCommandList)
{
    auto pWrapCommandList = static_cast<CommandList*>(pCommandList);
    A3D_ASSERT(pWrapCommandList != nullptr);

    auto pNativeCommandList = pWrapCommandList->GetD3D12GraphicsCommandList();
    A3D_ASSERT(pNativeCommandList != nullptr);

    pNativeCommandList->SetComputeRootSignature(m_pGlobalLayout->GetD3D12RootSignature());
    pNativeCommandList->SetPipelineState1(m_pStateObject);
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタセットレイアウトを取得します.
//-------------------------------------------------------------------------------------------------
DescriptorSetLayout* RayTracingPipelineState::GetDescriptorSetLayout() const
{ return m_pGlobalLayout; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool RayTracingPipelineState::Create(IDevice* pDevice, const RayTracingPipelineStateDesc* pDesc, IPipelineState** ppPipelineState)
{
    if (pDevice == nullptr || pDesc == nullptr || ppPipelineState == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new RayTracingPipelineState();
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out of Memory.");
        return false;
    }

    if (!instance->Init(pDevice, pDesc))
    {
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppPipelineState = instance;
    return true;
}

} // namespace a3d
