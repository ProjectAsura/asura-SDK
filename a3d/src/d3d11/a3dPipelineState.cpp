﻿//-------------------------------------------------------------------------------------------------
// File : a3dPipelineState.cpp
// Desc : Pipeline State Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------



namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      ブレンドファクターをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D11_BLEND ToNativeBlend( const a3d::BLEND_FACTOR& factor )
{
    D3D11_BLEND table[] = {
        D3D11_BLEND_ZERO,
        D3D11_BLEND_ONE,
        D3D11_BLEND_SRC_COLOR,
        D3D11_BLEND_INV_SRC_COLOR,
        D3D11_BLEND_SRC_ALPHA,
        D3D11_BLEND_INV_SRC_ALPHA,
        D3D11_BLEND_DEST_ALPHA,
        D3D11_BLEND_INV_DEST_ALPHA,
        D3D11_BLEND_DEST_COLOR,
        D3D11_BLEND_INV_DEST_COLOR,
        D3D11_BLEND_SRC_ALPHA_SAT,
        D3D11_BLEND_SRC1_COLOR,
        D3D11_BLEND_INV_SRC1_COLOR,
        D3D11_BLEND_SRC1_ALPHA,
        D3D11_BLEND_INV_SRC1_ALPHA
    };

    return table[factor];
}

//-------------------------------------------------------------------------------------------------
//      ブレンドオペレータをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D11_BLEND_OP ToNativeBlendOp( const a3d::BLEND_OP& operation )
{
    D3D11_BLEND_OP table[] = {
        D3D11_BLEND_OP_ADD,
        D3D11_BLEND_OP_SUBTRACT,
        D3D11_BLEND_OP_REV_SUBTRACT,
        D3D11_BLEND_OP_MIN,
        D3D11_BLEND_OP_MAX
    };

    return table[operation];
}

//-------------------------------------------------------------------------------------------------
//      論理オペレータをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D11_LOGIC_OP ToNativeLogicOp( const a3d::LOGIC_OP& operation )
{
    D3D11_LOGIC_OP table[] = {
        D3D11_LOGIC_OP_CLEAR,
        D3D11_LOGIC_OP_SET,
        D3D11_LOGIC_OP_COPY,
        D3D11_LOGIC_OP_COPY_INVERTED,
        D3D11_LOGIC_OP_NOOP,
        D3D11_LOGIC_OP_INVERT,
        D3D11_LOGIC_OP_AND,
        D3D11_LOGIC_OP_NAND,
        D3D11_LOGIC_OP_OR,
        D3D11_LOGIC_OP_NOR,
        D3D11_LOGIC_OP_XOR,
        D3D11_LOGIC_OP_EQUIV,
        D3D11_LOGIC_OP_AND_REVERSE,
        D3D11_LOGIC_OP_AND_INVERTED,
        D3D11_LOGIC_OP_OR_REVERSE,
        D3D11_LOGIC_OP_INVERT
    };

    return table[operation];
}

//-------------------------------------------------------------------------------------------------
//      カラーブレンドステートをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeRanderTargetBlendDesc( const a3d::ColorBlendState& state, D3D11_RENDER_TARGET_BLEND_DESC& result )
{
    result.BlendEnable      = (state.BlendEnable)   ? TRUE : FALSE;
    result.SrcBlend         = ToNativeBlend  ( state.SrcBlend );
    result.DestBlend        = ToNativeBlend  ( state.DstBlend );
    result.BlendOp          = ToNativeBlendOp( state.BlendOp );
    result.SrcBlendAlpha    = ToNativeBlend  ( state.SrcBlendAlpha );
    result.DestBlendAlpha   = ToNativeBlend  ( state.DstBlendAlpha );
    result.BlendOpAlpha     = ToNativeBlendOp( state.BlendOpAlpha );
    result.RenderTargetWriteMask = 0;

    if (state.EnableWriteR) { result.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_RED;   }
    if (state.EnableWriteG) { result.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_GREEN; }
    if (state.EnableWriteB) { result.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_BLUE;  }
    if (state.EnableWriteA) { result.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA; }
}

