//-------------------------------------------------------------------------------------------------
// File : a3dSampler.cpp
// Desc : Sampler Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      アドレスモードをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D12_TEXTURE_ADDRESS_MODE ToNativeAddressMode( a3d::TEXTURE_ADDRESS_MODE mode )
{
    D3D12_TEXTURE_ADDRESS_MODE table[] = {
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,
        D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE
    };

    return table[mode];
}

//-------------------------------------------------------------------------------------------------
//      フィルタに変換します.
//-------------------------------------------------------------------------------------------------
D3D12_FILTER ToNativeFilter
(
    a3d::FILTER_MODE min,
    a3d::FILTER_MODE mag,
    a3d::MIPMAP_MODE mip,
    bool             compare,
    bool             anisotropy
)
{
    auto reduction = (compare) ? D3D12_FILTER_REDUCTION_TYPE_COMPARISON
                               : D3D12_FILTER_REDUCTION_TYPE_STANDARD;

    if ( anisotropy )
    { return D3D12_ENCODE_ANISOTROPIC_FILTER( reduction ); }

    return D3D12_ENCODE_BASIC_FILTER( min, mag, mip, reduction );
}

//-------------------------------------------------------------------------------------------------
//      ボーダーカラーをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeBorderColor( a3d::BORDER_COLOR color, FLOAT* pResult )
{
    switch( color )
    {
    case a3d::BORDER_COLOR_TRANSPARENT_BLACK:
        {
            pResult[0] = 0.0f;
            pResult[1] = 0.0f;
            pResult[2] = 0.0f;
            pResult[3] = 0.0f;
        }
        break;

    case a3d::BORDER_COLOR_OPAQUE_BLACK:
        {
            pResult[0] = 0.0f;
            pResult[1] = 0.0f;
            pResult[2] = 0.0f;
            pResult[3] = 1.0f;
        }
        break;

    case a3d::BORDER_COLOR_OPAQUE_WHITE:
        {
            pResult[0] = 1.0f;
            pResult[1] = 1.0f;
            pResult[2] = 1.0f;
            pResult[3] = 1.0f;
        }
        break;
    }
}


} // namespace /* anonymous */

namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Sampler class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Sampler::Sampler()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pDescriptor (nullptr)
{ memset( &m_Desc, 0, sizeof(m_Desc) ); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Sampler::~Sampler()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期処理です.
//-------------------------------------------------------------------------------------------------
bool Sampler::Init(IDevice* pDevice, const SamplerDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT( pWrapDevice != nullptr );

    m_Desc.Filter         = ToNativeFilter(
                                pDesc->MinFilter,
                                pDesc->MagFilter,
                                pDesc->MipMapMode,
                                pDesc->CompareEnable,
                                pDesc->AnisotropyEnable );
    m_Desc.AddressU       = ToNativeAddressMode( pDesc->AddressU );
    m_Desc.AddressV       = ToNativeAddressMode( pDesc->AddressV );
    m_Desc.AddressW       = ToNativeAddressMode( pDesc->AddressW );
    m_Desc.MipLODBias     = pDesc->MipLodBias;
    m_Desc.MaxAnisotropy  = pDesc->MaxAnisotropy;
    m_Desc.ComparisonFunc = ToNativeComparisonFunc( pDesc->CompareOp );
    m_Desc.MinLOD         = pDesc->MinLod;
    m_Desc.MaxLOD         = pDesc->MaxLod;
    ToNativeBorderColor( pDesc->BorderColor, m_Desc.BorderColor );

    m_pDescriptor = pWrapDevice
                        ->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
                        ->CreateDescriptor();
    if (m_pDescriptor == nullptr)
    { return false; }

    auto pNativeDevice = pWrapDevice->GetD3D12Device();
    A3D_ASSERT( pNativeDevice != nullptr );

    pNativeDevice->CreateSampler( &m_Desc, m_pDescriptor->GetHandleCPU() );

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void Sampler::Term()
{
    SafeRelease(m_pDescriptor);
    SafeRelease(m_pDevice);
    memset( &m_Desc, 0, sizeof(m_Desc) );
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void Sampler::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void Sampler::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Sampler::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void Sampler::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
D3D12_SAMPLER_DESC Sampler::GetD3D12SamplerDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      ディスクリプタを取得します.
//-------------------------------------------------------------------------------------------------
const Descriptor* Sampler::GetDescriptor() const
{ return m_pDescriptor; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Sampler::Create(IDevice* pDevice, const SamplerDesc* pDesc, ISampler** ppSampler)
{
    if (pDevice == nullptr || pDesc == nullptr || ppSampler == nullptr)
    { return false; }

    auto instance = new Sampler();
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, pDesc))
    {
        SafeRelease(instance);
        return false;
    }

    *ppSampler = instance;
    return true;
}

} // namespace a3d
