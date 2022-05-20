//-------------------------------------------------------------------------------------------------
// File : a3dDescriptorSetLayout.cpp
// Desc : Descriptor Set Layout Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      ネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_SHADER_VISIBILITY ToNativeShaderVisibility( uint32_t stage )
{
    if ( stage == a3d::SHADER_STAGE_VS )
    { return D3D12_SHADER_VISIBILITY_VERTEX; }

    else if ( stage == a3d::SHADER_STAGE_DS )
    { return D3D12_SHADER_VISIBILITY_DOMAIN; }

    else if ( stage == a3d::SHADER_STAGE_HS )
    { return D3D12_SHADER_VISIBILITY_HULL; }

    //else if ( stage == a3d::SHADER_MASK_GS )
    //{ return D3D12_SHADER_VISIBILITY_GEOMETRY; }

    else if ( stage == a3d::SHADER_STAGE_PS )
    { return D3D12_SHADER_VISIBILITY_PIXEL; }

    else if ( stage == a3d::SHADER_STAGE_AS )
    { return D3D12_SHADER_VISIBILITY_AMPLIFICATION; }

    else if ( stage == a3d::SHADER_STAGE_MS )
    { return D3D12_SHADER_VISIBILITY_MESH; }

    return D3D12_SHADER_VISIBILITY_ALL;
}