//-------------------------------------------------------------------------------------------------
//      ブレンドステートをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeBlendDesc( const a3d::BlendState& state, D3D11_BLEND_DESC& result )
{
    result.AlphaToCoverageEnable = FALSE;
    result.IndependentBlendEnable = (state.IndependentBlendEnable) ? TRUE : FALSE;
    for(auto i=0; i<8; ++i)
    {
    #if 0 // 低スペックPCでも動くようにしたいので非サポート.
        //result.RenderTarget[i].LogicOpEnable = (state.LogicOpEnable) ? TRUE : FALSE;
        //result.RenderTarget[i].LogicOp       = ToNativeLogicOp(state.LogicOp);
    #endif
        ToNativeRanderTargetBlendDesc( state.ColorTarget[i], result.RenderTarget[i] );
    }
}

//-------------------------------------------------------------------------------------------------
//      ポリゴンモードをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D11_FILL_MODE ToNativeFillMode( const a3d::POLYGON_MODE& mode )
{
    D3D11_FILL_MODE table[] = {
        D3D11_FILL_SOLID,
        D3D11_FILL_WIREFRAME
    };

    return table[mode];
}

//-------------------------------------------------------------------------------------------------
//      カリングモードをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D11_CULL_MODE ToNativeCullMode( const a3d::CULL_MODE& mode )
{
    D3D11_CULL_MODE table[] = {
        D3D11_CULL_NONE,
        D3D11_CULL_FRONT,
        D3D11_CULL_BACK
    };

    return table[mode];
}

//-------------------------------------------------------------------------------------------------
//      ラスタライザ―ステートをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeRasterizerDesc( const a3d::RasterizerState& state, D3D11_RASTERIZER_DESC& result )
{
    result.FillMode                 = ToNativeFillMode( state.PolygonMode );
    result.CullMode                 = ToNativeCullMode( state.CullMode );
    result.FrontCounterClockwise    = ( state.FrontCounterClockWise ) ? TRUE : FALSE;
    result.DepthBias                = state.DepthBias;
    result.DepthBiasClamp           = state.DepthBiasClamp;
    result.SlopeScaledDepthBias     = state.SlopeScaledDepthBais;
    result.DepthClipEnable          = ( state.DepthClipEnable ) ? TRUE : FALSE;
#if 0 // 低スペックPCでも動くようにしたいので非サポート.
    //result.ConservativeRaster       = ( state.EnableConservativeRaster ) 
    //                                  ? D3D11_CONSERVATIVE_RASTERIZATION_MODE_ON
    //                                  : D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF;
#endif
}

//-------------------------------------------------------------------------------------------------
//      ステンシルオペレータをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D11_STENCIL_OP ToNativeStencilOp( const a3d::STENCIL_OP& operation )
{
    D3D11_STENCIL_OP table[] = {
        D3D11_STENCIL_OP_KEEP,
        D3D11_STENCIL_OP_ZERO,
        D3D11_STENCIL_OP_REPLACE,
        D3D11_STENCIL_OP_INCR_SAT,
        D3D11_STENCIL_OP_DECR_SAT,
        D3D11_STENCIL_OP_INVERT,
        D3D11_STENCIL_OP_INCR,
        D3D11_STENCIL_OP_DECR
    };

    return table[operation];
}

//-------------------------------------------------------------------------------------------------
//      ステンシルステートをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeDepthStencilOpDesc( const a3d::StencilState& state, D3D11_DEPTH_STENCILOP_DESC& result )
{
    result.StencilFailOp      = ToNativeStencilOp( state.StencilFailOp );
    result.StencilDepthFailOp = ToNativeStencilOp( state.StencilDepthFailOp );
    result.StencilPassOp      = ToNativeStencilOp( state.StencilPassOp );
    result.StencilFunc        = ToNativeComparisonFunc( state.StencilCompareOp );
}

