//-------------------------------------------------------------------------------------------------
// File : a3dAccelerationStructure.cpp
// Desc : Acceleration Structure For Ray Tracing.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace {

//-------------------------------------------------------------------------------------------------
//      バッファUAVを生成します.
//-------------------------------------------------------------------------------------------------
bool CreateBufferUAV
(
    ID3D12Device*           pDevice,
    D3D12MA::Allocator*     pAllocator,
    UINT64                  bufferSize,
    D3D12_RESOURCE_STATES   initState,
    ID3D12Resource**        ppResource,
    D3D12MA::Allocation**   ppAllocation
)
{
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment          = 0;
    desc.Width              = bufferSize;
    desc.Height             = 1;
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    D3D12MA::ALLOCATION_DESC allocDesc = {};
    allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    auto hr = pAllocator->CreateResource(
        &allocDesc, &desc, initState, nullptr, ppAllocation, IID_PPV_ARGS(ppResource));
    if (FAILED(hr))
    {
        A3D_LOG("Error : CreateResource() Failed. errcode = 0x%x", hr);
        return false;
    }
    return true;
}

D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE ToNativeType(a3d::ACCELERATION_STRUCTURE_TYPE type)
{
    return type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL
        ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL
        : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
}

D3D12_RAYTRACING_GEOMETRY_FLAGS ToNativeFlags(uint32_t flags)
{
    D3D12_RAYTRACING_GEOMETRY_FLAGS result = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;

    if (flags & a3d::GEOMETRY_FLAGS_OPAQUE)
    { result |= D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE; }
    if (flags & a3d::GEOMETRY_FLAGS_NO_DUPLICATE_ANYHIT_INVOCATION)
    { result |= D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION; }

    return result;
}