//-------------------------------------------------------------------------------------------------
//      ネイティブのディスクリプタレンジに変換します.
//------------------------------------------------------------------------------------------------
void ToNativeDescriptorRange( const a3d::DescriptorEntry& entry, D3D12_DESCRIPTOR_RANGE& result )
{
    switch(entry.Type)
    {
    case a3d::DESCRIPTOR_TYPE_CBV:
        result.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        break;

    case a3d::DESCRIPTOR_TYPE_SRV_T:
    case a3d::DESCRIPTOR_TYPE_SRV_B:
        result.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        break;

    case a3d::DESCRIPTOR_TYPE_UAV_T:
    case a3d::DESCRIPTOR_TYPE_UAV_B:
        result.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        break;

    case a3d::DESCRIPTOR_TYPE_SMP:
        result.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        break;
    }

    result.NumDescriptors                    = 1;
    result.BaseShaderRegister                = entry.ShaderRegister;
    result.RegisterSpace                     = 0;
    result.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

} // namespace /* anonymous */


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorSetLayout class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorSetLayout::DescriptorSetLayout()
: m_RefCount            (1)
, m_pDevice             (nullptr)
, m_pRootSignature      (nullptr)
, m_Type                (PIPELINE_STATE_TYPE_GRAPHICS)
{ memset( &m_Desc, 0, sizeof(m_Desc) ); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorSetLayout::~DescriptorSetLayout()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool DescriptorSetLayout::Init(IDevice* pDevice, const DescriptorSetLayoutDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pNativeDevice = m_pDevice->GetD3D12Device();
    A3D_ASSERT(pNativeDevice);

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    bool isCompute = false;
    for(auto i=0u; i<pDesc->EntryCount; ++i)
    {
        if (pDesc->Entries[i].ShaderStage == SHADER_STAGE_CS)
        {
            m_Type = PIPELINE_STATE_TYPE_COMPUTE;
            isCompute = true;
        }

        if (pDesc->Entries[i].ShaderStage == SHADER_STAGE_VS)
        {
            m_Type = PIPELINE_STATE_TYPE_GRAPHICS;
            if (isCompute)
            { return false; }
        }

        if ((pDesc->Entries[i].ShaderStage == SHADER_STAGE_AS)
         || (pDesc->Entries[i].ShaderStage == SHADER_STAGE_MS))
        {
            m_Type = PIPELINE_STATE_TYPE_MESHLET;
            if (isCompute)
            { return false; }
        }
    }

    {
        auto pEntries = new D3D12_DESCRIPTOR_RANGE [pDesc->EntryCount];
        A3D_ASSERT(pEntries);

        auto pParams = new D3D12_ROOT_PARAMETER [pDesc->EntryCount];
        A3D_ASSERT(pParams);

        auto mask = 0;
        bool shaders[6] = {};

        for(auto i=0u; i<pDesc->EntryCount; ++i)
        {
            ToNativeDescriptorRange(pDesc->Entries[i], pEntries[i]);
            pParams[i].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            pParams[i].DescriptorTable.NumDescriptorRanges = 1;
            pParams[i].DescriptorTable.pDescriptorRanges   = &pEntries[i];
            pParams[i].ShaderVisibility = ToNativeShaderVisibility( pDesc->Entries[i].ShaderStage );

            switch (pDesc->Entries[i].ShaderStage)
            {
            case SHADER_STAGE_VS:
                shaders[0] = true;
                break;

            case SHADER_STAGE_DS:
                shaders[1] = true;
                break;

            case SHADER_STAGE_HS:
                shaders[2] = true;
                break;

            case SHADER_STAGE_PS:
                shaders[3] = true;
                break;

            case SHADER_STAGE_AS:
                shaders[4] = true;
                break;

            case SHADER_STAGE_MS:
                shaders[5] = true;
                break;
            }
        }

        D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
        if (shaders[0])
        { flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; }
        else
        { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS; }
        if (!shaders[1])
        { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS; }
        if (!shaders[2])
        { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS; }
        if (!shaders[3])
        { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS; }
        if (!shaders[4])
        { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS; }
        if (!shaders[5])
        { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS; }

        if (pDesc->Flags & DESCRIPTORSET_LAYOUT_FLAG_LOCAL)
        { flags |= D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE; }

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters      = pDesc->EntryCount;
        desc.pParameters        = pParams;
        desc.NumStaticSamplers  = 0;
        desc.pStaticSamplers    = nullptr;
        desc.Flags              = flags;

        ID3DBlob* pSignatureBlob = nullptr;
        ID3DBlob* pErrorBlob = nullptr;

        auto hr = D3D12SerializeRootSignature( &desc,  D3D_ROOT_SIGNATURE_VERSION_1, &pSignatureBlob, &pErrorBlob);
        delete [] pParams;
        delete [] pEntries;

        if ( FAILED(hr) )
        {
            SafeRelease(pSignatureBlob);
            SafeRelease(pErrorBlob);
            A3D_LOG("Error : D3D12SerializeRootSignature() Failed. errcode = 0x%x", hr);

            return false;
        }

        hr = pNativeDevice->CreateRootSignature(
            0,
            pSignatureBlob->GetBufferPointer(),
            pSignatureBlob->GetBufferSize(),
            IID_PPV_ARGS(&m_pRootSignature));

        SafeRelease(pSignatureBlob);
        SafeRelease(pErrorBlob);

        if ( FAILED(hr) )
        {
            A3D_LOG("Error : ID3D12Device::CreateRootSignature() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSetLayout::Term()
{
    SafeRelease(m_pRootSignature);
    SafeRelease(m_pDevice);

    memset( &m_Desc, 0, sizeof(m_Desc) );
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void DescriptorSetLayout::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSetLayout::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DescriptorSetLayout::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void DescriptorSetLayout::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      ルートシグニチャを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12RootSignature* DescriptorSetLayout::GetD3D12RootSignature() const
{ return m_pRootSignature; }

//-------------------------------------------------------------------------------------------------
//      パイプラインタイプを取得します.
//-------------------------------------------------------------------------------------------------
uint8_t DescriptorSetLayout::GetType() const
{ return m_Type; }

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
const DescriptorSetLayoutDesc& DescriptorSetLayout::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool DescriptorSetLayout::Create
(
    IDevice*                        pDevice,
    const DescriptorSetLayoutDesc*  pDesc,
    IDescriptorSetLayout**          ppLayout
)
{
    if (pDevice == nullptr || pDesc == nullptr || ppLayout == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new DescriptorSetLayout;
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if (!instance->Init(pDevice, pDesc))
    {
        instance->Release();
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppLayout = instance;
    return true;
}

} // namespace a3d
