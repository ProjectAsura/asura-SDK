//-------------------------------------------------------------------------------------------------
// File : a3dShaderResourceView.cpp
// Desc : ShaderResourceView Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ShaderResourceView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
ShaderResourceView::ShaderResourceView()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pResource   (nullptr)
, m_pDescriptor (nullptr)
{ memset(&m_Desc, 0, sizeof(m_Desc)); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
ShaderResourceView::~ShaderResourceView()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool ShaderResourceView::Init(IDevice* pDevice, IResource* pResource, const ShaderResourceViewDesc* pDesc)
{
    if (pDevice == nullptr || pResource == nullptr || pDesc == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    Buffer*         pWrapperBuffer  = nullptr;
    Texture*        pWrapperTexture = nullptr;
    ID3D12Resource* pNativeResource = nullptr;
    if (pResource->GetKind() == RESOURCE_KIND_BUFFER)
    {
        pWrapperBuffer = static_cast<Buffer*>(pResource);
        if (pWrapperBuffer == nullptr)
        {
            A3D_LOG("Error : Invalid Argument.");
            return false;
        }

        pNativeResource = pWrapperBuffer->GetD3D12Resource();
        if (pNativeResource == nullptr)
        {
            A3D_LOG("Error : Invalid Argument.");
            return false;
        }
    }
    else if (pResource->GetKind() == RESOURCE_KIND_TEXTURE)
    {
        pWrapperTexture = static_cast<Texture*>(pResource);
        if (pWrapperTexture == nullptr)
        {
            A3D_LOG("Error : Invalid Argument.");
            return false;
        }

        pNativeResource = pWrapperTexture->GetD3D12Resource();
        if (pNativeResource == nullptr)
        {
            A3D_LOG("Error : Invalid Argument.");
            return false;
        }
    }
    else 
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT( pNativeDevice != nullptr );

    m_pResource = pResource;
    m_pResource->AddRef();

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    m_pDescriptor = m_pDevice->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
                             ->CreateDescriptor();
    if (m_pDescriptor == nullptr)
    {
        A3D_LOG("Error : CreateDescriptor() Failed.");
        return false;
    }

    D3D12_RESOURCE_DESC resDesc = pNativeResource->GetDesc();

    D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.ViewDimension  = ToNativeSRVDimension(pDesc->Dimension);
    desc.Format         = ToNativeViewFormat(pDesc->Format, false);
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    switch(desc.ViewDimension)
    {
    case D3D12_SRV_DIMENSION_BUFFER:
        {
            desc.Buffer.FirstElement        = pDesc->FirstElement;
            desc.Buffer.NumElements         = pDesc->ElementCount;
            desc.Buffer.StructureByteStride = 0;
            desc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
        }
        break;

    case D3D12_SRV_DIMENSION_TEXTURE1D:
        {
            desc.Texture1D.MipLevels            = pDesc->MipLevels;
            desc.Texture1D.MostDetailedMip      = 0;
            desc.Texture1D.ResourceMinLODClamp  = 0.0f;
        }
        break;

    case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
        {
            desc.Texture1DArray.ArraySize           = pDesc->FirstElement;
            desc.Texture1DArray.FirstArraySlice     = pDesc->ElementCount;
            desc.Texture1DArray.MipLevels           = pDesc->MipLevels;
            desc.Texture1DArray.MostDetailedMip     = 0;
            desc.Texture1DArray.ResourceMinLODClamp = 0.0f;
        }
        break;

    case D3D12_SRV_DIMENSION_TEXTURE2D:
        {
            if (resDesc.SampleDesc.Count > 1)
            {
                desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
            }
            else
            {
                desc.Texture2D.MipLevels            = pDesc->MipLevels;
                desc.Texture2D.MostDetailedMip      = 0;
                desc.Texture2D.PlaneSlice           = 0;
                desc.Texture2D.ResourceMinLODClamp  = 0.0f;
            }
        }
        break;

    case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
        {
            if (resDesc.SampleDesc.Count > 1)
            {
                desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                desc.Texture2DMSArray.ArraySize         = pDesc->ElementCount;
                desc.Texture2DMSArray.FirstArraySlice   = pDesc->FirstElement;
            }
            else
            {
                desc.Texture2DArray.ArraySize           = pDesc->ElementCount;
                desc.Texture2DArray.FirstArraySlice     = pDesc->FirstElement;
                desc.Texture2DArray.MipLevels           = pDesc->MipLevels;
                desc.Texture2DArray.MostDetailedMip     = 0;
                desc.Texture2DArray.PlaneSlice          = 0;
                desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
            }
        }
        break;

    case D3D12_SRV_DIMENSION_TEXTURE3D:
        {
            desc.Texture3D.MipLevels            = pDesc->MipLevels;
            desc.Texture3D.MostDetailedMip      = 0;
            desc.Texture3D.ResourceMinLODClamp  = 0.0f;
        }
        break;

    case D3D12_SRV_DIMENSION_TEXTURECUBE:
        {
            desc.TextureCube.MipLevels              = pDesc->MipLevels;
            desc.TextureCube.MostDetailedMip        = 0;
            desc.TextureCube.ResourceMinLODClamp    = 0.0f;
        }
        break;

    case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
        {
            desc.TextureCubeArray.First2DArrayFace      = pDesc->FirstElement;
            desc.TextureCubeArray.MipLevels             = pDesc->MipLevels;
            desc.TextureCubeArray.MostDetailedMip       = 0;
            desc.TextureCubeArray.NumCubes              = pDesc->ElementCount / 6;
            desc.TextureCubeArray.ResourceMinLODClamp   = 0.0f;
        }
        break;
    }

    pNativeDevice->CreateShaderResourceView(
        pNativeResource,
        &desc,
        m_pDescriptor->GetHandleCPU() );


    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void ShaderResourceView::Term()
{
    SafeRelease( m_pDescriptor );
    SafeRelease( m_pResource );
    SafeRelease( m_pDevice );
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void ShaderResourceView::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void ShaderResourceView::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t ShaderResourceView::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void ShaderResourceView::SetName(const char* name)
{ m_Name = name; }

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* ShaderResourceView::GetName() const
{ return m_Name.c_str(); }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void ShaderResourceView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
ShaderResourceViewDesc ShaderResourceView::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      ディスクリプタを取得します.
//-------------------------------------------------------------------------------------------------
const Descriptor* ShaderResourceView::GetDescriptor() const
{ return m_pDescriptor; }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します,
//-------------------------------------------------------------------------------------------------
IResource* ShaderResourceView::GetResource() const
{ return m_pResource; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool ShaderResourceView::Create
(
    IDevice*                        pDevice,
    IResource*                      pResource,
    const ShaderResourceViewDesc*   pDesc,
    IShaderResourceView**           ppTextureView
)
{
    if (pDevice == nullptr || pResource == nullptr || pDesc == nullptr || ppTextureView == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new ShaderResourceView();
    if ( instance == nullptr )
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if ( !instance->Init(pDevice, pResource, pDesc) )
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppTextureView = instance;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      D3D12_CPU_DESCRIPTOR_HANDLEを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE A3D_APIENTRY GetD3D12CpuDescriptorHandle(IShaderResourceView* pSRV)
{
    auto pWrapSRV = static_cast<ShaderResourceView*>(pSRV);
    if (pWrapSRV == nullptr)
    { return D3D12_CPU_DESCRIPTOR_HANDLE(); }

    A3D_ASSERT(pWrapSRV->GetDescriptor() != nullptr);
    return pWrapSRV->GetDescriptor()->GetHandleCPU();
}

//-------------------------------------------------------------------------------------------------
//      D3D12_GPU_DESCRIPTOR_HANDLEを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE A3D_APIENTRY GetD3D12GpuDescriptorHandle(IShaderResourceView* pSRV)
{
    auto pWrapSRV = static_cast<ShaderResourceView*>(pSRV);
    if (pWrapSRV == nullptr)
    { return D3D12_GPU_DESCRIPTOR_HANDLE(); }

    A3D_ASSERT(pWrapSRV->GetDescriptor() != nullptr);
    return pWrapSRV->GetDescriptor()->GetHandleGPU();
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタ番号を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t APIENTRY GetD3D12DescriptorIndex(IShaderResourceView* pSRV)
{
    auto pWrapSRV = static_cast<ShaderResourceView*>(pSRV);
    if (pWrapSRV == nullptr)
    { return UINT32_MAX; }

    A3D_ASSERT(pWrapSRV->GetDescriptor() != nullptr);
    return pWrapSRV->GetDescriptor()->GetDescriptorIndex();
}

} // namespace a3d