//-------------------------------------------------------------------------------------------------
//      深度ステンシルステートをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeDepthStencilDesc( const a3d::DepthStencilState& state, D3D11_DEPTH_STENCIL_DESC& result )
{
    result.DepthEnable      = ( state.DepthTestEnable ) ? TRUE : FALSE;
    result.DepthWriteMask   = ( state.DepthWriteEnable ) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
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
D3D11_INPUT_CLASSIFICATION ToNativeInputClassification( const a3d::INPUT_CLASSIFICATION& classification )
{
    D3D11_INPUT_CLASSIFICATION table[] = {
        D3D11_INPUT_PER_VERTEX_DATA,
        D3D11_INPUT_PER_INSTANCE_DATA
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
    D3D11_INPUT_ELEMENT_DESC&   result
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
    A3D_UNUSED(ppBlob);
    return true;
}

//-------------------------------------------------------------------------------------------------
//      パイプラインステートを関連付けます.
//-------------------------------------------------------------------------------------------------
void PipelineState::Bind
(
    ID3D11DeviceContext*    pDeviceContext,
    const float             blendFactor[4],
    uint32_t                stencilRef
)
{
    if (m_IsGraphicsPipeline)
    {
        const uint32_t sampleMask = 0xffffffff;
        pDeviceContext->RSSetState(m_pRS);
        pDeviceContext->OMSetBlendState(m_pBS, blendFactor, sampleMask);
        pDeviceContext->OMSetDepthStencilState(m_pDSS, stencilRef);

        pDeviceContext->IASetInputLayout(m_pIL);
        pDeviceContext->VSSetShader(m_pVS, nullptr, 0);
        pDeviceContext->DSSetShader(m_pDS, nullptr, 0);
        pDeviceContext->GSSetShader(m_pGS, nullptr, 0);
        pDeviceContext->HSSetShader(m_pHS, nullptr, 0);
        pDeviceContext->PSSetShader(m_pPS, nullptr, 0);
        pDeviceContext->CSSetShader(nullptr, nullptr, 0);
        pDeviceContext->IASetPrimitiveTopology(m_Topology);
    }
    else
    {
        pDeviceContext->VSSetShader(nullptr, nullptr, 0);
        pDeviceContext->DSSetShader(nullptr, nullptr, 0);
        pDeviceContext->GSSetShader(nullptr, nullptr, 0);
        pDeviceContext->HSSetShader(nullptr, nullptr, 0);
        pDeviceContext->PSSetShader(nullptr, nullptr, 0);
        pDeviceContext->CSSetShader(m_pCS, nullptr, 0);
    }
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

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    m_IsGraphicsPipeline = true;

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pD3D11Device = pWrapDevice->GetD3D11Device();
    A3D_ASSERT(pD3D11Device != nullptr);

    // 頂点シェーダ.
    if (pDesc->VertexShader.pByteCode != nullptr && pDesc->VertexShader.ByteCodeSize > 0)
    {
        auto hr = pD3D11Device->CreateVertexShader(
            pDesc->VertexShader.pByteCode,
            pDesc->VertexShader.ByteCodeSize,
            nullptr,
            &m_pVS);
        if (FAILED(hr))
        { return false; }
    }
 
    // ピクセルシェーダ.
    if (pDesc->PixelShader.pByteCode != nullptr && pDesc->PixelShader.ByteCodeSize > 0)
    {
        auto hr = pD3D11Device->CreatePixelShader(
            pDesc->PixelShader.pByteCode,
            pDesc->PixelShader.ByteCodeSize,
            nullptr,
            &m_pPS);
        if (FAILED(hr))
        { return false; }
    }

    // ドメインシェーダ.
    if (pDesc->DomainShader.pByteCode != nullptr && pDesc->DomainShader.ByteCodeSize > 0)
    {
        auto hr = pD3D11Device->CreateDomainShader(
            pDesc->DomainShader.pByteCode,
            pDesc->DomainShader.ByteCodeSize,
            nullptr,
            &m_pDS);
        if (FAILED(hr))
        { return false; }
    }

    // ハルシェーダ.
    if (pDesc->HullShader.pByteCode != nullptr && pDesc->HullShader.ByteCodeSize > 0)
    {
        auto hr = pD3D11Device->CreateHullShader(
            pDesc->HullShader.pByteCode,
            pDesc->HullShader.ByteCodeSize,
            nullptr,
            &m_pHS);
        if (FAILED(hr))
        { return false; }
    }

    // ジオメトリシェーダ
    if (pDesc->GeometryShader.pByteCode != nullptr && pDesc->GeometryShader.ByteCodeSize > 0)
    {
        auto hr = pD3D11Device->CreateGeometryShader(
            pDesc->GeometryShader.pByteCode,
            pDesc->GeometryShader.ByteCodeSize,
            nullptr,
            &m_pGS);
        if (FAILED(hr))
        { return false; }
    }

    // ラスタライザ―ステート.
    {
        D3D11_RASTERIZER_DESC desc = {};
        ToNativeRasterizerDesc(pDesc->RasterizerState, desc);

        auto hr = pD3D11Device->CreateRasterizerState(&desc, &m_pRS);
        if (FAILED(hr))
        { return false; }
    }

    // ブレンドステート.
    {
        D3D11_BLEND_DESC desc = {};
        ToNativeBlendDesc(pDesc->BlendState, desc);
        desc.AlphaToCoverageEnable = (pDesc->MultiSampleState.EnableAlphaToCoverage) ? TRUE : FALSE;

        auto hr = pD3D11Device->CreateBlendState(&desc, &m_pBS);
        if (FAILED(hr))
        { return false; }
    }

    // 深度ステンシルステート.
    {
        D3D11_DEPTH_STENCIL_DESC desc = {};
        ToNativeDepthStencilDesc(pDesc->DepthStencilState, desc);

        auto hr = pD3D11Device->CreateDepthStencilState(&desc, &m_pDSS);
        if (FAILED(hr))
        { return false; }
    }

    // 入力レイアウト.
    if (pDesc->VertexShader.pByteCode != nullptr && pDesc->VertexShader.ByteCodeSize > 0)
    {
        D3D11_INPUT_ELEMENT_DESC elementDesc[D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT];
        memset( elementDesc, 0, sizeof(elementDesc) );

        auto idx = 0;
        for(auto i=0u; i<pDesc->InputLayout.StreamCount; ++i)
        {
            for(auto j=0u; j<pDesc->InputLayout.pStreams[i].ElementCount; ++j)
            {
                ToNativeInputElementDesc( j, pDesc->InputLayout.pStreams[i], elementDesc[idx] );
                idx++;
            }
        }

        auto hr = pD3D11Device->CreateInputLayout(
            elementDesc,
            idx,
            pDesc->VertexShader.pByteCode,
            pDesc->VertexShader.ByteCodeSize,
            &m_pIL);
        if ( FAILED(hr) )
        { return false; }
    }

    m_Topology = ToNativePrimitive( pDesc->PrimitiveTopology, pDesc->TessellationState.PatchControlCount );

    return true;
}

//-------------------------------------------------------------------------------------------------
//      コンピュートパイプラインステートとして初期化します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::InitAsCompute(IDevice* pDevice, const ComputePipelineStateDesc* pDesc)
{
    A3D_UNUSED(pDesc);

    if (pDevice == nullptr || pDesc == nullptr ||
        pDesc->ComputeShader.pByteCode == nullptr ||
        pDesc->ComputeShader.ByteCodeSize == 0)
    { return false; }

    Term();

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    m_IsGraphicsPipeline = false;

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pD3D11Device = pWrapDevice->GetD3D11Device();
    A3D_ASSERT(pD3D11Device != nullptr);

    {
        auto hr = pD3D11Device->CreateComputeShader(
            pDesc->ComputeShader.pByteCode,
            pDesc->ComputeShader.ByteCodeSize,
            nullptr,
            &m_pCS);
        if (FAILED(hr))
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void PipelineState::Term()
{
    SafeRelease(m_pVS);
    SafeRelease(m_pDS);
    SafeRelease(m_pGS);
    SafeRelease(m_pHS);
    SafeRelease(m_pPS);
    SafeRelease(m_pCS);
    SafeRelease(m_pRS);
    SafeRelease(m_pBS);
    SafeRelease(m_pDSS);
    SafeRelease(m_pIL);
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