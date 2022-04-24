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
    auto entryPoint = a3d::FindEntryPoint(binary.pByteCode, binary.ByteCodeSize);
    if (entryPoint == nullptr)
    { return false; }

    VkShaderModuleCreateInfo info = {};
    info.sType      = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.pNext      = nullptr;
    info.flags      = 0;
    info.pCode      = reinterpret_cast<const uint32_t*>(binary.pByteCode);
    info.codeSize   = binary.ByteCodeSize;

    auto ret = vkCreateShaderModule(device, &info, nullptr, &pInfo->module);
    if ( ret != VK_SUCCESS )
    { return false; }

    pInfo->sType                = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pInfo->pNext                = nullptr;
    pInfo->flags                = 0;
    pInfo->stage                = stage;
    pInfo->pName                = entryPoint;
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
//      頂点アトリビュートに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeVertexAttribute
(
    const a3d::InputElementDesc&        element,
    VkVertexInputAttributeDescription*  pDesc
)
{
    pDesc->location = element.Location;
    pDesc->binding  = element.StreamIndex;
    pDesc->format   = ToNativeFormat(element.Format);
    pDesc->offset   = element.OffsetInBytes;
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
    // 必要な数を一回数える.
    auto bindingCount       = 0;
    auto prevStreamIndex    = -1;
    for(auto i=0u; i<desc.ElementCount; ++i)
    {
        if (prevStreamIndex != desc.pElements[i].StreamIndex)
        {
            bindingCount++;
            prevStreamIndex = desc.pElements[i].StreamIndex;
        }
    }

    pOutBind = new VkVertexInputBindingDescription[bindingCount];
    A3D_ASSERT(pOutBind != nullptr);

    // インデックスをリセット.
    prevStreamIndex       = -1;
    auto currBindingIndex = -1;
    for(auto i=0u; i<desc.ElementCount; ++i)
    {
        if (prevStreamIndex != desc.pElements[i].StreamIndex)
        {
            currBindingIndex++;

            pOutBind[currBindingIndex].binding   = desc.pElements[i].StreamIndex;
            pOutBind[currBindingIndex].stride    = ToByte(desc.pElements[i].Format);
            pOutBind[currBindingIndex].inputRate = ToNativeVertexInputRate(desc.pElements[i].InputClass);

            // ストリーム番号更新.
            prevStreamIndex  = desc.pElements[i].StreamIndex;
        }
        else
        {
            pOutBind[currBindingIndex].stride += ToByte(desc.pElements[i].Format);
        }
    }

    pOutAttr = new VkVertexInputAttributeDescription[desc.ElementCount];
    A3D_ASSERT(pOutAttr != nullptr);

    auto index = 0;
    for(auto i=0u; i<desc.ElementCount; ++i)
    {
        ToNativeVertexAttribute(desc.pElements[i], &pOutAttr[i]);
    }

    pInfo->sType                            = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pInfo->pNext                            = nullptr;
    pInfo->flags                            = 0;
    pInfo->vertexBindingDescriptionCount    = bindingCount;
    pInfo->pVertexBindingDescriptions       = pOutBind;
    pInfo->vertexAttributeDescriptionCount  = desc.ElementCount;
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

    if (pInfo->patchControlPoints == 0)
    { pInfo->patchControlPoints = 1; }

    if (pInfo->patchControlPoints > 32)
    { pInfo->patchControlPoints = 32; }
}

