//-------------------------------------------------------------------------------------------------
// File : a3dPipelineState.cpp
// Desc : Pipeline State Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

///////////////////////////////////////////////////////////////////////////////////////////////////
// SubObject class
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename DataType, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE EnumType>
class alignas(void*) SubObject
{
public:
    SubObject() noexcept
    : m_EnumType   (EnumType)
    , m_DataType   (DataType())
    { /* DO_NOTHING */ }

    SubObject(DataType const& value) noexcept
    : m_EnumType   (EnumType)
    , m_DataType   (value)
    { /* DO_NOTHING */ }

    SubObject& operator = (DataType const& value) noexcept
    {
        m_EnumType  = EnumType;
        m_DataType = value;
        return *this;
    }

    operator DataType const&() const noexcept 
    { return m_DataType; }

    operator DataType&() noexcept 
    { return m_DataType; }

    DataType* operator&() noexcept
    { return &m_DataType; }

    DataType const* operator&() const noexcept
    { return &m_DataType; }

private:
    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE m_EnumType;
    DataType                            m_DataType;
};

using D3D12_PSS_ROOT_SIGNATURE = SubObject< ID3D12RootSignature*,           D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE >;
using D3D12_PSS_AS             = SubObject< D3D12_SHADER_BYTECODE,          D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS >;
using D3D12_PSS_MS             = SubObject< D3D12_SHADER_BYTECODE,          D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS >;
using D3D12_PSS_PS             = SubObject< D3D12_SHADER_BYTECODE,          D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS >;
using D3D12_PSS_BLEND          = SubObject< D3D12_BLEND_DESC,               D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND >;
using D3D12_PSS_SAMPLE_MASK    = SubObject< UINT,                           D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK >;
using D3D12_PSS_RASTERIZER     = SubObject< D3D12_RASTERIZER_DESC,          D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER >;
using D3D12_PSS_DEPTH_STENCIL  = SubObject< D3D12_DEPTH_STENCIL_DESC,       D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL >;
using D3D12_PSS_RTV_FORMATS    = SubObject< D3D12_RT_FORMAT_ARRAY,          D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS >;
using D3D12_PSS_DSV_FORMAT     = SubObject< DXGI_FORMAT,                    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT >;
using D3D12_PSS_SAMPLE_DESC    = SubObject< DXGI_SAMPLE_DESC,               D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC >;
using D3D12_PSS_NODE_MASK      = SubObject< UINT,                           D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK >;
using D3D12_PSS_CACHED_PSO     = SubObject< D3D12_CACHED_PIPELINE_STATE,    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO >;
using D3D12_PSS_FLAGS          = SubObject< D3D12_PIPELINE_STATE_FLAGS,     D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS >;


///////////////////////////////////////////////////////////////////////////////
// D3D12_GEOMETRY_PIPELINE_STATE_DESC structure
///////////////////////////////////////////////////////////////////////////////
struct D3D12_GEOMETRY_PIPELINE_STATE_DESC
{
    D3D12_PSS_ROOT_SIGNATURE  RootSignature;
    D3D12_PSS_AS              AS;
    D3D12_PSS_MS              MS;
    D3D12_PSS_PS              PS;
    D3D12_PSS_BLEND           BlendState;
    D3D12_PSS_SAMPLE_MASK     SampleMask;
    D3D12_PSS_RASTERIZER      RasterizerState;
    D3D12_PSS_DEPTH_STENCIL   DepthStencilState;
    D3D12_PSS_RTV_FORMATS     RTVFormats;
    D3D12_PSS_DSV_FORMAT      DSVFormat;
    D3D12_PSS_SAMPLE_DESC     SampleDesc;
    D3D12_PSS_CACHED_PSO      CachedPSO;
};

//-------------------------------------------------------------------------------------------------
//      シェーダバイナリをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeShaderByteCode( const a3d::ShaderBinary& binary, D3D12_SHADER_BYTECODE& result )
{
    result.BytecodeLength  = binary.ByteCodeSize;
    result.pShaderBytecode = binary.pByteCode;
}