void ToNativeGeometryDescs
(
    uint32_t                        count,
    const a3d::GeometryDesc*        pDescs,
    D3D12_RAYTRACING_GEOMETRY_DESC* pResults
)
{
    for(auto i=0u; i<count; ++i)
    {
        auto& src = pDescs[i];
        auto& dst = pResults[i];
        if (src.Type == a3d::GEOMETRY_TYPE_TRIANGLES)
        {
            auto pWrapVB = static_cast<a3d::Buffer*>(src.Triangles.pVertexBuffer);
            auto pWrapIB = static_cast<a3d::Buffer*>(src.Triangles.pIndexBuffer);
            auto pWrapTB = static_cast<a3d::Buffer*>(src.Triangles.pTransformBuffer);
            A3D_ASSERT(pWrapVB != nullptr);

            dst.Type                                    = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
            dst.Flags                                   = ToNativeFlags(src.Flags);
            dst.Triangles.VertexBuffer.StartAddress     = pWrapVB->GetD3D12Resource()->GetGPUVirtualAddress() + src.Triangles.VertexOffset;
            dst.Triangles.VertexBuffer.StrideInBytes    = src.Triangles.VertexStride;
            dst.Triangles.VertexCount                   = src.Triangles.VertexCount;
            dst.Triangles.VertexFormat                  = ToNativeFormat(src.Triangles.VertexFormat);

            if (pWrapIB != nullptr)
            {
                dst.Triangles.IndexBuffer   = pWrapIB->GetD3D12Resource()->GetGPUVirtualAddress() + src.Triangles.IndexOffset;
                dst.Triangles.IndexCount    = src.Triangles.IndexCount;
                dst.Triangles.IndexFormat   = ToNativeFormat(src.Triangles.IndexFormat);
            }
            else
            {
                dst.Triangles.IndexBuffer   = 0;
                dst.Triangles.IndexCount    = 0;
                dst.Triangles.IndexFormat   = DXGI_FORMAT_UNKNOWN;
            }

            if (pWrapTB != nullptr)
            {
                dst.Triangles.Transform3x4 = pWrapTB->GetD3D12Resource()->GetGPUVirtualAddress() + src.Triangles.TransformOffset;
            }
            else
            {
                dst.Triangles.Transform3x4 = 0;
            }
        }
        else if (src.Type == a3d::GEOMETRY_TYPE_AABBS)
        {
            auto pWrapBuffer = static_cast<a3d::Buffer*>(src.AABBs.pBuffer);
            A3D_ASSERT(pWrapBuffer != nullptr);

            dst.Type  = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
            dst.Flags = ToNativeFlags(src.Flags);

            dst.AABBs.AABBCount             = src.AABBs.BoxCount;
            dst.AABBs.AABBs.StartAddress    = pWrapBuffer->GetD3D12Resource()->GetGPUVirtualAddress() + src.AABBs.Offset;
            dst.AABBs.AABBs.StrideInBytes   = src.AABBs.Stride;
        }
    }
}

} // namespace


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// AccelerationStructure class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
AccelerationStructure::AccelerationStructure()
: m_RefCount        (1)
, m_pDevice         (nullptr)
, m_pGeometryDescs  (nullptr)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
AccelerationStructure::~AccelerationStructure()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理です.
//-------------------------------------------------------------------------------------------------
bool AccelerationStructure::Init(IDevice* pDevice, const AccelerationStructureDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto pWrapDevice = static_cast<Device*>(pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    m_pDevice = pWrapDevice;
    m_pDevice->AddRef();

    // 加速機構の入力設定.
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.DescsLayout      = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.Flags            = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS(pDesc->Flags);
    inputs.Type             = ToNativeType(pDesc->Type);
    inputs.NumDescs         = pDesc->Count;

    if (pDesc->Type == ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
    {
        m_pGeometryDescs = static_cast<D3D12_RAYTRACING_GEOMETRY_DESC*>(a3d_alloc(sizeof(D3D12_RAYTRACING_GEOMETRY_DESC) * pDesc->Count, alignof(D3D12_RAYTRACING_GEOMETRY_DESC)));
        ToNativeGeometryDescs(pDesc->Count, pDesc->pDescs, m_pGeometryDescs);
        inputs.pGeometryDescs = m_pGeometryDescs;
    }
    else
    {
        auto pWrapBuffer = static_cast<Buffer*>(pDesc->pInstanceBuffer);
        A3D_ASSERT(pWrapBuffer != nullptr);
        inputs.InstanceDescs = pWrapBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
    }

    // ビルド前情報を取得.
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO preBuildInfo = {};
    pWrapDevice->GetD3D12Device()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &preBuildInfo);
    if (preBuildInfo.ResultDataMaxSizeInBytes == 0)
    { return false; }

    // スクラッチバッファを生成.
    if (!CreateBufferUAV(
        pWrapDevice->GetD3D12Device(),
        pWrapDevice->GetAllocator(),
        preBuildInfo.ScratchDataSizeInBytes,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        &m_Scratch.pResource,
        &m_Scratch.pAllocation))
    {
        A3D_LOG("Error : CreateUAVBuffer() Failed.");
        return false;
    }

    // 加速機構用バッファを生成.
    if (!CreateBufferUAV(
        pWrapDevice->GetD3D12Device(),
        pWrapDevice->GetAllocator(),
        preBuildInfo.ResultDataMaxSizeInBytes,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        &m_Structure.pResource,
        &m_Structure.pAllocation))
    {
        A3D_LOG("Error : CreateUAVBuffer() Failed.");
        return false;
    }

    memset(&m_BuildDesc, 0, sizeof(m_BuildDesc));
    m_BuildDesc.Inputs                              = inputs;
    m_BuildDesc.ScratchAccelerationStructureData    = m_Scratch  .pResource->GetGPUVirtualAddress();
    m_BuildDesc.DestAccelerationStructureData       = m_Structure.pResource->GetGPUVirtualAddress();

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void AccelerationStructure::Term()
{
    m_Scratch  .Release();
    m_Structure.Release();
    SafeRelease(m_pDevice);

    if (m_pGeometryDescs != nullptr)
    {
        a3d_free(m_pGeometryDescs);
        m_pGeometryDescs = nullptr;
    }

    memset(&m_BuildDesc, 0, sizeof(m_BuildDesc));
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void AccelerationStructure::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void AccelerationStructure::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t AccelerationStructure::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void AccelerationStructure::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      デバイスアドレスを取得します.
//-------------------------------------------------------------------------------------------------
uint64_t AccelerationStructure::GetDeviceAddress() const
{
    if (m_Structure.pResource == nullptr)
    { return 0; }

    return m_Structure.pResource->GetGPUVirtualAddress();
}

//-------------------------------------------------------------------------------------------------
//      ビルドします.
//-------------------------------------------------------------------------------------------------
void AccelerationStructure::Build(ID3D12GraphicsCommandList6* pCommandList)
{
    if (pCommandList == nullptr)
    { return; }

    // 加速機構を構築.
    pCommandList->BuildRaytracingAccelerationStructure(&m_BuildDesc, 0, nullptr);

    // UAVバリアを張っておく.
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.UAV.pResource   = m_Structure.pResource;
    pCommandList->ResourceBarrier(1, &barrier);
}

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* AccelerationStructure::GetD3D12Resource() const
{ return m_Structure.pResource; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool AccelerationStructure::Create
(
    IDevice*                            pDevice,
    const AccelerationStructureDesc*    pDesc,
    IAccelerationStructure**            ppAS
)
{
    if (pDevice == nullptr || pDesc == nullptr || ppAS == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new AccelerationStructure();
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out of Memory.");
        return false;
    }

    if (!instance->Init(pDevice, pDesc))
    {
        A3D_LOG("Error : AccelerationStructure::Init() Failed.");
        return false;
    }

    *ppAS = instance;
    return true;
}

} // namespace a3d
