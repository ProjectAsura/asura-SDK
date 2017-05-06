//-------------------------------------------------------------------------------------------------
// File : a3dDescriptorSet.cpp
// Desc : DescriptorSet Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

void ConstantBufferBinderVS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pBufferView = static_cast<a3d::BufferView*>(pResource);
    A3D_ASSERT(pBufferView != nullptr);
    auto pBuffer = pBufferView->GetD3D11Buffer();
    pContext->VSSetConstantBuffers(slot, 1, &pBuffer);
}

void ConstantBufferBinderDS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pBufferView = static_cast<a3d::BufferView*>(pResource);
    A3D_ASSERT(pBufferView != nullptr);
    auto pBuffer = pBufferView->GetD3D11Buffer();
    pContext->DSSetConstantBuffers(slot, 1, &pBuffer);
}

void ConstantBufferBinderGS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pBufferView = static_cast<a3d::BufferView*>(pResource);
    A3D_ASSERT(pBufferView != nullptr);
    auto pBuffer = pBufferView->GetD3D11Buffer();
    pContext->GSSetConstantBuffers(slot, 1, &pBuffer);
}

void ConstantBufferBinderHS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pBufferView = static_cast<a3d::BufferView*>(pResource);
    A3D_ASSERT(pBufferView != nullptr);
    auto pBuffer = pBufferView->GetD3D11Buffer();
    pContext->HSSetConstantBuffers(slot, 1, &pBuffer);
}

void ConstantBufferBinderPS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pBufferView = static_cast<a3d::BufferView*>(pResource);
    A3D_ASSERT(pBufferView != nullptr);
    auto pBuffer = pBufferView->GetD3D11Buffer();
    pContext->PSSetConstantBuffers(slot, 1, &pBuffer);
}

void ConstantBufferBinderCS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pBufferView = static_cast<a3d::BufferView*>(pResource);
    A3D_ASSERT(pBufferView != nullptr);
    auto pBuffer = pBufferView->GetD3D11Buffer();
    pContext->CSSetConstantBuffers(slot, 1, &pBuffer);
}

void SamplerBinderVS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pSampler = static_cast<ID3D11SamplerState*>(pResource);
    A3D_ASSERT(pSampler != nullptr);
    pContext->VSSetSamplers(slot, 1, &pSampler);
}

void SamplerBinderDS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pSampler = static_cast<ID3D11SamplerState*>(pResource);
    A3D_ASSERT(pSampler != nullptr);
    pContext->DSSetSamplers(slot, 1, &pSampler);
}

void SamplerBinderGS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pSampler = static_cast<ID3D11SamplerState*>(pResource);
    A3D_ASSERT(pSampler != nullptr);
    pContext->GSSetSamplers(slot, 1, &pSampler);
}

void SamplerBinderHS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pSampler = static_cast<ID3D11SamplerState*>(pResource);
    A3D_ASSERT(pSampler != nullptr);
    pContext->HSSetSamplers(slot, 1, &pSampler);
}

void SamplerBinderPS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pSampler = static_cast<ID3D11SamplerState*>(pResource);
    A3D_ASSERT(pSampler != nullptr);
    pContext->PSSetSamplers(slot, 1, &pSampler);
}

void SamplerBinderCS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pSampler = static_cast<ID3D11SamplerState*>(pResource);
    A3D_ASSERT(pSampler != nullptr);
    pContext->CSSetSamplers(slot, 1, &pSampler);
}

void ShaderResourceViewBinderVS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pView = static_cast<ID3D11ShaderResourceView*>(pResource);
    A3D_ASSERT(pView != nullptr);
    pContext->VSSetShaderResources(slot, 1, &pView);
}

void ShaderResourceViewBinderDS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pView = static_cast<ID3D11ShaderResourceView*>(pResource);
    A3D_ASSERT(pView != nullptr);
    pContext->DSSetShaderResources(slot, 1, &pView);
}

void ShaderResourceViewBinderGS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pView = static_cast<ID3D11ShaderResourceView*>(pResource);
    A3D_ASSERT(pView != nullptr);
    pContext->GSSetShaderResources(slot, 1, &pView);
}

