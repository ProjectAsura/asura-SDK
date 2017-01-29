//-------------------------------------------------------------------------------------------------
// File : a3dDescriptorSet.cpp
// Desc : DescriptorSet Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorSet class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorSet::DescriptorSet()
: m_RefCount(1)
, m_pDevice (nullptr)
, m_pLayout (nullptr)
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
    DescriptorSetLayout*        pLayout
)
{
    if (pDevice == nullptr || pLayout == nullptr)
    { return false; }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    m_pLayout = pLayout;
    m_pLayout->AddRef();

    auto& desc = pLayout->GetDesc();
    m_Handles.resize(desc.EntryCount);

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::Term()
{
    m_Handles.clear();

    SafeRelease(m_pLayout);
    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      テクスチャを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetTexture(uint32_t index, ITextureView* pResource)
{
    A3D_ASSERT(size_t(index) < m_Handles.size());

    auto pWrapView = static_cast<TextureView*>(pResource);
    A3D_ASSERT(pWrapView != nullptr);

    m_Handles[index] = pWrapView->GetShaderDescriptor()->GetHandleGPU();
}

//-------------------------------------------------------------------------------------------------
//      バッファを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetBuffer(uint32_t index, IBufferView* pResource)
{
    A3D_ASSERT(size_t(index) < m_Handles.size());

    auto pWrapView = static_cast<BufferView*>(pResource);
    A3D_ASSERT(pWrapView != nullptr);

    m_Handles[index] = pWrapView->GetDescriptor()->GetHandleGPU();
}

//-------------------------------------------------------------------------------------------------
//      サンプラーを設定します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::SetSampler(uint32_t index, ISampler* pSampler)
{
    A3D_ASSERT(size_t(index) < m_Handles.size());

    auto pWrapSampler = static_cast<Sampler*>(pSampler);
    A3D_ASSERT( pWrapSampler != nullptr );

    m_Handles[index] = pWrapSampler->GetDescriptor()->GetHandleGPU();
}

//-------------------------------------------------------------------------------------------------
//      ディスクリプタテーブルを設定する描画コマンドを発行します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::Bind(ICommandList* pCommandList)
{
    auto pWrapCommandList = static_cast<CommandList*>(pCommandList);
    A3D_ASSERT(pWrapCommandList != nullptr);

    auto pNativeCommandList = pWrapCommandList->GetD3D12GraphicsCommandList();
    A3D_ASSERT(pNativeCommandList != nullptr);

    if (m_pLayout->IsGraphicsPipeline())
    {
        pNativeCommandList->SetGraphicsRootSignature( m_pLayout->GetD3D12RootSignature() );

        for(size_t i=0; i<m_Handles.size(); ++i)
        { pNativeCommandList->SetGraphicsRootDescriptorTable( uint32_t(i), m_Handles[i] ); }
    }
    else
    {
        pNativeCommandList->SetComputeRootSignature( m_pLayout->GetD3D12RootSignature() );

        for(size_t i=0u; i<m_Handles.size(); ++i)
        { pNativeCommandList->SetComputeRootDescriptorTable( uint32_t(i), m_Handles[i] ); }
    }
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool DescriptorSet::Create
(
    IDevice*                    pDevice,
    DescriptorSetLayout*        pLayout,
    IDescriptorSet**            ppDescriptorSet
)
{
    if (pDevice         == nullptr 
    || pLayout          == nullptr 
    || ppDescriptorSet  == nullptr)
    { return false; }

    auto instance = new DescriptorSet;
    if ( instance == nullptr )
    { return false; }

    if ( !instance->Init(pDevice, pLayout ) )
    {
        SafeRelease(instance);
        return false;
    }

    *ppDescriptorSet = instance;
    return true;
}

} // namespace a3d
