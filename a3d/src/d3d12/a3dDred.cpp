//-----------------------------------------------------------------------------
// File : a3dDred.cpp
// Desc : DRED Helper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

#define ITEM_MSG(var) case var : { return #var; }


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const size_t kMaxOpNameLength = strlen("EMITRAYTRACINGACCELERATIONSTRUCTUREPOSTBUILDINFO");


//-----------------------------------------------------------------------------
//      D3D12_AUTO_BREADCRUMB_OP を文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_AUTO_BREADCRUMB_OP op)
{
    switch(op)
    {
        case D3D12_AUTO_BREADCRUMB_OP_SETMARKER:
            { return "SETMARKER"; }
        case D3D12_AUTO_BREADCRUMB_OP_BEGINEVENT:
            { return "BEGINEVENT"; }
        case D3D12_AUTO_BREADCRUMB_OP_ENDEVENT:
            { return "ENDEVEN"; }
        case D3D12_AUTO_BREADCRUMB_OP_DRAWINSTANCED:
            { return "DRAWINSTANCED"; }
        case D3D12_AUTO_BREADCRUMB_OP_DRAWINDEXEDINSTANCED:
            { return "DRAWINDEXEDINSTANCED"; }
        case D3D12_AUTO_BREADCRUMB_OP_EXECUTEINDIRECT:
            { return "EXECUTEINDIRECT"; }
        case D3D12_AUTO_BREADCRUMB_OP_DISPATCH:
            { return "DISPATCH"; }
        case D3D12_AUTO_BREADCRUMB_OP_COPYBUFFERREGION:
            { return "COPYBUFFERREGION"; }
        case D3D12_AUTO_BREADCRUMB_OP_COPYTEXTUREREGION:
            { return "COPYTEXTUREREGION"; }
        case D3D12_AUTO_BREADCRUMB_OP_COPYRESOURCE:
            { return "COPYRESOURCE"; }
        case D3D12_AUTO_BREADCRUMB_OP_COPYTILES:
            { return "COPYTILES"; }
        case D3D12_AUTO_BREADCRUMB_OP_RESOLVESUBRESOURCE:
            { return "RESOLVESUBRESOURCE"; }
        case D3D12_AUTO_BREADCRUMB_OP_CLEARRENDERTARGETVIEW:
            { return "CLEARRENDERTARGETVIEW"; }
        case D3D12_AUTO_BREADCRUMB_OP_CLEARUNORDEREDACCESSVIEW:
            { return "CLEARUNORDEREDACCESSVIEW"; }
        case D3D12_AUTO_BREADCRUMB_OP_CLEARDEPTHSTENCILVIEW:
            { return "CLEARDEPTHSTENCILVIEW"; }
        case D3D12_AUTO_BREADCRUMB_OP_RESOURCEBARRIER:
            { return "RESOURCEBARRIER"; }
        case D3D12_AUTO_BREADCRUMB_OP_EXECUTEBUNDLE:
            { return "EXECUTEBUNDLE"; }
        case D3D12_AUTO_BREADCRUMB_OP_PRESENT:
            { return "PRESENT"; }
        case D3D12_AUTO_BREADCRUMB_OP_RESOLVEQUERYDATA:
            { return "RESOLVEQUERYDATA"; }
        case D3D12_AUTO_BREADCRUMB_OP_BEGINSUBMISSION:
            { return "BEGINSUBMISSION"; }
        case D3D12_AUTO_BREADCRUMB_OP_ENDSUBMISSION: 
            { return "ENDSUBMISSION"; }
        case D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME:
            { return "DECODEFRAME"; }
        case D3D12_AUTO_BREADCRUMB_OP_PROCESSFRAMES:
            { return "PROCESSFRAMES"; }
        case D3D12_AUTO_BREADCRUMB_OP_ATOMICCOPYBUFFERUINT:
            { return "ATOMICCOPYBUFFERUINT"; }
        case D3D12_AUTO_BREADCRUMB_OP_ATOMICCOPYBUFFERUINT64:
            { return "ATOMICCOPYBUFFERUINT64"; }
        case D3D12_AUTO_BREADCRUMB_OP_RESOLVESUBRESOURCEREGION:
            { return "RESOLVESUBRESOURCEREGION"; }
        case D3D12_AUTO_BREADCRUMB_OP_WRITEBUFFERIMMEDIATE:
            { return "WRITEBUFFERIMMEDIATE"; }
        case D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME1:
            { return "DECODEFRAME1"; }
        case D3D12_AUTO_BREADCRUMB_OP_SETPROTECTEDRESOURCESESSION:
            { return "SETPROTECTEDRESOURCESESSION"; }
        case D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME2:
            { return "DECODEFRAME2"; }
        case D3D12_AUTO_BREADCRUMB_OP_PROCESSFRAMES1:
            { return "PROCESSFRAMES1"; }
        case D3D12_AUTO_BREADCRUMB_OP_BUILDRAYTRACINGACCELERATIONSTRUCTURE:
            { return "BUILDRAYTRACINGACCELERATIONSTRUCTURE"; }
        case D3D12_AUTO_BREADCRUMB_OP_EMITRAYTRACINGACCELERATIONSTRUCTUREPOSTBUILDINFO:
            { return "EMITRAYTRACINGACCELERATIONSTRUCTUREPOSTBUILDINFO"; }
        case D3D12_AUTO_BREADCRUMB_OP_COPYRAYTRACINGACCELERATIONSTRUCTURE:
            { return "COPYRAYTRACINGACCELERATIONSTRUCTURE"; }
        case D3D12_AUTO_BREADCRUMB_OP_DISPATCHRAYS:
            { return "DISPATCHRAYS"; }
        case D3D12_AUTO_BREADCRUMB_OP_INITIALIZEMETACOMMAND:
            { return "INITIALIZEMETACOMMAND"; }
        case D3D12_AUTO_BREADCRUMB_OP_EXECUTEMETACOMMAND:
            { return "EXECUTEMETACOMMAND"; }
        case D3D12_AUTO_BREADCRUMB_OP_ESTIMATEMOTION:
            { return "ESTIMATEMOTIO"; }
        case D3D12_AUTO_BREADCRUMB_OP_RESOLVEMOTIONVECTORHEAP:
            { return "RESOLVEMOTIONVECTORHEAP"; }
        case D3D12_AUTO_BREADCRUMB_OP_SETPIPELINESTATE1:
            { return "SETPIPELINESTATE1"; }
        case D3D12_AUTO_BREADCRUMB_OP_INITIALIZEEXTENSIONCOMMAND:
            { return "INITIALIZEEXTENSIONCOMMAND"; }
        case D3D12_AUTO_BREADCRUMB_OP_EXECUTEEXTENSIONCOMMAND:
            { return "EXECUTEEXTENSIONCOMMAND"; }
        case D3D12_AUTO_BREADCRUMB_OP_DISPATCHMESH:
            { return "DISPATCHMESH"; }
        case D3D12_AUTO_BREADCRUMB_OP_ENCODEFRAME:
            { return "ENCODEFRAME"; }
        case D3D12_AUTO_BREADCRUMB_OP_RESOLVEENCODEROUTPUTMETADATA:
            { return "RESOLVEENCODEROUTPUTMETADATA"; }
    }

    return "UNKNOWN";
}