void ShaderResourceViewBinderHS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pView = static_cast<ID3D11ShaderResourceView*>(pResource);
    A3D_ASSERT(pView != nullptr);
    pContext->HSSetShaderResources(slot, 1, &pView);
}

void ShaderResourceViewBinderPS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pView = static_cast<ID3D11ShaderResourceView*>(pResource);
    A3D_ASSERT(pView != nullptr);
    pContext->PSSetShaderResources(slot, 1, &pView);
}

void ShaderResourceViewBinderCS(ID3D11DeviceContext* pContext, uint32_t slot, void* pResource)
{
    auto pView = static_cast<ID3D11ShaderResourceView*>(pResource);
    pContext->CSSetShaderResources(slot, 1, &pView);
}

void ConstantBufferUpdater(ID3D11DeviceContext* pContext, void* pResource)
{
    auto pBufferView = static_cast<a3d::BufferView*>(pResource);
    pBufferView->UpdateSubsource(pContext);
}

void EmptyUpdater(ID3D11DeviceContext*, void*)
{ /* DO_NOTHING */ }

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
    DescriptorSetLayoutDesc*    pDesc
)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    m_pLayoutDesc = pDesc;

    m_pBindInfo = new BindInfo [pDesc->EntryCount];
    if (m_pBindInfo == nullptr)
    { return false; }

    memset( m_pBindInfo, 0, sizeof(BindInfo) * pDesc->EntryCount);

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::Term()
{
    if (m_pBindInfo != nullptr)
    {
        delete [] m_pBindInfo;
        m_pBindInfo = nullptr;
    }

    if (m_pLayoutDesc != nullptr)
    { m_pLayoutDesc = nullptr; }

    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      テクスチャを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetTexture(uint32_t index, ITextureView* pResource, RESOURCE_STATE state)
{
    // Vulkan用の引数のためD3D11では使いません.
    A3D_UNUSED(state);

    auto pWrapTextureView = static_cast<TextureView*>(pResource);
    A3D_ASSERT(pWrapTextureView != nullptr);

    auto pD3D11ShaderResourceView = pWrapTextureView->GetD3D11ShaderResourceView();
    A3D_ASSERT(pD3D11ShaderResourceView != nullptr);

    auto& entry = m_pLayoutDesc->Entries[index];
    auto& info  = m_pBindInfo[index];

    info.Slot         = entry.ShaderRegister;
    info.pResource    = pD3D11ShaderResourceView;
    info.Updater      = EmptyUpdater;

    auto count = 0;
    if (entry.ShaderMask & SHADER_MASK_VERTEX)
    {
        info.Binder[count] = ShaderResourceViewBinderVS;
        count++;
    }

    if (entry.ShaderMask & SHADER_MASK_DOMAIN)
    {
        info.Binder[count] = ShaderResourceViewBinderDS;
        count++;
    }

    if (entry.ShaderMask & SHADER_MASK_GEOMETRY)
    {
        info.Binder[count] = ShaderResourceViewBinderGS;
        count++;
    }

    if (entry.ShaderMask & SHADER_MASK_HULL)
    {
        info.Binder[count] = ShaderResourceViewBinderHS;
        count++;
    }

    if (entry.ShaderMask & SHADER_MASK_PIXEL)
    {
        info.Binder[count] = ShaderResourceViewBinderPS;
        count++;
    }

    if (entry.ShaderMask & SHADER_MASK_COMPUTE)
    {
        info.Binder[count] = ShaderResourceViewBinderCS;
        count++;
    }

    info.StageCount = count;
}

//-------------------------------------------------------------------------------------------------
//      バッファを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetBuffer(uint32_t index, IBufferView* pResource)
{
    auto pWrapBufferView = static_cast<BufferView*>(pResource);
    A3D_ASSERT(pWrapBufferView != nullptr);

    auto pD3D11Buffer = pWrapBufferView->GetD3D11Buffer();
    A3D_ASSERT(pD3D11Buffer != nullptr);

    auto& entry = m_pLayoutDesc->Entries[index];
    auto& info  = m_pBindInfo[index];

    info.Slot         = entry.ShaderRegister;
    info.pResource    = pWrapBufferView;
    info.Updater      = ConstantBufferUpdater;

    auto count = 0;
    if (entry.ShaderMask & SHADER_MASK_VERTEX)
    {
        info.Binder[count] = ConstantBufferBinderVS;
        count++;
    }

    if (entry.ShaderMask & SHADER_MASK_DOMAIN)
    {
        info.Binder[count] = ConstantBufferBinderDS;
        count++;
    }

    if (entry.ShaderMask & SHADER_MASK_GEOMETRY)
    {
        info.Binder[count] = ConstantBufferBinderGS;
        count++;
    }

    if (entry.ShaderMask & SHADER_MASK_HULL)
    {
        info.Binder[count] = ConstantBufferBinderHS;
        count++;
    }

    if (entry.ShaderMask & SHADER_MASK_PIXEL)
    {
        info.Binder[count] = ConstantBufferBinderPS;
        count++;
    }

    if (entry.ShaderMask & SHADER_MASK_COMPUTE)
    {
        info.Binder[count] = ConstantBufferBinderCS;
        count++;
    }

    info.StageCount = count;
}

//-------------------------------------------------------------------------------------------------
//      サンプラーを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetSampler(uint32_t index, ISampler* pSampler)
{
    auto pWrapSampler = static_cast<Sampler*>(pSampler);
    A3D_ASSERT(pWrapSampler != nullptr);

    auto pD3D11SamplerState = pWrapSampler->GetD3D11SamplerState();
    A3D_ASSERT(pD3D11SamplerState != nullptr);

    auto& entry = m_pLayoutDesc->Entries[index];
    auto& info  = m_pBindInfo[index];

    info.Slot           = entry.ShaderRegister;
    info.pResource      = pD3D11SamplerState;
    info.Updater        = EmptyUpdater;

    auto count = 0;
    if (entry.ShaderMask & SHADER_MASK_VERTEX)
    {
        info.Binder[count] = SamplerBinderVS;
        count++;
    }

    if (entry.ShaderMask & SHADER_MASK_DOMAIN)
    {
        info.Binder[count] = SamplerBinderDS;
        count++;
    }

    if (entry.ShaderMask & SHADER_MASK_GEOMETRY)
    {
        info.Binder[count] = SamplerBinderGS;
        count++;
    }

    if (entry.ShaderMask & SHADER_MASK_HULL)
    {
        info.Binder[count] = SamplerBinderHS;
        count++;
    }

    if (entry.ShaderMask & SHADER_MASK_PIXEL)
    {
        info.Binder[count] = SamplerBinderPS;
        count++;
    }

    if (entry.ShaderMask & SHADER_MASK_COMPUTE)
    {
        info.Binder[count] = SamplerBinderCS;
        count++;
    }

    info.StageCount = count;
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタセットを関連付けます.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::Bind(ID3D11DeviceContext* pDeviceContext)
{
    for(auto i=0u; i<m_pLayoutDesc->EntryCount; ++i)
    {
        auto& info = m_pBindInfo[i];

        for(auto j=0u; j<info.StageCount; ++j)
        {
            auto binder = info.Binder[j];
            binder(pDeviceContext, info.Slot, info.pResource);
        }
    }
}

//-------------------------------------------------------------------------------------------------
//     サブリソースを更新します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::UpdateSubreosurce(ID3D11DeviceContext* pDeviceContext)
{
    for(auto i=0u; i<m_pLayoutDesc->EntryCount; ++i)
    {
        auto& info = m_pBindInfo[i];

        for(auto j=0u; j<info.StageCount; ++j)
        {
            auto updater = info.Updater;
            updater(pDeviceContext, info.pResource);
        }
    }
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool DescriptorSet::Create
(
    IDevice*                    pDevice,
    DescriptorSetLayoutDesc*    pDesc,
    IDescriptorSet**            ppDescriptorSet
)
{
    if (pDevice         == nullptr 
    || pDesc            == nullptr 
    || ppDescriptorSet  == nullptr)
    { return false; }

    auto instance = new DescriptorSet;
    if ( instance == nullptr )
    { return false; }

    if ( !instance->Init(pDevice, pDesc ) )
    {
        SafeRelease(instance);
        return false;
    }

    *ppDescriptorSet = instance;
    return true;
}

} // namespace a3d
