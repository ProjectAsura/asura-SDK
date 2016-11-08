//-------------------------------------------------------------------------------------------------
// File : a3dDescriptorSet.cpp
// Desc : DescriptorSet Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      定数バッファビューに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeConstantBufferView
(
    a3d::IResource* pResource,
    uint64_t offset,
    uint64_t size,
    D3D12_CONSTANT_BUFFER_VIEW_DESC& result
)
{
    ID3D12Resource* pNativeResource = nullptr;
    if (pResource->GetPrivateData() == a3d::RESOURCE_OBJECT_TYPE_BUFFER)
    {
        auto pWrapResource = reinterpret_cast<a3d::Buffer*>(pResource);
        A3D_ASSERT(pWrapResource != nullptr);

        pNativeResource = pWrapResource->GetD3D12Resource();
    }
    else
    {
        auto pWrapResource = reinterpret_cast<a3d::Texture*>(pResource);
        A3D_ASSERT(pWrapResource != nullptr);

        pNativeResource = pWrapResource->GetD3D12Resource();
    }

    result.BufferLocation = pNativeResource->GetGPUVirtualAddress() + offset;
    result.SizeInBytes    = static_cast<uint32_t>(size);
}

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeShaderResourceView
(
    a3d::Buffer* pResource,
    uint64_t offset,
    uint64_t size,
    D3D12_SHADER_RESOURCE_VIEW_DESC& result
)
{
    auto& nativeDesc = pResource->GetD3D12Resource()->GetDesc();

    auto wrapDesc = pResource->GetDesc();

    result.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    result.Buffer.FirstElement          = offset;
    result.Buffer.NumElements           = (wrapDesc.EnableRow) ? uint32_t(size) : uint32_t(size / wrapDesc.Stride);
    result.Buffer.StructureByteStride   = (wrapDesc.EnableRow) ? 0 : wrapDesc.Stride;
    result.Buffer.Flags                 = (wrapDesc.EnableRow)
                                         ? D3D12_BUFFER_SRV_FLAG_RAW 
                                         : D3D12_BUFFER_SRV_FLAG_NONE;
}

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeShaderResourceView
(
    a3d::Texture* pResource,
    uint64_t offset,
    uint64_t size,
    D3D12_SHADER_RESOURCE_VIEW_DESC& result
)
{
    auto& nativeDesc = pResource->GetD3D12Resource()->GetDesc();

    auto wrapDesc = pResource->GetDesc();

    result.Format = nativeDesc.Format;
    result.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(
        wrapDesc.ComponentMapping.R,
        wrapDesc.ComponentMapping.G,
        wrapDesc.ComponentMapping.B,
        wrapDesc.ComponentMapping.A
    );

    switch(nativeDesc.Dimension)
    {
    case D3D12_RESOURCE_DIMENSION_BUFFER:
        {
            result.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            result.Buffer.FirstElement          = offset;
            result.Buffer.NumElements           = uint32_t(size);
            result.Buffer.StructureByteStride   = 0;
            result.Buffer.Flags                 = D3D12_BUFFER_SRV_FLAG_RAW;
        }
        break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
        {
            if (nativeDesc.DepthOrArraySize > 1)
            {
                result.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                result.Texture1DArray.ArraySize           = nativeDesc.DepthOrArraySize;
                result.Texture1DArray.FirstArraySlice     = 0;
                result.Texture1DArray.MipLevels           = nativeDesc.MipLevels;
                result.Texture1DArray.MostDetailedMip     = 0;
                result.Texture1DArray.ResourceMinLODClamp = 0.0f;
            }
            else
            {
                result.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                result.Texture1D.MipLevels            = nativeDesc.MipLevels;
                result.Texture1D.MostDetailedMip      = 0;
                result.Texture1D.ResourceMinLODClamp  = 0.0f;
            }
        }
        break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
        {
            if (nativeDesc.DepthOrArraySize > 1)
            {
                if (wrapDesc.Dimension == a3d::RESOURCE_DIMENSION_CUBEMAP)
                {
                    if (nativeDesc.DepthOrArraySize > 6)
                    {
                        result.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                        result.TextureCubeArray.First2DArrayFace      = 0;
                        result.TextureCubeArray.MipLevels             = nativeDesc.MipLevels;
                        result.TextureCubeArray.MostDetailedMip       = 0;
                        result.TextureCubeArray.NumCubes              = nativeDesc.DepthOrArraySize / 6;
                        result.TextureCubeArray.ResourceMinLODClamp   = 0.0f;
                    }
                    else
                    {
                        result.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                        result.TextureCube.MipLevels              = nativeDesc.MipLevels;
                        result.TextureCube.MostDetailedMip        = 0;
                        result.TextureCube.ResourceMinLODClamp    = 0.0f;
                    }
                }
                else
                {
                    if (nativeDesc.SampleDesc.Count > 1)
                    {
                        result.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                        result.Texture2DMSArray.ArraySize       = nativeDesc.DepthOrArraySize;
                        result.Texture2DMSArray.FirstArraySlice = 0;
                    }
                    else
                    {
                        result.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                        result.Texture2DArray.ArraySize           = nativeDesc.DepthOrArraySize;
                        result.Texture2DArray.FirstArraySlice     = 0;
                        result.Texture2DArray.MipLevels           = nativeDesc.MipLevels;
                        result.Texture2DArray.MostDetailedMip     = 0;
                        result.Texture2DArray.PlaneSlice          = 0;
                        result.Texture2DArray.ResourceMinLODClamp = 0.0f;
                    }
                }
            }
            else
            {
                if (nativeDesc.SampleDesc.Count > 1)
                {
                    result.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    result.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                    result.Texture2D.MipLevels            = nativeDesc.MipLevels;
                    result.Texture2D.MostDetailedMip      = 0;
                    result.Texture2D.PlaneSlice           = 0;
                    result.Texture2D.ResourceMinLODClamp  = 0.0f;
                }
            }
        }
        break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
        {
            result.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            result.Texture3D.MipLevels            = nativeDesc.MipLevels;
            result.Texture3D.MostDetailedMip      = 0;
            result.Texture3D.ResourceMinLODClamp  = 0.0f;
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
//      アンオーダードアクセスビューに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeUnorderedAccessView( a3d::Buffer* pResource, uint64_t offset, uint64_t size, D3D12_UNORDERED_ACCESS_VIEW_DESC& result )
{
    auto wrapDesc = pResource->GetDesc();
    result.Format = pResource->GetD3D12Resource()->GetDesc().Format;

    result.Format                      = pResource->GetD3D12Resource()->GetDesc().Format;
    result.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
    result.Buffer.CounterOffsetInBytes = 0;
    result.Buffer.FirstElement         = 0;
    result.Buffer.Flags                = (wrapDesc.EnableRow) ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;
    result.Buffer.NumElements          = (wrapDesc.EnableRow) ? uint32_t(wrapDesc.Size) : uint32_t(wrapDesc.Size / wrapDesc.Stride);
    result.Buffer.StructureByteStride  = (wrapDesc.EnableRow) ? 0 : wrapDesc.Stride;
}

//-------------------------------------------------------------------------------------------------
//      アンオーダードアクセスビューに変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeUnorderedAccessView
(
    a3d::Texture* pResource,
    uint64_t offset,
    uint64_t size,
    D3D12_UNORDERED_ACCESS_VIEW_DESC& result
)
{
    auto wrapDesc = pResource->GetDesc();

    result.Format = pResource->GetD3D12Resource()->GetDesc().Format;

    switch(wrapDesc.Dimension)
    {
    case a3d::RESOURCE_DIMENSION_BUFFER:
        {
            result.Format                      = pResource->GetD3D12Resource()->GetDesc().Format;
            result.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
            result.Buffer.CounterOffsetInBytes = 0;
            result.Buffer.FirstElement         = offset;
            result.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_RAW;
            result.Buffer.NumElements          = uint32_t(size);
            result.Buffer.StructureByteStride  = 0;
        }
        break;

    case a3d::RESOURCE_DIMENSION_TEXTURE1D:
        {
            if (wrapDesc.DepthOrArraySize > 1)
            {
                result.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                result.Texture1DArray.ArraySize       = wrapDesc.DepthOrArraySize;
                result.Texture1DArray.FirstArraySlice = 0;
                result.Texture1DArray.MipSlice        = 0;
            }
            else
            {
                result.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
                result.Texture1D.MipSlice = 0;
            }
        }
        break;

    case a3d::RESOURCE_DIMENSION_TEXTURE2D:
        {
            if (wrapDesc.DepthOrArraySize > 1)
            {
                result.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                result.Texture2DArray.ArraySize       = wrapDesc.DepthOrArraySize;
                result.Texture2DArray.FirstArraySlice = 0;
                result.Texture2DArray.MipSlice        = 0;
                result.Texture2DArray.PlaneSlice      = 0;
            }
            else
            {
                result.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                result.Texture2D.MipSlice     = 0;
                result.Texture2D.PlaneSlice   = 0;
            }
        }
        break;
 
    case a3d::RESOURCE_DIMENSION_TEXTURE3D:
        {
            result.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
            result.Texture3D.FirstWSlice  = 0;
            result.Texture3D.MipSlice     = 0;
            result.Texture3D.WSize        = wrapDesc.DepthOrArraySize;
        }
        break;
    }
}

} // namespace /* anonymous */


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorSet class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorSet::DescriptorSet()
: m_RefCount            (1)
, m_pDevice             (nullptr)
, m_pLayoutDesc         (nullptr)
, m_pDescriptors        (nullptr)
, m_pDescriptorInfos    (nullptr)
, m_IsGraphicsPipeline  (true)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorSet::~DescriptorSet()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorSet::GetCount() const 
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool DescriptorSet::Init
(
    IDevice*                    pDevice,
    DescriptorSetLayoutDesc*    pDesc,
    Descriptor**                ppDescriptors,
    bool                        isGraphicsPipeline
)
{
    if (pDevice == nullptr || pDesc == nullptr || ppDescriptors == nullptr)
    { return false; }

    Term();

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    m_pLayoutDesc  = pDesc;
    m_pDescriptors = ppDescriptors;

    m_IsGraphicsPipeline = isGraphicsPipeline;

    if (pDesc->EntryCount > 0)
    {
        m_pDescriptorInfos = new (std::nothrow) DescriptorInfo[pDesc->EntryCount];
        if (m_pDescriptorInfos == nullptr)
        { return false; }

        memset( m_pDescriptorInfos, 0, sizeof(DescriptorInfo) * pDesc->EntryCount );
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::Term()
{
    if (m_pLayoutDesc != nullptr)
    {
        for(auto i=0u; i<m_pLayoutDesc->EntryCount; ++i)
        {
            m_pDescriptors[i]->Release();
            ResetDescriptorInfo(m_pDescriptorInfos[i]);
        }

        delete [] m_pDescriptorInfos;
        m_pDescriptorInfos = nullptr;

        delete [] m_pDescriptors;
        m_pDescriptors = nullptr;

        m_pLayoutDesc = nullptr;
    }

    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      テクスチャを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetTexture(uint32_t index, ITexture* pResource)
{
    A3D_ASSERT(index < m_pLayoutDesc->EntryCount);

    ResetDescriptorInfo(m_pDescriptorInfos[index]);

    m_pDescriptorInfos[index].pResource = pResource;
    m_pDescriptorInfos[index].pResource->AddRef();
    m_pDescriptorInfos[index].Offset = 0;
    m_pDescriptorInfos[index].Size   = pResource->GetDesc().Width;
}

//-------------------------------------------------------------------------------------------------
//      バッファを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetBuffer(uint32_t index, IBuffer* pResource)
{
    A3D_ASSERT(index < m_pLayoutDesc->EntryCount);

    ResetDescriptorInfo(m_pDescriptorInfos[index]);

    m_pDescriptorInfos[index].pResource = pResource;
    m_pDescriptorInfos[index].pResource->AddRef();
    m_pDescriptorInfos[index].Offset = 0;
    m_pDescriptorInfos[index].Size   = pResource->GetDesc().Size;
}

//-------------------------------------------------------------------------------------------------
//      バッファを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetBuffer(uint32_t index, IBuffer* pResource, uint64_t size, uint64_t offset)
{
    A3D_ASSERT(index < m_pLayoutDesc->EntryCount);

    ResetDescriptorInfo(m_pDescriptorInfos[index]);

    m_pDescriptorInfos[index].pResource = pResource;
    m_pDescriptorInfos[index].pResource->AddRef();
    m_pDescriptorInfos[index].Offset = offset;
    m_pDescriptorInfos[index].Size   = size;
}

//-------------------------------------------------------------------------------------------------
//      サンプラーを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetSampler(uint32_t index, ISampler* pSampler)
{
    A3D_ASSERT(index < m_pLayoutDesc->EntryCount);

    ResetDescriptorInfo(m_pDescriptorInfos[index]);

    m_pDescriptorInfos[index].pSampler = pSampler;
    m_pDescriptorInfos[index].pSampler->AddRef();
    m_pDescriptorInfos[index].Offset = 0;
    m_pDescriptorInfos[index].Size   = 0;
}

//-------------------------------------------------------------------------------------------------
//      更新処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::Update()
{
    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice != nullptr);

    for(auto i=0u; i<m_pLayoutDesc->EntryCount; ++i)
    {
        switch(m_pLayoutDesc->Entries[i].Type)
        {
        case DESCRIPTOR_TYPE_CBV:
            {
                D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
                ToNativeConstantBufferView(
                    m_pDescriptorInfos[i].pResource,
                    m_pDescriptorInfos[i].Offset,
                    m_pDescriptorInfos[i].Size,
                    desc );
                pNativeDevice->CreateConstantBufferView(&desc, m_pDescriptors[i]->GetHandleCPU());
            }
            break;

        case DESCRIPTOR_TYPE_SRV:
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
                ID3D12Resource* pNativeResource = nullptr;

                if (m_pDescriptorInfos[i].pResource->GetPrivateData() == RESOURCE_OBJECT_TYPE_BUFFER)
                {
                    auto pResource = reinterpret_cast<Buffer*>(m_pDescriptorInfos[i].pResource);
                    A3D_ASSERT(pResource != nullptr);

                    ToNativeShaderResourceView(
                        pResource,
                        m_pDescriptorInfos[i].Offset,
                        m_pDescriptorInfos[i].Size,
                        desc );
                    pNativeResource = pResource->GetD3D12Resource();
                }
                else
                {
                    auto pResource = reinterpret_cast<Texture*>(m_pDescriptorInfos[i].pResource);
                    A3D_ASSERT(pResource != nullptr);

                    ToNativeShaderResourceView(
                        pResource,
                        m_pDescriptorInfos[i].Offset,
                        m_pDescriptorInfos[i].Size,
                        desc );
                    pNativeResource = pResource->GetD3D12Resource();
                }

                pNativeDevice->CreateShaderResourceView(
                    pNativeResource,
                    &desc,
                    m_pDescriptors[i]->GetHandleCPU());
            }
            break;

        case DESCRIPTOR_TYPE_UAV:
            {
                D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
                ID3D12Resource* pNativeResource = nullptr;

                if (m_pDescriptorInfos[i].pResource->GetPrivateData() == RESOURCE_OBJECT_TYPE_BUFFER)
                {
                    auto pResource = reinterpret_cast<Buffer*>(m_pDescriptorInfos[i].pResource);
                    A3D_ASSERT(pResource != nullptr);

                    ToNativeUnorderedAccessView( 
                        pResource,
                        m_pDescriptorInfos[i].Offset,
                        m_pDescriptorInfos[i].Size,
                        desc );
                    pNativeResource = pResource->GetD3D12Resource();
                }
                else
                {
                    auto pResource = reinterpret_cast<Texture*>(m_pDescriptorInfos[i].pResource);
                    A3D_ASSERT(pResource != nullptr);

                    ToNativeUnorderedAccessView(
                        pResource,
                        m_pDescriptorInfos[i].Offset,
                        m_pDescriptorInfos[i].Size,
                        desc );
                    pNativeResource = pResource->GetD3D12Resource();
                }

                pNativeDevice->CreateUnorderedAccessView(
                    pNativeResource,
                    nullptr,
                    &desc,
                    m_pDescriptors[i]->GetHandleCPU());
            }
            break;

        case DESCRIPTOR_TYPE_SMP:
            {
                auto pSampler = reinterpret_cast<Sampler*>(m_pDescriptorInfos[i].pSampler);
                A3D_ASSERT(pSampler != nullptr);

                auto desc = pSampler->GetD3D12SamplerDesc();
                pNativeDevice->CreateSampler(&desc, m_pDescriptors[i]->GetHandleCPU());
            }
            break;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタテーブルを設定する描画コマンドを発行します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::Bind(ICommandList* pCommandList)
{
    auto pWrapCommandList = reinterpret_cast<CommandList*>(pCommandList);
    A3D_ASSERT(pWrapCommandList != nullptr);

    auto pNativeCommandList = pWrapCommandList->GetD3D12GraphicsCommandList();
    A3D_ASSERT(pNativeCommandList != nullptr);

    if (m_IsGraphicsPipeline)
    {
        for(auto i=0u; i<m_pLayoutDesc->EntryCount; ++i)
        {
            auto handleGPU = m_pDescriptors[i]->GetHandleGPU();
            pNativeCommandList->SetGraphicsRootDescriptorTable( i, handleGPU );
        }
    }
    else
    {
        for(auto i=0u; i<m_pLayoutDesc->EntryCount; ++i)
        {
            auto handleGPU = m_pDescriptors[i]->GetHandleGPU();
            pNativeCommandList->SetComputeRootDescriptorTable( i, handleGPU );
        }
    }
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタ情報をリセットします.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::ResetDescriptorInfo(DescriptorInfo& info)
{
    SafeRelease(info.pResource);
    SafeRelease(info.pSampler);
    info.Size   = 0;
    info.Offset = 0;
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool DescriptorSet::Create
(
    IDevice*                    pDevice,
    DescriptorSetLayoutDesc*    pDesc,
    Descriptor**                ppDescriptors,
    bool                        isGraphicsPipeline,
    IDescriptorSet**            ppDescriptorSet
)
{
    if (pDevice         == nullptr 
    || pDesc            == nullptr 
    || ppDescriptors    == nullptr
    || ppDescriptorSet  == nullptr)
    { return false; }

    auto instance = new (std::nothrow) DescriptorSet;
    if ( instance == nullptr )
    { return false; }

    if ( !instance->Init(pDevice, pDesc, ppDescriptors, isGraphicsPipeline ) )
    {
        SafeRelease(instance);
        return false;
    }

    *ppDescriptorSet = instance;
    return true;
}

} // namespace a3d