//-------------------------------------------------------------------------------------------------
//      ブレンドファクターをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_BLEND ToNativeBlend( const a3d::BLEND_FACTOR& factor )
{
    D3D12_BLEND table[] = {
        D3D12_BLEND_ZERO,
        D3D12_BLEND_ONE,
        D3D12_BLEND_SRC_COLOR,
        D3D12_BLEND_INV_SRC_COLOR,
        D3D12_BLEND_SRC_ALPHA,
        D3D12_BLEND_INV_SRC_ALPHA,
        D3D12_BLEND_DEST_ALPHA,
        D3D12_BLEND_INV_DEST_ALPHA,
        D3D12_BLEND_DEST_COLOR,
        D3D12_BLEND_INV_DEST_COLOR,
        D3D12_BLEND_SRC_ALPHA_SAT,
        D3D12_BLEND_SRC1_COLOR,
        D3D12_BLEND_INV_SRC1_COLOR,
        D3D12_BLEND_SRC1_ALPHA,
        D3D12_BLEND_INV_SRC1_ALPHA
    };

    return table[factor];
}

//-------------------------------------------------------------------------------------------------
//      ブレンドオペレータをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_BLEND_OP ToNativeBlendOp( const a3d::BLEND_OP& operation )
{
    D3D12_BLEND_OP table[] = {
        D3D12_BLEND_OP_ADD,
        D3D12_BLEND_OP_SUBTRACT,
        D3D12_BLEND_OP_REV_SUBTRACT,
        D3D12_BLEND_OP_MIN,
        D3D12_BLEND_OP_MAX
    };

    return table[operation];
}

//-------------------------------------------------------------------------------------------------
//      カラーブレンドステートをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeRanderTargetBlendDesc( const a3d::ColorBlendState& state, D3D12_RENDER_TARGET_BLEND_DESC& result )
{
    result.BlendEnable      = (state.BlendEnable)   ? TRUE : FALSE;
    result.SrcBlend         = ToNativeBlend  ( state.SrcBlend );
    result.DestBlend        = ToNativeBlend  ( state.DstBlend );
    result.BlendOp          = ToNativeBlendOp( state.BlendOp );
    result.SrcBlendAlpha    = ToNativeBlend  ( state.SrcBlendAlpha );
    result.DestBlendAlpha   = ToNativeBlend  ( state.DstBlendAlpha );
    result.BlendOpAlpha     = ToNativeBlendOp( state.BlendOpAlpha );
    result.RenderTargetWriteMask = 0;

    if (state.EnableWriteR) { result.RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_RED;   }
    if (state.EnableWriteG) { result.RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_GREEN; }
    if (state.EnableWriteB) { result.RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_BLUE;  }
    if (state.EnableWriteA) { result.RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_ALPHA; }
}

//-------------------------------------------------------------------------------------------------
//      ブレンドステートをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeBlendDesc( const a3d::BlendState& state, D3D12_BLEND_DESC& result, BOOL alphaToCoverage )
{
    result.AlphaToCoverageEnable  = alphaToCoverage;
    result.IndependentBlendEnable = (state.IndependentBlendEnable) ? TRUE : FALSE;
    for(auto i=0; i<8; ++i)
    {
        result.RenderTarget[i].LogicOpEnable = FALSE;
        result.RenderTarget[i].LogicOp       = D3D12_LOGIC_OP_NOOP;
        ToNativeRanderTargetBlendDesc( state.RenderTarget[i], result.RenderTarget[i] );
    }
}

//-------------------------------------------------------------------------------------------------
//      ポリゴンモードをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_FILL_MODE ToNativeFillMode( const a3d::POLYGON_MODE& mode )
{
    D3D12_FILL_MODE table[] = {
        D3D12_FILL_MODE_SOLID,
        D3D12_FILL_MODE_WIREFRAME
    };

    return table[mode];
}