//-----------------------------------------------------------------------------
//      D3D12_DRED_ALLOCATION_TYPE を文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_DRED_ALLOCATION_TYPE type)
{
    switch(type)
    {
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_QUEUE:
            { return "COMMAND_QUEUE"; }
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_ALLOCATOR:
            { return "COMMAND_ALLOCATOR"; }
        case D3D12_DRED_ALLOCATION_TYPE_PIPELINE_STATE:
            { return "PIPELINE_STATE"; }
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_LIST:
            { return "COMMAND_LIST"; }
        case D3D12_DRED_ALLOCATION_TYPE_FENCE:
            { return "FENCE"; }
        case D3D12_DRED_ALLOCATION_TYPE_DESCRIPTOR_HEAP:
            { return "DESCRIPTOR_HEAP"; }
        case D3D12_DRED_ALLOCATION_TYPE_HEAP:
            { return "HEAP"; }
        case D3D12_DRED_ALLOCATION_TYPE_QUERY_HEAP:
            { return "QUERY_HEAP"; }
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_SIGNATURE:
            { return "COMMAND_SIGNATURE"; }
        case D3D12_DRED_ALLOCATION_TYPE_PIPELINE_LIBRARY:
            { return "PIPELINE_LIBRARY"; }
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_DECODER:
            { return "VIDEO_DECODER"; }
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_PROCESSOR:
            { return "VIDEO_PROCESSOR"; }
        case D3D12_DRED_ALLOCATION_TYPE_RESOURCE:
            { return "RESOURCE"; }
        case D3D12_DRED_ALLOCATION_TYPE_PASS:
            { return "PASS"; }
        case D3D12_DRED_ALLOCATION_TYPE_CRYPTOSESSION:
            { return "CRYPTOSESSION"; }
        case D3D12_DRED_ALLOCATION_TYPE_CRYPTOSESSIONPOLICY:
            { return "CRYPTOSESSIONPOLICY"; }
        case D3D12_DRED_ALLOCATION_TYPE_PROTECTEDRESOURCESESSION:
            { return "PROTECTEDRESOURCESESSION"; }
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_DECODER_HEAP:
            { return "VIDEO_DECODER_HEAP"; }
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_POOL:
            { return "COMMAND_POOL"; }
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_RECORDER:
            { return "COMMAND_RECORDER"; }
        case D3D12_DRED_ALLOCATION_TYPE_STATE_OBJECT:
            { return "STATE_OBJECT"; }
        case D3D12_DRED_ALLOCATION_TYPE_METACOMMAND:
            { return "METACOMMAND"; }
        case D3D12_DRED_ALLOCATION_TYPE_SCHEDULINGGROUP:
            { return "SCHEDULINGGROUP"; }
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_MOTION_ESTIMATOR:
            { return "VIDEO_MOTION_ESTIMATOR"; }
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_MOTION_VECTOR_HEAP:
            { return "VIDEO_MOTION_VECTOR_HEAP"; }
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_EXTENSION_COMMAND:
            { return "VIDEO_EXTENSION_COMMAND"; }
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_ENCODER:
            { return "VIDEO_ENCODER"; }
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_ENCODER_HEAP:
            { return "VIDEO_ENCODER_HEAP"; }
        case D3D12_DRED_ALLOCATION_TYPE_INVALID:
            { return "INVALID"; }
    }

    return "UNKNOWN";
}

