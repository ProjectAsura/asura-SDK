//-------------------------------------------------------------------------------------------------
// File : a3dPipelineState.cpp
// Desc : Pipeline State Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      シェーダステージ情報に変換します.
//-------------------------------------------------------------------------------------------------
bool ToNativeShaderStageInfo
(
    VkDevice                            device,
    const a3d::ShaderBinary&            binary,
    VkShaderStageFlagBits               stage,
    VkPipelineShaderStageCreateInfo*    pInfo
)
{
    VkShaderModuleCreateInfo info = {};
    info.sType      = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.pNext      = nullptr;
    info.flags      = 0;
    info.pCode      = static_cast<uint32_t*>(binary.pByteCode);
    info.codeSize   = binary.ByteCodeSize;

    auto ret = vkCreateShaderModule(device, &info, nullptr, &pInfo->module);
    if ( ret != VK_SUCCESS )
    { return false; }

    pInfo->sType                = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pInfo->pNext                = nullptr;
    pInfo->flags                = 0;
    pInfo->stage                = stage;
    pInfo->pName                = binary.EntryPoint;
    pInfo->pSpecializationInfo  = nullptr;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      頂点インプットレートに変換します.
//-------------------------------------------------------------------------------------------------
VkVertexInputRate ToNativeVertexInputRate(a3d::INPUT_CLASSIFICATION classification)
{
    VkVertexInputRate table[] = {
        VK_VERTEX_INPUT_RATE_VERTEX,
        VK_VERTEX_INPUT_RATE_INSTANCE
    };

    return table[classification];
}

//-------------------------------------------------------------------------------------------------
//      頂点入力バインディングに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeVertexInputBinding
(
    const a3d::InputStreamDesc&      stream,
    VkVertexInputBindingDescription* pDesc
)
{
    pDesc->binding   = stream.StreamIndex;
    pDesc->stride    = stream.StrideInBytes;
    pDesc->inputRate = ToNativeVertexInputRate(stream.InputClass);
}

//-------------------------------------------------------------------------------------------------
//      頂点アトリビュートに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeVertexAttribute
(
    uint32_t                            elementIndex,
    const a3d::InputStreamDesc&         stream,
    VkVertexInputAttributeDescription*  pDesc
)
{
    pDesc->location = stream.pElements[elementIndex].BindLocation;
    pDesc->binding  = stream.StreamIndex;
    pDesc->format   = ToNativeFormat(stream.pElements[elementIndex].Format);
    pDesc->offset   = stream.pElements[elementIndex].OffsetInBytes;
}

//-------------------------------------------------------------------------------------------------
//      頂点入力ステートに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeVertexInputState
(
    const a3d::InputLayoutDesc&             desc,
    VkPipelineVertexInputStateCreateInfo*   pInfo,
    VkVertexInputBindingDescription*        pOutBind,   // 呼び出し側で解放すること.
    VkVertexInputAttributeDescription*      pOutAttr    // 呼び出し側で解放すること.
)
{
    auto attributeCount = 0;
    for(auto i=0u; i<desc.StreamCount; ++i)
    { attributeCount += desc.pStreams[i].ElementCount; }

    pOutBind = new VkVertexInputBindingDescription[desc.StreamCount];
    A3D_ASSERT(pOutBind != nullptr);

    pOutAttr = new VkVertexInputAttributeDescription[attributeCount];
    A3D_ASSERT(pOutAttr != nullptr);

    auto index = 0;
    for(auto i=0u; i<desc.StreamCount; ++i)
    {
        ToNativeVertexInputBinding(desc.pStreams[i], &pOutBind[i]);
        for(auto j=0u; j<desc.pStreams[i].ElementCount; ++j)
        {
            ToNativeVertexAttribute(j, desc.pStreams[i], &pOutAttr[index]);
            index++;
        }
    }

    pInfo->sType                            = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pInfo->pNext                            = nullptr;
    pInfo->flags                            = 0;
    pInfo->vertexBindingDescriptionCount    = desc.StreamCount;
    pInfo->pVertexBindingDescriptions       = pOutBind;
    pInfo->vertexAttributeDescriptionCount  = attributeCount;
    pInfo->pVertexAttributeDescriptions     = pOutAttr;
}

//-------------------------------------------------------------------------------------------------
//      入力アセンブリステートに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeInputAssemblyState
(
    a3d::PRIMITIVE_TOPOLOGY                 topology,
    VkPipelineInputAssemblyStateCreateInfo* pInfo
)
{
    VkPrimitiveTopology table[] = {
        VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
        VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
        VK_PRIMITIVE_TOPOLOGY_PATCH_LIST
    };

    pInfo->sType                    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pInfo->pNext                    = nullptr;
    pInfo->flags                    = 0;
    pInfo->topology                 = table[topology];
    pInfo->primitiveRestartEnable   = VK_FALSE;
}

//-------------------------------------------------------------------------------------------------
//      テッセレーションステートに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeTessellationState
(
    const a3d::TessellationState&           state,
    VkPipelineTessellationStateCreateInfo*  pInfo
)
{
    pInfo->sType                = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    pInfo->pNext                = nullptr;
    pInfo->flags                = 0;
    pInfo->patchControlPoints   = state.PatchControlCount;
}

//-------------------------------------------------------------------------------------------------
//      ビューポートステートに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeViewportState
(
    a3d::IFrameBuffer*                  pFrameBuffer,
    VkViewport*                         pViewport,
    VkRect2D*                           pScissor,
    VkPipelineViewportStateCreateInfo*  pInfo
)
{
    auto pWrapFrameBuffer = reinterpret_cast<a3d::FrameBuffer*>(pFrameBuffer);
    A3D_ASSERT(pWrapFrameBuffer != nullptr);

    auto w = pWrapFrameBuffer->GetWidth();
    auto h = pWrapFrameBuffer->GetHeight();

    pViewport->x        = 0.0f;
    pViewport->y        = 0.0f;
    pViewport->width    = static_cast<float>(w);
    pViewport->height   = static_cast<float>(h);
    pViewport->minDepth = 0.0f;
    pViewport->maxDepth = 1.0f;

    pScissor->offset.x      = 0;
    pScissor->offset.y      = 0;
    pScissor->extent.width  = w;
    pScissor->extent.height = h;

    pInfo->sType            = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pInfo->pNext            = nullptr;
    pInfo->flags            = 0;
    pInfo->viewportCount    = 1;
    pInfo->pViewports       = pViewport;
    pInfo->scissorCount     = 1;
    pInfo->pScissors        = pScissor;
}

//-------------------------------------------------------------------------------------------------
//      ポリゴンモードに変換します.
//-------------------------------------------------------------------------------------------------
VkPolygonMode ToNativePolygonMode(a3d::POLYGON_MODE mode)
{
    VkPolygonMode table[] = {
        VK_POLYGON_MODE_FILL,
        VK_POLYGON_MODE_LINE
    };

    return table[mode];
}

//-------------------------------------------------------------------------------------------------
//      カルモードに変換します.
//-------------------------------------------------------------------------------------------------
VkCullModeFlags ToNativeCullMode(a3d::CULL_MODE mode)
{
    VkCullModeFlags table[] = {
        VK_CULL_MODE_NONE,
        VK_CULL_MODE_FRONT_BIT,
        VK_CULL_MODE_BACK_BIT
    };

    return table[mode];
}

//-------------------------------------------------------------------------------------------------
//      ラスタライゼーションステートに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeRasterizationState
(
    const a3d::RasterizerState&             state,
    VkPipelineRasterizationStateCreateInfo* pInfo
)
{
    pInfo->sType                    = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pInfo->pNext                    = nullptr;
    pInfo->flags                    = 0;
    pInfo->depthClampEnable         = (state.DepthClipEnable) ? VK_TRUE : VK_FALSE;
    pInfo->rasterizerDiscardEnable  = VK_FALSE;
    pInfo->polygonMode              = ToNativePolygonMode( state.PolygonMode );
    pInfo->cullMode                 = ToNativeCullMode( state.CullMode );
    pInfo->frontFace                = (state.FrontCounterClockWise) ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    pInfo->depthBiasEnable          = (state.DepthBias != 0) ? VK_TRUE : VK_FALSE;
    pInfo->depthBiasConstantFactor  = static_cast<float>(state.DepthBias);
    pInfo->depthBiasClamp           = state.DepthBiasClamp;
    pInfo->depthBiasSlopeFactor     = state.SlopeScaledDepthBais;
    pInfo->lineWidth                = 1.0f;
}

//-------------------------------------------------------------------------------------------------
//      サンプル数フラグに変換します.
//-------------------------------------------------------------------------------------------------
VkSampleCountFlagBits ToNativeSampleCountFlagBits(uint32_t sampleCount)
{
    if (sampleCount >= 64)
    { return VK_SAMPLE_COUNT_64_BIT; }
    else if (sampleCount >= 32)
    { return VK_SAMPLE_COUNT_32_BIT; }
    else if (sampleCount >= 16)
    { return VK_SAMPLE_COUNT_16_BIT; }
    else if (sampleCount >= 8)
    { return VK_SAMPLE_COUNT_8_BIT; }
    else if (sampleCount >= 4)
    { return VK_SAMPLE_COUNT_4_BIT; }
    else if (sampleCount >= 2)
    { return VK_SAMPLE_COUNT_2_BIT; }
    else 
    { return VK_SAMPLE_COUNT_1_BIT; }
}

//-------------------------------------------------------------------------------------------------
//      マルチサンプルステートに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeMultisampleState
(
    const a3d::MultiSampleState&            state,
    VkPipelineMultisampleStateCreateInfo*   pInfo
)
{
    pInfo->sType                    = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pInfo->pNext                    = nullptr;
    pInfo->flags                    = 0;
    pInfo->rasterizationSamples     = ToNativeSampleCountFlagBits(state.SampleCount);
    pInfo->sampleShadingEnable      = (state.EnableMultiSample) ? VK_TRUE : VK_FALSE;
    pInfo->minSampleShading         = 0.0f;
    pInfo->pSampleMask              = nullptr;
    pInfo->alphaToCoverageEnable    = (state.EnableAlphaToCoverage) ? VK_TRUE : VK_FALSE;
    pInfo->alphaToOneEnable         = VK_FALSE;
}

//-------------------------------------------------------------------------------------------------
//      ステンシル操作に変換します.
//-------------------------------------------------------------------------------------------------
VkStencilOp ToNativeStencilOp(a3d::STENCIL_OP value)
{
    VkStencilOp table[] = {
        VK_STENCIL_OP_KEEP,
        VK_STENCIL_OP_ZERO,
        VK_STENCIL_OP_REPLACE,
        VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        VK_STENCIL_OP_INVERT,
        VK_STENCIL_OP_INCREMENT_AND_WRAP,
        VK_STENCIL_OP_DECREMENT_AND_WRAP,
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      ステンシル操作ステートに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeStencilOpState(const a3d::StencilState& state, VkStencilOpState* pState)
{
    pState->failOp      = ToNativeStencilOp(state.StencilFailOp);
    pState->passOp      = ToNativeStencilOp(state.StencilPassOp);
    pState->depthFailOp = ToNativeStencilOp(state.StencilDepthFailOp);
    pState->compareOp   = a3d::ToNativeCompareOp(state.StencilCompareOp);
    pState->compareMask = 0;
    pState->writeMask   = 0;
    pState->reference   = 0;
}

//-------------------------------------------------------------------------------------------------
//      深度ステンシルステートに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeDepthStencilState
(
    const a3d::DepthStencilState&           state,
    VkPipelineDepthStencilStateCreateInfo*  pInfo
)
{
    pInfo->sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pInfo->pNext                 = nullptr;
    pInfo->flags                 = 0;
    pInfo->depthTestEnable       = (state.DepthTestEnable)  ? VK_TRUE : VK_FALSE;
    pInfo->depthWriteEnable      = (state.DepthWriteEnable) ? VK_TRUE : VK_FALSE;
    pInfo->depthCompareOp        = a3d::ToNativeCompareOp(state.DepthCompareOp);
    pInfo->depthBoundsTestEnable = VK_FALSE;
    pInfo->stencilTestEnable     = (state.StencilTestEnable) ? VK_TRUE : VK_FALSE;
    ToNativeStencilOpState(state.FrontFace, &pInfo->front);
    ToNativeStencilOpState(state.BackFace,  &pInfo->back);
    pInfo->minDepthBounds        = 0.0f;
    pInfo->maxDepthBounds        = 0.0f;
    pInfo->front.writeMask       = state.StencilWriteMask;
    pInfo->front.reference       = state.StencllReadMask;
    pInfo->front.compareMask     = 0;//UINT32_MAX;
    pInfo->back.writeMask        = state.StencilWriteMask;
    pInfo->back.reference        = state.StencllReadMask;
    pInfo->back.compareMask      = 0;//UINT32_MAX;
}

//-------------------------------------------------------------------------------------------------
//      ブレンドファクターに変換します.
//-------------------------------------------------------------------------------------------------
VkBlendFactor ToNativeBlendFactor(a3d::BLEND_FACTOR value)
{
    VkBlendFactor table[] = {
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_SRC_COLOR,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
        VK_BLEND_FACTOR_SRC_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        VK_BLEND_FACTOR_DST_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
        VK_BLEND_FACTOR_DST_COLOR,
        VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
        VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
        VK_BLEND_FACTOR_SRC1_COLOR,
        VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
        VK_BLEND_FACTOR_SRC1_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      ブレンド操作に変換します.
//-------------------------------------------------------------------------------------------------
VkBlendOp ToNativeBlendOp(a3d::BLEND_OP value)
{
    VkBlendOp table[] = {
        VK_BLEND_OP_ADD,
        VK_BLEND_OP_SUBTRACT,
        VK_BLEND_OP_REVERSE_SUBTRACT,
        VK_BLEND_OP_MIN,
        VK_BLEND_OP_MAX
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      カラーブレンドアタッチメントステートに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeColorBlendAttachmentState
(
    const a3d::ColorBlendState&             state,
    VkPipelineColorBlendAttachmentState*    pInfo
)
{
    pInfo->blendEnable          = (state.BlendEnable) ? VK_TRUE : VK_FALSE;
    pInfo->srcColorBlendFactor  = ToNativeBlendFactor(state.SrcBlend);
    pInfo->dstColorBlendFactor  = ToNativeBlendFactor(state.DstBlend);
    pInfo->colorBlendOp         = ToNativeBlendOp(state.BlendOp);
    pInfo->srcAlphaBlendFactor  = ToNativeBlendFactor(state.SrcBlendAlpha);
    pInfo->dstAlphaBlendFactor  = ToNativeBlendFactor(state.DstBlendAlpha);
    pInfo->alphaBlendOp         = ToNativeBlendOp(state.BlendOpAlpha);
    pInfo->colorWriteMask       = 0;

    if (state.EnableWriteR)
    { pInfo->colorWriteMask |= VK_COLOR_COMPONENT_R_BIT; }
    if (state.EnableWriteG)
    { pInfo->colorWriteMask |= VK_COLOR_COMPONENT_G_BIT; }
    if (state.EnableWriteB)
    { pInfo->colorWriteMask |= VK_COLOR_COMPONENT_B_BIT; }
    if (state.EnableWriteA)
    { pInfo->colorWriteMask |= VK_COLOR_COMPONENT_A_BIT; }

}

//-------------------------------------------------------------------------------------------------
//      論理操作に変換します.
//-------------------------------------------------------------------------------------------------
VkLogicOp ToNativeLogicOp(a3d::LOGIC_OP value)
{
    VkLogicOp table[] = {
        VK_LOGIC_OP_CLEAR,
        VK_LOGIC_OP_SET,
        VK_LOGIC_OP_COPY,
        VK_LOGIC_OP_COPY_INVERTED,
        VK_LOGIC_OP_NO_OP,
        VK_LOGIC_OP_INVERT,
        VK_LOGIC_OP_AND,
        VK_LOGIC_OP_NAND,
        VK_LOGIC_OP_OR,
        VK_LOGIC_OP_NOR,
        VK_LOGIC_OP_XOR,
        VK_LOGIC_OP_EQUIVALENT,
        VK_LOGIC_OP_AND_REVERSE,
        VK_LOGIC_OP_AND_INVERTED,
        VK_LOGIC_OP_OR_REVERSE,
        VK_LOGIC_OP_OR_INVERTED
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      カラーブレンドステートを設定します.
//-------------------------------------------------------------------------------------------------
void ToNativeColorBlendState
(
    const a3d::BlendState&                  state,
    uint32_t                                colorTargetCount,
    VkPipelineColorBlendAttachmentState*    pAttachments,
    VkPipelineColorBlendStateCreateInfo*    pInfo
)
{ 
    for(auto i=0u; i<colorTargetCount; ++i)
    { ToNativeColorBlendAttachmentState(state.ColorTarget[i], &pAttachments[i]); }

    pInfo->sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pInfo->pNext             = nullptr;
    pInfo->flags             = 0;
    pInfo->logicOpEnable     = (state.LogicOpEnable) ? VK_TRUE : VK_FALSE;
    pInfo->logicOp           = ToNativeLogicOp(state.LogicOp);
    pInfo->attachmentCount   = colorTargetCount;
    pInfo->pAttachments      = pAttachments;
    pInfo->blendConstants[0] = 0.0f;
    pInfo->blendConstants[1] = 0.0f;
    pInfo->blendConstants[2] = 0.0f;
    pInfo->blendConstants[3] = 0.0f;
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
: m_RefCount        (1)
, m_pDevice         (nullptr)
, m_PipelineState   (null_handle)
, m_BindPoint       (VK_PIPELINE_BIND_POINT_GRAPHICS)
, m_PipelineCache   (null_handle)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
PipelineState::~PipelineState()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      グラフィックスパイプラインステートとして初期化します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::InitAsGraphics(IDevice* pDevice, const GraphicsPipelineStateDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    if (pDesc->pFrameBuffer == nullptr || pDesc->pLayout == nullptr)
    { return false; }

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    m_BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    // パイプラインキャッシュを生成.
    {
        VkPipelineCacheCreateInfo info = {};
        info.sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        info.pNext           = nullptr;
        info.flags           = 0;
        info.initialDataSize = (pDesc->pCachedPSO != nullptr) ? size_t(pDesc->pCachedPSO->GetBufferSize()) : 0;
        info.pInitialData    = (pDesc->pCachedPSO != nullptr) ? pDesc->pCachedPSO->GetBufferPointer() : nullptr;

        auto ret = vkCreatePipelineCache(pNativeDevice, &info, nullptr, &m_PipelineCache);
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    // パイプラインステートを生成.
    {
        VkPipelineShaderStageCreateInfo         shaderInfos[5]       = {};
        VkVertexInputAttributeDescription*      pInputAttrs          = nullptr;
        VkVertexInputBindingDescription*        pBindingDescs        = nullptr;
        VkPipelineVertexInputStateCreateInfo    vertexInputState     = {};
        VkPipelineInputAssemblyStateCreateInfo  inputAssemblyState   = {};
        VkPipelineTessellationStateCreateInfo   tessellationState    = {};
        VkPipelineViewportStateCreateInfo       viewportState        = {};
        VkPipelineRasterizationStateCreateInfo  rasterizerState      = {};
        VkPipelineMultisampleStateCreateInfo    multisampleState     = {};
        VkPipelineDepthStencilStateCreateInfo   depthStencilState    = {};
        VkPipelineColorBlendAttachmentState     colorAttachments[8]  = {};
        VkPipelineColorBlendStateCreateInfo     colorBlendState      = {};

        auto shaderCount = 0;
        if (pDesc->VertexShader.pByteCode != nullptr && pDesc->VertexShader.ByteCodeSize != 0)
        {
            auto ret = ToNativeShaderStageInfo(
                pNativeDevice,
                pDesc->VertexShader,
                VK_SHADER_STAGE_VERTEX_BIT,
                &shaderInfos[shaderCount]);
            A3D_ASSERT(ret == true);
            A3D_UNUSED(ret);
            shaderCount++;
        }

        if (pDesc->DomainShader.pByteCode != nullptr && pDesc->DomainShader.ByteCodeSize != 0)
        {
            auto ret = ToNativeShaderStageInfo(
                pNativeDevice,
                pDesc->DomainShader,
                VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                &shaderInfos[shaderCount]);
            A3D_ASSERT(ret == true);
            A3D_UNUSED(ret);
            shaderCount++;
        }

        if (pDesc->HullShader.pByteCode != nullptr && pDesc->HullShader.ByteCodeSize != 0)
        {
            auto ret = ToNativeShaderStageInfo(
                pNativeDevice,
                pDesc->HullShader,
                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                &shaderInfos[shaderCount]);
            A3D_ASSERT(ret == true);
            A3D_UNUSED(ret);
            shaderCount++;
        }

        if (pDesc->GeometryShader.pByteCode != nullptr && pDesc->GeometryShader.ByteCodeSize != 0)
        {
            auto ret = ToNativeShaderStageInfo(
                pNativeDevice,
                pDesc->GeometryShader,
                VK_SHADER_STAGE_GEOMETRY_BIT,
                &shaderInfos[shaderCount]);
            A3D_ASSERT(ret == true);
            A3D_UNUSED(ret);
            shaderCount++;
        }

        if (pDesc->PixelShader.pByteCode != nullptr && pDesc->PixelShader.ByteCodeSize != 0)
        {
            auto ret = ToNativeShaderStageInfo(
                pNativeDevice,
                pDesc->PixelShader,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                &shaderInfos[shaderCount]);
            A3D_ASSERT(ret == true);
            A3D_UNUSED(ret);
            shaderCount++;
        }

        VkDynamicState dynamicElements[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pNext              = nullptr;
        dynamicState.flags              = 0;
        dynamicState.dynamicStateCount  = 2;
        dynamicState.pDynamicStates     = dynamicElements;

        auto pWrapFrameBuffer = reinterpret_cast<FrameBuffer*>(pDesc->pFrameBuffer);
        A3D_ASSERT(pWrapFrameBuffer != nullptr);

        VkViewport viewports[16];
        VkRect2D scissors[16];

        ToNativeVertexInputState  (pDesc->InputLayout, &vertexInputState, pBindingDescs, pInputAttrs);
        ToNativeInputAssemblyState(pDesc->PrimitiveTopology, &inputAssemblyState);
        ToNativeTessellationState (pDesc->TessellationState, &tessellationState);
        ToNativeRasterizationState(pDesc->RasterizerState,   &rasterizerState);
        ToNativeMultisampleState  (pDesc->MultiSampleState,  &multisampleState);
        ToNativeDepthStencilState (pDesc->DepthStencilState, &depthStencilState);
        ToNativeViewportState(pDesc->pFrameBuffer, viewports, scissors, &viewportState);
        ToNativeColorBlendState(
            pDesc->BlendState,
            pWrapFrameBuffer->GetColorTargetCount(),
            colorAttachments,
            &colorBlendState );

        auto pWrapLayout = reinterpret_cast<DescriptorSetLayout*>(pDesc->pLayout);
        A3D_ASSERT(pWrapLayout != nullptr);

        VkGraphicsPipelineCreateInfo info = {};
        info.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        info.pNext                  = nullptr;
        info.flags                  = 0;
        info.stageCount             = shaderCount;
        info.pStages                = shaderInfos;
        info.pVertexInputState      = &vertexInputState;
        info.pInputAssemblyState    = &inputAssemblyState;
        info.pTessellationState     = &tessellationState;
        info.pViewportState         = &viewportState;
        info.pRasterizationState    = &rasterizerState;
        info.pMultisampleState      = &multisampleState;
        info.pDepthStencilState     = &depthStencilState;
        info.pColorBlendState       = &colorBlendState;
        info.pDynamicState          = &dynamicState;
        info.layout                 = pWrapLayout->GetVulkanPipelineLayout();
        info.renderPass             = pWrapFrameBuffer->GetRenderPass();
        info.subpass                = 0;
        info.basePipelineHandle     = null_handle;
        info.basePipelineIndex      = 0;
       
        auto ret = vkCreateGraphicsPipelines(pNativeDevice, m_PipelineCache, 1, &info, nullptr, &m_PipelineState);

        // メモリを解放.
        delete [] pBindingDescs;
        delete [] pInputAttrs;

        for(auto i=0; i<shaderCount; ++i)
        {
            if (shaderInfos[i].module != VK_NULL_HANDLE)
            {
                vkDestroyShaderModule(pNativeDevice, shaderInfos[i].module, nullptr);
                shaderInfos[i].module = VK_NULL_HANDLE;
            }
        }

        if ( ret != VK_SUCCESS )
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      コンピュートパイプランステートとして初期化します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::InitAsCompute(IDevice* pDevice, const ComputePipelineStateDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    if (pDesc->pLayout                   == nullptr
    || pDesc->ComputeShader.ByteCodeSize == 0 
    || pDesc->ComputeShader.pByteCode    == nullptr)
    { return false;}

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    m_BindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

    // パイプラインキャッシュを生成.
    {
        VkPipelineCacheCreateInfo info = {};
        info.sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        info.pNext           = nullptr;
        info.flags           = 0;
        info.initialDataSize = (pDesc->pCachedPSO != nullptr) ? size_t(pDesc->pCachedPSO->GetBufferSize()) : 0;
        info.pInitialData    = (pDesc->pCachedPSO != nullptr) ? pDesc->pCachedPSO->GetBufferPointer() : nullptr;

        auto ret = vkCreatePipelineCache(pNativeDevice, &info, nullptr, &m_PipelineCache);
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    // コンピュートパイプラインを生成します.
    {
        auto pWrapLayout = reinterpret_cast<DescriptorSetLayout*>(pDesc->pLayout);
        A3D_ASSERT(pWrapLayout != nullptr);

        VkComputePipelineCreateInfo info = {};
        info.sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        info.pNext              = nullptr;
        info.flags              = 0;
        info.layout             = pWrapLayout->GetVulkanPipelineLayout();
        info.basePipelineHandle = null_handle;
        info.basePipelineIndex  = 0;
        ToNativeShaderStageInfo(pNativeDevice, pDesc->ComputeShader, VK_SHADER_STAGE_COMPUTE_BIT, &info.stage);

        auto ret = vkCreateComputePipelines(pNativeDevice, m_PipelineCache, 1, &info, nullptr, &m_PipelineState);
        if ( ret != VK_SUCCESS )
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void PipelineState::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    if (m_PipelineState != null_handle)
    {
        vkDestroyPipeline( pNativeDevice, m_PipelineState, nullptr );
        m_PipelineState = null_handle;
    }

    if (m_PipelineCache != null_handle)
    {
        vkDestroyPipelineCache( pNativeDevice, m_PipelineCache, nullptr );
        m_PipelineCache = null_handle;
    }

    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void PipelineState::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
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
//      キャッシュを取得します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::GetCachedBlob(IBlob** ppBlob)
{
    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    size_t  size  = 0;
    void*   pData = nullptr;
    auto ret = vkGetPipelineCacheData(pNativeDevice, m_PipelineCache, &size, pData);
    if ( ret != VK_SUCCESS )
    { return false; }

    if (!Blob::Create(size, ppBlob))
    { return false; }

    void* pPtr = (*ppBlob)->GetBufferPointer();
    memcpy(pPtr, pData, size);

    return true;
}

//-------------------------------------------------------------------------------------------------
//      パイプラインステートを取得します.
//-------------------------------------------------------------------------------------------------
VkPipeline PipelineState::GetVulkanPipeline() const
{ return m_PipelineState; }

//-------------------------------------------------------------------------------------------------
//      パイプラインバインドポイントを取得します.
//-------------------------------------------------------------------------------------------------
VkPipelineBindPoint PipelineState::GetVulkanPipelineBindPoint() const
{ return m_BindPoint; }

//-------------------------------------------------------------------------------------------------
//      グラフィクスパイプランステートとして生成します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::CreateAsGraphics
(
    IDevice*                            pDevice,
    const GraphicsPipelineStateDesc*    pDesc,
    IPipelineState**                    ppPipelineState
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