//-------------------------------------------------------------------------------------------------
//      カリングモードをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_CULL_MODE ToNativeCullMode( const a3d::CULL_MODE& mode )
{
    D3D12_CULL_MODE table[] = {
        D3D12_CULL_MODE_NONE,
        D3D12_CULL_MODE_FRONT,
        D3D12_CULL_MODE_BACK
    };

    return table[mode];
}

//-------------------------------------------------------------------------------------------------
//      ラスタライザ―ステートをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeRasterizerDesc( const a3d::RasterizerState& state, D3D12_RASTERIZER_DESC& result )
{
    result.FillMode                 = ToNativeFillMode( state.PolygonMode );
    result.CullMode                 = ToNativeCullMode( state.CullMode );
    result.FrontCounterClockwise    = ( state.FrontCounterClockWise ) ? TRUE : FALSE;
    result.DepthBias                = state.DepthBias;
    result.DepthBiasClamp           = state.DepthBiasClamp;
    result.SlopeScaledDepthBias     = state.SlopeScaledDepthBias;
    result.DepthClipEnable          = ( state.DepthClipEnable ) ? TRUE : FALSE;
    result.ConservativeRaster       = ( state.EnableConservativeRaster ) 
                                      ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON
                                      : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
}

//-------------------------------------------------------------------------------------------------
//      ステンシルオペレータをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_STENCIL_OP ToNativeStencilOp( const a3d::STENCIL_OP& operation )
{
    D3D12_STENCIL_OP table[] = {
        D3D12_STENCIL_OP_KEEP,
        D3D12_STENCIL_OP_ZERO,
        D3D12_STENCIL_OP_REPLACE,
        D3D12_STENCIL_OP_INCR_SAT,
        D3D12_STENCIL_OP_DECR_SAT,
        D3D12_STENCIL_OP_INVERT,
        D3D12_STENCIL_OP_INCR,
        D3D12_STENCIL_OP_DECR
    };

    return table[operation];
}

//-------------------------------------------------------------------------------------------------
//      ステンシルステートをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeDepthStencilOpDesc( const a3d::StencilTestDesc& state, D3D12_DEPTH_STENCILOP_DESC& result )
{
    result.StencilFailOp      = ToNativeStencilOp( state.StencilFailOp );
    result.StencilDepthFailOp = ToNativeStencilOp( state.StencilDepthFailOp );
    result.StencilPassOp      = ToNativeStencilOp( state.StencilPassOp );
    result.StencilFunc        = ToNativeComparisonFunc( state.StencilCompareOp );
}

//-------------------------------------------------------------------------------------------------
//      深度ステートをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeDepthDesc( const a3d::DepthState& ds, D3D12_DEPTH_STENCIL_DESC& result )
{
    result.DepthEnable      = ( ds.DepthTestEnable ) ? TRUE : FALSE;
    result.DepthWriteMask   = ( ds.DepthWriteEnable ) ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    result.DepthFunc        = a3d::ToNativeComparisonFunc( ds.DepthCompareOp );
}

//-------------------------------------------------------------------------------------------------
//      ステンシルステートをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeStencilDesc(const a3d::StencilState& ss, D3D12_DEPTH_STENCIL_DESC& result)
{
    result.StencilEnable    = ( ss.StencilTestEnable ) ? TRUE : FALSE;
    result.StencilReadMask  = ss.StencllReadMask;
    result.StencilWriteMask = ss.StencilWriteMask;
    ToNativeDepthStencilOpDesc( ss.FrontFace, result.FrontFace );
    ToNativeDepthStencilOpDesc( ss.BackFace,  result.BackFace );
}

//-------------------------------------------------------------------------------------------------
//      入力分類をネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_INPUT_CLASSIFICATION ToNativeInputClassification( const a3d::INPUT_CLASSIFICATION& classification )
{
    D3D12_INPUT_CLASSIFICATION table[] = {
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
        D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA
    };

    return table[classification];
}