//-------------------------------------------------------------------------------------------------
//      ビューポートステートに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeViewportState
(
    VkViewport*                         pViewport,
    VkRect2D*                           pScissor,
    VkPipelineViewportStateCreateInfo*  pInfo
)
{
    pViewport->x        = 0.0f;
    pViewport->y        = 0.0f;
    pViewport->width    = 1.0f;
    pViewport->height   = 1.0f;
    pViewport->minDepth = 0.0f;
    pViewport->maxDepth = 1.0f;

    pScissor->offset.x      = 0;
    pScissor->offset.y      = 0;
    pScissor->extent.width  = 1;
    pScissor->extent.height = 1;

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
    pInfo->depthBiasSlopeFactor     = state.SlopeScaledDepthBias;
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
void ToNativeStencilOpState(const a3d::StencilTestDesc& state, VkStencilOpState* pState)
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
//      深度ステートに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeDepthState
(
    const a3d::DepthState&           state,
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
}

//-------------------------------------------------------------------------------------------------
//      ステンシルステートに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeStencilState
(
    const a3d::StencilState&                state,
    VkPipelineDepthStencilStateCreateInfo*  pInfo
)
{
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
    { ToNativeColorBlendAttachmentState(state.RenderTarget[i], &pAttachments[i]); }

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
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    if (pDesc->pLayout == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
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
        {
            A3D_LOG("Error : vkCreatePipelineCache() Failed. VkResult = %s", ToString(ret));
            return false;
        }
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
        if (pDesc->VS.pByteCode != nullptr && pDesc->VS.ByteCodeSize != 0)
        {
            auto ret = ToNativeShaderStageInfo(
                pNativeDevice,
                pDesc->VS,
                VK_SHADER_STAGE_VERTEX_BIT,
                &shaderInfos[shaderCount]);
            A3D_ASSERT(ret == true);
            A3D_UNUSED(ret);
            shaderCount++;
        }

        if (pDesc->DS.pByteCode != nullptr && pDesc->DS.ByteCodeSize != 0)
        {
            auto ret = ToNativeShaderStageInfo(
                pNativeDevice,
                pDesc->DS,
                VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                &shaderInfos[shaderCount]);
            A3D_ASSERT(ret == true);
            A3D_UNUSED(ret);
            shaderCount++;
        }

        if (pDesc->HS.pByteCode != nullptr && pDesc->HS.ByteCodeSize != 0)
        {
            auto ret = ToNativeShaderStageInfo(
                pNativeDevice,
                pDesc->HS,
                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                &shaderInfos[shaderCount]);
            A3D_ASSERT(ret == true);
            A3D_UNUSED(ret);
            shaderCount++;
        }

    #if 0
        //if (pDesc->GS.pByteCode != nullptr && pDesc->GS.ByteCodeSize != 0)
        //{
        //    auto ret = ToNativeShaderStageInfo(
        //        pNativeDevice,
        //        pDesc->GS,
        //        VK_SHADER_STAGE_GEOMETRY_BIT,
        //        &shaderInfos[shaderCount]);
        //    A3D_ASSERT(ret == true);
        //    A3D_UNUSED(ret);
        //    shaderCount++;
        //}
    #endif

        if (pDesc->PS.pByteCode != nullptr && pDesc->PS.ByteCodeSize != 0)
        {
            auto ret = ToNativeShaderStageInfo(
                pNativeDevice,
                pDesc->PS,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                &shaderInfos[shaderCount]);
            A3D_ASSERT(ret == true);
            A3D_UNUSED(ret);
            shaderCount++;
        }

        VkDynamicState dynamicElements[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_BLEND_CONSTANTS,
            VK_DYNAMIC_STATE_STENCIL_REFERENCE
        };

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pNext              = nullptr;
        dynamicState.flags              = 0;
        dynamicState.dynamicStateCount  = 4;
        dynamicState.pDynamicStates     = dynamicElements;

        VkViewport viewports[16];
        VkRect2D scissors[16];

        ToNativeVertexInputState  (pDesc->InputLayout, &vertexInputState, pBindingDescs, pInputAttrs);
        ToNativeInputAssemblyState(pDesc->PrimitiveTopology, &inputAssemblyState);
        ToNativeTessellationState (pDesc->TessellationState, &tessellationState);
        ToNativeRasterizationState(pDesc->RasterizerState,   &rasterizerState);
        ToNativeMultisampleState  (pDesc->MultiSampleState,  &multisampleState);
        ToNativeDepthState        (pDesc->DepthState,        &depthStencilState);
        ToNativeStencilState      (pDesc->StencilState,      &depthStencilState);
        ToNativeViewportState(viewports, scissors, &viewportState);
        ToNativeColorBlendState(
            pDesc->BlendState,
            pDesc->RenderTargetCount,
            colorAttachments,
            &colorBlendState );

        VkFormat colorFormats[8] = {};
        for(auto i=0; i<8; ++i)
        { colorFormats[i] = ToNativeFormat(pDesc->RenderTarget[i]); }

        VkPipelineRenderingCreateInfoKHR renderInfo = {};
        renderInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        renderInfo.pNext                    = nullptr;
        renderInfo.colorAttachmentCount     = pDesc->RenderTargetCount;
        renderInfo.pColorAttachmentFormats  = colorFormats;
        renderInfo.depthAttachmentFormat    = ToNativeFormat(pDesc->DepthTarget);
        renderInfo.stencilAttachmentFormat  = VK_FORMAT_UNDEFINED;;

        auto pWrapLayout = static_cast<DescriptorSetLayout*>(pDesc->pLayout);
        A3D_ASSERT(pWrapLayout != nullptr);

        VkGraphicsPipelineCreateInfo info = {};
        info.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        info.pNext                  = &renderInfo;
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
        info.renderPass             = null_handle;
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
        {
            A3D_LOG("Error : vkCreateGraphicsPipelines() Failed. VkResult = %s", ToString(ret));
            return false;
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      コンピュートパイプランステートとして初期化します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::InitAsCompute(IDevice* pDevice, const ComputePipelineStateDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    if (pDesc->pLayout         == nullptr
     || pDesc->CS.ByteCodeSize == 0 
     || pDesc->CS.pByteCode    == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
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
        {
            A3D_LOG("Error : vkCreatePipelineCache() Failed. VkResult = %s", ToString(ret));
            return false;
        }
    }

    // コンピュートパイプラインを生成します.
    {
        auto pWrapLayout = static_cast<DescriptorSetLayout*>(pDesc->pLayout);
        A3D_ASSERT(pWrapLayout != nullptr);

        VkComputePipelineCreateInfo info = {};
        info.sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        info.pNext              = nullptr;
        info.flags              = 0;
        info.layout             = pWrapLayout->GetVulkanPipelineLayout();
        info.basePipelineHandle = null_handle;
        info.basePipelineIndex  = 0;
        ToNativeShaderStageInfo(pNativeDevice, pDesc->CS, VK_SHADER_STAGE_COMPUTE_BIT, &info.stage);

        auto ret = vkCreateComputePipelines(pNativeDevice, m_PipelineCache, 1, &info, nullptr, &m_PipelineState);
        if ( ret != VK_SUCCESS )
        {
            A3D_LOG("Error : vkCreateComputePipelines() Failed. VkResult = %s", ToString(ret));
            return false;
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      メッシュパイプランステートとして初期化します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::InitAsMesh(IDevice* pDevice, const MeshShaderPipelineStateDesc* pDesc)
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

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
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
        {
            A3D_LOG("Error : vkCreatePipelineCache() Failed. VkResult = %s", ToString(ret));
            return false;
        }
    }

    // パイプラインステートを生成.
    {
        VkPipelineShaderStageCreateInfo         shaderInfos[3]       = {};
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
        if (pDesc->AS.pByteCode != nullptr && pDesc->AS.ByteCodeSize != 0)
        {
            auto ret = ToNativeShaderStageInfo(
                pNativeDevice,
                pDesc->AS,
                VK_SHADER_STAGE_TASK_BIT_NV,
                &shaderInfos[shaderCount]);
            A3D_ASSERT(ret == true);
            A3D_UNUSED(ret);
            shaderCount++;
        }

        if (pDesc->MS.pByteCode != nullptr && pDesc->MS.ByteCodeSize != 0)
        {
            auto ret = ToNativeShaderStageInfo(
                pNativeDevice,
                pDesc->MS,
                VK_SHADER_STAGE_MESH_BIT_NV,
                &shaderInfos[shaderCount]);
            A3D_ASSERT(ret == true);
            A3D_UNUSED(ret);
            shaderCount++;
        }

        if (pDesc->PS.pByteCode != nullptr && pDesc->PS.ByteCodeSize != 0)
        {
            auto ret = ToNativeShaderStageInfo(
                pNativeDevice,
                pDesc->PS,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                &shaderInfos[shaderCount]);
            A3D_ASSERT(ret == true);
            A3D_UNUSED(ret);
            shaderCount++;
        }

        VkDynamicState dynamicElements[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_BLEND_CONSTANTS,
            VK_DYNAMIC_STATE_STENCIL_REFERENCE
        };

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pNext              = nullptr;
        dynamicState.flags              = 0;
        dynamicState.dynamicStateCount  = 4;
        dynamicState.pDynamicStates     = dynamicElements;

        VkViewport  viewports[16];
        VkRect2D    scissors[16];

        ToNativeRasterizationState(pDesc->RasterizerState,   &rasterizerState);
        ToNativeMultisampleState  (pDesc->MultiSampleState,  &multisampleState);
        ToNativeDepthState        (pDesc->DepthState,        &depthStencilState);
        ToNativeStencilState      (pDesc->StencilState,      &depthStencilState);
        ToNativeViewportState(viewports, scissors, &viewportState);
        ToNativeColorBlendState(
            pDesc->BlendState,
            pDesc->RenderTargetCount,
            colorAttachments,
            &colorBlendState );

        VkFormat colorFormats[8] = {};
        for(auto i=0u; i<pDesc->RenderTargetCount; ++i)
        { colorFormats[i] = ToNativeFormat(pDesc->RenderTarget[i]); }

        VkFormat stencilFormat = VK_FORMAT_UNDEFINED;
        if (pDesc->DepthTarget == RESOURCE_FORMAT_D24_UNORM_S8_UINT)
        { stencilFormat = ToNativeFormat(pDesc->DepthTarget); }

        VkPipelineRenderingCreateInfoKHR renderInfo = {};
        renderInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        renderInfo.pNext                    = nullptr;
        renderInfo.colorAttachmentCount     = pDesc->RenderTargetCount;
        renderInfo.pColorAttachmentFormats  = colorFormats;
        renderInfo.depthAttachmentFormat    = ToNativeFormat(pDesc->DepthTarget);
        renderInfo.stencilAttachmentFormat  = stencilFormat;

        auto pWrapLayout = static_cast<DescriptorSetLayout*>(pDesc->pLayout);
        A3D_ASSERT(pWrapLayout != nullptr);

        VkGraphicsPipelineCreateInfo info = {};
        info.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        info.pNext                  = &renderInfo;
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
        info.renderPass             = null_handle;
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
        {
            A3D_LOG("Error : vkCreateGraphicsPipelines() Failed. VkResult = %s", ToString(ret));
            return false;
        }
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

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
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
    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    size_t  size  = 0;
    void*   pData = nullptr;
    auto ret = vkGetPipelineCacheData(pNativeDevice, m_PipelineCache, &size, pData);
    if ( ret != VK_SUCCESS )
    {
        A3D_LOG("Error : vkGetPipelineCacheData() Failed. VkResult = %s", ToString(ret));
        return false;
    }

    if (!Blob::Create(size, ppBlob))
    {
        A3D_LOG("Error : Blob::Create() Failed.");
        return false;
    }

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
//      メッシュパイプラインステートとして生成します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::CreateAsMesh
(
    IDevice*                            pDevice,
    const MeshShaderPipelineStateDesc*  pDesc,
    IPipelineState**                    ppPipelineState
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
