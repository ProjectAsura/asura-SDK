//-------------------------------------------------------------------------------------------------
// File : a3dPipelineState.cpp
// Desc : Pipeline State Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

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
//      論理オペレータをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_LOGIC_OP ToNativeLogicOp( const a3d::LOGIC_OP& operation )
{
    D3D12_LOGIC_OP table[] = {
        D3D12_LOGIC_OP_CLEAR,
        D3D12_LOGIC_OP_SET,
        D3D12_LOGIC_OP_COPY,
        D3D12_LOGIC_OP_COPY_INVERTED,
        D3D12_LOGIC_OP_NOOP,
        D3D12_LOGIC_OP_INVERT,
        D3D12_LOGIC_OP_AND,
        D3D12_LOGIC_OP_NAND,
        D3D12_LOGIC_OP_OR,
        D3D12_LOGIC_OP_NOR,
        D3D12_LOGIC_OP_XOR,
        D3D12_LOGIC_OP_EQUIV,
        D3D12_LOGIC_OP_AND_REVERSE,
        D3D12_LOGIC_OP_AND_INVERTED,
        D3D12_LOGIC_OP_OR_REVERSE,
        D3D12_LOGIC_OP_INVERT
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
void ToNativeBlendDesc( const a3d::BlendState& state, D3D12_BLEND_DESC& result )
{
    result.AlphaToCoverageEnable = FALSE;
    result.IndependentBlendEnable = (state.IndependentBlendEnable) ? TRUE : FALSE;
    for(auto i=0; i<8; ++i)
    {
        result.RenderTarget[i].LogicOpEnable = (state.LogicOpEnable) ? TRUE : FALSE;
        result.RenderTarget[i].LogicOp       = ToNativeLogicOp(state.LogicOp);
        ToNativeRanderTargetBlendDesc( state.ColorTarget[i], result.RenderTarget[i] );
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
void ToNativeDepthStencilOpDesc( const a3d::StencilState& state, D3D12_DEPTH_STENCILOP_DESC& result )
{
    result.StencilFailOp      = ToNativeStencilOp( state.StencilFailOp );
    result.StencilDepthFailOp = ToNativeStencilOp( state.StencilDepthFailOp );
    result.StencilPassOp      = ToNativeStencilOp( state.StencilPassOp );
    result.StencilFunc        = ToNativeComparisonFunc( state.StencilCompareOp );
}

//-------------------------------------------------------------------------------------------------
//      深度ステンシルステートをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeDepthStencilDesc( const a3d::DepthStencilState& state, D3D12_DEPTH_STENCIL_DESC& result )
{
    result.DepthEnable      = ( state.DepthTestEnable ) ? TRUE : FALSE;
    result.DepthWriteMask   = ( state.DepthWriteEnable ) ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    result.DepthFunc        = a3d::ToNativeComparisonFunc( state.DepthCompareOp );
    result.StencilEnable    = ( state.StencilTestEnable ) ? TRUE : FALSE;
    result.StencilReadMask  = state.StencllReadMask;
    result.StencilWriteMask = state.StencilWriteMask;
    ToNativeDepthStencilOpDesc( state.FrontFace, result.FrontFace );
    ToNativeDepthStencilOpDesc( state.BackFace,  result.BackFace );
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
    uint32_t                    index,
    const a3d::InputStreamDesc& state,
    D3D12_INPUT_ELEMENT_DESC&   result
)
{
    A3D_ASSERT( index < state.ElementCount );
    auto &element = state.pElements[index];

    result.SemanticName         = element.SemanticName;
    result.SemanticIndex        = element.SemanticIndex;
    result.Format               = a3d::ToNativeFormat( element.Format );
    result.InputSlot            = state.StreamIndex;
    result.AlignedByteOffset    = element.OffsetInBytes;
    result.InputSlotClass       = ToNativeInputClassification(state.InputClass);
}

//-------------------------------------------------------------------------------------------------
//      マルチサンプルステートをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeSampleDesc( const a3d::MultiSampleState& state, DXGI_SAMPLE_DESC& result )
{
    result.Count   = state.SampleCount;
    result.Quality = 0;
}

//-------------------------------------------------------------------------------------------------
//      パイプラインステートキャッシュをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativePipelieStateCache( const a3d::IBlob* pBlob, D3D12_CACHED_PIPELINE_STATE& state )
{
    state.CachedBlobSizeInBytes = (pBlob != nullptr) ? SIZE_T(pBlob->GetBufferSize()) : 0;
    state.pCachedBlob           = (pBlob != nullptr) ? pBlob->GetBufferPointer() : nullptr;
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
, m_IsGraphicsPipeline  (true)
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
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void PipelineState::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      キャッシュデータを取得します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::GetCachedBlob(IBlob** ppBlob)
{
    ID3DBlob* pD3DBlob;
    auto hr = m_pPipelineState->GetCachedBlob(&pD3DBlob);
    if ( FAILED(hr) )
    {
        SafeRelease(pD3DBlob);
        return false;
    }

    if (!Blob::Create(pD3DBlob->GetBufferSize(), ppBlob))
    {
        SafeRelease(pD3DBlob);
        return false;
    }

    auto ptr = (*ppBlob)->GetBufferPointer();
    memcpy( ptr, pD3DBlob->GetBufferPointer(), pD3DBlob->GetBufferSize() );

    SafeRelease(pD3DBlob);
    return true;
}

//-------------------------------------------------------------------------------------------------
//      コマンドを発行します.
//-------------------------------------------------------------------------------------------------
void PipelineState::Issue(ICommandList* pCommandList)
{
    auto pWrapCommandList = static_cast<CommandList*>(pCommandList);
    A3D_ASSERT(pWrapCommandList != nullptr);

    auto pNativeCommandList = pWrapCommandList->GetD3D12GraphicsCommandList();
    A3D_ASSERT(pNativeCommandList != nullptr);

    pNativeCommandList->SetPipelineState(m_pPipelineState);

    if (m_IsGraphicsPipeline)
    { pNativeCommandList->IASetPrimitiveTopology(m_PrimitiveTopology); }
}

//-------------------------------------------------------------------------------------------------
//      グラフィックスパイプラインステートとして初期化します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::InitAsGraphics(IDevice* pDevice, const GraphicsPipelineStateDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    if (pDesc->pFrameBuffer == nullptr || pDesc->pLayout == nullptr)
    { return false; }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    m_IsGraphicsPipeline = true;

    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    auto pWrapDescriptorLayout = static_cast<DescriptorSetLayout*>(pDesc->pLayout);
    A3D_ASSERT(pWrapDescriptorLayout != nullptr);

    auto pWrapFrameBuffer = static_cast<FrameBuffer*>(pDesc->pFrameBuffer);
    A3D_ASSERT(pWrapFrameBuffer != nullptr);

    D3D12_INPUT_ELEMENT_DESC elementDesc[D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT] = {};

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature     = pWrapDescriptorLayout->GetD3D12RootSignature();
    ToNativeShaderByteCode  ( pDesc->VertexShader      , desc.VS );
    ToNativeShaderByteCode  ( pDesc->PixelShader       , desc.PS );
    ToNativeShaderByteCode  ( pDesc->DomainShader      , desc.DS );
    ToNativeShaderByteCode  ( pDesc->HullShader        , desc.HS );
    ToNativeShaderByteCode  ( pDesc->GeometryShader    , desc.GS );
    ToNativeBlendDesc       ( pDesc->BlendState        , desc.BlendState );
    ToNativeRasterizerDesc  ( pDesc->RasterizerState   , desc.RasterizerState );
    ToNativeDepthStencilDesc( pDesc->DepthStencilState , desc.DepthStencilState );
    ToNativeSampleDesc      ( pDesc->MultiSampleState  , desc.SampleDesc );
    desc.BlendState.AlphaToCoverageEnable = (pDesc->MultiSampleState.EnableAlphaToCoverage) ? TRUE : FALSE;
    desc.SampleMask                       = UINT32_MAX;

    auto idx = 0;
    for(auto i=0u; i<pDesc->InputLayout.StreamCount; ++i)
    {
        for(auto j=0u; j<pDesc->InputLayout.pStreams[i].ElementCount; ++j)
        {
            ToNativeInputElementDesc( j, pDesc->InputLayout.pStreams[i], elementDesc[idx] );
            idx++;
        }
    }

    desc.InputLayout.NumElements         = idx;
    desc.InputLayout.pInputElementDescs  = elementDesc;
    desc.IBStripCutValue                 = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    desc.PrimitiveTopologyType           = a3d::ToNativePrimitiveTopology( pDesc->PrimitiveTopology );
    desc.NumRenderTargets                = pWrapFrameBuffer->GetColorCount();
    ToNativePipelieStateCache( pDesc->pCachedPSO, desc.CachedPSO );

    memcpy(desc.RTVFormats, pWrapFrameBuffer->GetD3D12RenderTargetViewFormats(), pWrapFrameBuffer->GetColorCount() );
    desc.DSVFormat = pWrapFrameBuffer->GetD3D12DepthStencilViewFormat();

    auto hr = pNativeDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_pPipelineState));
    if ( FAILED(hr) )
    { return false; }

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
    { return false; }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    m_IsGraphicsPipeline = false;

    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    auto pWrapDescriptorLayout = static_cast<DescriptorSetLayout*>(pDesc->pLayout);
    A3D_ASSERT(pWrapDescriptorLayout != nullptr);

    D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature = pWrapDescriptorLayout->GetD3D12RootSignature();
    ToNativeShaderByteCode   ( pDesc->ComputeShader, desc.CS );
    ToNativePipelieStateCache( pDesc->pCachedPSO, desc.CachedPSO );

    auto hr = pNativeDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(&m_pPipelineState));
    if ( FAILED(hr) )
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void PipelineState::Term()
{
    SafeRelease(m_pPipelineState);
    SafeRelease(m_pDevice);
}

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
    { return false; }

    auto instance = new PipelineState;
    if (instance == nullptr)
    { return false; }

    if (!instance->InitAsGraphics(pDevice, pDesc))
    {
        SafeRelease(instance);
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
    { return false; }

    auto instance = new PipelineState;
    if (instance == nullptr)
    { return false; }

    if (!instance->InitAsCompute(pDevice, pDesc))
    {
        SafeRelease(instance);
        return false;
    }

    *ppPipelineState = instance;
    return true;
}

} // namespace a3d