//-------------------------------------------------------------------------------------------------
//     入力要素設定をネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeInputElementDesc
(
    const a3d::InputElementDesc&    element,
    D3D12_INPUT_ELEMENT_DESC&       result
)
{
    result.SemanticName         = element.SemanticName;
    result.SemanticIndex        = element.SemanticIndex;
    result.Format               = a3d::ToNativeFormat( element.Format );
    result.InputSlot            = element.StreamIndex;
    result.AlignedByteOffset    = element.OffsetInBytes;
    result.InputSlotClass       = ToNativeInputClassification(element.InputClass);
    result.InstanceDataStepRate = 0;
}

//-------------------------------------------------------------------------------------------------
//      マルチサンプルステートをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeSampleDesc( const a3d::MultiSampleState& state, DXGI_SAMPLE_DESC& result )
{
    result.Count   = state.SampleCount;
    result.Quality = 0;
}

} // namespace /* anonymous */

namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// PipelineState class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
PipelineState::PipelineState()
: m_RefCount            (1)
, m_pDevice             (nullptr)
, m_pPipelineState      (nullptr)
, m_PrimitiveTopology   (D3D_PRIMITIVE_TOPOLOGY_UNDEFINED)
, m_pLayout             (nullptr)
, m_Type                (PIPELINE_STATE_TYPE_GRAPHICS)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
PipelineState::~PipelineState()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void PipelineState::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解法処理を行います.
//-------------------------------------------------------------------------------------------------
void PipelineState::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t PipelineState::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void PipelineState::SetName(const char* name)
{
    m_Name = name;
    m_pPipelineState->SetPrivateData(WKPDID_D3DDebugObjectName, m_Name.size(), m_Name.c_str());
}

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* PipelineState::GetName() const
{ return m_Name.c_str(); }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void PipelineState::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      パイプラインステートタイプを取得します.
//-------------------------------------------------------------------------------------------------
PIPELINE_STATE_TYPE PipelineState::GetType() const
{ return m_Type; }

//-------------------------------------------------------------------------------------------------
//      コマンドを発行します.
//-------------------------------------------------------------------------------------------------
void PipelineState::Issue(ICommandList* pCommandList)
{
    auto pWrapCommandList = static_cast<CommandList*>(pCommandList);
    A3D_ASSERT(pWrapCommandList != nullptr);

    auto pNativeCommandList = pWrapCommandList->GetD3D12GraphicsCommandList();
    A3D_ASSERT(pNativeCommandList != nullptr);

    if (IsGraphics())
    { pNativeCommandList->SetGraphicsRootSignature(m_pLayout->GetD3D12RootSignature()); }
    else if (m_Type == PIPELINE_STATE_TYPE_COMPUTE)
    { pNativeCommandList->SetComputeRootSignature(m_pLayout->GetD3D12RootSignature()); }

    pNativeCommandList->SetPipelineState(m_pPipelineState);

    if (m_Type == PIPELINE_STATE_TYPE_GRAPHICS)
    { pNativeCommandList->IASetPrimitiveTopology(m_PrimitiveTopology); }
}