//-----------------------------------------------------------------------------
//      HRESULT を文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(HRESULT hr)
{
    switch(hr)
    {
        ITEM_MSG(DXGI_ERROR_ACCESS_DENIED)
        ITEM_MSG(DXGI_ERROR_ACCESS_LOST)
        ITEM_MSG(DXGI_ERROR_ALREADY_EXISTS)
        ITEM_MSG(DXGI_ERROR_CANNOT_PROTECT_CONTENT)
        ITEM_MSG(DXGI_ERROR_DEVICE_HUNG)
        ITEM_MSG(DXGI_ERROR_DEVICE_REMOVED)
        ITEM_MSG(DXGI_ERROR_DEVICE_RESET)
        ITEM_MSG(DXGI_ERROR_DRIVER_INTERNAL_ERROR)
        ITEM_MSG(DXGI_ERROR_FRAME_STATISTICS_DISJOINT)
        ITEM_MSG(DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE)
        ITEM_MSG(DXGI_ERROR_INVALID_CALL)
        ITEM_MSG(DXGI_ERROR_MORE_DATA)
        ITEM_MSG(DXGI_ERROR_NAME_ALREADY_EXISTS)
        ITEM_MSG(DXGI_ERROR_NONEXCLUSIVE)
        ITEM_MSG(DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
        ITEM_MSG(DXGI_ERROR_NOT_FOUND)
        ITEM_MSG(DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED)
        ITEM_MSG(DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE)
        ITEM_MSG(DXGI_ERROR_SDK_COMPONENT_MISSING)
        ITEM_MSG(DXGI_ERROR_SESSION_DISCONNECTED)
        ITEM_MSG(DXGI_ERROR_UNSUPPORTED)
        ITEM_MSG(DXGI_ERROR_WAIT_TIMEOUT)
        ITEM_MSG(DXGI_ERROR_WAS_STILL_DRAWING)

        ITEM_MSG(D3D12_ERROR_ADAPTER_NOT_FOUND)
        ITEM_MSG(D3D12_ERROR_DRIVER_VERSION_MISMATCH)
        ITEM_MSG(D3D12_ERROR_INVALID_REDIST)
        ITEM_MSG(E_FAIL)
        ITEM_MSG(E_INVALIDARG)
        ITEM_MSG(E_OUTOFMEMORY)
        ITEM_MSG(E_NOTIMPL)
        ITEM_MSG(S_FALSE)
        ITEM_MSG(S_OK)
    }

    return "UNKNOWN";
}

//-----------------------------------------------------------------------------
//      D3D12_DRED_DEVICE_STATE を文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_DRED_DEVICE_STATE state)
{
    switch(state)
    {
    case D3D12_DRED_DEVICE_STATE_UNKNOWN:
        { return "UNKNOWN"; }
    case D3D12_DRED_DEVICE_STATE_HUNG:
        { return "HUNG"; }
    case D3D12_DRED_DEVICE_STATE_PAGEFAULT:
        { return "PAGEFAULT"; }
    }

    return "UNKNOWN";
}

///////////////////////////////////////////////////////////////////////////////
// MarkerStack class
///////////////////////////////////////////////////////////////////////////////
class MarkerStack
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    MarkerStack(uint32_t count)
    {
        m_Index = 0;
        m_Count = count;

        if (count > 0)
        {
            m_Stack = static_cast<uint32_t*>(a3d_alloc(sizeof(uint32_t) * count, 4));
        }
    }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~MarkerStack()
    {
        if (m_Stack != nullptr)
        {
            a3d_free(m_Stack);
            m_Stack = nullptr;
        }
    }

    //-------------------------------------------------------------------------
    //! @brief      インデックスを取得します.
    //-------------------------------------------------------------------------
    uint32_t GetIndex() const 
    { return m_Index; }

    //-------------------------------------------------------------------------
    //! @brief      スタックに積みます.
    //-------------------------------------------------------------------------
    void Push(uint32_t item)
    {
        if (m_Index >= m_Count)
        { return; }

        m_Stack[m_Index] = item;
        m_Index++;
    }

    //-------------------------------------------------------------------------
    //! @brief      スタックから取り出します.
    //-------------------------------------------------------------------------
    uint32_t Pop()
    {
        uint32_t result = 0;
        if (m_Index > 0)
        {
            result = m_Stack[m_Index - 1];
            m_Stack[m_Index - 1] = 0;
            m_Index--;
        }

        return result;
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    uint32_t    m_Index = 0;
    uint32_t*   m_Stack = nullptr;
    uint32_t    m_Count = 0;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace

namespace a3d {

//-----------------------------------------------------------------------------
//      D3D12_DRED_ALLOCATION_NODE を出力します.
//-----------------------------------------------------------------------------
void ReportAllocationNode(const D3D12_DRED_ALLOCATION_NODE* pAllocationNode)
{
    const D3D12_DRED_ALLOCATION_NODE* pNode = pAllocationNode;
    uint64_t nodeIndex = 0;

    while(pNode != nullptr)
    {
        OutputLog("  NodeIndex= %u: Type(%s), ObjectNameA(%s), ObjectNameW(%ls)",
            nodeIndex,
            ToString(pNode->AllocationType),
            pNode->ObjectNameA,
            pNode->ObjectNameW);

        pNode = pNode->pNext;
        nodeIndex++;
    }
}

//-----------------------------------------------------------------------------
//      D3D12_DRED_ALLOCATION_NODE1 を出力します.
//-----------------------------------------------------------------------------
void ReportAllocationNode1(const D3D12_DRED_ALLOCATION_NODE1* pAllocationNode)
{
    const D3D12_DRED_ALLOCATION_NODE1* pNode = pAllocationNode;
    uint64_t nodeIndex = 0;

    while(pNode != nullptr)
    {
        OutputLog("  NodeIndex= %u: Type(%s), ObjectNameA(%s), ObjectNameW(%ls), Object(0x%p)",
            nodeIndex,
            ToString(pNode->AllocationType),
            pNode->ObjectNameA,
            pNode->ObjectNameW,
            pNode->pObject);

        pNode = pNode->pNext;
        nodeIndex++;
    }
}

//-----------------------------------------------------------------------------
//      D3D12_AUTO_BREADCRUMB_NODE を出力します.
//-----------------------------------------------------------------------------
void ReportBreadCrumbsNode(const D3D12_AUTO_BREADCRUMB_NODE* pBreadcrumbNode)
{
    const D3D12_AUTO_BREADCRUMB_NODE* pNode = pBreadcrumbNode;
    uint64_t nodeIndex = 0;

    while(pNode != nullptr)
    {
        uint32_t lastOp = (pNode->pLastBreadcrumbValue == nullptr) ? 0 : *pNode->pLastBreadcrumbValue;

        OutputLog("NodeIndex = %u:", nodeIndex);
        OutputLog("  CommandList         : 0x%p DebugNameA(%s), DebugNameW(%ls)",
            pNode->pCommandList,
            pNode->pCommandListDebugNameA,
            pNode->pCommandListDebugNameW);
        OutputLog("  CommandQueue        : 0x%p DebugNameA(%s), DebugNameW(%ls)",
            pNode->pCommandQueue,
            pNode->pCommandQueueDebugNameA,
            pNode->pCommandQueueDebugNameW);
        OutputLog("  BreadcrumbCount     : %u", pNode->BreadcrumbCount);
        OutputLog("  LastBreadcrumbValue : %u", lastOp);
        OutputLog("  CommandHistory      :");

        auto count = pNode->BreadcrumbCount;
        for(auto i=0u; i<count; ++i)
        {
            const char* process = "[-]";
            if (i < lastOp)
            { process = "[reached]"; }
            else if (i == lastOp && i != 0)
            { process = "[#]"; }

            OutputLog("    CommandIndex=%05u: %9s OpName(%s)", i, process, ToString(pNode->pCommandHistory[i]));
        }

        pNode = pNode->pNext;
        nodeIndex++;
    }
}

//-----------------------------------------------------------------------------
//      D3D12_AUTO_BREADCRUMB_NODE1 を出力します.
//-----------------------------------------------------------------------------
void ReportBreadCrumbsNode1(const D3D12_AUTO_BREADCRUMB_NODE1* pBreadcrumbNode)
{
    const D3D12_AUTO_BREADCRUMB_NODE1* pNode = pBreadcrumbNode;
    uint64_t nodeIndex = 0;

    while(pNode != nullptr)
    {
        uint32_t lastOp = (pNode->pLastBreadcrumbValue == nullptr) ? 0 : *pNode->pLastBreadcrumbValue;

        OutputLog("NodeIndex = %u:", nodeIndex);
        OutputLog("  CommandList             : 0x%p DebugNameA(%s), DebugNameW(%ls)",
            pNode->pCommandList,
            pNode->pCommandListDebugNameA,
            pNode->pCommandListDebugNameW);
        OutputLog("  CommandQueue            : 0x%p DebugNameA(%s), DebugNameW(%ls)",
            pNode->pCommandQueue,
            pNode->pCommandQueueDebugNameA,
            pNode->pCommandQueueDebugNameW);
        OutputLog("  BreadcrumbCount         : %u", pNode->BreadcrumbCount);
        OutputLog("  LastBreadcrumbValue     : %u", lastOp);
        OutputLog("  BreadcrumbContextsCount : %u", pNode->BreadcrumbContextsCount);
        OutputLog("  CommandHistory          :");

        MarkerStack stack(pNode->BreadcrumbContextsCount);

        auto count = pNode->BreadcrumbCount;
        for(auto i=0u; i<count; ++i)
        {
            const char* process = "[-]";
            if (i < lastOp)
            { process = "[reached]"; }
            else if (i == lastOp && i != 0)
            { process = "[#]"; }

            auto op = pNode->pCommandHistory[i];
            auto opName = ToString(op);
            auto spaceCount = kMaxOpNameLength - strlen(opName);
            auto space = (spaceCount == 0) ? "" : " ";
            
            const wchar_t* marker = L"NO_MARKER";
            uint32_t contextIndex = stack.GetIndex();
            if (pNode->pBreadcrumbContexts != nullptr)
            {
                if (op == D3D12_AUTO_BREADCRUMB_OP_BEGINEVENT)
                {
                    for(auto j=0u; j<pNode->BreadcrumbContextsCount; ++j)
                    {
                        if (pNode->pBreadcrumbContexts[j].BreadcrumbIndex == i)
                        {
                            contextIndex = i;
                            stack.Push(j);
                            break;
                        }
                    }
                }
                else if (op == D3D12_AUTO_BREADCRUMB_OP_ENDEVENT)
                {
                    contextIndex = stack.Pop();
                }

                if (contextIndex < pNode->BreadcrumbContextsCount)
                {
                    marker = pNode->pBreadcrumbContexts[contextIndex].pContextString;
                    if (marker == nullptr)
                    { marker = L"NO_MARKER"; }
                }
            }

            OutputLog("    CommandIndex=%05u: %9s OpName(%s) %*s Marker(%ls)", i, process, opName, spaceCount, space, marker);
        }

        pNode = pNode->pNext;
        nodeIndex++;
    }
}

//-----------------------------------------------------------------------------
//      D3D12_DRED_PAGE_FAULT_OUTPUT を出力します.
//-----------------------------------------------------------------------------
void ReportPageFaultOutput(const D3D12_DRED_PAGE_FAULT_OUTPUT& pageFault)
{
    OutputLog("PageFault Virtual Address   : %lu", pageFault.PageFaultVA);
    OutputLog("Existing Allocation Node     :");
    ReportAllocationNode(pageFault.pHeadExistingAllocationNode);
    OutputLog("Recent Freed Allocation Node :");
    ReportAllocationNode(pageFault.pHeadRecentFreedAllocationNode);
}

//-----------------------------------------------------------------------------
//      D3D12_DRED_PAGE_FAULT_OUTPUT1 を出力します.
//-----------------------------------------------------------------------------
void ReportPageFaultOutput1(const D3D12_DRED_PAGE_FAULT_OUTPUT1& pageFault)
{
    OutputLog("PageFault Virtual Address   : %lu", pageFault.PageFaultVA);
    OutputLog("Existing Allocation Node     :");
    ReportAllocationNode1(pageFault.pHeadExistingAllocationNode);
    OutputLog("Recent Freed Allocation Node :");
    ReportAllocationNode1(pageFault.pHeadRecentFreedAllocationNode);
}

//-----------------------------------------------------------------------------
//      D3D12_DRED_PAGE_FAULT_OUTPUT2 を出力します.
//-----------------------------------------------------------------------------
void ReportPageFaultOutput2(const D3D12_DRED_PAGE_FAULT_OUTPUT2& pageFault)
{
    OutputLog("PageFault Virtual Address    : %lu", pageFault.PageFaultVA);
    OutputLog("Existing Allocation Node     :");
    ReportAllocationNode1(pageFault.pHeadExistingAllocationNode);
    OutputLog("Recent Freed Allocation Node :");
    ReportAllocationNode1(pageFault.pHeadRecentFreedAllocationNode);
}

//-----------------------------------------------------------------------------
//      DRED 1.0 で取得できる情報を出力します.
//-----------------------------------------------------------------------------
bool ReportDRED_1_0(ID3D12Device* pDevice)
{
    ID3D12DeviceRemovedExtendedData* pDred = nullptr;
    auto hr = pDevice->QueryInterface(IID_PPV_ARGS(&pDred));
    if (FAILED(hr))
    { return false; }

    OutputLog("----- DRED 1.0 -----");

    D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT breadcrumbs = {};
    hr = pDred->GetAutoBreadcrumbsOutput(&breadcrumbs);
    if (SUCCEEDED(hr))
    { ReportBreadCrumbsNode(breadcrumbs.pHeadAutoBreadcrumbNode); }

    D3D12_DRED_PAGE_FAULT_OUTPUT pageFault = {};
    hr = pDred->GetPageFaultAllocationOutput(&pageFault);
    if (SUCCEEDED(hr))
    { ReportPageFaultOutput(pageFault); }

    OutputLog("----- End DRED 1.0 -----");
    SafeRelease(pDred);

    return true;
}

//-----------------------------------------------------------------------------
//      DRED 1.1 で取得できる情報を出力します.
//-----------------------------------------------------------------------------
bool ReportDRED_1_1(ID3D12Device* pDevice)
{
    ID3D12DeviceRemovedExtendedData1* pDred = nullptr;
    auto hr = pDevice->QueryInterface(IID_PPV_ARGS(&pDred));
    if (FAILED(hr))
    { return false; }

    OutputLog("----- DRED 1.1 -----");

    D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1 breadcrumbs1 = {};
    hr = pDred->GetAutoBreadcrumbsOutput1(&breadcrumbs1);
    if (SUCCEEDED(hr))
    { ReportBreadCrumbsNode1(breadcrumbs1.pHeadAutoBreadcrumbNode); }
    else
    {
        D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT breadcrumbs = {};
        hr = pDred->GetAutoBreadcrumbsOutput(&breadcrumbs);
        if (SUCCEEDED(hr))
        { ReportBreadCrumbsNode(breadcrumbs.pHeadAutoBreadcrumbNode); }
    }

    D3D12_DRED_PAGE_FAULT_OUTPUT1 pageFault1 = {};
    hr = pDred->GetPageFaultAllocationOutput1(&pageFault1);
    if (SUCCEEDED(hr))
    { ReportPageFaultOutput1(pageFault1); }
    else
    {
        D3D12_DRED_PAGE_FAULT_OUTPUT pageFault = {};
        hr = pDred->GetPageFaultAllocationOutput(&pageFault);
        if (SUCCEEDED(hr))
        { ReportPageFaultOutput(pageFault); }
    }

    OutputLog("----- End DRED 1.1 -----");
    SafeRelease(pDred);

    return true;
}

//-----------------------------------------------------------------------------
//      DRED 1.2 で取得できる情報を出力します.
//-----------------------------------------------------------------------------
bool ReportDRED_1_2(ID3D12Device* pDevice)
{
    ID3D12DeviceRemovedExtendedData2* pDred = nullptr;
    auto hr = pDevice->QueryInterface(IID_PPV_ARGS(&pDred));
    if (FAILED(hr))
    { return false; }

    OutputLog("----- DRED 1.2 -----");

    auto state = pDred->GetDeviceState();
    OutputLog("Device State : %s", ToString(state));

    D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1 breadcrumbs1 = {};
    hr = pDred->GetAutoBreadcrumbsOutput1(&breadcrumbs1);
    if (SUCCEEDED(hr))
    { ReportBreadCrumbsNode1(breadcrumbs1.pHeadAutoBreadcrumbNode); }
    else
    {
        D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT breadcrumbs = {};
        hr = pDred->GetAutoBreadcrumbsOutput(&breadcrumbs);
        if (SUCCEEDED(hr))
        { ReportBreadCrumbsNode(breadcrumbs.pHeadAutoBreadcrumbNode); }
    }

    D3D12_DRED_PAGE_FAULT_OUTPUT2 pageFault2 = {};
    hr = pDred->GetPageFaultAllocationOutput2(&pageFault2);
    if (SUCCEEDED(hr))
    { ReportPageFaultOutput2(pageFault2); }
    else
    {
        D3D12_DRED_PAGE_FAULT_OUTPUT1 pageFault1 = {};
        hr = pDred->GetPageFaultAllocationOutput1(&pageFault1);
        if (SUCCEEDED(hr))
        { ReportPageFaultOutput1(pageFault1); }
        else
        {
            D3D12_DRED_PAGE_FAULT_OUTPUT pageFault = {};
            hr = pDred->GetPageFaultAllocationOutput(&pageFault);
            if (SUCCEEDED(hr))
            { ReportPageFaultOutput(pageFault); }
        }
    }

    OutputLog("----- End DRED 1.2 -----");
    SafeRelease(pDred);

    return true;
}

//-----------------------------------------------------------------------------
//      DRED情報を出力します.
//-----------------------------------------------------------------------------
void ReportDRED(Device* pDevice)
{
    if (pDevice == nullptr)
    { return; }

    auto pNativeDevice = pDevice->GetD3D12Device();
    if (pNativeDevice == nullptr)
    { return; }

    auto hr = pNativeDevice->GetDeviceRemovedReason();
    OutputLog("Device Removed Reason : %s", ToString(hr));

    {
        ID3D12DebugDevice* pDebugDevice = nullptr;
        hr = pNativeDevice->QueryInterface(IID_PPV_ARGS(&pDebugDevice));
        if (SUCCEEDED(hr))
        { pDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL); }
        SafeRelease(pDebugDevice);
    }

    if (ReportDRED_1_2(pNativeDevice))
    { return; }

    if (ReportDRED_1_1(pNativeDevice))
    { return; }

    if (ReportDRED_1_0(pNativeDevice))
    { return; }

    OutputLog("----- NO DRED Info -----");
}

} // namespace a3d

#undef ITEM_MSG