//-------------------------------------------------------------------------------------------------
//      グラフィックスパイプラインステートとして初期化します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::InitAsGraphics(IDevice* pDevice, const GraphicsPipelineStateDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    if (pDesc->pLayout == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    m_Type = PIPELINE_STATE_TYPE_GRAPHICS;

    m_pLayout = static_cast<DescriptorSetLayout*>(pDesc->pLayout);
    m_pLayout->AddRef();

    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    D3D12_INPUT_ELEMENT_DESC elementDesc[D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT] = {};

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature     = m_pLayout->GetD3D12RootSignature();
    ToNativeShaderByteCode  ( pDesc->VS                , desc.VS );
    ToNativeShaderByteCode  ( pDesc->PS                , desc.PS );
    ToNativeShaderByteCode  ( pDesc->DS                , desc.DS );
    ToNativeShaderByteCode  ( pDesc->HS                , desc.HS );
    //ToNativeShaderByteCode  ( pDesc->GS                , desc.GS );
    ToNativeBlendDesc       ( pDesc->BlendState        , desc.BlendState, pDesc->MultiSampleState.EnableAlphaToCoverage );
    ToNativeRasterizerDesc  ( pDesc->RasterizerState   , desc.RasterizerState );
    ToNativeDepthDesc       ( pDesc->DepthState        , desc.DepthStencilState );
    ToNativeStencilDesc     ( pDesc->StencilState      , desc.DepthStencilState );
    ToNativeSampleDesc      ( pDesc->MultiSampleState  , desc.SampleDesc );
    desc.SampleMask         = UINT32_MAX;

    auto idx = 0;
    for(auto i=0u; i<pDesc->InputLayout.ElementCount; ++i)
    {
        ToNativeInputElementDesc( pDesc->InputLayout.pElements[i], elementDesc[idx] );
        idx++;
    }

    desc.InputLayout.NumElements         = idx;
    desc.InputLayout.pInputElementDescs  = elementDesc;
    desc.IBStripCutValue                 = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    desc.PrimitiveTopologyType           = a3d::ToNativePrimitiveTopology( pDesc->PrimitiveTopology );
    desc.NumRenderTargets                = pDesc->RenderTargetCount;

    for (auto i=0u; i<pDesc->RenderTargetCount; ++i)
    { desc.RTVFormats[i] = a3d::ToNativeFormat(pDesc->RenderTarget[i]); }
    desc.DSVFormat = a3d::ToNativeFormat(pDesc->DepthTarget);

    auto hr = pNativeDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_pPipelineState));
    if ( FAILED(hr) )
    {
        A3D_LOG("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
        return false;
    }

    m_PrimitiveTopology = a3d::ToNativePrimitive( 
        pDesc->PrimitiveTopology,
        pDesc->TessellationState.PatchControlCount );

    return true;
}

//-------------------------------------------------------------------------------------------------
//      コンピュートパイプラインステートとして初期化します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::InitAsCompute(IDevice* pDevice, const ComputePipelineStateDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    m_Type = PIPELINE_STATE_TYPE_COMPUTE;

    m_pLayout = static_cast<DescriptorSetLayout*>(pDesc->pLayout);
    m_pLayout->AddRef();

    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    auto pWrapDescriptorLayout = static_cast<DescriptorSetLayout*>(pDesc->pLayout);
    A3D_ASSERT(pWrapDescriptorLayout != nullptr);

    D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature = pWrapDescriptorLayout->GetD3D12RootSignature();
    ToNativeShaderByteCode   ( pDesc->CS, desc.CS );

    auto hr = pNativeDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(&m_pPipelineState));
    if ( FAILED(hr) )
    {
        A3D_LOG("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      メッシュレットパイプラインステートとして初期化します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::InitAsMesh(IDevice* pDevice, const MeshletPipelineStateDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto pWrapDevice = static_cast<Device*>(pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    // レイトレ機能が有効化チェック.
    if (!m_pDevice->GetInfo().SupportMeshShader)
    {
        A3D_LOG("Error : Mesh Shader feature is not supported by hardware.");
        return false;
    }

    Term();

    m_pDevice = pWrapDevice;
    m_pDevice->AddRef();

    m_Type = PIPELINE_STATE_TYPE_MESHLET;

    m_pLayout = static_cast<DescriptorSetLayout*>(pDesc->pLayout);
    m_pLayout->AddRef();

    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    auto pWrapDescriptorLayout = static_cast<DescriptorSetLayout*>(pDesc->pLayout);
    A3D_ASSERT(pWrapDescriptorLayout != nullptr);

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = pDesc->RenderTargetCount;
    for (auto i=0u; i<pDesc->RenderTargetCount; ++i)
    { rtvFormats.RTFormats[i] = a3d::ToNativeFormat(pDesc->RenderTarget[i]); }

    D3D12_GEOMETRY_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.RootSignature = pWrapDescriptorLayout->GetD3D12RootSignature();
    ToNativeShaderByteCode( pDesc->AS, psoDesc.AS );
    ToNativeShaderByteCode( pDesc->MS, psoDesc.MS );
    ToNativeShaderByteCode( pDesc->PS, psoDesc.PS );
    ToNativeBlendDesc     ( pDesc->BlendState        , psoDesc.BlendState,  pDesc->MultiSampleState.EnableAlphaToCoverage);
    ToNativeRasterizerDesc( pDesc->RasterizerState   , psoDesc.RasterizerState );
    ToNativeDepthDesc     ( pDesc->DepthState        , psoDesc.DepthStencilState );
    ToNativeStencilDesc   ( pDesc->StencilState      , psoDesc.DepthStencilState );
    ToNativeSampleDesc    ( pDesc->MultiSampleState  , psoDesc.SampleDesc );
    psoDesc.RTVFormats    = rtvFormats;
    psoDesc.DSVFormat     = a3d::ToNativeFormat(pDesc->DepthTarget);
    psoDesc.SampleMask    = UINT32_MAX;

    D3D12_PIPELINE_STATE_STREAM_DESC pssDesc = {};
    pssDesc.SizeInBytes                     = sizeof(psoDesc);
    pssDesc.pPipelineStateSubobjectStream   = &psoDesc;

    // パイプラインステート生成.
    auto hr = pNativeDevice->CreatePipelineState(&pssDesc, IID_PPV_ARGS(&m_pPipelineState));
    if (FAILED(hr))
    {
        A3D_LOG("Error : ID3D12Device::CreatePipelineState8) Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void PipelineState::Term()
{
    SafeRelease(m_pPipelineState);
    SafeRelease(m_pLayout);
    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      グラフィクスパイプラインかどうか?
//-------------------------------------------------------------------------------------------------
bool PipelineState::IsGraphics() const
{ return m_Type == PIPELINE_STATE_TYPE_GRAPHICS || m_Type == PIPELINE_STATE_TYPE_MESHLET; }

//-------------------------------------------------------------------------------------------------
//      ディスクリプタセットレイアウトを取得します.
//-------------------------------------------------------------------------------------------------
DescriptorSetLayout* PipelineState::GetDescriptorSetLayout() const
{ return m_pLayout; }

//-------------------------------------------------------------------------------------------------
//      グラフィックスパイプラインステートとして生成します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::CreateAsGraphics
(
    IDevice*                         pDevice,
    const GraphicsPipelineStateDesc* pDesc,
    IPipelineState**                 ppPipelineState
)
{
    if (pDevice == nullptr || pDesc == nullptr || ppPipelineState == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new PipelineState;
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if (!instance->InitAsGraphics(pDevice, pDesc))
    {
        SafeRelease(instance);
        A3D_LOG("Error : InitAsGraphics() Failed.");
        return false;
    }

    *ppPipelineState = instance;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      コンピュートパイプラインステートとして生成します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::CreateAsCompute
(
    IDevice*                        pDevice,
    const ComputePipelineStateDesc* pDesc,
    IPipelineState**                ppPipelineState
)
{
    if (pDevice == nullptr || pDesc == nullptr || ppPipelineState == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new PipelineState;
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if (!instance->InitAsCompute(pDevice, pDesc))
    {
        SafeRelease(instance);
        A3D_LOG("Error : InitAsCompute() Failed.");
        return false;
    }

    *ppPipelineState = instance;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      メッシュシェーダパイプラインステートとして生成します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::CreateAsMesh
(
    IDevice*                        pDevice,
    const MeshletPipelineStateDesc* pDesc,
    IPipelineState**                ppPipelineState
)
{
    if (pDevice == nullptr || pDesc == nullptr || ppPipelineState == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new PipelineState;
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if (!instance->InitAsMesh(pDevice, pDesc))
    {
        SafeRelease(instance);
        A3D_LOG("Error : InitAsMesh() Failed.");
        return false;
    }

    *ppPipelineState = instance;
    return true;
}

} // namespace a3d
